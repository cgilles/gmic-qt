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

#include "gmicbqmtool.h"

// Qt includes

#include <QWidget>
#include <QEventLoop>

// digikam includes

#include "digikam_debug.h"
#include "dimg.h"

// Local includes

#include "gmiccommandwidget.h"
#include "gmicbqmprocessor.h"

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicBqmTool::Private
{
public:

    Private() = default;

    GmicCommandWidget* gmicWidget   = nullptr;
    GmicBqmProcessor* gmicProcessor = nullptr;
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
    d->gmicWidget    = new GmicCommandWidget();
    m_settingsWidget = d->gmicWidget;

    connect(d->gmicWidget, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings GmicBqmTool::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("GmicBqmToolCommand"), QString());

    return settings;
}

void GmicBqmTool::slotAssignSettings2Widget()
{
    QString command = settings()[QLatin1String("GmicBqmToolCommand")].toString();

//    d->gmicWidget->setPluginParameters(parameters);
}

void GmicBqmTool::slotSettingsChanged()
{
    QString command = QLatin1String("fx_watermark_visible \"digiKam\",0.664,27,60,1,25,0,0.5");

    BatchToolSettings settings;

    settings.insert(QLatin1String("GmicBqmToolCommand"), command);

    BatchTool::slotSettingsChanged(settings);
}

bool GmicBqmTool::toolOperations()
{
    if (!loadToDImg())
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: cannot load image!";

        return false;
    }

//    QString command = settings()[QLatin1String("GmicBqmToolCommand")].toString();

    QString command  = QLatin1String("fx_watermark_visible \"digiKam\",0.664,27,60,1,25,0,0.5");
    d->gmicProcessor = new GmicBqmProcessor(this);
    d->gmicProcessor->setInputImage(image());

    if (!d->gmicProcessor->setProcessingCommand(command))
    {
        delete d->gmicProcessor;
        d->gmicProcessor = nullptr;
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: cannot setup Gmic command!";

        return false;
    }

    d->gmicProcessor->startProcessing();

    QEventLoop loop;

    connect(d->gmicProcessor, SIGNAL(signalDone(QString)),
            &loop, SLOT(quit()));

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: started Gmic command...";

    loop.exec();

    bool b  = d->gmicProcessor->processingComplete();
    image() = d->gmicProcessor->outputImage();

    delete d->gmicProcessor;
    d->gmicProcessor = nullptr;

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: Gmic command completed:" << b;

    if (!b)
    {
        return false;
    }

    return (savefromDImg());
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
