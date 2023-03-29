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

// Local includes

#include "dimg.h"
#include "bqm_widget.h"

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

    m_settingsWidget    = new Bqm_Widget();
/*
    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));
*/
    BatchTool::registerSettingsWidget();
}

BatchToolSettings GmicQtBqmTool::defaultSettings()
{
    BatchToolSettings settings;
/*
    settings.insert(QLatin1String("GmicQtBqmToolMethod"), ReduceUniformNoise);
*/
    return settings;
}

void GmicQtBqmTool::slotAssignSettings2Widget()
{
/*
    m_comboBox->setCurrentIndex(settings()[QLatin1String("GmicQtBqmToolMethod")].toInt());
*/
}

void GmicQtBqmTool::slotSettingsChanged()
{
    BatchToolSettings settings;
/*
    settings.insert(QLatin1String("GmicQtBqmToolMethod"), (int)m_comboBox->currentIndex());
*/
    BatchTool::slotSettingsChanged(settings);
}

bool GmicQtBqmTool::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }
/*
    int type = settings()[QLatin1String("GmicQtBqmToolMethod")].toInt();

    GreycstorationContainer settings;
    settings.setGmicQtBqmToolDefaultSettings();

    switch (type)
    {
        case ReduceUniformNoise:
        {
            settings.amplitude = 40.0;
            break;
        }

        case ReduceJPEGArtefacts:
        {
            settings.sharpness = 0.3F;
            settings.sigma     = 1.0;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }

        case ReduceTexturing:
        {
            settings.sharpness = 0.5F;
            settings.sigma     = 1.5;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }
    }

    m_cimgIface = new GreycstorationFilter(this);
    m_cimgIface->setMode(GreycstorationFilter::Restore);
    m_cimgIface->setOriginalImage(image());
    m_cimgIface->setSettings(settings);
    m_cimgIface->setup();

    applyFilter(m_cimgIface);

    delete m_cimgIface;
    m_cimgIface = nullptr;
*/
    return (savefromDImg());
}

void GmicQtBqmTool::cancel()
{
/*
    if (m_cimgIface)
    {
        m_cimgIface->cancelFilter();
    }
*/
    BatchTool::cancel();
}

} // namespace DigikamBqmGmicQtPlugin
