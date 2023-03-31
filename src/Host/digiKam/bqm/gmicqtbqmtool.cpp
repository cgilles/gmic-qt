/*
*  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
*  editors, offering hundreds of filters thanks to the underlying G'MIC
*  image processing framework.
*
*  Copyright (C) 2019-2023 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "GmicQt.h"
#include "bqm_processor.h"

namespace DigikamBqmGmicQtPlugin
{

GmicQtBqmTool::GmicQtBqmTool(QObject* const parent)
    : BatchTool(QLatin1String("GmicQtBqmTool"), EnhanceTool, parent)
{
}

GmicQtBqmTool::~GmicQtBqmTool()
{
}

BatchTool* GmicQtBqmTool::clone(QObject* const parent) const
{
    return new GmicQtBqmTool(parent);
}

void GmicQtBqmTool::registerSettingsWidget()
{
    m_gmicWidget     = new Bqm_Widget();
    m_settingsWidget = m_gmicWidget;

    connect(m_gmicWidget, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings GmicQtBqmTool::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("GmicQtBqmToolCommand"),    QString());
    settings.insert(QLatin1String("GmicQtBqmToolFilterPath"), QString());
    settings.insert(QLatin1String("GmicQtBqmToolInputMode"),  (int)InputMode::Unspecified);
    settings.insert(QLatin1String("GmicQtBqmToolOutputMode"), (int)OutputMode::Unspecified);

    return settings;
}

void GmicQtBqmTool::slotAssignSettings2Widget()
{
    RunParameters parameters;

    parameters.command    = settings()[QLatin1String("GmicQtBqmToolCommand")].toString().toStdString();
    parameters.filterPath = settings()[QLatin1String("GmicQtBqmToolFilterPath")].toString().toStdString();
    parameters.inputMode  = (InputMode)settings()[QLatin1String("GmicQtBqmToolInputMode")].toInt();
    parameters.outputMode = (OutputMode)settings()[QLatin1String("GmicQtBqmToolOutputMode")].toInt();

    m_gmicWidget->setPluginParameters(parameters);
}

void GmicQtBqmTool::slotSettingsChanged()
{
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Receive settings changed signal from G'MIC-Qt";

    RunParameters parameters = m_gmicWidget->pluginParameters();

    BatchToolSettings settings;

    settings.insert(QLatin1String("GmicQtBqmToolCommand"),    QString::fromStdString(parameters.command));
    settings.insert(QLatin1String("GmicQtBqmToolFilterPath"), QString::fromStdString(parameters.filterPath));
    settings.insert(QLatin1String("GmicQtBqmToolInputMode"),  (int)parameters.inputMode);
    settings.insert(QLatin1String("GmicQtBqmToolOutputMode"), (int)parameters.outputMode);

    BatchTool::slotSettingsChanged(settings);
}

bool GmicQtBqmTool::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    RunParameters parameters;

    parameters.command    = settings()[QLatin1String("GmicQtBqmToolCommand")].toString().toStdString();
    parameters.filterPath = settings()[QLatin1String("GmicQtBqmToolFilterPath")].toString().toStdString();
    parameters.inputMode  = (InputMode)settings()[QLatin1String("GmicQtBqmToolInputMode")].toInt();
    parameters.outputMode = (OutputMode)settings()[QLatin1String("GmicQtBqmToolOutputMode")].toInt();
    m_gmicProcessor       = new Bqm_Processor(this);

    if (m_gmicProcessor->setPluginParameters(parameters))
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
