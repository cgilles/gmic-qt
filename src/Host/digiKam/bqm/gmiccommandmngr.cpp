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

#include "gmiccommandmngr.h"

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

#include "gmiccommandnode.h"

namespace DigikamBqmGmicQtPlugin
{

RemoveGmicCommand::RemoveGmicCommand(GmicCommandManager* const mngr,
                                     GmicCommandNode* const parent,
                                     int row)
    : QUndoCommand(QObject::tr("Remove Filter")),
      m_row            (row),
      m_bookmarkManager(mngr),
      m_node           (parent->children().value(row)),
      m_parent         (parent)
{
}

RemoveGmicCommand::~RemoveGmicCommand()
{
    if (m_done && !m_node->parent())
    {
        delete m_node;
    }
}

void RemoveGmicCommand::undo()
{
    m_parent->add(m_node, m_row);

    Q_EMIT m_bookmarkManager->entryAdded(m_node);

    m_done = false;
}

void RemoveGmicCommand::redo()
{
    m_parent->remove(m_node);

    Q_EMIT m_bookmarkManager->entryRemoved(m_parent, m_row, m_node);

    m_done = true;
}

// --------------------------------------------------------------

InsertGmicCommand::InsertGmicCommand(GmicCommandManager* const mngr,
                                     GmicCommandNode* const parent,
                                     GmicCommandNode* const node,
                                     int row)
    : RemoveGmicCommand(mngr, parent, row)
{
    setText(QObject::tr("Insert Filter"));
    m_node = node;
}

void InsertGmicCommand::undo()
{
    RemoveGmicCommand::redo();
}

void InsertGmicCommand::redo()
{
    RemoveGmicCommand::undo();
}

// --------------------------------------------------------------

class Q_DECL_HIDDEN ChangeGmicCommand::Private
{
public:

    Private() = default;

    GmicCommandManager* manager   = nullptr;
    GmicCommandData     type      = Command;
    QString             oldValue;
    QString             newValue;
    GmicCommandNode*    node      = nullptr;
};

ChangeGmicCommand::ChangeGmicCommand(GmicCommandManager* const mngr,
                                     GmicCommandNode* const node,
                                     const QString& newValue,
                                     GmicCommandData type)
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
            setText(QObject::tr("G'MIC Command Change"));
            break;
        }
    }
}

ChangeGmicCommand::~ChangeGmicCommand()
{
    delete d;
}

void ChangeGmicCommand::undo()
{
    switch (d->type)
    {
        case Title:
        {
            d->node->title = d->oldValue;
            break;
        }

        case Desc:
        {
            d->node->desc  = d->oldValue;
            break;
        }

        default:    // Gmic Command
        {
            d->node->command = d->oldValue;
            break;
        }
    }

    Q_EMIT d->manager->entryChanged(d->node);
}

void ChangeGmicCommand::redo()
{
    switch (d->type)
    {
        case Title:
        {
            d->node->title = d->newValue;
            break;
        }

        case Desc:
        {
            d->node->desc  = d->newValue;
            break;
        }

        default:    // Gmic Command
        {
            d->node->command   = d->newValue;
            break;
        }
    }

    Q_EMIT d->manager->entryChanged(d->node);
}

// --------------------------------------------------------------

class Q_DECL_HIDDEN GmicCommandModel::Private
{
public:

    Private() = default;

    GmicCommandManager* manager   = nullptr;
    bool                endMacro  = false;
};

GmicCommandModel::GmicCommandModel(GmicCommandManager* const mngr, QObject* const parent)
    : QAbstractItemModel(parent),
      d                 (new Private)
{
    d->manager = mngr;

    connect(d->manager, SIGNAL(entryAdded(GmicCommandNode*)),
            this, SLOT(entryAdded(GmicCommandNode*)));

    connect(d->manager, SIGNAL(entryRemoved(GmicCommandNode*,int,GmicCommandNode*)),
            this, SLOT(entryRemoved(GmicCommandNode*,int,GmicCommandNode*)));

    connect(d->manager, SIGNAL(entryChanged(GmicCommandNode*)),
            this, SLOT(entryChanged(GmicCommandNode*)));
}

GmicCommandModel::~GmicCommandModel()
{
    delete d;
}

GmicCommandManager* GmicCommandModel::bookmarksManager() const
{
    return d->manager;
}

QModelIndex GmicCommandModel::index(GmicCommandNode* node) const
{
    GmicCommandNode* const parent = node->parent();

    if (!parent)
    {
        return QModelIndex();
    }

    return createIndex(parent->children().indexOf(node), 0, node);
}

void GmicCommandModel::entryAdded(GmicCommandNode* item)
{
    Q_ASSERT(item && item->parent());

    int row                       = item->parent()->children().indexOf(item);
    GmicCommandNode* const parent = item->parent();

    // item was already added so remove before beginInsertRows is called

    parent->remove(item);
    beginInsertRows(index(parent), row, row);
    parent->add(item, row);
    endInsertRows();
}

void GmicCommandModel::entryRemoved(GmicCommandNode* parent, int row, GmicCommandNode* item)
{
    // item was already removed, re-add so beginRemoveRows works

    parent->add(item, row);
    beginRemoveRows(index(parent), row, row);
    parent->remove(item);
    endRemoveRows();
}

void GmicCommandModel::entryChanged(GmicCommandNode* item)
{
    QModelIndex idx = index(item);

    Q_EMIT dataChanged(idx, idx);
}

bool GmicCommandModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if ((row < 0) || (count <= 0) || ((row + count) > rowCount(parent)))
    {
        return false;
    }

    GmicCommandNode* const bookmarkNode = node(parent);

    for (int i = (row + count - 1) ; i >= row ; --i)
    {
        GmicCommandNode* const node = bookmarkNode->children().at(i);
        d->manager->removeCommand(node);
    }

    if (d->endMacro)
    {
        d->manager->undoRedoStack()->endMacro();
        d->endMacro = false;
    }

    return true;
}

QVariant GmicCommandModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QVariant GmicCommandModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (index.model() != this))
    {
        return QVariant();
    }

    const GmicCommandNode* const commandNode = node(index);

    switch (role)
    {
        case Qt::EditRole:
        case Qt::DisplayRole:
        {
            if (commandNode->type() == GmicCommandNode::Separator)
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

        case GmicCommandModel::CommandRole:
        {
            return commandNode->command;
        }

        case GmicCommandModel::DateAddedRole:
        {
            return commandNode->dateAdded;
        }

        case GmicCommandModel::TypeRole:
        {
            return commandNode->type();
        }

        case GmicCommandModel::SeparatorRole:
        {
            return (commandNode->type() == GmicCommandNode::Separator);
        }

        case Qt::DecorationRole:
        {
            if (index.column() == 0)
            {
                if (commandNode->type() == GmicCommandNode::Item)
                {
                    return QIcon::fromTheme(QLatin1String("run"));
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

int GmicCommandModel::columnCount(const QModelIndex& parent) const
{
    return ((parent.column() > 0) ? 0 : 2);
}

int GmicCommandModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
    {
        return 0;
    }

    if (!parent.isValid())
    {
        return d->manager->commands()->children().count();
    }

    const GmicCommandNode* const item = static_cast<GmicCommandNode*>(parent.internalPointer());

    return item->children().count();
}

QModelIndex GmicCommandModel::index(int row, int column, const QModelIndex& parent) const
{
    if ((row < 0) || (column < 0) || (row >= rowCount(parent)) || (column >= columnCount(parent)))
    {
        return QModelIndex();
    }

    // get the parent node

    GmicCommandNode* const parentNode = node(parent);

    return createIndex(row, column, parentNode->children().at(row));
}

QModelIndex GmicCommandModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    GmicCommandNode* const itemNode   = node(index);
    GmicCommandNode* const parentNode = (itemNode ? itemNode->parent() : nullptr);

    if (!parentNode || (parentNode == d->manager->commands()))
    {
        return QModelIndex();
    }

    // get the parent's row

    GmicCommandNode* const grandParentNode = parentNode->parent();
    int parentRow                          = grandParentNode->children().indexOf(parentNode);

    Q_ASSERT(parentRow >= 0);

    return createIndex(parentRow, 0, parentNode);
}

bool GmicCommandModel::hasChildren(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return true;
    }

    const GmicCommandNode* const parentNode = node(parent);

    return (
            (parentNode->type() == GmicCommandNode::Folder) ||
            (parentNode->type() == GmicCommandNode::RootFolder)
           );
}

Qt::ItemFlags GmicCommandModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags                = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    GmicCommandNode* const commandNode = node(index);

    if (commandNode->type() != GmicCommandNode::RootFolder)
    {
        flags |= Qt::ItemIsDragEnabled;
    }

    if (
        (commandNode->type() != GmicCommandNode::Separator) &&
        (commandNode->type() != GmicCommandNode::RootFolder)
       )
    {
        flags |= Qt::ItemIsEditable;
    }

    if (hasChildren(index))
    {
        flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}

Qt::DropActions GmicCommandModel::supportedDropActions() const
{
    return (Qt::CopyAction | Qt::MoveAction);
}

QStringList GmicCommandModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/gmiccommands.xbel");

    return types;
}

QMimeData* GmicCommandModel::mimeData(const QModelIndexList& indexes) const
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
        XbelWriter writer;
        const GmicCommandNode* const parentNode = node(index);
        writer.write(&buffer, parentNode);
        stream << encodedData;
    }

    mimeData->setData(QLatin1String("application/gmiccommands.xbel"), data);

    return mimeData;
}

bool GmicCommandModel::dropMimeData(const QMimeData* data,
                                    Qt::DropAction action,
                                    int row, int column,
                                    const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction)
    {
        return true;
    }

    if (!data->hasFormat(QLatin1String("application/gmiccommands.xbel")) || column > 0)
    {
        return false;
    }

    QByteArray ba = data->data(QLatin1String("application/gmiccommands.xbel"));
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

        XbelReader reader;
        GmicCommandNode* const rootNode  = reader.read(&buffer);
        QList<GmicCommandNode*> children = rootNode->children();

        for (int i = 0 ; i < children.count() ; ++i)
        {
            GmicCommandNode* const commandNode = children.at(i);
            rootNode->remove(commandNode);
            row                               = qMax(0, row);
            GmicCommandNode* const parentNode = node(parent);
            d->manager->addCommand(parentNode, commandNode, row);
            d->endMacro                       = true;
        }

        delete rootNode;
    }

    return true;
}

bool GmicCommandModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || (flags(index) & Qt::ItemIsEditable) == 0)
    {
        return false;
    }

    GmicCommandNode* const item = node(index);

    switch (role)
    {
        case Qt::EditRole:
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

        case GmicCommandModel::CommandRole:
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

GmicCommandNode* GmicCommandModel::node(const QModelIndex& index) const
{
    GmicCommandNode* const itemNode = static_cast<GmicCommandNode*>(index.internalPointer());

    if (!itemNode)
    {
        return d->manager->commands();
    }

    return itemNode;
}

// --------------------------------------------------------------

AddGmicCommandProxyModel::AddGmicCommandProxyModel(QObject* const parent)
    : QSortFilterProxyModel(parent)
{
}

int AddGmicCommandProxyModel::columnCount(const QModelIndex& parent) const
{
    return qMin(1, QSortFilterProxyModel::columnCount(parent));
}

bool AddGmicCommandProxyModel::filterAcceptsRow(int srow, const QModelIndex& sparent) const
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

class Q_DECL_HIDDEN GmicCommandManager::Private
{
public:

    Private() = default;

    bool              loaded             = false;
    GmicCommandNode*  commandRootNode    = nullptr;
    GmicCommandModel* commandModel       = nullptr;
    QUndoStack        commands;
    QString           commandsFile;
};

GmicCommandManager::GmicCommandManager(const QString& commandsFile, QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->commandsFile = commandsFile;
    load();
}

GmicCommandManager::~GmicCommandManager()
{
    delete d->commandRootNode;
    delete d;
}

void GmicCommandManager::changeExpanded()
{
}

void GmicCommandManager::load()
{
    if (d->loaded)
    {
        return;
    }

    qCDebug(DIGIKAM_GEOIFACE_LOG) << "Loading G'MIC filter from" << d->commandsFile;
    d->loaded = true;

    XbelReader reader;
    d->commandRootNode = reader.read(d->commandsFile);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(nullptr, QObject::tr("Loading Commands"),
                             QObject::tr("Error when loading G'MIC filters on line %1, column %2:\n%3")
                                  .arg(reader.lineNumber())
                                  .arg(reader.columnNumber())
                                  .arg(reader.errorString()));
    }
}

void GmicCommandManager::save()
{
    if (!d->loaded)
    {
        return;
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Saving Gmic Commands to" << d->commandsFile;

    XbelWriter writer;

    if (!writer.write(d->commandsFile, d->commandRootNode))
    {
        qCWarning(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicCommandManager: error saving to" << d->commandsFile;
    }
}

void GmicCommandManager::addCommand(GmicCommandNode* const parent,
                                    GmicCommandNode* const node, int row)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(parent);

    InsertGmicCommand* const command = new InsertGmicCommand(this, parent, node, row);
    d->commands.push(command);
}

void GmicCommandManager::removeCommand(GmicCommandNode* const node)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    GmicCommandNode* const parent    = node->parent();
    int row                          = parent->children().indexOf(node);
    RemoveGmicCommand* const command = new RemoveGmicCommand(this, parent, row);
    d->commands.push(command);
}

void GmicCommandManager::setTitle(GmicCommandNode* const node, const QString& newTitle)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeGmicCommand* const command = new ChangeGmicCommand(this, node, newTitle,
                                                             ChangeGmicCommand::Title);
    d->commands.push(command);
}

void GmicCommandManager::setCommand(GmicCommandNode* const node, const QString& newCommand)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeGmicCommand* const command = new ChangeGmicCommand(this, node, newCommand,
                                                             ChangeGmicCommand::Command);
    d->commands.push(command);
}

void GmicCommandManager::setComment(GmicCommandNode* const node, const QString& newDesc)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeGmicCommand* const command = new ChangeGmicCommand(this, node, newDesc,
                                                             ChangeGmicCommand::Desc);
    d->commands.push(command);
}

GmicCommandNode* GmicCommandManager::commands()
{
    if (!d->loaded)
    {
        load();
    }

    return d->commandRootNode;
}

GmicCommandModel* GmicCommandManager::commandsModel()
{
    if (!d->commandModel)
    {
        d->commandModel = new GmicCommandModel(this, this);
    }

    return d->commandModel;
}

QUndoStack* GmicCommandManager::undoRedoStack() const
{
    return &d->commands;
}

void GmicCommandManager::importCommands()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open File"),
                                                    QString(),
                                                    QObject::tr("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
    {
        return;
    }

    XbelReader reader;
    GmicCommandNode* const importRootNode = reader.read(fileName);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(nullptr, QObject::tr("Loading Filters"),
                             QObject::tr("Error when loading G'MIC filters on line %1, column %2:\n%3")
                                  .arg(reader.lineNumber())
                                  .arg(reader.columnNumber())
                                  .arg(reader.errorString()));
    }

    importRootNode->setType(GmicCommandNode::Folder);
    importRootNode->title = QObject::tr("Imported %1").arg(QLocale().toString(QDate::currentDate(), QLocale::ShortFormat));
    addCommand(commands(), importRootNode);
}

void GmicCommandManager::exportCommands()
{
    QString fileName = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save File"),
                                                    QObject::tr("%1 Gmic Commands.xbel")
                                                    .arg(QCoreApplication::applicationName()),
                                                    QObject::tr("XBEL (*.xbel *.xml)"));
    if (fileName.isEmpty())
    {
        return;
    }

    XbelWriter writer;

    if (!writer.write(fileName, d->commandRootNode))
    {
        QMessageBox::critical(nullptr, QObject::tr("Export filters"),
                                       QObject::tr("Error saving G'MIC filters"));
    }
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmiccommandmngr.cpp"
