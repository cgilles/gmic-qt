/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 *  Description: digiKam Batch Queue Manager plugin for Gmic-Qt.
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

GmicFilterChainIsLessThanHandler GmicFilterChainView::isLessThanHandler() const
{
    GmicFilterChain* const p = dynamic_cast<GmicFilterChain*>(parent());

    if (p)
    {
        return p->isLessThanHandler();
    }

    return nullptr;
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