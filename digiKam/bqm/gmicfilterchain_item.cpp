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

GmicFilterChainViewItem::GmicFilterChainViewItem(GmicFilterChainView* const view,
                                                 const QString& title,
                                                 const QString& command)
    : QTreeWidgetItem(view),
      d              (new Private)
{
    setTitle(title);
    setCommand(command);
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    d->view = view;
}

GmicFilterChainViewItem::~GmicFilterChainViewItem()
{
    delete d;
}

void GmicFilterChainViewItem::setCommand(const QString& command)
{
    d->command = command;
    setText(GmicFilterChainView::Command, d->command);
}

QString GmicFilterChainViewItem::command() const
{
    return d->command;
}

void GmicFilterChainViewItem::setTitle(const QString& title)
{
    d->title = title;
    setText(GmicFilterChainView::Title, d->title);
}

QString GmicFilterChainViewItem::title() const
{
    return d->title;
}

void GmicFilterChainViewItem::setIndex(int index)
{
    d->index = index;
    setText(GmicFilterChainView::Index, QString::fromLatin1("%1").arg(d->index + 1));
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