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

#include "gmicfilterwidget.h"
#include "gmicbqmprocessor.h"

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

    QString path = settings()[QLatin1String("GmicBqmToolPath")].toString();

    d->gmicWidget->setCurrentPath(path);

    d->changeSettings = true;
}

void GmicBqmTool::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;

        settings.insert(QLatin1String("GmicBqmToolCommand"), d->gmicWidget->currentGmicFilter());
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

    QString path     = settings()[QLatin1String("GmicBqmToolPath")].toString();
    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: running G'MIC filter" << path;

    QString command  = settings()[QLatin1String("GmicBqmToolCommand")].toString();

    if (command.isEmpty())
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: G'MIC filter command is null!";

        return false;
    }

    d->gmicProcessor = new GmicBqmProcessor(this);
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

    bool b  = d->gmicProcessor->processingComplete();
    image() = d->gmicProcessor->outputImage();

    delete d->gmicProcessor;
    d->gmicProcessor = nullptr;

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: G'MIC filter completed:" << b;

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

#include "moc_gmicbqmtool.cpp"
