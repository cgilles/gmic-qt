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

class GmicFilterManager;
class GmicFilterNode;

class RemoveGmicFilter : public QUndoCommand
{
public:

    explicit RemoveGmicFilter(GmicFilterManager* const mngr,
                              GmicFilterNode* const parent,
                              int row);
    ~RemoveGmicFilter() override;

    void undo()         override;
    void redo()         override;

protected:

    int                m_row             = 0;
    GmicFilterManager* m_bookmarkManager = nullptr;
    GmicFilterNode*    m_node            = nullptr;
    GmicFilterNode*    m_parent          = nullptr;
    bool               m_done            = false;
};

//---------------------------------------------------------------------------------

class InsertGmicFilter : public RemoveGmicFilter
{
public:

    explicit InsertGmicFilter(GmicFilterManager* const mngr,
                              GmicFilterNode* const parent,
                              GmicFilterNode* const node,
                              int row);

    void undo() override;
    void redo() override;
};

//---------------------------------------------------------------------------------

class ChangeGmicFilter : public QUndoCommand
{
public:

    enum GmicFilterData
    {
        Command = 0,
        Title,
        Desc
    };

public:

    explicit ChangeGmicFilter(GmicFilterManager* const mngr,
                              GmicFilterNode* const node,
                              const QString& newValue,
                              GmicFilterData type);
    ~ChangeGmicFilter()   override;

    void undo()           override;
    void redo()           override;

private:

    class Private;
    Private* const d = nullptr;
};

/**
 * GmicFilterModel is a QAbstractItemModel wrapper around the BookmarkManager
 */
class GmicFilterModel : public QAbstractItemModel
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

    explicit GmicFilterModel(GmicFilterManager* const mngr,
                             QObject* const parent = nullptr);
    ~GmicFilterModel()                                                                               override;

    GmicFilterManager* bookmarksManager()                                                     const;
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
    GmicFilterNode* node(const QModelIndex& index)                                            const;
    QModelIndex index(GmicFilterNode* node)                                                   const;

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                      int column, const QModelIndex& parent)                                         override;

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex())                   override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)           override;

public Q_SLOTS:

    void signalEntryAdded(GmicFilterNode* item);
    void signalEntryRemoved(GmicFilterNode* parent, int row, GmicFilterNode* item);
    void signalEntryChanged(GmicFilterNode* item);

private:

    class Private;
    Private* const d = nullptr;
};

/**
 *  Proxy model that filters out the Gmic Commands so only the folders
 *  are left behind.  Used in the add command dialog combobox.
 */
class AddGmicFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    explicit AddGmicFilterProxyModel(QObject* const parent = nullptr);

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

    int columnCount(const QModelIndex&)                         const override;

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
class GmicFilterManager : public QObject
{
    Q_OBJECT

public:

    explicit GmicFilterManager(const QString& bookmarksFile,
                               QObject* const parent = nullptr);
    ~GmicFilterManager()                                              override;

    void addCommand(GmicFilterNode* const parent,
                    GmicFilterNode* const node,
                    int row = -1);

    void removeCommand(GmicFilterNode* const node);

    void setTitle(GmicFilterNode* const node, const QString& newTitle);
    void setCommand(GmicFilterNode* const node, const QString& newcommand);
    void setComment(GmicFilterNode* const node, const QString& newDesc);
    void changeExpanded();

    GmicFilterNode*  commands();
    GmicFilterModel* commandsModel();
    QUndoStack*      undoRedoStack() const;

    void save();
    void load();

Q_SIGNALS:

    void signalEntryAdded(GmicFilterNode* item);
    void signalEntryRemoved(GmicFilterNode* parent, int row, GmicFilterNode* item);
    void signalEntryChanged(GmicFilterNode* item);

public Q_SLOTS:

    void slotImportFilters();
    void slotExportFilters();

private:

    class Private;
    Private* const d = nullptr;

    friend class RemoveGmicFilter;
    friend class ChangeGmicFilter;
};

} // namespace DigikamBqmGmicQtPlugin

#endif // DIGIKAM_GMIC_COMMAND_MNGR_H
