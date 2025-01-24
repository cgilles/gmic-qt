/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam Batch Queue Manager plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gmicfiltermngr_p.h"

namespace DigikamBqmGmicQtPlugin
{

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

} // namespace DigikamBqmGmicQtPlugin
