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

class Q_DECL_HIDDEN GmicFilterChain::Private
{
public:

    Private() = default;

    bool                        allowRAW                = true;
    bool                        allowDuplicate          = false;
    bool                        controlButtonsEnabled   = true;
    int                         iconSize                = 48;

    CtrlButton*                 addButton               = nullptr;
    CtrlButton*                 removeButton            = nullptr;
    CtrlButton*                 moveUpButton            = nullptr;
    CtrlButton*                 moveDownButton          = nullptr;
    CtrlButton*                 clearButton             = nullptr;
    CtrlButton*                 loadButton              = nullptr;
    CtrlButton*                 saveButton              = nullptr;
    QWidget*                    extraWidget             = nullptr;   ///< Extra widget append to the end of control buttons layout.

    QList<QUrl>                 processItems;
    DWorkingPixmap*             progressPix             = nullptr;
    int                         progressCount           = 0;
    QTimer*                     progressTimer           = nullptr;

    GmicFilterChainView*             listView                = nullptr;
    ThumbnailLoadThread*        thumbLoadThread         = ThumbnailLoadThread::defaultThread();

    GmicFilterChainIsLessThanHandler isLessThan;
};

GmicFilterChain::GmicFilterChain(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->progressPix    = new DWorkingPixmap(this);
    d->listView       = new GmicFilterChainView(this);
    d->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // --------------------------------------------------------

    d->addButton      = new CtrlButton(QIcon::fromTheme(QLatin1String("list-add")).pixmap(16, 16),      this);
    d->removeButton   = new CtrlButton(QIcon::fromTheme(QLatin1String("list-remove")).pixmap(16, 16),   this);
    d->moveUpButton   = new CtrlButton(QIcon::fromTheme(QLatin1String("go-up")).pixmap(16, 16),         this);
    d->moveDownButton = new CtrlButton(QIcon::fromTheme(QLatin1String("go-down")).pixmap(16, 16),       this);
    d->clearButton    = new CtrlButton(QIcon::fromTheme(QLatin1String("edit-clear")).pixmap(16, 16),    this);
    d->loadButton     = new CtrlButton(QIcon::fromTheme(QLatin1String("document-open")).pixmap(16, 16), this);
    d->saveButton     = new CtrlButton(QIcon::fromTheme(QLatin1String("document-save")).pixmap(16, 16), this);

    d->addButton->setToolTip(i18nc("@info", "Add new images to the list"));
    d->removeButton->setToolTip(i18nc("@info", "Remove selected images from the list"));
    d->moveUpButton->setToolTip(i18nc("@info", "Move current selected image up in the list"));
    d->moveDownButton->setToolTip(i18nc("@info", "Move current selected image down in the list"));
    d->clearButton->setToolTip(i18nc("@info", "Clear the list."));
    d->loadButton->setToolTip(i18nc("@info", "Load a saved list."));
    d->saveButton->setToolTip(i18nc("@info", "Save the list."));

    d->progressTimer  = new QTimer(this);

    // --------------------------------------------------------

    setIconSize(d->iconSize);
    setControlButtons(Add | Remove | MoveUp | MoveDown | Clear | Save | Load ); // add all buttons      (default)
    setControlButtonsPlacement(ControlButtonsBelow);                            // buttons on the bottom (default)
    enableDragAndDrop(true);                                                    // enable drag and drop (default)

    // --------------------------------------------------------

    connect(d->listView, &GmicFilterChainView::signalAddedDropedItems,
            this, &GmicFilterChain::slotAddImages);

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));

    connect(d->listView, &GmicFilterChainView::signalItemClicked,
            this, &GmicFilterChain::signalItemClicked);

    connect(d->listView, &GmicFilterChainView::signalContextMenuRequested,
            this, &GmicFilterChain::signalContextMenuRequested);

    // queue this connection because itemSelectionChanged is emitted
    // while items are deleted, and accessing selectedItems at that
    // time causes a crash ...

    connect(d->listView, &GmicFilterChainView::itemSelectionChanged,
            this, &GmicFilterChain::slotImageListChanged, Qt::QueuedConnection);

    connect(this, &GmicFilterChain::signalImageListChanged,
            this, &GmicFilterChain::slotImageListChanged);

    // --------------------------------------------------------

    connect(d->addButton, &CtrlButton::clicked,
            this, &GmicFilterChain::slotAddItems);

    connect(d->removeButton, &CtrlButton::clicked,
            this, &GmicFilterChain::slotRemoveItems);

    connect(d->moveUpButton, &CtrlButton::clicked,
            this, &GmicFilterChain::slotMoveUpItems);

    connect(d->moveDownButton, &CtrlButton::clicked,
            this, &GmicFilterChain::slotMoveDownItems);

    connect(d->clearButton, &CtrlButton::clicked,
            this, &GmicFilterChain::slotClearItems);

    connect(d->loadButton, &CtrlButton::clicked,
            this, &GmicFilterChain::slotLoadItems);

    connect(d->saveButton, &CtrlButton::clicked,
            this, &GmicFilterChain::slotSaveItems);

    connect(d->progressTimer, &QTimer::timeout,
            this, &GmicFilterChain::slotProgressTimerDone);

    // --------------------------------------------------------

    QTimer::singleShot(1000, this, SIGNAL(signalImageListChanged()));
}

GmicFilterChain::~GmicFilterChain()
{
    delete d;
}

void GmicFilterChain::enableControlButtons(bool enable)
{
    d->controlButtonsEnabled = enable;
    slotImageListChanged();
}

void GmicFilterChain::enableDragAndDrop(const bool enable)
{
    d->listView->enableDragAndDrop(enable);
}

void GmicFilterChain::appendControlButtonsWidget(QWidget* const widget)
{
    d->extraWidget = widget;
}

QBoxLayout* GmicFilterChain::setControlButtonsPlacement(ControlButtonPlacement placement)
{
    delete layout();

    QBoxLayout* lay               = nullptr;        // Layout instance to return;
    const int spacing             = layoutSpacing();


    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->addWidget(d->listView, 1, 1, 1, 1);
    mainLayout->setRowStretch(1, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);

    // --------------------------------------------------------

    QHBoxLayout* const hBtnLayout = new QHBoxLayout;
    hBtnLayout->addWidget(d->moveUpButton);
    hBtnLayout->addWidget(d->moveDownButton);
    hBtnLayout->addWidget(d->addButton);
    hBtnLayout->addWidget(d->removeButton);
    hBtnLayout->addWidget(d->loadButton);
    hBtnLayout->addWidget(d->saveButton);
    hBtnLayout->addWidget(d->clearButton);
    hBtnLayout->addStretch(1);

    if (d->extraWidget)
    {
        hBtnLayout->addWidget(d->extraWidget);
    }

    // --------------------------------------------------------

    QVBoxLayout* const vBtnLayout = new QVBoxLayout;
    vBtnLayout->addWidget(d->moveUpButton);
    vBtnLayout->addWidget(d->moveDownButton);
    vBtnLayout->addWidget(d->addButton);
    vBtnLayout->addWidget(d->removeButton);
    vBtnLayout->addWidget(d->loadButton);
    vBtnLayout->addWidget(d->saveButton);
    vBtnLayout->addWidget(d->clearButton);
    vBtnLayout->addStretch(1);

    if (d->extraWidget)
    {
        vBtnLayout->addWidget(d->extraWidget);
    }

    // --------------------------------------------------------

    switch (placement)
    {
        case ControlButtonsAbove:
        {
            lay = hBtnLayout;
            mainLayout->addLayout(hBtnLayout, 0, 1, 1, 1);
            delete vBtnLayout;
            break;
        }

        case ControlButtonsBelow:
        {
            lay = hBtnLayout;
            mainLayout->addLayout(hBtnLayout, 2, 1, 1, 1);
            delete vBtnLayout;
            break;
        }

        case ControlButtonsLeft:
        {
            lay = vBtnLayout;
            mainLayout->addLayout(vBtnLayout, 1, 0, 1, 1);
            delete hBtnLayout;
            break;
        }

        case ControlButtonsRight:
        {
            lay = vBtnLayout;
            mainLayout->addLayout(vBtnLayout, 1, 2, 1, 1);
            delete hBtnLayout;
            break;
        }

        case NoControlButtons:
        default:
        {
            delete vBtnLayout;
            delete hBtnLayout;

            // set all buttons invisible

            setControlButtons(ControlButtons());

            if (d->extraWidget)
            {
                d->extraWidget->setVisible(false);
            }

            break;
        }
    }

    setLayout(mainLayout);

    return lay;
}

void GmicFilterChain::setControlButtons(ControlButtons buttonMask)
{
    d->addButton->setVisible(buttonMask & Add);
    d->removeButton->setVisible(buttonMask & Remove);
    d->moveUpButton->setVisible(buttonMask & MoveUp);
    d->moveDownButton->setVisible(buttonMask & MoveDown);
    d->clearButton->setVisible(buttonMask & Clear);
    d->loadButton->setVisible(buttonMask & Load);
    d->saveButton->setVisible(buttonMask & Save);
}

void GmicFilterChain::setAllowDuplicate(bool allow)
{
  d->allowDuplicate = allow;
}

void GmicFilterChain::setAllowRAW(bool allow)
{
    d->allowRAW = allow;
}

void GmicFilterChain::setIconSize(int size)
{
    if      (size < 16)
    {
        d->iconSize = 16;
    }
    else if (size > 128)
    {
        d->iconSize = 128;
    }
    else
    {
        d->iconSize = size;
    }

    d->listView->setIconSize(QSize(iconSize(), iconSize()));
}

int GmicFilterChain::iconSize() const
{
    return d->iconSize;
}

void GmicFilterChain::loadImagesFromCurrentSelection()
{
    bool selection = checkSelection();

    if (selection && d->iface)
    {
        QList<QUrl> images = d->iface->currentSelectedItems();

        if (!images.isEmpty())
        {
            slotAddImages(images);
        }
    }
    else
    {
        loadImagesFromCurrentAlbum();
    }
}

void GmicFilterChain::loadImagesFromCurrentAlbum()
{
    if (!d->iface)
    {
        return;
    }

    QList<QUrl> images = d->iface->currentAlbumItems();

    if (!images.isEmpty())
    {
        slotAddImages(images);
    }
}

bool GmicFilterChain::checkSelection()
{
    if (!d->iface)
    {
        return false;
    }

    QList<QUrl> images = d->iface->currentSelectedItems();

    return (!images.isEmpty());
}

void GmicFilterChain::slotAddImages(const QList<QUrl>& list)
{
    if (list.count() == 0)
    {
        return;
    }

    QList<QUrl> urls;
    bool raw = false;

    for (QList<QUrl>::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        QUrl imageUrl = *it;

        // Check if the new item already exist in the list.

        bool found    = false;

        QTreeWidgetItemIterator iter(d->listView);

        while (*iter)
        {
            GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(*iter);

            if (item && (item->url() == imageUrl))
            {
                found = true;
            }

            ++iter;
        }

        if (d->allowDuplicate || !found)
        {
            // if RAW files are not allowed, skip the image

            if (!d->allowRAW && DRawDecoder::isRawFile(imageUrl))
            {
                raw = true;
                continue;
            }

            new GmicFilterChainViewItem(listView(), imageUrl);
            urls.append(imageUrl);
        }
    }

    Q_EMIT signalAddItems(urls);
    Q_EMIT signalImageListChanged();
    Q_EMIT signalFoundRAWImages(raw);
}

void GmicFilterChain::slotAddItems()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(objectName());
    QUrl lastFileUrl        = QUrl::fromLocalFile(grp.readEntry("Last Image Path",
                                                  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)));

    QList<QUrl> urls        = ImageDialog::getImageURLs(this, lastFileUrl);

    if (!urls.isEmpty())
    {
        slotAddImages(urls);
        grp.writeEntry("Last Image Path", urls.first().adjusted(QUrl::RemoveFilename).toLocalFile());
        config->sync();
    }
}

void GmicFilterChain::slotRemoveItems()
{
    QList<QTreeWidgetItem*> selectedItemsList = d->listView->selectedItems();
    QList<int> itemsIndex;

    for (QList<QTreeWidgetItem*>::const_iterator it = selectedItemsList.constBegin() ;
         it != selectedItemsList.constEnd() ; ++it)
    {
        GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (item)
        {
            itemsIndex.append(d->listView->indexFromItem(item).row());

            if (d->processItems.contains(item->url()))
            {
                d->processItems.removeAll(item->url());
            }

            d->listView->removeItemWidget(*it, 0);
            delete *it;
        }
    }

    Q_EMIT signalRemovedItems(itemsIndex);
    Q_EMIT signalImageListChanged();
}

void GmicFilterChain::slotMoveUpItems()
{
    // move above item down, then we don't have to fix the focus

    QModelIndex curIndex   = listView()->currentIndex();

    if (!curIndex.isValid())
    {
        return;
    }

    QModelIndex aboveIndex = listView()->indexAbove(curIndex);

    if (!aboveIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* const temp  = listView()->takeTopLevelItem(aboveIndex.row());
    listView()->insertTopLevelItem(curIndex.row(), temp);

    // this is a quick fix. We loose the extra tags in flickr upload, but at list we don't get a crash

    GmicFilterChainViewItem* const uw = dynamic_cast<GmicFilterChainViewItem*>(temp);

    if (uw)
    {
        uw->updateItemWidgets();
    }

    Q_EMIT signalImageListChanged();
    Q_EMIT signalMoveUpItem();
}

void GmicFilterChain::slotMoveDownItems()
{
    // move below item up, then we don't have to fix the focus

    QModelIndex curIndex   = listView()->currentIndex();

    if (!curIndex.isValid())
    {
        return;
    }

    QModelIndex belowIndex = listView()->indexBelow(curIndex);

    if (!belowIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* const temp  = listView()->takeTopLevelItem(belowIndex.row());
    listView()->insertTopLevelItem(curIndex.row(), temp);

    // This is a quick fix. We can loose extra tags in uploader, but at least we don't get a crash

    GmicFilterChainViewItem* const uw = dynamic_cast<GmicFilterChainViewItem*>(temp);

    if (uw)
    {
        uw->updateItemWidgets();
    }

    Q_EMIT signalImageListChanged();
    Q_EMIT signalMoveDownItem();
}

void GmicFilterChain::slotClearItems()
{
    listView()->selectAll();
    slotRemoveItems();
    listView()->clear();
}

void GmicFilterChain::removeItemByUrl(const QUrl& url)
{
    bool found;
    QList<int> itemsIndex;

    do
    {
        found = false;
        QTreeWidgetItemIterator it(d->listView);

        while (*it)
        {
            GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(*it);

            if (item && (item->url() == url))
            {
                itemsIndex.append(d->listView->indexFromItem(item).row());

                if (d->processItems.contains(item->url()))
                {
                    d->processItems.removeAll(item->url());
                }

                delete item;
                found = true;
                break;
            }

            ++it;
        }
    }
    while (found);

    Q_EMIT signalRemovedItems(itemsIndex);
    Q_EMIT signalImageListChanged();
}

QList<QUrl> GmicFilterChain::imageUrls(bool onlyUnprocessed) const
{
    QList<QUrl> list;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (item)
        {
            if ((onlyUnprocessed == false) || (item->state() != GmicFilterChainViewItem::Success))
            {
                list.append(item->url());
            }
        }

        ++it;
    }

    return list;
}

void GmicFilterChain::slotProgressTimerDone()
{
    if (!d->processItems.isEmpty())
    {
        Q_FOREACH (const QUrl& url, d->processItems)
        {
            GmicFilterChainViewItem* const item = listView()->findItem(url);

            if (item)
            {
                item->setProgressAnimation(d->progressPix->frameAt(d->progressCount));
            }
        }

        d->progressCount++;

        if (d->progressCount == 8)
        {
            d->progressCount = 0;
        }

        d->progressTimer->start(300);
    }
}

void GmicFilterChain::processing(const QUrl& url)
{
    GmicFilterChainViewItem* const item = listView()->findItem(url);

    if (item)
    {
        d->processItems.append(url);
        d->listView->setCurrentItem(item, true);
        d->listView->scrollToItem(item);
        d->progressTimer->start(300);
    }
}

void GmicFilterChain::processed(const QUrl& url, bool success)
{
    GmicFilterChainViewItem* const item = listView()->findItem(url);

    if (item)
    {
        d->processItems.removeAll(url);
        item->setProcessedIcon(QIcon::fromTheme(success ? QLatin1String("dialog-ok-apply")
                                                        : QLatin1String("dialog-cancel")).pixmap(16, 16));
        item->setState(success ? GmicFilterChainViewItem::Success
                               : GmicFilterChainViewItem::Failed);

        if (d->processItems.isEmpty())
        {
            d->progressTimer->stop();
        }
    }
}

void GmicFilterChain::cancelProcess()
{
    Q_FOREACH (const QUrl& url, d->processItems)
    {
        processed(url, false);
    }
}

void GmicFilterChain::clearProcessedStatus()
{
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        GmicFilterChainViewItem* const lvItem = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (lvItem)
        {
            lvItem->setProcessedIcon(QIcon());
        }

        ++it;
    }
}

GmicFilterChainView* GmicFilterChain::listView() const
{
    return d->listView;
}

void GmicFilterChain::slotImageListChanged()
{
    const QList<QTreeWidgetItem*> selectedItemsList = d->listView->selectedItems();
    const bool haveImages                           = !(imageUrls().isEmpty())         && d->controlButtonsEnabled;
    const bool haveSelectedImages                   = !(selectedItemsList.isEmpty())   && d->controlButtonsEnabled;
    const bool haveOnlyOneSelectedImage             = (selectedItemsList.count() == 1) && d->controlButtonsEnabled;

    d->removeButton->setEnabled(haveSelectedImages);
    d->moveUpButton->setEnabled(haveOnlyOneSelectedImage);
    d->moveDownButton->setEnabled(haveOnlyOneSelectedImage);
    d->clearButton->setEnabled(haveImages);

    // All buttons are enabled / disabled now, but the "Add" button should always be
    // enabled, if the buttons are not explicitly disabled with enableControlButtons()

    d->addButton->setEnabled(d->controlButtonsEnabled);

    // TODO: should they be enabled by default now?

    d->loadButton->setEnabled(d->controlButtonsEnabled);
    d->saveButton->setEnabled(d->controlButtonsEnabled);
}

GmicFilterChainViewItem* GmicFilterChainView::getCurrentItem() const
{
    QTreeWidgetItem* const currentTreeItem = currentItem();

    if (!currentTreeItem)
    {
        return nullptr;
    }

    return dynamic_cast<GmicFilterChainViewItem*>(currentTreeItem);
}

QUrl GmicFilterChain::getCurrentUrl() const
{
    GmicFilterChainViewItem* const currentItem = d->listView->getCurrentItem();

    if (!currentItem)
    {
        return QUrl();
    }

    return currentItem->url();
}

void GmicFilterChain::setCurrentUrl(const QUrl& url)
{
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        GmicFilterChainViewItem* const item = dynamic_cast<GmicFilterChainViewItem*>(*it);

        if (item && (item->url() == url))
        {
            d->listView->setCurrentItem(item);
            return;
        }

        ++it;
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
