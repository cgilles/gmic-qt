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

#pragma once

// digiKam includes

#include "ditemtooltip.h"

class QTreeWidgetItem;
class QTreeWidget;

using namespace Digikam;

namespace DigikamBqmGmicQtPlugin
{

class GmicFilterToolTip : public DItemToolTip
{
    Q_OBJECT

public:

    explicit GmicFilterToolTip(QTreeWidget* const view);
    ~GmicFilterToolTip()        override;

    void setToolTipString(const QString& tip);
    void setItem(QTreeWidgetItem* const item);

    void show();

protected:

    QRect   repositionRect()    override;
    QString tipContents()       override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin
