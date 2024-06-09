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

#include "gmicfiltertooltip.h"

// Qt includes

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QString>
#include <QRect>

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicFilterToolTip::Private
{
public:

    Private() = default;

    QString          tip;

    QTreeWidget*     view = nullptr;
    QTreeWidgetItem* item = nullptr;
};

GmicFilterToolTip::GmicFilterToolTip(QTreeWidget* const view)
    : DItemToolTip(),
      d           (new Private)
{
    d->view = view;
}

GmicFilterToolTip::~GmicFilterToolTip()
{
    delete d;
}

void GmicFilterToolTip::setToolTipString(const QString& tip)
{
    d->tip = tip;
}

void GmicFilterToolTip::setItem(QTreeWidgetItem* const item)
{
    d->item = item;

    if (!d->item)
    {
        hide();
    }
    else
    {
        show();
    }
}

void GmicFilterToolTip::show()
{
    updateToolTip();
    reposition();

    if (isHidden() && !toolTipIsEmpty())
    {
        DItemToolTip::show();
    }
}

QRect GmicFilterToolTip::repositionRect()
{
    if (!d->item)
    {
        return QRect();
    }

    QRect rect = d->view->visualItemRect(d->item);
    rect.moveTopLeft(d->view->viewport()->mapToGlobal(rect.topLeft()));

    return rect;
}

QString GmicFilterToolTip::tipContents()
{
    return d->tip;
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfiltertooltip.cpp"
