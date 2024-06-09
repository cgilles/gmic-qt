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

#include "gmicfilterlistview.h"

// Qt includes

#include <QPointer>
#include <QTimer>
#include <QMenu>
#include <QAction>

// Local includes

#include "gmicfilterlistviewitem.h"
#include "gmicfiltertooltip.h"

namespace DigikamBqmGmicQtPlugin
{

 class Q_DECL_HIDDEN BlackFrameListView::Private
{
public:

    Private() = default;

    QTimer*            toolTipTimer = nullptr;
    BlackFrameToolTip* toolTip      = nullptr;
    QTreeWidgetItem*   toolTipItem  = nullptr;
};

BlackFrameListView::BlackFrameListView(QWidget* const parent)
    : QTreeWidget(parent),
      d          (new Private)
{
    setColumnCount(3);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setIconSize(QSize(GmicFilterListViewItem::THUMB_WIDTH, GmicFilterListViewItem::THUMB_WIDTH));
    viewport()->setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    QStringList labels;
    labels.append(i18n("Preview"));
    labels.append(i18n("Size"));
    labels.append(i18nc("This is a column which will contain the amount of HotPixels "
                        "found in the black frame file", "Hot Pixels"));
    setHeaderLabels(labels);

    d->toolTip      = new BlackFrameToolTip(this);
    d->toolTipTimer = new QTimer(this);

    connect(this, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(d->toolTipTimer, SIGNAL(timeout()),
            this, SLOT(slotToolTip()));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu()));
}

BlackFrameListView::~BlackFrameListView()
{
    delete d->toolTip;
    delete d;
}

bool BlackFrameListView::contains(const QUrl& url)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        GmicFilterListViewItem* const item = dynamic_cast<GmicFilterListViewItem*>(*it);

        if (item && (item->frameUrl() == url))
        {
            return true;
        }

        ++it;
    }

    return false;
}

bool BlackFrameListView::isSelected(const QUrl& url)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        GmicFilterListViewItem* const item = dynamic_cast<GmicFilterListViewItem*>(*it);

        if (item && item->isSelected() && (item->frameUrl() == url))
        {
            return true;
        }

        ++it;
    }

    return false;
}

QUrl BlackFrameListView::currentUrl()
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        GmicFilterListViewItem* const item = dynamic_cast<GmicFilterListViewItem*>(*it);

        if (item && item->isSelected())
        {
            return item->frameUrl();
        }

        ++it;
    }

    return QUrl();
}

void BlackFrameListView::slotSelectionChanged()
{
    GmicFilterListViewItem* const item = dynamic_cast<GmicFilterListViewItem*>(currentItem());

    if (item)
    {
        item->emitHotPixelsParsed();
    }
}

void BlackFrameListView::slotHotPixelsParsed(const QList<HotPixelProps>& hotPixels,
                                             const QUrl& url)
{
    if (isSelected(url))
    {
        Q_EMIT signalBlackFrameSelected(hotPixels, url);
    }
}

void BlackFrameListView::hideToolTip()
{
    d->toolTipItem = nullptr;
    d->toolTipTimer->stop();
    slotToolTip();
}

bool BlackFrameListView::acceptToolTip(const QPoint& pos) const
{
    if (columnAt(pos.x()) == 0)
    {
        return true;
    }

    return false;
}

void BlackFrameListView::slotToolTip()
{
    d->toolTip->setItem(d->toolTipItem);
}

void BlackFrameListView::mouseMoveEvent(QMouseEvent* e)
{
    if (e->buttons() == Qt::NoButton)
    {
        GmicFilterListViewItem* const item = dynamic_cast<GmicFilterListViewItem*>(itemAt(e->pos()));

        if (!isActiveWindow() || !item)
        {
            hideToolTip();
            return;
        }

        if (item != d->toolTipItem)
        {
            hideToolTip();

            if (acceptToolTip(e->pos()))
            {
                d->toolTipItem = item;
                d->toolTip->setToolTipString(item->toolTipString());
                d->toolTipTimer->setSingleShot(true);
                d->toolTipTimer->start(500);
            }
        }

        if ((item == d->toolTipItem) && !acceptToolTip(e->pos()))
        {
            hideToolTip();
        }

        return;
    }

    hideToolTip();
    QTreeWidget::mouseMoveEvent(e);
}

void BlackFrameListView::wheelEvent(QWheelEvent* e)
{
    hideToolTip();
    QTreeWidget::wheelEvent(e);
}

void BlackFrameListView::keyPressEvent(QKeyEvent* e)
{
    hideToolTip();
    QTreeWidget::keyPressEvent(e);
}

void BlackFrameListView::focusOutEvent(QFocusEvent* e)
{
    hideToolTip();
    QTreeWidget::focusOutEvent(e);
}

void BlackFrameListView::leaveEvent(QEvent* e)
{
    hideToolTip();
    QTreeWidget::leaveEvent(e);
}

void BlackFrameListView::slotContextMenu()
{
    if (!viewport()->isEnabled())
    {
        return;
    }

    QMenu popmenu(this);

    QAction* const removeAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove Black Frame"), this);
    QAction* const clearAction  = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")),  i18n("Clear List"),         this);

    popmenu.addAction(removeAction);
    popmenu.addSeparator();
    popmenu.addAction(clearAction);

    QAction* const choice = popmenu.exec(QCursor::pos());

    if      (choice == removeAction)
    {
        QList<QTreeWidgetItem*> list = selectedItems();

        if (!list.isEmpty())
        {
            GmicFilterListViewItem* const item = dynamic_cast<GmicFilterListViewItem*>(list.first());

            if (item)
            {
                QUrl url = item->frameUrl();
                delete item;

                Q_EMIT signalBlackFrameRemoved(url);
            }
        }
    }
    else if (choice == clearAction)
    {
        clear();

        Q_EMIT signalClearBlackFrameList();
    }
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfilterlistview.cpp"
