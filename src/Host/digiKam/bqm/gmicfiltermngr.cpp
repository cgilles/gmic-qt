/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 *  Description: digiKam Batch Queue Manager plugin for GmicQt.
 *
 *  G'MIC-Qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  G'MIC-Qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "gmicfiltermngr.h"

// Qt includes

#include <QBuffer>
#include <QFile>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QIcon>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QDebug>
#include <QApplication>
#include <QFileDialog>

// digiKam includes

#include "digikam_debug.h"

// Local includes

#include "gmicfilternode.h"

namespace DigikamBqmGmicQtPlugin
{

RemoveGmicFilter::RemoveGmicFilter(GmicFilterManager* const mngr,
                                   GmicFilterNode* const parent,
                                     int row)
    : QUndoCommand(QObject::tr("Remove Filter")),
      m_row            (row),
      m_bookmarkManager(mngr),
      m_node           (parent->children().value(row)),
      m_parent         (parent)
{
}

RemoveGmicFilter::~RemoveGmicFilter()
{
    if (m_done && !m_node->parent())
    {
        delete m_node;
    }
}

void RemoveGmicFilter::undo()
{
    m_parent->add(m_node, m_row);

    Q_EMIT m_bookmarkManager->signalEntryAdded(m_node);

    m_done = false;
}

void RemoveGmicFilter::redo()
{
    m_parent->remove(m_node);

    Q_EMIT m_bookmarkManager->signalEntryRemoved(m_parent, m_row, m_node);

    m_done = true;
}

// --------------------------------------------------------------

InsertGmicFilter::InsertGmicFilter(GmicFilterManager* const mngr,
                                   GmicFilterNode* const parent,
                                   GmicFilterNode* const node,
                                   int row)
    : RemoveGmicFilter(mngr, parent, row)
{
    setText(QObject::tr("Insert Filter"));
    m_node = node;
}

void InsertGmicFilter::undo()
{
    RemoveGmicFilter::redo();
}

void InsertGmicFilter::redo()
{
    RemoveGmicFilter::undo();
}

// --------------------------------------------------------------

class Q_DECL_HIDDEN ChangeGmicFilter::Private
{
public:

    Private() = default;

    GmicFilterManager* manager   = nullptr;
    GmicFilterData     type      = Command;
    QString             oldValue;
    QString             newValue;
    GmicFilterNode*    node      = nullptr;
};

ChangeGmicFilter::ChangeGmicFilter(GmicFilterManager* const mngr,
                                   GmicFilterNode* const node,
                                   const QString& newValue,
                                   GmicFilterData type)
    : QUndoCommand(),
      d           (new Private)
{
    d->manager  = mngr;
    d->type     = type;
    d->newValue = newValue;
    d->node     = node;

    switch (d->type)
    {
        case Title:
        {
            d->oldValue = d->node->title;
            setText(QObject::tr("Title Change"));
            break;
        }

        case Desc:
        {
            d->oldValue = d->node->desc;
            setText(QObject::tr("Comment Change"));
            break;
        }

        default:    // Gmic Command
        {
            d->oldValue = d->node->command;
            setText(QObject::tr("Command Change"));
            break;
        }
    }
}

ChangeGmicFilter::~ChangeGmicFilter()
{
    delete d;
}

void ChangeGmicFilter::undo()
{
    switch (d->type)
    {
        case Title:
        {
            d->node->title   = d->oldValue;
            break;
        }

        case Desc:
        {
            d->node->desc    = d->oldValue;
            break;
        }

        default:    // Gmic Command
        {
            d->node->command = d->oldValue;
            break;
        }
    }

    Q_EMIT d->manager->signalEntryChanged(d->node);
}

void ChangeGmicFilter::redo()
{
    switch (d->type)
    {
        case Title:
        {
            d->node->title   = d->newValue;
            break;
        }

        case Desc:
        {
            d->node->desc    = d->newValue;
            break;
        }

        default:    // Gmic Command
        {
            d->node->command = d->newValue;
            break;
        }
    }

    Q_EMIT d->manager->signalEntryChanged(d->node);
}

// --------------------------------------------------------------

class Q_DECL_HIDDEN GmicFilterModel::Private
{
public:

    Private() = default;

    GmicFilterManager* manager   = nullptr;
    bool               endMacro  = false;
};

GmicFilterModel::GmicFilterModel(GmicFilterManager* const mngr, QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
    d->manager = mngr;

    connect(d->manager, SIGNAL(signalEntryAdded(GmicFilterNode*)),
            this, SLOT(signalEntryAdded(GmicFilterNode*)));

    connect(d->manager, SIGNAL(signalEntryRemoved(GmicFilterNode*,int,GmicFilterNode*)),
            this, SLOT(signalEntryRemoved(GmicFilterNode*,int,GmicFilterNode*)));

    connect(d->manager, SIGNAL(signalEntryChanged(GmicFilterNode*)),
            this, SLOT(signalEntryChanged(GmicFilterNode*)));
}

GmicFilterModel::~GmicFilterModel()
{
    delete d;
}

GmicFilterManager* GmicFilterModel::bookmarksManager() const
{
    return d->manager;
}

QModelIndex GmicFilterModel::index(GmicFilterNode* node) const
{
    GmicFilterNode* const parent = node->parent();

    if (!parent)
    {
        return QModelIndex();
    }

    return createIndex(parent->children().indexOf(node), 0, node);
}

void GmicFilterModel::signalEntryAdded(GmicFilterNode* item)
{
    Q_ASSERT(item && item->parent());

    int row                       = item->parent()->children().indexOf(item);
    GmicFilterNode* const parent = item->parent();

    // item was already added so remove before beginInsertRows is called

    parent->remove(item);
    beginInsertRows(index(parent), row, row);
    parent->add(item, row);
    endInsertRows();
}

void GmicFilterModel::signalEntryRemoved(GmicFilterNode* parent, int row, GmicFilterNode* item)
{
    // item was already removed, re-add so beginRemoveRows works

    parent->add(item, row);
    beginRemoveRows(index(parent), row, row);
    parent->remove(item);
    endRemoveRows();
}

void GmicFilterModel::signalEntryChanged(GmicFilterNode* item)
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

    GmicFilterNode* const bookmarkNode = node(parent);

    for (int i = (row + count - 1) ; i >= row ; --i)
    {
        GmicFilterNode* const node = bookmarkNode->children().at(i);
        d->manager->removeCommand(node);
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
        case Qt::DisplayRole:
        {
            if (commandNode->type() == GmicFilterNode::Separator)
            {
                switch (index.column())
                {
                    case 0:
                    {
                        return QString(50, QChar(0xB7));
                    }

                    case 1:
                    {
                        return QString();
                    }
                }
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
            return commandNode->command;
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
                    return QIcon(":resources/gmic_hat.png");
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
        (row < 0) || (column < 0) ||
        (row >= rowCount(parent)) ||
        (column >= columnCount(parent))
       )
    {
        return QModelIndex();
    }

    // get the parent node

    GmicFilterNode* const parentNode = node(parent);

    return createIndex(row, column, parentNode->children().at(row));
}

QModelIndex GmicFilterModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    GmicFilterNode* const itemNode   = node(index);
    GmicFilterNode* const parentNode = (itemNode ? itemNode->parent() : nullptr);

    if (!parentNode || (parentNode == d->manager->commands()))
    {
        return QModelIndex();
    }

    // get the parent's row

    GmicFilterNode* const grandParentNode = parentNode->parent();
    int parentRow                          = grandParentNode->children().indexOf(parentNode);

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

    Qt::ItemFlags flags               = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    GmicFilterNode* const commandNode = node(index);

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
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    Q_FOREACH (QModelIndex index, indexes)
    {
        if ((index.column() != 0) || !index.isValid())
        {
            continue;
        }

        QByteArray encodedData;
        QBuffer buffer(&encodedData);
        buffer.open(QBuffer::ReadWrite);
        GmicXmlWriter writer;
        const GmicFilterNode* const parentNode = node(index);
        writer.write(&buffer, parentNode);
        stream << encodedData;
    }

    mimeData->setData(QLatin1String("application/gmicfilters.xml"), data);

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

    if (!data->hasFormat(QLatin1String("application/gmicfilters.xml")) || column > 0)
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
        GmicFilterNode* const rootNode  = reader.read(&buffer);
        QList<GmicFilterNode*> children = rootNode->children();

        for (int i = 0 ; i < children.count() ; ++i)
        {
            GmicFilterNode* const commandNode = children.at(i);
            rootNode->remove(commandNode);
            row                               = qMax(0, row);
            GmicFilterNode* const parentNode  = node(parent);
            d->manager->addCommand(parentNode, commandNode, row);
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

// --------------------------------------------------------------

AddGmicFilterProxyModel::AddGmicFilterProxyModel(QObject* const parent)
    : QSortFilterProxyModel(parent)
{
}

int AddGmicFilterProxyModel::columnCount(const QModelIndex& parent) const
{
    return qMin(1, QSortFilterProxyModel::columnCount(parent));
}

bool AddGmicFilterProxyModel::filterAcceptsRow(int srow, const QModelIndex& sparent) const
{
    QModelIndex idx = sourceModel()->index(srow, 0, sparent);

    return sourceModel()->hasChildren(idx);
}

// --------------------------------------------------------------

TreeProxyModel::TreeProxyModel(QObject* const parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

int TreeProxyModel::columnCount(const QModelIndex&) const
{
    // 1th column : Title
    // 2th column : Comment

    return 2;
}

bool TreeProxyModel::filterAcceptsRow(int srow, const QModelIndex& sparent) const
{
    QModelIndex index = sourceModel()->index(srow, 0, sparent);

    if (!index.isValid())
    {
        return false;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    if (index.data().toString().contains(filterRegularExpression()))

#else

    if (index.data().toString().contains(filterRegExp()))

#endif

    {
        return true;
    }

    for (int i = 0 ; i < sourceModel()->rowCount(index) ; ++i)
    {
        if (filterAcceptsRow(i, index))
        {
            return true;
        }
    }

    return false;
}

void TreeProxyModel::emitResult(bool v)
{
    Q_EMIT signalFilterAccepts(v);
}

// --------------------------------------------------------------

class Q_DECL_HIDDEN GmicFilterManager::Private
{
public:

    Private() = default;

    bool             loaded             = false;
    GmicFilterNode*  commandRootNode    = nullptr;
    GmicFilterModel* commandModel       = nullptr;
    QUndoStack       commands;
    QString          commandsFile;
};

GmicFilterManager::GmicFilterManager(const QString& commandsFile, QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->commandsFile = commandsFile;
    load();
}

GmicFilterManager::~GmicFilterManager()
{
    delete d->commandRootNode;
    delete d;
}

void GmicFilterManager::changeExpanded()
{
}

void GmicFilterManager::load()
{
    if (d->loaded)
    {
        return;
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Loading G'MIC filters from" << d->commandsFile;
    d->loaded = true;

    GmicXmlReader reader;
    d->commandRootNode = reader.read(d->commandsFile);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(nullptr, QObject::tr("Loading Filters"),
                             QObject::tr("Error when loading G'MIC filters on line %1, column %2:\n%3")
                                  .arg(reader.lineNumber())
                                  .arg(reader.columnNumber())
                                  .arg(reader.errorString()));
    }
}

void GmicFilterManager::save()
{
    if (!d->loaded)
    {
        return;
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Saving G'MIC Filters to" << d->commandsFile;

    GmicXmlWriter writer;

    if (!writer.write(d->commandsFile, d->commandRootNode))
    {
        qCWarning(DIGIKAM_DPLUGIN_BQM_LOG) << "Error saving G'MIC filters to" << d->commandsFile;
    }
}

void GmicFilterManager::addCommand(GmicFilterNode* const parent,
                                   GmicFilterNode* const node, int row)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(parent);

    InsertGmicFilter* const command = new InsertGmicFilter(this, parent, node, row);
    d->commands.push(command);
}

void GmicFilterManager::removeCommand(GmicFilterNode* const node)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    GmicFilterNode* const parent    = node->parent();
    int row                         = parent->children().indexOf(node);
    RemoveGmicFilter* const command = new RemoveGmicFilter(this, parent, row);
    d->commands.push(command);
}

void GmicFilterManager::setTitle(GmicFilterNode* const node, const QString& newTitle)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeGmicFilter* const command = new ChangeGmicFilter(this, node, newTitle,
                                                           ChangeGmicFilter::Title);
    d->commands.push(command);
}

void GmicFilterManager::setCommand(GmicFilterNode* const node, const QString& newCommand)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeGmicFilter* const command = new ChangeGmicFilter(this, node, newCommand,
                                                           ChangeGmicFilter::Command);
    d->commands.push(command);
}

void GmicFilterManager::setComment(GmicFilterNode* const node, const QString& newDesc)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeGmicFilter* const command = new ChangeGmicFilter(this, node, newDesc,
                                                           ChangeGmicFilter::Desc);
    d->commands.push(command);
}

GmicFilterNode* GmicFilterManager::commands()
{
    if (!d->loaded)
    {
        load();
    }

    return d->commandRootNode;
}

GmicFilterModel* GmicFilterManager::commandsModel()
{
    if (!d->commandModel)
    {
        d->commandModel = new GmicFilterModel(this, this);
    }

    return d->commandModel;
}

QUndoStack* GmicFilterManager::undoRedoStack() const
{
    return &d->commands;
}

void GmicFilterManager::slotImportFilters()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open File"),
                                                    QString(),
                                                    QObject::tr("XML (*.xml)"));
    if (fileName.isEmpty())
    {
        return;
    }

    GmicXmlReader reader;
    GmicFilterNode* const importRootNode = reader.read(fileName);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(nullptr, QObject::tr("Loading Filters"),
                             QObject::tr("Error when loading G'MIC filters on line %1, column %2:\n%3")
                                  .arg(reader.lineNumber())
                                  .arg(reader.columnNumber())
                                  .arg(reader.errorString()));
    }

    importRootNode->setType(GmicFilterNode::Folder);
    importRootNode->title = QObject::tr("Imported %1").arg(QLocale().toString(QDate::currentDate(), QLocale::ShortFormat));
    addCommand(commands(), importRootNode);
}

void GmicFilterManager::slotExportFilters()
{
    QString fileName = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save File"),
                                                    QObject::tr("%1 Gmic Filters.xml")
                                                    .arg(QCoreApplication::applicationName()),
                                                    QObject::tr("XML ( *.xml)"));
    if (fileName.isEmpty())
    {
        return;
    }

    GmicXmlWriter writer;

    if (!writer.write(fileName, d->commandRootNode))
    {
        QMessageBox::critical(nullptr, QObject::tr("Export filters"),
                                       QObject::tr("Error saving G'MIC filters"));
    }
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfiltermngr.cpp"
