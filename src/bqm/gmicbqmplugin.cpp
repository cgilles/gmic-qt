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
    return QLatin1String("bqm-gmictools");
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
