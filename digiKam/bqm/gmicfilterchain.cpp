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

GmicFilterChain::GmicFilterChain(QWidget* const parent)
    : QGroupBox(parent),
      d        (new Private)
{
    setTitle(tr("Chained G'MIC Filters"));
    d->listView       = new GmicFilterChainView(this);
    d->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // --------------------------------------------------------

    d->editButton     = new CtrlButton(QIcon::fromTheme(QLatin1String("document-edit")),
                                       tr("Edit the current G'MIC filter"),
                                       this, SLOT(slotEditItem()));
    d->moveUpButton   = new CtrlButton(QIcon::fromTheme(QLatin1String("go-up")),
                                       tr("Move current selected G'MIC filter up in the list"),
                                       this, SLOT(slotMoveUpItems()));
    d->moveDownButton = new CtrlButton(QIcon::fromTheme(QLatin1String("go-down")),
                                       tr("Move current selected G'MIC filter down in the list"),
                                       this, SLOT(slotMoveDownItems()));
    d->addButton      = new CtrlButton(QIcon::fromTheme(QLatin1String("list-add")),
                                       tr("Add new G'MIC filter to the list"),
                                       this, SIGNAL(signalAddItem()));
    d->removeButton   = new CtrlButton(QIcon::fromTheme(QLatin1String("list-remove")),
                                       tr("Remove selected G'MIC filters from the list"),
                                       this, SLOT(slotRemoveItems()));
    d->clearButton    = new CtrlButton(QIcon::fromTheme(QLatin1String("edit-clear")),
                                       tr("Clear the list."),
                                       this, SLOT(slotClearItems()));

    // --------------------------------------------------------

    const int spacing = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    QHBoxLayout* const hBtnLayout = new QHBoxLayout;
    hBtnLayout->addWidget(d->editButton);
    hBtnLayout->addWidget(d->moveUpButton);
    hBtnLayout->addWidget(d->moveDownButton);
    hBtnLayout->addWidget(d->addButton);
    hBtnLayout->addWidget(d->removeButton);
    hBtnLayout->addWidget(d->clearButton);
    hBtnLayout->addStretch(1);

    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->addWidget(d->listView, 1, 1, 1, 1);
    mainLayout->setRowStretch(1, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    mainLayout->addLayout(hBtnLayout, 2, 1, 1, 1);

    setLayout(mainLayout);

    // --------------------------------------------------------

    connect(d->listView, &GmicFilterChainView::signalEditItem,
            this, &GmicFilterChain::signalEditItem);

    // queue this connection because itemSelectionChanged is emitted
    // while items are deleted, and accessing selectedItems at that
    // time causes a crash ...

    connect(d->listView, &GmicFilterChainView::itemSelectionChanged,
            this, &GmicFilterChain::slotItemListChanged,
            Qt::QueuedConnection);

    connect(this, &GmicFilterChain::signalItemListChanged,
            this, &GmicFilterChain::slotItemListChanged);

    // --------------------------------------------------------

    QTimer::singleShot(1000, this, SIGNAL(signalItemListChanged()));
}

GmicFilterChain::~GmicFilterChain()
{
    delete d;
}

void GmicFilterChain::slotClearItems()
{
   if (!d->listView->topLevelItemCount())
    {
        return;
    }

    if (QMessageBox::question(this, QObject::tr("Clear List"),
                              QObject::tr("Do you want to clear the list of "
                                          "chained G'MIC filters?"),
                                     QMessageBox::Yes | QMessageBox::No
                                     ) == QMessageBox::No)
    {
        return;
    }

    d->listView->selectAll();
    slotRemoveItems();
    d->listView->clear();
}

void GmicFilterChain::slotRemoveItems()
{
    QList<QTreeWidgetItem*> selectedItemsList = d->listView->selectedItems();

    if (selectedItemsList.isEmpty())
    {
        return;
    }

    if (QMessageBox::question(this, QObject::tr("Remove Filters"),
                              QObject::tr("Do you want to remove the current "
                                          "selected G'MIC filters from the list?"),
                                     QMessageBox::Yes | QMessageBox::No
                                     ) == QMessageBox::No)
    {
        return;
    }

    QList<int> itemsIndex;

    for (QList<QTreeWidgetItem*>::const_iterator it = selectedItemsList.constBegin() ;
         it != selectedItemsList.constEnd() ; ++it)
    {
        GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (item)
        {
            itemsIndex.append(d->listView->indexFromItem(item).row());

            d->listView->removeItemWidget(*it, 0);
            delete *it;
        }
    }

    Q_EMIT signalRemovedItems(itemsIndex);
    Q_EMIT signalItemListChanged();
}

void GmicFilterChain::slotMoveUpItems()
{
    // move above item down, then we don't have to fix the focus

    QModelIndex curIndex   = d->listView->currentIndex();

    if (!curIndex.isValid())
    {
        return;
    }

    QModelIndex aboveIndex = d->listView->indexAbove(curIndex);

    if (!aboveIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* const temp  = d->listView->takeTopLevelItem(aboveIndex.row());
    d->listView->insertTopLevelItem(curIndex.row(), temp);

    Q_EMIT signalItemListChanged();
    Q_EMIT signalMoveUpItem();
}

void GmicFilterChain::slotMoveDownItems()
{
    // move below item up, then we don't have to fix the focus

    QModelIndex curIndex   = d->listView->currentIndex();

    if (!curIndex.isValid())
    {
        return;
    }

    QModelIndex belowIndex = d->listView->indexBelow(curIndex);

    if (!belowIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* const temp  = d->listView->takeTopLevelItem(belowIndex.row());
    d->listView->insertTopLevelItem(curIndex.row(), temp);

    Q_EMIT signalItemListChanged();
    Q_EMIT signalMoveDownItem();
}

QString GmicFilterChain::currentCommand() const
{
    QString command;
    const GmicFilterChainViewItem* const item = d->listView->currentFilterItem();

    if (item)
    {
        command = item->command();
    }

    return command;
}

void GmicFilterChain::setChainedFilters(const QMap<QString, QVariant>& filters)
{
    d->listView->clear();

    QStringList names = filters.keys();
    int index         = 0;

    foreach (const QVariant& cmd, filters.values())
    {
        new GmicFilterChainViewItem(d->listView, names[index], cmd.toString());
        index++;
    }
}

QMap<QString, QVariant> GmicFilterChain::chainedFilters() const
{
    QMap<QString, QVariant> map;

    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        const GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (item)
        {
            map.insert(item->title(), item->command());
        }

        ++it;
    }

    return map;
}

QStringList GmicFilterChain::chainedCommands() const
{
    QStringList list;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        const GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (item)
        {
            list.append(item->command());
        }

        ++it;
    }

    return list;
}

void GmicFilterChain::slotItemListChanged()
{
    const QList<QTreeWidgetItem*> selectedItemsList = d->listView->selectedItems();

    const bool haveItems                            = !chainedFilters().isEmpty();
    const bool haveSelectedItems                    = !selectedItemsList.isEmpty();
    const bool haveOnlyOneSelectedItem              = (selectedItemsList.count() == 1);

    d->editButton->setEnabled(haveOnlyOneSelectedItem);
    d->removeButton->setEnabled(haveSelectedItems);
    d->moveUpButton->setEnabled(haveOnlyOneSelectedItem);
    d->moveDownButton->setEnabled(haveOnlyOneSelectedItem);
    d->clearButton->setEnabled(haveItems);
    d->addButton->setEnabled(true);

    d->listView->refreshIndex();
}

void GmicFilterChain::createNewFilter(const QString& title, const QString& command)
{
    new GmicFilterChainViewItem(
                                d->listView,
                                title,
                                command
                               );

    Q_EMIT signalItemListChanged();
}

void GmicFilterChain::slotEditItem()
{
    const GmicFilterChainViewItem* const item =
        dynamic_cast<GmicFilterChainViewItem*>(d->listView->currentItem());

    if (item)
    {
        Q_EMIT signalEditItem(item->command());
    }
}

void GmicFilterChain::updateCurrentFilter(const QString& title, const QString& command)
{
    GmicFilterChainViewItem* const item =
        dynamic_cast<GmicFilterChainViewItem*>(d->listView->currentItem());

    if (item)
    {
        item->setTitle(title);
        item->setCommand(command);

        Q_EMIT signalItemListChanged();
    }
}

void GmicFilterChain::setIsLessThanHandler(GmicFilterChainIsLessThanHandler fncptr)
{
    d->isLessThan = fncptr;
}

GmicFilterChainIsLessThanHandler GmicFilterChain::isLessThanHandler() const
{
    return d->isLessThan;
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfilterchain.cpp"

#include "moc_gmicfilterchain_p.cpp"