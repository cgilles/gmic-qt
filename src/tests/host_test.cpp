/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam GmicQt tests.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QDataStream>
#include <QTextStream>
#include <QStringList>

// digiKam includes

#include "digikam_debug.h"
#include "metaengine.h"

// Local includes

#include "Common.h"
#include "Host/GmicQtHost.h"
#include "gmicqtimageconverter.h"

namespace DigikamBqmGmicQtPlugin
{

extern QString s_imagePath;

} // namespace DigikamBqmGmicQtPlugin

using namespace Digikam;
using namespace DigikamGmicQtPluginCommon;
using namespace DigikamBqmGmicQtPlugin;

/**
 * GMic-Qt plugin functions
 * See documentation from GmicQtHost.h for details.
 */

namespace GmicQtHost
{

const QString ApplicationName          = QLatin1String("digiKam");
const char* const ApplicationShortname = GMIC_QT_XSTRINGIFY(GMIC_HOST);
const bool DarkThemeIsDefault          = false;

void getImageSize(int* width,
                  int* height)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Calling GmicQt getImageSize()";
    MetaEngine meta;

    if (meta.load(s_imagePath))
    {
        *width  = meta.getItemDimensions().width();
        *height = meta.getItemDimensions().height();
    }
    else
    {
        *width  = 0;
        *height = 0;
    }
}

void getLayersExtent(int* width,
                     int* height,
                     GmicQt::InputMode mode)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Calling GmicQt getLayersExtent(): InputMode="
                               << (int)mode;

    getImageSize(width, height);

    qCDebug(DIGIKAM_TESTS_LOG) << "W=" << *width;
    qCDebug(DIGIKAM_TESTS_LOG) << "H=" << *height;
}

void getCroppedImages(cimg_library::CImgList<gmic_pixel_type>& images,
                      cimg_library::CImgList<char>& imageNames,
                      double x,
                      double y,
                      double width,
                      double height,
                      GmicQt::InputMode mode)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Calling GmicQt getCroppedImages()";

    if (mode == GmicQt::InputMode::NoInput || s_imagePath.isNull())
    {
        images.assign();
        imageNames.assign();

        return;
    }

    DImg* const input_image = new DImg(s_imagePath);
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

    QString name  = QString::fromUtf8("pos(0,0),name(%1)").arg(QLatin1String("Image Editor Canvas"));
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

    delete input_image;
}

void applyColorProfile(cimg_library::CImg<gmic_pixel_type>& images) // cppcheck-suppress constParameterReference
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Calling GmicQt applyColorProfile()";

    Q_UNUSED(images);
}

void showMessage(const char* message)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Calling GmicQt showMessage()";
    qCDebug(DIGIKAM_TESTS_LOG) << "G'MIC-Qt:" << message;
}

void outputImages(cimg_library::CImgList<gmic_pixel_type>& images,  // cppcheck-suppress constParameterReference
                  const cimg_library::CImgList<char>& imageNames,
                  GmicQt::OutputMode mode)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Calling GmicQt outputImages()";

    Q_UNUSED(images);
    Q_UNUSED(imageNames);
    Q_UNUSED(mode);
}

} // namespace GmicQtHost
