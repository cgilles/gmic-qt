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

class Q_DECL_HIDDEN GmicFilterChainViewItem::Private
{
public:

    Private() = default;

    bool                        hasThumb = false;       ///< True if thumbnails is a real photo thumbs

    int                         rating   = -1;          ///< Image Rating from host.
    QString                     comments;               ///< Image comments from host.
    QStringList                 tags;                   ///< List of keywords from host.
    QUrl                        url;                    ///< Image url provided by host.
    QPixmap                     thumb;                  ///< Image thumbnail.
    GmicFilterChainView*             view     = nullptr;
    State                       state    = Waiting;
};

GmicFilterChainViewItem::GmicFilterChainViewItem(GmicFilterChainView* const view, const QUrl& url)
    : QTreeWidgetItem(view),
      d              (new Private)
{
    setUrl(url);
    setRating(-1);
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable);

    d->view      = view;
    int iconSize = d->view->iconSize().width();
    setThumb(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(iconSize, iconSize, QIcon::Disabled), false);
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Creating new ImageListViewItem with url " << d->url
                                 << " for list view " << d->view;
*/
}

GmicFilterChainViewItem::~GmicFilterChainViewItem()
{
    delete d;
}

bool GmicFilterChainViewItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

void GmicFilterChainViewItem::updateInformation()
{
    if (d->view->iface())
    {
        DItemInfo info(d->view->iface()->itemInfo(d->url));

        setComments(info.comment());
        setTags(info.keywords());
        setRating(info.rating());
    }
}

void GmicFilterChainViewItem::setUrl(const QUrl& url)
{
    d->url = url;
    setText(GmicFilterChainView::Filename, d->url.fileName());
}

QUrl GmicFilterChainViewItem::url() const
{
    return d->url;
}

void GmicFilterChainViewItem::setComments(const QString& comments)
{
    d->comments = comments;
}

QString GmicFilterChainViewItem::comments() const
{
    return d->comments;
}

void GmicFilterChainViewItem::setTags(const QStringList& tags)
{
    d->tags = tags;
}

QStringList GmicFilterChainViewItem::tags() const
{
    return d->tags;
}

void GmicFilterChainViewItem::setRating(int rating)
{
    d->rating = rating;
}

int GmicFilterChainViewItem::rating() const
{
    return d->rating;
}

void GmicFilterChainViewItem::setPixmap(const QPixmap& pix)
{
    QIcon icon = QIcon(pix);

    // We make sure the preview icon stays the same regardless of the role.

    icon.addPixmap(pix, QIcon::Selected, QIcon::On);
    icon.addPixmap(pix, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pix, QIcon::Active,   QIcon::On);
    icon.addPixmap(pix, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::Off);
    setIcon(GmicFilterChainView::Thumbnail, icon);
}

void GmicFilterChainViewItem::setThumb(const QPixmap& pix, bool hasThumb)
{
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Received new thumbnail for url " << d->url
                                 << ". My view is " << d->view;
*/
    if (!d->view)
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "This item do not have a tree view. "
                                        << "This should never happen!";
        return;
    }

    int iconSize = qMax<int>(d->view->iconSize().width(), d->view->iconSize().height());
    QPixmap pixmap(iconSize + 2, iconSize + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()  / 2) - (pix.width()  / 2),
                 (pixmap.height() / 2) - (pix.height() / 2), pix);
    d->thumb     = pixmap;
    setPixmap(d->thumb);

    d->hasThumb  = hasThumb;
}

void GmicFilterChainViewItem::setProgressAnimation(const QPixmap& pix)
{
    QPixmap overlay = d->thumb;
    QPixmap mask(overlay.size());
    mask.fill(QColor(128, 128, 128, 192));
    QPainter p(&overlay);
    p.drawPixmap(0, 0, mask);
    p.drawPixmap((overlay.width()  / 2) - (pix.width()  / 2),
                 (overlay.height() / 2) - (pix.height() / 2), pix);
    setPixmap(overlay);
}

void GmicFilterChainViewItem::setProcessedIcon(const QIcon& icon)
{
    setIcon(GmicFilterChainView::Filename, icon);

    // reset thumbnail back to no animation pix.

    setPixmap(d->thumb);
}

void GmicFilterChainViewItem::setState(State state)
{
    d->state = state;
}

GmicFilterChainViewItem::State GmicFilterChainViewItem::state() const
{
    return d->state;
}

GmicFilterChainView* GmicFilterChainViewItem::view() const
{
    return d->view;
}

bool GmicFilterChainViewItem::operator<(const QTreeWidgetItem& other) const
{
    if (d->view->isLessThanHandler())
    {
        return d->view->isLessThanHandler()(this, other);
    }

    return QTreeWidgetItem::operator<(other);
}

} // namespace DigikamBqmGmicQtPlugin
