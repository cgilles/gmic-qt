/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 *  Description: digiKam Batch Queue Manager plugin for Gmic-Qt.
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

// Qt includes

#include <QDataStream>
#include <QTextStream>
#include <QStringList>

// digiKam includes

#include "digikam_debug.h"
#include "bqminfoiface.h"

// Local includes

#include "Common.h"
#include "Host/GmicQtHost.h"
#include "gmicqtimageconverter.h"

namespace DigikamBqmGmicQtPlugin
{

extern BqmInfoIface* s_infoIface;

} // namespace DigikamBqmGmicQtPlugin

using namespace DigikamBqmGmicQtPlugin;
using namespace DigikamEditorGmicQtPlugin;

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

    QueuePoolItemsList list = s_infoIface->selectedItemInfoListFromCurrentQueue();

    if (!list.isEmpty())
    {
        ItemInfoSet inf = list.first();
        *width          = inf.info.dimensions().width();
        *height         = inf.info.dimensions().height();
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

    QueuePoolItemsList list = s_infoIface->selectedItemInfoListFromCurrentQueue();

    if (mode == GmicQt::InputMode::NoInput || list.isEmpty())
    {
        images.assign();
        imageNames.assign();

        return;
    }

    DImg* const input_image = new DImg(list.first().info.filePath());
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

    delete input_image;
}

void applyColorProfile(cimg_library::CImg<gmic_pixel_type>& images)
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

    Q_UNUSED(images);
    Q_UNUSED(imageNames);
    Q_UNUSED(mode);
}

} // namespace GmicQtHost
