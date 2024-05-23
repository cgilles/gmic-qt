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

#ifndef DIGIKAM_GMIC_COMMAND_MNGR_H
#define DIGIKAM_GMIC_COMMAND_MNGR_H

// Qt includes

#include <QObject>
#include <QAbstractItemModel>
#include <QUndoCommand>
#include <QSortFilterProxyModel>

namespace DigikamBqmGmicQtPlugin
{

class GmicCommandManager;
class GmicCommandNode;

class RemoveGmicCommand : public QUndoCommand
{
public:

    explicit RemoveGmicCommand(GmicCommandManager* const mngr,
                               GmicCommandNode* const parent,
                               int row);
    ~RemoveGmicCommand() override;

    void undo()          override;
    void redo()          override;

protected:

    int                 m_row             = 0;
    GmicCommandManager* m_bookmarkManager = nullptr;
    GmicCommandNode*    m_node            = nullptr;
    GmicCommandNode*    m_parent          = nullptr;
    bool                m_done            = false;
};

//---------------------------------------------------------------------------------

class InsertGmicCommand : public RemoveGmicCommand
{
public:

    explicit InsertGmicCommand(GmicCommandManager* const mngr,
                               GmicCommandNode* const parent,
                               GmicCommandNode* const node,
                               int row);

    void undo() override;
    void redo() override;
};

//---------------------------------------------------------------------------------

class ChangeGmicCommand : public QUndoCommand
{
public:

    enum GmicCommandData
    {
        Command = 0,
        Title,
        Desc
    };

public:

    explicit ChangeGmicCommand(GmicCommandManager* const mngr,
                               GmicCommandNode* const node,
                               const QString& newValue,
                               GmicCommandData type);
    ~ChangeGmicCommand()  override;

    void undo()           override;
    void redo()           override;

private:

    class Private;
    Private* const d = nullptr;
};

/**
 * GmicCommandModel is a QAbstractItemModel wrapper around the BookmarkManager
 */
class GmicCommandModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum Roles
    {
        TypeRole          = Qt::UserRole + 1,
        CommandRole       = Qt::UserRole + 2,
        SeparatorRole     = Qt::UserRole + 3,
        DateAddedRole     = Qt::UserRole + 4
    };

public:

    explicit GmicCommandModel(GmicCommandManager* const mngr,
                              QObject* const parent = nullptr);
    ~GmicCommandModel()                                                                               override;

    GmicCommandManager* bookmarksManager()                                                    const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)                       const override;
    int columnCount(const QModelIndex& parent = QModelIndex())                                const override;
    int rowCount(const QModelIndex& parent = QModelIndex())                                   const override;
    QModelIndex index(int, int, const QModelIndex& = QModelIndex())                           const override;
    QModelIndex parent(const QModelIndex& index= QModelIndex())                               const override;
    Qt::ItemFlags flags(const QModelIndex& index)                                             const override;
    Qt::DropActions supportedDropActions ()                                                   const override;
    QMimeData* mimeData(const QModelIndexList& indexes)                                       const override;
    QStringList mimeTypes()                                                                   const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex())                               const override;
    GmicCommandNode* node(const QModelIndex& index)                                           const;
    QModelIndex index(GmicCommandNode* node)                                                  const;

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                      int column, const QModelIndex& parent)                                         override;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex())                   override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)           override;

public Q_SLOTS:

    void entryAdded(GmicCommandNode* item);
    void entryRemoved(GmicCommandNode* parent, int row, GmicCommandNode* item);
    void entryChanged(GmicCommandNode* item);

private:

    class Private;
    Private* const d = nullptr;
};

/**
 *  Proxy model that filters out the Gmic Commands so only the folders
 *  are left behind.  Used in the add command dialog combobox.
 */
class AddGmicCommandProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit AddGmicCommandProxyModel(QObject* const parent = nullptr);

    int columnCount(const QModelIndex& parent = QModelIndex())  const override;

protected:

    bool filterAcceptsRow(int srow, const QModelIndex& sparent) const override;
};

//---------------------------------------------------------------------------------

class TreeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit TreeProxyModel(QObject* const parent = nullptr);

    int columnCount(const QModelIndex&) const                        override;

Q_SIGNALS:

    void signalFilterAccepts(bool);

protected:

    bool filterAcceptsRow(int srow, const QModelIndex& sparent) const override;

private:

    void emitResult(bool v);
};

/**
 *  Gmic Command manager, owner of the commands, loads, saves and basic tasks
 */
class GmicCommandManager : public QObject
{
    Q_OBJECT

public:

    explicit GmicCommandManager(const QString& bookmarksFile, QObject* const parent = nullptr);
    ~GmicCommandManager() override;

    void addCommand(GmicCommandNode* const parent, GmicCommandNode* const node, int row = -1);
    void removeCommand(GmicCommandNode* const node);
    void setTitle(GmicCommandNode* const node, const QString& newTitle);
    void setCommand(GmicCommandNode* const node, const QString& newcommand);
    void setComment(GmicCommandNode* const node, const QString& newDesc);
    void changeExpanded();

    GmicCommandNode*  commands();
    GmicCommandModel* commandsModel();
    QUndoStack*       undoRedoStack() const;

    void save();
    void load();

Q_SIGNALS:

    void entryAdded(GmicCommandNode* item);
    void entryRemoved(GmicCommandNode* parent, int row, GmicCommandNode* item);
    void entryChanged(GmicCommandNode* item);

public Q_SLOTS:

    void importCommands();
    void exportCommands();

private:

    class Private;
    Private* const d = nullptr;

    friend class RemoveGmicCommand;
    friend class ChangeGmicCommand;
};

} // namespace DigikamBqmGmicQtPlugin

#endif // DIGIKAM_GMIC_COMMAND_MNGR_H
