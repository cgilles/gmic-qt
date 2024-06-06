/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 *  Description: digiKam plugin for GmicQt.
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

#include "gmicqtcommon.h"

// Qt includes

#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QObject>

// digiKam includes

#include "digikam_debug.h"
#include "digikam_globals.h"

// Libfftw includes

#ifdef cimg_use_fftw3
#   include <fftw3.h>
#endif

// Local includes

#include "gmic.h"

namespace DigikamGmicQtPluginCommon
{

QString s_gmicQtPluginDetails(const QString& title)
{
    QImage img(":resources/logos.png");
    QByteArray byteArray;
    QBuffer    buffer(&byteArray);
    img.save(&buffer, "PNG");

    QString logo = QString::fromLatin1("<p><img src=\"data:image/png;base64,%1\"></p>")
                   .arg(QString::fromLatin1(byteArray.toBase64().data()));

    return QObject::tr("<p><b>%1</b></p>"
              "<p><b>Overview:</b></p>"
                "<p>G'MIC-Qt is a versatile front-end to the image processing framework G'MIC</p>"
                "<p>G'MIC is a full-featured open-source framework for image processing. "
                "It provides several user interfaces to convert / manipulate / filter / "
                "visualize generic image datasets, ranging from 1D scalar signals to 3D+t sequences "
                "of multi-spectral volumetric images, hence including 2D color images.</p>"
              "<p><b>Credits:</b></p>"
                "%2<br/>"
                "<a href='https://gmic.eu/'>G'MIC</a><br/>"
                "<a href='https://www.greyc.fr'>GREYC</a><br/>"
                "<a href='https://www.cnrs.fr'>CNRS</a><br/>"
                "<a href='https://www.unicaen.fr'>Normandy University</a><br/>"
                "<a href='https://www.ensicaen.fr'>Ensicaen</a><br/>"
              "<p><b>Configuration:</b></p>"
                "Libcimg version: %3<br/>"
                "Libgmic version: %4<br/>"
             ).arg(title)
              .arg(logo)
              .arg(cimg_version)
              .arg(gmic_version)

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

QList<DPluginAuthor> s_gmicQtPluginAuthors()
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019-2024"))
            << DPluginAuthor(QString::fromUtf8("Sébastien Fourey"),
                             QString::fromUtf8("Sebastien dot Fourey at ensicaen dot fr"),
                             QString::fromUtf8("(C) 2017-2024"),
                             QString::fromUtf8("G'MIC plugin"))
            << DPluginAuthor(QString::fromUtf8("David Tschumperlé"),
                             QString::fromUtf8("David dot Tschumperle at ensicaen dot fr"),
                             QString::fromUtf8("(C) 2008-2024"),
                             QString::fromUtf8("G'MIC core"))
            ;
}

QIcon s_gmicQtPluginIcon()
{
    return QIcon(":resources/gmic_hat.png");
}

} // namespace DigikamGmicQtPluginCommon
