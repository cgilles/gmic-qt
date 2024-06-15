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

#pragma once

// Qt includes

#include <QObject>
#include <QVariant>
#include <QModelIndex>
#include <QMimeData>
#include <QAbstractItemModel>

namespace DigikamBqmGmicQtPlugin
{

class GmicFilterManager;
class GmicFilterNode;

/**
 * GmicFilterModel is a QAbstractItemModel wrapper around the GmicfilterManager
 */
class GmicFilterModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum Roles
    {
        TypeRole      = Qt::UserRole + 1,
        CommandRole   = Qt::UserRole + 2,
        SeparatorRole = Qt::UserRole + 3,
        DateAddedRole = Qt::UserRole + 4
    };

public:

    explicit GmicFilterModel(GmicFilterManager* const mngr,
                             QObject* const parent = nullptr);
    ~GmicFilterModel()                                                        override;

    GmicFilterManager* manager()                                        const;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole)                     const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex& parent = QModelIndex())          const override;
    int rowCount(const QModelIndex& parent = QModelIndex())             const override;
    QModelIndex index(int, int, const QModelIndex& = QModelIndex())     const override;
    QModelIndex parent(const QModelIndex& index= QModelIndex())         const override;
    Qt::ItemFlags flags(const QModelIndex& index)                       const override;
    Qt::DropActions supportedDropActions ()                             const override;
    QMimeData* mimeData(const QModelIndexList& indexes)                 const override;
    QStringList mimeTypes()                                             const override;
    bool hasChildren(const QModelIndex& parent = QModelIndex())         const override;
    GmicFilterNode* node(const QModelIndex& index)                      const;
    QModelIndex index(GmicFilterNode* node)                             const;

    bool dropMimeData(const QMimeData* data, Qt::DropAction action,
                      int row, int column, const QModelIndex& parent)         override;

    bool removeRows(int row, int count,
                    const QModelIndex& parent = QModelIndex())                override;

    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole)                                     override;

public Q_SLOTS:

    void slotEntryAdded(GmicFilterNode* item);
    void slotEntryRemoved(GmicFilterNode* parent, int row, GmicFilterNode* item);
    void slotEntryChanged(GmicFilterNode* item);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin
