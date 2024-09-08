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

#include "gmicbqmprocessor.h"

// digiKam includes

#include "digikam_debug.h"

// Local includes

#include "Common.h"
#include "FilterThread.h"
#include "GmicStdlib.h"
#include "Misc.h"
#include "Updater.h"
#include "GmicQt.h"
#include "gmicqtimageconverter.h"

using namespace DigikamGmicQtPluginCommon;
using namespace GmicQt;

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicBqmProcessor::Private
{
public:

    Private() = default;

    FilterThread*                   filterThread = nullptr;
    gmic_library::gmic_list<float>* gmicImages   = new gmic_library::gmic_list<gmic_pixel_type>;

    QTimer                          timer;
    QString                         filterName;

    QString                         command;
    bool                            completed    = false;

    DImg                            inImage;
    DImg                            outImage;
};

GmicBqmProcessor::GmicBqmProcessor(QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    GmicStdLib::Array = Updater::getInstance()->buildFullStdlib();
}

GmicBqmProcessor::~GmicBqmProcessor()
{
    delete d->gmicImages;
    delete d;
}

void GmicBqmProcessor::setInputImage(const DImg& inImage)
{
    d->inImage = inImage;
}

bool GmicBqmProcessor::setProcessingCommand(const QString& command)
{
    if (command.isEmpty())
    {
        qCWarning(DIGIKAM_DPLUGIN_BQM_LOG) << "The G'MIC command is empty.";

        return false;
    }
    else
    {
        d->command    = command;
        d->filterName = QString::fromLatin1("Custom command (%1)").arg(elided(d->command, 35));
    }

    return true;
}

void GmicBqmProcessor::startProcessing()
{
    gmic_list<char> imageNames;

    d->gmicImages->assign(1);
    imageNames.assign(1);

    QString name  = QString::fromUtf8("pos(0,0),name(%1)").arg(QLatin1String("Batch Queue Manager"));
    QByteArray ba = name.toUtf8();
    gmic_image<char>::string(ba.constData()).move_to(imageNames[0]);

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Processing image size"
                                     << d->inImage.size();

    GMicQtImageConverter::convertDImgtoCImg(
                                            d->inImage.copy(
                                                            0, 0,
                                                            d->inImage.width(),
                                                            d->inImage.height()
                                                           ),
                                            *d->gmicImages[0]
                                           );

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << QString::fromUtf8("G'MIC: %1").arg(d->command);

    QString env = QString::fromLatin1("_input_layers=%1").arg((int)DefaultInputMode);
    env        += QString::fromLatin1(" _output_mode=%1").arg((int)DefaultOutputMode);
    env        += QString::fromLatin1(" _output_messages=%1").arg((int)OutputMessageMode::VerboseConsole);

    d->filterThread = new FilterThread(this,
                                       QLatin1String("skip 0"),
                                       d->command,
                                       env);

    d->filterThread->swapImages(*d->gmicImages);
    d->filterThread->setImageNames(imageNames);

    d->completed = false;

    connect(d->filterThread, &FilterThread::finished,
            this, &GmicBqmProcessor::slotProcessingFinished);

    d->timer.setInterval(250);

    connect(&d->timer, &QTimer::timeout,
            this, &GmicBqmProcessor::slotSendProgressInformation);

    d->timer.start();
    d->filterThread->start();
}

void GmicBqmProcessor::slotSendProgressInformation()
{
    if (d->filterThread)
    {
        Q_EMIT signalProgress(d->filterThread->progress());
    }
}

void GmicBqmProcessor::slotProcessingFinished()
{
    d->timer.stop();
    QString errorMessage;
    QStringList status = d->filterThread->gmicStatus();

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "G'MIC Filter status" << status;

    if (d->filterThread->failed())
    {
        qCWarning(DIGIKAM_DPLUGIN_BQM_LOG) << "G'MIC Filter execution failed!";

        errorMessage = d->filterThread->errorMessage();

        if (errorMessage.isEmpty())
        {
            errorMessage = QLatin1String("G'MIC Filter execution failed without error message.");
        }

        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << errorMessage;
        d->completed = false;
    }
    else
    {
        gmic_list<gmic_pixel_type> images = d->filterThread->images();

        if (!d->filterThread->aborted())
        {
            GMicQtImageConverter::convertCImgtoDImg(
                                                    images[0],
                                                    d->outImage,
                                                    d->inImage.sixteenBit()
                                                   );

            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "G'MIC Filter execution completed!";

            d->completed = true;
        }
        else
        {
            qCWarning(DIGIKAM_DPLUGIN_BQM_LOG) << "G'MIC Filter execution aborted...";
        }

    }

    d->filterThread->deleteLater();
    d->filterThread = nullptr;

    Q_EMIT signalDone(errorMessage);
}

void GmicBqmProcessor::cancel()
{
    if (d->filterThread)
    {
        d->filterThread->abortGmic();
    }
}

DImg GmicBqmProcessor::outputImage() const
{
    return d->outImage;
}

QString GmicBqmProcessor::processingCommand() const
{
    return d->command;
}

QString GmicBqmProcessor::filterName() const
{
    return d->filterName;
}

bool GmicBqmProcessor::processingComplete() const
{
    return d->completed;
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicbqmprocessor.cpp"
