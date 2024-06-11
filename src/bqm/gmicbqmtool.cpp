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

#include "gmicbqmtool.h"

// Qt includes

#include <QWidget>
#include <QEventLoop>

// digikam includes

#include "digikam_debug.h"
#include "filteraction.h"
#include "dimg.h"

// Local includes

#include "gmicfilterwidget.h"
#include "gmicbqmprocessor.h"
#include "gmicqtcommon.h"
#include "GmicQt.h"

using namespace DigikamGmicQtPluginCommon;

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicBqmTool::Private
{
public:

    Private() = default;

    GmicFilterWidget* gmicWidget     = nullptr;
    GmicBqmProcessor* gmicProcessor  = nullptr;

    bool              changeSettings = true;
};

GmicBqmTool::GmicBqmTool(QObject* const parent)
    : BatchTool(QLatin1String("GmicBqmTool"), EnhanceTool, parent),
      d        (new Private)
{
}

GmicBqmTool::~GmicBqmTool()
{
    delete d;
}

BatchTool* GmicBqmTool::clone(QObject* const parent) const
{
    return new GmicBqmTool(parent);
}

void GmicBqmTool::registerSettingsWidget()
{
    d->gmicWidget    = new GmicFilterWidget();
    d->gmicWidget->setPlugin(plugin());
    m_settingsWidget = d->gmicWidget;

    connect(d->gmicWidget, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings GmicBqmTool::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("GmicBqmToolCommand"), QString());
    settings.insert(QLatin1String("GmicBqmToolPath"),    QString());

    return settings;
}

void GmicBqmTool::slotAssignSettings2Widget()
{
    d->changeSettings = false;

    QString path      = settings().value(QLatin1String("GmicBqmToolPath")).toString();

    d->gmicWidget->setCurrentPath(path);

    d->changeSettings = true;
}

void GmicBqmTool::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;

        settings.insert(QLatin1String("GmicBqmToolCommand"), d->gmicWidget->currentGmicChainedCommands());
        settings.insert(QLatin1String("GmicBqmToolPath"),    d->gmicWidget->currentPath());

        BatchTool::slotSettingsChanged(settings);
    }
}

bool GmicBqmTool::toolOperations()
{
    if (!loadToDImg())
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: cannot load image!";

        return false;
    }

    QString path     = settings().value(QLatin1String("GmicBqmToolPath")).toString();
    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: running G'MIC filter" << path;

    QString command  = settings().value(QLatin1String("GmicBqmToolCommand")).toString();

    if (command.isEmpty())
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: G'MIC filter command is null!";

        return false;
    }

    d->gmicProcessor = new GmicBqmProcessor();
    d->gmicProcessor->setInputImage(image());

    if (!d->gmicProcessor->setProcessingCommand(command))
    {
        delete d->gmicProcessor;
        d->gmicProcessor = nullptr;
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: cannot setup G'MIC filter!";

        return false;
    }

    d->gmicProcessor->startProcessing();

    QEventLoop loop;

    connect(d->gmicProcessor, SIGNAL(signalDone(QString)),
            &loop, SLOT(quit()));

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: started G'MIC filter...";

    loop.exec();

    bool b   = d->gmicProcessor->processingComplete();
    DImg out = d->gmicProcessor->outputImage();
    image().putImageData(out.width(), out.height(), out.sixteenBit(), out.hasAlpha(), out.bits());

    FilterAction action = s_gmicQtFilterAction(
                                               command,
                                               path,
                                               (int)GmicQt::DefaultInputMode,
                                               (int)GmicQt::DefaultOutputMode,
                                               d->gmicProcessor->filterName()
                                              );

    image().addFilterAction(action);

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: G'MIC filter completed:" << b;

    if (b)
    {
        b = savefromDImg();
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: G'MIC flush image data completed:" << b;

    delete d->gmicProcessor;
    d->gmicProcessor = nullptr;

    return b;
}

void GmicBqmTool::cancel()
{
    if (d->gmicProcessor)
    {
        d->gmicProcessor->cancel();
    }

    BatchTool::cancel();
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicbqmtool.cpp"
