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

#include "gmicqtplugin.h"

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
#include "gmicqtbqmtool.h"

using namespace GmicQt;

namespace DigikamBqmGmicQtPlugin
{

GmicQtPlugin::GmicQtPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

GmicQtPlugin::~GmicQtPlugin()
{
}

QString GmicQtPlugin::name() const
{
    return QString::fromUtf8("GmicQt");
}

QString GmicQtPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon GmicQtPlugin::icon() const
{
    return QIcon(":resources/gmic_hat.png");
}

QString GmicQtPlugin::description() const
{
    return tr("A tool for G'MIC-Qt");
}

QString GmicQtPlugin::details() const
{
    QImage img(":resources/logos.png");
    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    img.save(&buffer, "PNG");

    QString logo = QString::fromLatin1("<p><img src=\"data:image/png;base64,%1\"></p>")
                   .arg(QString::fromLatin1(byteArray.toBase64().data()));

    return tr("<p><b>An Image Editor tool for G'MIC-Qt.</b></p>"
              "<p><b>Overview:</b></p>"
                "<p>G'MIC-Qt is a versatile front-end to the image processing framework G'MIC</p>"
                "<p>G'MIC is a full-featured open-source framework for image processing. "
                "It provides several user interfaces to convert / manipulate / filter / "
                "visualize generic image datasets, ranging from 1D scalar signals to 3D+t sequences "
                "of multi-spectral volumetric images, hence including 2D color images.</p>"
              "<p><b>Credits:</b></p>"
                "%1<br/>"
                "<a href='https://gmic.eu/'>G'MIC</a><br/>"
                "<a href='https://www.greyc.fr'>GREYC</a><br/>"
                "<a href='https://www.cnrs.fr'>CNRS</a><br/>"
                "<a href='https://www.unicaen.fr'>Normandy University</a><br/>"
                "<a href='https://www.ensicaen.fr'>Ensicaen</a><br/>"
              "<p><b>Configuration:</b></p>"
                "Libgmic version: %2<br/>"
             ).arg(logo)
              .arg(gmic_version)

#ifdef cimg_use_fftw3
             + QString::fromUtf8("Libfftw3 version: %1<br/>").arg(fftw_version)
#endif

#ifdef cimg_use_fftw3_singlethread
             + QString::fromUtf8("Use FFTW3 single thread: yes<br/>")
#else
             + QString::fromUtf8("Use FFTW3 single thread: no<br/>")
#endif

#ifdef cimg_use_curl
             + QString::fromUtf8("Use Curl: yes<br/>")
#else
             + QString::fromUtf8("Use Curl: no<br/>")
#endif

#ifdef cimg_use_openmp
             + QString::fromUtf8("Use OpenMP: yes<br/>")
#else
             + QString::fromUtf8("Use OpenMP: no<br/>")
#endif

    ;
}

QString GmicQtPlugin::handbookSection() const
{
    return QLatin1String("batch_queue");
}

QString GmicQtPlugin::handbookChapter() const
{
    return QLatin1String("base_tools");
}

QString GmicQtPlugin::handbookReference() const
{
    return QLatin1String("bqm-enhancetools");
}

QList<DPluginAuthor> GmicQtPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2023"))
            << DPluginAuthor(QString::fromUtf8("Sébastien Fourey"),
                             QString::fromUtf8("Sebastien dot Fourey at ensicaen dot fr"),
                             QString::fromUtf8("(C) 2017-2023"),
                             QString::fromUtf8("G'MIC plugin"))
            << DPluginAuthor(QString::fromUtf8("David Tschumperlé"),
                             QString::fromUtf8("David dot Tschumperle at ensicaen dot fr"),
                             QString::fromUtf8("(C) 2008-2023"),
                             QString::fromUtf8("G'MIC core"))
            ;
}

void GmicQtPlugin::setup(QObject* const parent)
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

    GmicQtBqmTool* const tool = new GmicQtBqmTool(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace DigikamBqmGmicQtPlugin