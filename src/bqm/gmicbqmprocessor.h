/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam Batch Queue Manager plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QString>

// digiKam includes

#include "dimg.h"

using namespace Digikam;

namespace DigikamBqmGmicQtPlugin
{

class GmicBqmProcessor : public QObject
{
    Q_OBJECT

public:

    explicit GmicBqmProcessor(QObject* const parent = nullptr);
    ~GmicBqmProcessor()                   override;

    QString processingCommand()     const;
    QString filterName()            const;
    bool processingComplete()       const;
    DImg outputImage()              const;

    void setInputImage(const DImg& inImage);
    bool setProcessingCommand(const QString& command);
    void startProcessing();
    void cancel();

Q_SIGNALS:

    void signalDone(const QString& errorMessage);
    void signalProgress(float progress);

private Q_SLOTS:

    void slotSendProgressInformation();
    void slotProcessingFinished();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin
