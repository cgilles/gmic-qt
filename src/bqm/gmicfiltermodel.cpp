/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam Batch Queue Manager plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2024 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gmicfiltermodel.h"

// Qt includes

#include <QBuffer>

// digiKam includes

#include "digikam_debug.h"
#include "ditemtooltip.h"

// Local includes

#include "gmicfiltermngr.h"
#include "gmicfilternode.h"
#include "gmicqtcommon.h"

using namespace Digikam;
using namespace DigikamGmicQtPluginCommon;

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicFilterModel::Private
{
public:

    Private() = default;

    GmicFilterManager* manager   = nullptr;
    bool               endMacro  = false;
};

// -----------------------------------------------------------------


GmicFilterModel::GmicFilterModel(GmicFilterManager* const mngr, QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
    d->manager = mngr;

    connect(d->manager, SIGNAL(signalEntryAdded(GmicFilterNode*)),
            this, SLOT(slotEntryAdded(GmicFilterNode*)));

    connect(d->manager, SIGNAL(signalEntryRemoved(GmicFilterNode*,int,GmicFilterNode*)),
            this, SLOT(slotEntryRemoved(GmicFilterNode*,int,GmicFilterNode*)));

    connect(d->manager, SIGNAL(signalEntryChanged(GmicFilterNode*)),
            this, SLOT(slotEntryChanged(GmicFilterNode*)));
}

GmicFilterModel::~GmicFilterModel()
{
    delete d;
}

GmicFilterManager* GmicFilterModel::manager() const
{
    return d->manager;
}

QModelIndex GmicFilterModel::index(GmicFilterNode* node) const
{
    const GmicFilterNode* const item = node->parent();

    if (!item)
    {
        return QModelIndex();
    }

    return createIndex(item->children().indexOf(node), 0, node);
}

void GmicFilterModel::slotEntryAdded(GmicFilterNode* item)
{
    Q_ASSERT(item && item->parent());

    int row                     = item->parent()->children().indexOf(item);
    GmicFilterNode* const pitem = item->parent();

    // item was already added so remove before beginInsertRows is called

    pitem->remove(item);
    beginInsertRows(index(pitem), row, row);
    pitem->add(item, row);
    endInsertRows();
}

void GmicFilterModel::slotEntryRemoved(GmicFilterNode* parent, int row, GmicFilterNode* item)
{
    // item was already removed, re-add so beginRemoveRows works

    parent->add(item, row);
    beginRemoveRows(index(parent), row, row);
    parent->remove(item);
    endRemoveRows();
}

void GmicFilterModel::slotEntryChanged(GmicFilterNode* item)
{
    QModelIndex idx = index(item);

    Q_EMIT dataChanged(idx, idx);
}

bool GmicFilterModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if ((row < 0) || (count <= 0) || ((row + count) > rowCount(parent)))
    {
        return false;
    }

    const GmicFilterNode* const fnode = node(parent);

    for (int i = (row + count - 1) ; i >= row ; --i)
    {
        GmicFilterNode* const item = fnode->children().at(i);
        d->manager->removeEntry(item);
    }

    if (d->endMacro)
    {
        d->manager->undoRedoStack()->endMacro();
        d->endMacro = false;
    }

    return true;
}

QVariant GmicFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
    {
        switch (section)
        {
            case 0:
            {
                return QObject::tr("Title");
            }

            case 1:
            {
                return QObject::tr("Comment");
            }
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant GmicFilterModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (index.model() != this))
    {
        return QVariant();
    }

    const GmicFilterNode* const commandNode = node(index);

    switch (role)
    {
        case Qt::ToolTipRole:
        {
            if (commandNode->type() == GmicFilterNode::Item)
            {
                DToolTipStyleSheet cnt;
                QString tip    = cnt.tipHeader;
                QString header = commandNode->title;

                tip += cnt.headBeg + header + cnt.headEnd;

                tip += cnt.cellBeg +  QObject::tr("G'MIC items:") + cnt.cellMid;
                tip += QString::number(commandNode->commands.count()) + cnt.cellEnd;

                tip += cnt.cellBeg +  QObject::tr("Chained Filters:") + cnt.cellMid;
                tip += cnt.breakString(commandNode->commands.keys().join(QLatin1String(", "))) + cnt.cellEnd;

                tip += cnt.cellBeg +  QObject::tr("Description:") + cnt.cellMid;
                tip += cnt.breakString(commandNode->desc) + cnt.cellEnd;

                tip += cnt.tipFooter;

                return tip;
            }

            return QVariant();
        }

        case Qt::DisplayRole:
        {
            if (commandNode->type() == GmicFilterNode::Separator)
            {
                // NOTE: separator is draw in the delegate implmeentation.

                return QVariant();
            }

            switch (index.column())
            {
                case 0:
                {
                    return commandNode->title;
                }

                case 1:
                {
                    return commandNode->desc;
                }
            }

            break;
        }

        case GmicFilterModel::CommandRole:
        {
            return commandNode->commands;
        }

        case GmicFilterModel::DateAddedRole:
        {
            return commandNode->dateAdded;
        }

        case GmicFilterModel::TypeRole:
        {
            return commandNode->type();
        }

        case GmicFilterModel::SeparatorRole:
        {
            return (commandNode->type() == GmicFilterNode::Separator);
        }

        case Qt::DecorationRole:
        {
            if (index.column() == 0)
            {
                if      (commandNode->type() == GmicFilterNode::Item)
                {
                    return QIcon::fromTheme(QLatin1String("process-working-symbolic"));
                }
                else if (commandNode->type() == GmicFilterNode::RootFolder)
                {
                    return s_gmicQtPluginIcon();
                }
                else if (commandNode->type() == GmicFilterNode::Separator)
                {
                    return QIcon();
                }
                else
                {
                    return QIcon::fromTheme(QLatin1String("folder"));
                }
            }
        }
    }

    return QVariant();
}

int GmicFilterModel::columnCount(const QModelIndex& parent) const
{
    return ((parent.column() > 0) ? 0 : 2);
}

int GmicFilterModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    if (!parent.isValid())
    {
        return d->manager->commands()->children().count();
    }

    const GmicFilterNode* const item = static_cast<GmicFilterNode*>(parent.internalPointer());

    return item->children().count();
}

QModelIndex GmicFilterModel::index(int row, int column, const QModelIndex& parent) const
{
    if (
        (row    <  0)                ||
        (column <  0)                ||
        (row    >= rowCount(parent)) ||
        (column >= columnCount(parent))
       )
    {
        return QModelIndex();
    }

    // get the parent node

    const GmicFilterNode* const parentNode = node(parent);

    return createIndex(row, column, parentNode->children().at(row));
}

QModelIndex GmicFilterModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    const GmicFilterNode* const itemNode = node(index);
    GmicFilterNode* const parentNode     = (itemNode ? itemNode->parent() : nullptr);

    if (!parentNode || (parentNode == d->manager->commands()))
    {
        return QModelIndex();
    }

    // get the parent's row

    const GmicFilterNode* const grandParentNode = parentNode->parent();
    int parentRow                               = grandParentNode->children().indexOf(parentNode);

    Q_ASSERT(parentRow >= 0);

    return createIndex(parentRow, 0, parentNode);
}

bool GmicFilterModel::hasChildren(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return true;
    }

    const GmicFilterNode* const parentNode = node(parent);

    return (
            (parentNode->type() == GmicFilterNode::Folder) ||
            (parentNode->type() == GmicFilterNode::RootFolder)
           );
}

Qt::ItemFlags GmicFilterModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags                     = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    const GmicFilterNode* const commandNode = node(index);

    if (commandNode->type() != GmicFilterNode::RootFolder)
    {
        flags |= Qt::ItemIsDragEnabled;
    }

    if (hasChildren(index))
    {
        flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}

Qt::DropActions GmicFilterModel::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}

QStringList GmicFilterModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/gmicfilters.xml");

    return types;
}

QMimeData* GmicFilterModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* const mimeData = new QMimeData();
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);

    Q_FOREACH (QModelIndex id, indexes)
    {
        if ((id.column() != 0) || !id.isValid())
        {
            continue;
        }

        QByteArray encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadWrite);
        GmicXmlWriter writer;
        const GmicFilterNode* const parentNode = node(id);
        writer.write(&buffer, parentNode, QString());
        stream << encodedData;
    }

    mimeData->setData(QLatin1String("application/gmicfilters.xml"), ba);

    return mimeData;
}

bool GmicFilterModel::dropMimeData(const QMimeData* data,
                                   Qt::DropAction action,
                                   int row, int column,
                                   const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    if (!data->hasFormat(QLatin1String("application/gmicfilters.xml")) || (column > 0))
    {
        return false;
    }

    QByteArray ba = data->data(QLatin1String("application/gmicfilters.xml"));
    QDataStream stream(&ba, QIODevice::ReadOnly);

    if (stream.atEnd())
    {
        return false;
    }

    QUndoStack* const undoStack = d->manager->undoRedoStack();
    undoStack->beginMacro(QLatin1String("Move Filters"));

    while (!stream.atEnd())
    {
        QByteArray encodedData;
        stream >> encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadOnly);

        GmicXmlReader reader;
        QString currentPath;    // Not used.

        GmicFilterNode* const rootNode  = reader.read(&buffer, false, currentPath);

        QList<GmicFilterNode*> children = rootNode->children();

        for (int i = 0 ; i < children.count() ; ++i)
        {
            GmicFilterNode* const commandNode = children.at(i);
            rootNode->remove(commandNode);
            row                               = qMax(0, row);
            GmicFilterNode* const parentNode  = node(parent);
            d->manager->addEntry(parentNode, commandNode, row);
            d->endMacro                       = true;
        }

        delete rootNode;
    }

    return true;
}

bool GmicFilterModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    GmicFilterNode* const item = node(index);

    switch (role)
    {
        case Qt::DisplayRole:
        {
            if (index.column() == 0)
            {
                d->manager->setTitle(item, value.toString());
                break;
            }

            if (index.column() == 1)
            {
                d->manager->setComment(item, value.toString());
                break;
            }

            return false;
        }

        case GmicFilterModel::CommandRole:
        {
            d->manager->setCommand(item, value.toString());
            break;
        }

        default:
        {
            return false;
        }
    }

    return true;
}

GmicFilterNode* GmicFilterModel::node(const QModelIndex& index) const
{
    GmicFilterNode* const itemNode = static_cast<GmicFilterNode*>(index.internalPointer());

    if (!itemNode)
    {
        return d->manager->commands();
    }

    return itemNode;
}

} // namespace DigikamBqmGmicQtPlugin
