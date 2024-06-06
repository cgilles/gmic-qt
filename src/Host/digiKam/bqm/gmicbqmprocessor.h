/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 *  Description: digiKam Batch Queue Manager plugin for GmicQt.
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

    explicit GmicBqmProcessor(QObject* const parent);
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
