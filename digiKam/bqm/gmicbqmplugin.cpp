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

#include "gmicbqmplugin.h"

// Qt includes

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QEventLoop>
#include <QPointer>
#include <QImage>
#include <QBuffer>
#include <QByteArray>

// Libfftw includes

#ifdef cimg_use_fftw3
#   include <fftw3.h>
#endif

// Local includes

#include "LanguageSettings.h"
#include "GmicQt.h"
#include "Widgets/InOutPanel.h"
#include "Settings.h"
#include "gmic.h"
#include "gmicqtcommon.h"
#include "gmicbqmtool.h"

using namespace GmicQt;
using namespace DigikamGmicQtPluginCommon;

namespace DigikamBqmGmicQtPlugin
{

GmicBqmPlugin::GmicBqmPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

QString GmicBqmPlugin::name() const
{
    return QString::fromUtf8("G'MIC");
}

QString GmicBqmPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon GmicBqmPlugin::icon() const
{
    return s_gmicQtPluginIcon();
}

QString GmicBqmPlugin::description() const
{
    return tr("A tool to apply the G'MIC filters to images");
}

QString GmicBqmPlugin::details() const
{
    return s_gmicQtPluginDetails(tr("A Batch Queue Manager tool for G'MIC processor."));
}

QString GmicBqmPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString GmicBqmPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString GmicBqmPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> GmicBqmPlugin::authors() const
{
    return s_gmicQtPluginAuthors();
}

void GmicBqmPlugin::setup(QObject* const parent)
{
    // Code inspired from GmicQt.cpp::run() and host_none.cpp::main()

    Settings::load(GmicQt::UserInterfaceMode::Full);
    LanguageSettings::installTranslators();

    // ---

    std::list<GmicQt::InputMode> disabledInputModes;
    disabledInputModes.push_back(GmicQt::InputMode::NoInput);
    // disabledInputModes.push_back(InputMode::Active);
    disabledInputModes.push_back(GmicQt::InputMode::All);
    disabledInputModes.push_back(GmicQt::InputMode::ActiveAndBelow);
    disabledInputModes.push_back(GmicQt::InputMode::ActiveAndAbove);
    disabledInputModes.push_back(GmicQt::InputMode::AllVisible);
    disabledInputModes.push_back(GmicQt::InputMode::AllInvisible);

    std::list<GmicQt::OutputMode> disabledOutputModes;
    // disabledOutputModes.push_back(GmicQt::OutputMode::InPlace);
    disabledOutputModes.push_back(GmicQt::OutputMode::NewImage);
    disabledOutputModes.push_back(GmicQt::OutputMode::NewLayers);
    disabledOutputModes.push_back(GmicQt::OutputMode::NewActiveLayers);

    for (const GmicQt::InputMode& mode : disabledInputModes)
    {
        GmicQt::InOutPanel::disableInputMode(mode);
    }

    for (const GmicQt::OutputMode& mode : disabledOutputModes)
    {
        GmicQt::InOutPanel::disableOutputMode(mode);
    }

    GmicBqmTool* const tool = new GmicBqmTool(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicbqmplugin.cpp"
