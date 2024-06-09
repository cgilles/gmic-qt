/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam image editor plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2024 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QDataStream>
#include <QTextStream>

// digiKam includes

#include "imageiface.h"
#include "digikam_debug.h"

// Local includes

#include "Common.h"
#include "Host/GmicQtHost.h"
#include "gmicqtwindow.h"
#include "gmicqtcommon.h"
#include "gmicqtimageconverter.h"

namespace DigikamGmicQtPluginCommon
{

extern GMicQtWindow* s_mainWindow;

} // namespace DigikamGmicQtPluginCommon

using namespace DigikamGmicQtPluginCommon;

/**
 * GMic-Qt plugin functions
 * See documentation from GmicQtHost.h for details.
 */

namespace GmicQtHost
{

const QString ApplicationName          = QString("digiKam");
const char* const ApplicationShortname = GMIC_QT_XSTRINGIFY(GMIC_HOST);
const bool DarkThemeIsDefault          = false;

void getImageSize(int* width,
                  int* height)
{
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Calling GmicQt getImageSize()";

    ImageIface iface;
    QSize size = iface.originalSize();

    *width     = size.width();
    *height    = size.height();
}

void getLayersExtent(int* width,
                     int* height,
                     GmicQt::InputMode mode)
{
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Calling GmicQt getLayersExtent(): InputMode="
                                        << (int)mode;

    getImageSize(width, height);

    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "W=" << *width;
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "H=" << *height;
}

void getCroppedImages(cimg_library::CImgList<gmic_pixel_type>& images,
                      cimg_library::CImgList<char>& imageNames,
                      double x,
                      double y,
                      double width,
                      double height,
                      GmicQt::InputMode mode)
{
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Calling GmicQt getCroppedImages()";

    if (mode == GmicQt::InputMode::NoInput)
    {
        images.assign();
        imageNames.assign();

        return;
    }

    ImageIface iface;
    DImg* const input_image = iface.original();
    const bool entireImage  = (
                               (x      < 0.0) &&
                               (y      < 0.0) &&
                               (width  < 0.0) &&
                               (height < 0.0))
                              ;

    if (entireImage)
    {
        x      = 0.0;
        y      = 0.0;
        width  = 1.0;
        height = 1.0;
    }

    images.assign(1);
    imageNames.assign(1);

    QString name  = QString("pos(0,0),name(%1)").arg("Image Editor Canvas");
    QByteArray ba = name.toUtf8();
    gmic_image<char>::string(ba.constData()).move_to(imageNames[0]);

    const int ix = static_cast<int>(entireImage ? 0
                                                : std::floor(x * input_image->width()));

    const int iy = static_cast<int>(entireImage ? 0
                                                : std::floor(y * input_image->height()));

    const int iw = entireImage ? input_image->width()
                               : std::min(
                                          static_cast<int>(input_image->width()  - ix),
                                          static_cast<int>(1 + std::ceil(width  * input_image->width()))
                                         );

    const int ih = entireImage ? input_image->height()
                               : std::min(
                                          static_cast<int>(input_image->height() - iy),
                                          static_cast<int>(1 + std::ceil(height * input_image->height()))
                                         );

    GMicQtImageConverter::convertDImgtoCImg(input_image->copy(ix, iy, iw, ih), images[0]);
}

void applyColorProfile(cimg_library::CImg<gmic_pixel_type>& images) // cppcheck-suppress constParameterReference
{
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Calling GmicQt applyColorProfile()";

    Q_UNUSED(images);
}

void showMessage(const char* message)
{
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Calling GmicQt showMessage()";
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "G'MIC-Qt:" << message;
}

void outputImages(cimg_library::CImgList<gmic_pixel_type>& images,
                  const cimg_library::CImgList<char>& imageNames,
                  GmicQt::OutputMode mode)
{
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Calling GmicQt outputImages()";

    if (images.size() > 0)
    {
        ImageIface iface;
        DImg dest;
        GMicQtImageConverter::convertCImgtoDImg(images[0], dest, iface.originalSixteenBit());

        // See bug #462137: force to save current filter applied
        // to the image to store settings in history.

        if (DigikamGmicQtPluginCommon::s_mainWindow)
        {
            DigikamGmicQtPluginCommon::s_mainWindow->saveParameters();
        }

        GmicQt::RunParameters parameters = lastAppliedFilterRunParameters(GmicQt::ReturnedRunParametersFlag::AfterFilterExecution);

        FilterAction action = s_gmicQtFilterAction(
                                                   QString::fromStdString(parameters.command),
                                                   QString::fromStdString(parameters.filterPath),
                                                   (int)parameters.inputMode,
                                                   (int)parameters.outputMode,
                                                   QString::fromStdString(parameters.filterName())
                                                  );

        iface.setOriginal(QString::fromUtf8("G'MIC-Qt - %1").arg(QString::fromStdString(parameters.filterName())),
                          action, dest);
    }
}

} // namespace GmicQtHost
