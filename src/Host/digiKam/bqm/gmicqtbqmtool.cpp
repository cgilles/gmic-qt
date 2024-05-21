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

#include "gmicqtbqmtool.h"

// Qt includes

#include <QWidget>

// digikam includes

#include "digikam_debug.h"
#include "dimg.h"
#include "bqm_widget.h"

// Local includes

#include "bqm_processor.h"

namespace DigikamBqmGmicQtPlugin
{

GmicQtBqmTool::GmicQtBqmTool(QObject* const parent)
    : BatchTool(QLatin1String("GmicQtBqmTool"), EnhanceTool, parent)
{
}

BatchTool* GmicQtBqmTool::clone(QObject* const parent) const
{
    return new GmicQtBqmTool(parent);
}

void GmicQtBqmTool::registerSettingsWidget()
{
    m_gmicWidget     = new QWidget();
    m_settingsWidget = m_gmicWidget;

    connect(m_gmicWidget, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings GmicQtBqmTool::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("GmicQtBqmToolCommand"), QString());

    return settings;
}

void GmicQtBqmTool::slotAssignSettings2Widget()
{
    QString command = settings()[QLatin1String("GmicQtBqmToolCommand")].toString();

//    m_gmicWidget->setPluginParameters(parameters);
}

void GmicQtBqmTool::slotSettingsChanged()
{
    QString command = QLatin1String("fx_watermark_visible \"digiKam\",0.664,27,60,1,25,0,0.5");

    BatchToolSettings settings;

    settings.insert(QLatin1String("GmicQtBqmToolCommand"), command);

    BatchTool::slotSettingsChanged(settings);
}

bool GmicQtBqmTool::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

//    QString command = settings()[QLatin1String("GmicQtBqmToolCommand")].toString();

    QString command = QLatin1String("fx_watermark_visible \"digiKam\",0.664,27,60,1,25,0,0.5");
    m_gmicProcessor = new Bqm_Processor(this);

    if (m_gmicProcessor->setPluginParameters(command, image()))
    {
        delete m_gmicProcessor;
        m_gmicProcessor = nullptr;

        return false;
    }

    m_gmicProcessor->startProcessing();

    QEventLoop loop;

    connect(m_gmicProcessor, SIGNAL(done(QString)),
            &loop, SLOT(quit()));

    loop.exec();

    QString error = m_gmicProcessor->error();

    image() = m_gmicProcessor->outputImage();

    delete m_gmicProcessor;
    m_gmicProcessor = nullptr;

    if (!error.isEmpty())
    {
        return false;
    }

    return (savefromDImg());
}

void GmicQtBqmTool::cancel()
{
    if (m_gmicProcessor)
    {
        m_gmicProcessor->cancel();
    }

    BatchTool::cancel();
}

} // namespace DigikamBqmGmicQtPlugin
