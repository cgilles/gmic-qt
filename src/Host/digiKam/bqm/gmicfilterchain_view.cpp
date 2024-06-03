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

    enableDragAndDrop(true);

    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setColumnCount(8);
    setHeaderLabels(QStringList() << i18nc("@title", "Thumbnail")
                                  << i18nc("@title", "File Name")
                                  << i18nc("@title", "User1")
                                  << i18nc("@title", "User2")
                                  << i18nc("@title", "User3")
                                  << i18nc("@title", "User4")
                                  << i18nc("@title", "User5")
                                  << i18nc("@title", "User6"));
    hideColumn(User1);
    hideColumn(User2);
    hideColumn(User3);
    hideColumn(User4);
    hideColumn(User5);
    hideColumn(User6);

    header()->setSectionResizeMode(User1, QHeaderView::Interactive);
    header()->setSectionResizeMode(User2, QHeaderView::Interactive);
    header()->setSectionResizeMode(User3, QHeaderView::Interactive);
    header()->setSectionResizeMode(User4, QHeaderView::Interactive);
    header()->setSectionResizeMode(User5, QHeaderView::Interactive);
    header()->setSectionResizeMode(User6, QHeaderView::Stretch);

    connect(this, &GmicFilterChainView::itemClicked,
            this, &GmicFilterChainView::slotItemClicked);
}

DInfoInterface* GmicFilterChainView::iface() const
{
    GmicFilterChain* const p = dynamic_cast<GmicFilterChain*>(parent());

    if (p)
    {
        return p->iface();
    }

    return nullptr;
}

void GmicFilterChainView::enableDragAndDrop(const bool enable)
{
    setDragEnabled(enable);
    viewport()->setAcceptDrops(enable);
    setDragDropMode(enable ? QAbstractItemView::InternalMove : QAbstractItemView::NoDragDrop);
    setDragDropOverwriteMode(enable);
    setDropIndicatorShown(enable);
}

void GmicFilterChainView::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        GmicFilterChain* const view = dynamic_cast<GmicFilterChain*>(parent());

        if (view)
        {
            view->updateThumbnail(item->url());
        }
    }

    QTreeWidget::drawRow(p, opt, index);
}

void GmicFilterChainView::slotItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)

    if (!item)
    {
        return;
    }

    Q_EMIT signalItemClicked(item);
}

void GmicFilterChainView::setColumnLabel(ColumnType column, const QString& label)
{
    headerItem()->setText(column, label);
}

void GmicFilterChainView::setColumnEnabled(ColumnType column, bool enable)
{
    if (enable)
    {
        showColumn(column);
    }
    else
    {
        hideColumn(column);
    }
}

void GmicFilterChainView::setColumn(ColumnType column, const QString& label, bool enable)
{
    setColumnLabel(column, label);
    setColumnEnabled(column, enable);
}

GmicFilterChainViewItem* GmicFilterChainView::findItem(const QUrl& url)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        GmicFilterChainViewItem* const lvItem = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (lvItem && (lvItem->url() == url))
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

void GmicFilterChainView::contextMenuEvent(QContextMenuEvent* e)
{
    QTreeWidget::contextMenuEvent(e);

    Q_EMIT signalContextMenuRequested();
}

void GmicFilterChainView::dragEnterEvent(QDragEnterEvent* e)
{
    m_itemDraged = QTreeWidget::currentItem();

    QTreeWidget::dragEnterEvent(e);

    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void GmicFilterChainView::dragMoveEvent(QDragMoveEvent* e)
{
    QTreeWidget::dragMoveEvent(e);

    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void GmicFilterChainView::dropEvent(QDropEvent* e)
{
    QTreeWidget::dropEvent(e);
    QList<QUrl> list = e->mimeData()->urls();
    QList<QUrl> urls;

    Q_FOREACH (const QUrl& url, list)
    {
        QFileInfo fi(url.toLocalFile());

        if (fi.isFile() && fi.exists())
        {
            urls.append(url);
        }
    }

    if (!urls.isEmpty())
    {
        Q_EMIT signalAddedDropedItems(urls);
    }

    scrollToItem(m_itemDraged);
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

} // namespace DigikamBqmGmicQtPlugin
