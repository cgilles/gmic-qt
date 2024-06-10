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

#include "gmicfilterchain_p.h"

namespace DigikamBqmGmicQtPlugin
{

GmicFilterChainView::GmicFilterChainView(GmicFilterChain* const parent)
    : QTreeWidget(parent)
{
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setUniformRowHeights(true);
    setAlternatingRowColors(true);
    setExpandsOnDoubleClick(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setColumnCount(NumberOfColumns);
    setHeaderLabels(QStringList() << tr("Id")
                                  << tr("Name")
                                  << tr("G'MIC Command"));

    header()->setSectionResizeMode(Index,   QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(Title,   QHeaderView::Interactive);
    header()->setSectionResizeMode(Command, QHeaderView::Stretch);

    connect(this, &GmicFilterChainView::itemDoubleClicked,
            this, &GmicFilterChainView::slotItemDoubleClicked);
}

void GmicFilterChainView::slotItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)

    const GmicFilterChainViewItem* const fitem = dynamic_cast<GmicFilterChainViewItem*>(item);

    if (!fitem)
    {
        return;
    }

    Q_EMIT signalEditItem(fitem->command());
}

GmicFilterChainViewItem* GmicFilterChainView::findItem(const QString& title)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        GmicFilterChainViewItem* const lvItem = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (lvItem && (lvItem->title() == title))
        {
            return lvItem;
        }

        ++it;
    }

    return nullptr;
}

QModelIndex GmicFilterChainView::indexFromItem(GmicFilterChainViewItem* item, int column) const
{
    return QTreeWidget::indexFromItem(item, column);
}

GmicFilterChainViewItem* GmicFilterChainView::currentFilterItem() const
{
    QTreeWidgetItem* const currentTreeItem = currentItem();

    if (!currentTreeItem)
    {
        return nullptr;
    }

    return dynamic_cast<GmicFilterChainViewItem*>(currentTreeItem);
}

void GmicFilterChainView::refreshIndex()
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (item)
        {
            item->setIndex(indexOfTopLevelItem(item));
        }

        ++it;
    }
}

} // namespace DigikamBqmGmicQtPlugin
