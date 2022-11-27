/*
*  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
*  editors, offering hundreds of filters thanks to the underlying G'MIC
*  image processing framework.
*
*  Copyright (C) 2019-2022 Gilles Caulier <caulier dot gilles at gmail dot com>
*
*  Description: digiKam image editor plugin for GmicQt.
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

#include "gmicqttoolplugin.h"

// Qt includes

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QEventLoop>
#include <QPointer>

// Local includes

#include "LanguageSettings.h"
#include "Settings.h"
#include "GmicQt.h"
#include "Widgets/InOutPanel.h"
#include "gmicqtwindow.h"

using namespace GmicQt;

namespace DigikamEditorGmicQtPlugin
{

GMicQtWindow* s_mainWindow = nullptr;

GmicQtToolPlugin::GmicQtToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

GmicQtToolPlugin::~GmicQtToolPlugin()
{
}

QString GmicQtToolPlugin::name() const
{
    return QString::fromUtf8("GmicQt");
}

QString GmicQtToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon GmicQtToolPlugin::icon() const
{
    return QIcon(":resources/gmic_hat.png");
}

QString GmicQtToolPlugin::description() const
{
    return tr("A tool for G'MIC-Qt");
}

QString GmicQtToolPlugin::details() const
{
    return tr("<p>This Image Editor tool for G'MIC-Qt.</p>"
              "<p>G'MIC-Qt is a versatile front-end to the image processing framework G'MIC</p>"
              "<p>G'MIC is a full-featured open-source framework for image processing. "
              "It provides several user interfaces to convert / manipulate / filter / "
              "visualize generic image datasets, ranging from 1D scalar signals to 3D+t sequences "
              "of multi-spectral volumetric images, hence including 2D color images.</p>"
              "<p>More details: https://gmic.eu/</p>");
}

QList<DPluginAuthor> GmicQtToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2022"))
            << DPluginAuthor(QString::fromUtf8("Sébastien Fourey"),
                             QString::fromUtf8("Sebastien dot Fourey at ensicaen dot fr"),
                             QString::fromUtf8("(C) 2017-2020"),
                             QString::fromUtf8("G'MIC plugin"))
            << DPluginAuthor(QString::fromUtf8("David Tschumperlé"),
                             QString::fromUtf8("David dot Tschumperle at ensicaen dot fr"),
                             QString::fromUtf8("(C) 2008-2020"),
                             QString::fromUtf8("G'MIC core"))
            ;
}

void GmicQtToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(tr("G'MIC-Qt..."));
    ac->setObjectName(QLatin1String("editorwindow_gmicqt"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotGmicQt()));

    addAction(ac);
}

void GmicQtToolPlugin::slotGmicQt()
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

    // ---

    /**
     * We need to backup QApplication instance properties between plugin sessions else we can
     * seen side effects, for example with the settings to host in RC file.
     */

    s_mainWindow             = new GMicQtWindow(nullptr);
    RunParameters parameters = lastAppliedFilterRunParameters(GmicQt::ReturnedRunParametersFlag::AfterFilterExecution);
    s_mainWindow->setPluginParameters(parameters);

    // We want a non modal dialog here.

    s_mainWindow->setWindowFlags(Qt::Tool | Qt::Dialog);
    s_mainWindow->setWindowModality(Qt::ApplicationModal);

    if (QSettings().value("Config/MainWindowMaximized", false).toBool())
    {
        s_mainWindow->showMaximized();
    }
    else
    {
        s_mainWindow->show();
    }

    // Bug #462066 : force to load filters list at start-up.

    s_mainWindow->updateFiltersFromSources(0, false);

    // Wait than main widget is closed.

    QEventLoop loop;

    connect(s_mainWindow, SIGNAL(destroyed()),
            &loop, SLOT(quit()));

    loop.exec();

    delete s_mainWindow;
}

} // namespace DigikamEditorGmicQtPlugin
