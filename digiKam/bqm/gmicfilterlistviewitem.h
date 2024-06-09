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

// Qt includes

#include <QList>
#include <QString>
#include <QTreeWidget>

namespace DigikamBqmGmicQtPlugin
{

class GmicFilterListViewItem : public QObject,
                               public QTreeWidgetItem
{
    Q_OBJECT

public:

    enum BlackFrameConst
    {
        // Columns
        PREVIEW     = 0,
        SIZE        = 1,
        HOTPIXELS   = 2,

        // Thumbnail properties
        THUMB_WIDTH = 150
    };

public:

    explicit GmicFilterListViewItem(QTreeWidget* const parent, const QUrl& url);
    ~GmicFilterListViewItem()     override;

    QUrl    frameUrl()      const;
    QString toolTipString() const;

    void emitHotPixelsParsed();

Q_SIGNALS:

    void signalHotPixelsParsed(const QList<HotPixelProps>&, const QUrl&);

private Q_SLOTS:

    void slotHotPixelsParsed(const QList<HotPixelProps>&);
    void slotLoadingProgress(float);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin
