/*
*  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
*  editors, offering hundreds of filters thanks to the underlying G'MIC
*  image processing framework.
*
*  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QDataStream>
#include <QString>
#include <QTextStream>

// Local includes

#include "gmicqtwindow.h"
#include "Common.h"
#include "Host/GmicQtHost.h"
#include "gmic.h"

// digiKam includes

#include "imageiface.h"
#include "digikam_debug.h"

namespace DigikamEditorGmicQtPlugin
{

extern GMicQtWindow* s_mainWindow;

} // namespace DigikamEditorGmicQtPlugin

using namespace Digikam;

namespace GmicQtHost
{
    const QString ApplicationName          = QString("digiKam");
    const char* const ApplicationShortname = GMIC_QT_XSTRINGIFY(GMIC_HOST);
    const bool DarkThemeIsDefault          = false;

} // namespace GmicQtHost

// Helper methods for DImg to CImg container conversions

namespace
{

inline unsigned char float2uchar_bounded(const float& in)
{
    return (
            (in < 0.0f) ? 0
                        : (in > 255.0f) ? 255
                                        : static_cast<unsigned char>(in)
           );
}

inline unsigned short float2ushort_bounded(const float& in)
{
    return (
            (in < 0.0f) ? 0
                        : (in > 65535.0f) ? 65535
                                          : static_cast<unsigned short>(in)
           );
}

void convertCImgtoDImg(const cimg_library::CImg<float>& in, DImg& out, bool sixteenBit)
{
    Q_ASSERT_X(
               (in.spectrum() <= 4),
               "ImageConverter::convert()",
               QString("bad input spectrum (%1)").arg(in.spectrum()).toLatin1()
              );

    bool alpha = ((in.spectrum() == 4) || (in.spectrum() == 2));
    out        = DImg(in.width(), in.height(), sixteenBit, alpha);

    if      (in.spectrum() == 4) // RGB + Alpha
    {
        qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "GMicQt: convert CImg to DImg: RGB+Alpha image"
                                            << "(" << (sixteenBit+1) * 8 << "bits)";

        const float* srcR = in.data(0, 0, 0, 0);
        const float* srcG = in.data(0, 0, 0, 1);
        const float* srcB = in.data(0, 0, 0, 2);
        const float* srcA = in.data(0, 0, 0, 3);
        int height        = out.height();

        for (int y = 0 ; y < height ; ++y)
        {
            int n = in.width();

            if (sixteenBit)
            {
                unsigned short* dst = (unsigned short*)out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2ushort_bounded(*srcR++) * 256;
                    dst[1] = float2ushort_bounded(*srcG++) * 256;
                    dst[0] = float2ushort_bounded(*srcB++) * 256;
                    dst[3] = float2ushort_bounded(*srcA++) * 256;
                    dst   += 4;
                }
            }
            else
            {
                unsigned char* dst = out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2uchar_bounded(*srcR++);
                    dst[1] = float2uchar_bounded(*srcG++);
                    dst[0] = float2uchar_bounded(*srcB++);
                    dst[3] = float2uchar_bounded(*srcA++);
                    dst   += 4;
                }
            }
        }
    }
    else if (in.spectrum() == 3) // RGB
    {
        qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "GMicQt: convert CImg to DImg: RGB image"
                                            << "(" << (sixteenBit+1) * 8 << "bits)";

        const float* srcR = in.data(0, 0, 0, 0);
        const float* srcG = in.data(0, 0, 0, 1);
        const float* srcB = in.data(0, 0, 0, 2);
        int height        = out.height();

        for (int y = 0 ; y < height ; ++y)
        {
            int n = in.width();

            if (sixteenBit)
            {
                unsigned short* dst = (unsigned short*)out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2ushort_bounded(*srcR++) * 256;
                    dst[1] = float2ushort_bounded(*srcG++) * 256;
                    dst[0] = float2ushort_bounded(*srcB++) * 256;
                    dst[3] = 0xFFFF;
                    dst   += 4;
                }
            }
            else
            {
                unsigned char* dst = out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2uchar_bounded(*srcR++);
                    dst[1] = float2uchar_bounded(*srcG++);
                    dst[0] = float2uchar_bounded(*srcB++);
                    dst[3] = 0xFF;
                    dst   += 4;
                }
            }
        }
    }
    else if (in.spectrum() == 2) // Gray levels + Alpha
    {
        qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "GMicQt: convert CImg to DImg: Gray+Alpha image"
                                            << "(" << (sixteenBit+1) * 8 << "bits)";

        const float* src  = in.data(0, 0, 0, 0);
        const float* srcA = in.data(0, 0, 0, 1);
        int height        = out.height();

        for (int y = 0 ; y < height ; ++y)
        {
            int n = in.width();

            if (sixteenBit)
            {
                unsigned short* dst = (unsigned short*)out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2ushort_bounded(*src) * 256;
                    dst[1] = float2ushort_bounded(*src) * 256;
                    dst[0] = float2ushort_bounded(*src) * 256;
                    dst[3] = float2ushort_bounded(*srcA++) * 256;
                    src++;
                    dst   += 4;
                }
            }
            else
            {
                unsigned char* dst = out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2uchar_bounded(*src);
                    dst[1] = float2uchar_bounded(*src);
                    dst[0] = float2uchar_bounded(*src);
                    dst[3] = float2uchar_bounded(*srcA++);
                    src++;
                    dst   += 4;
                }
            }
        }
    }
    else // Gray levels
    {
        qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "GMicQt: convert CImg to DImg: Gray image"
                                            << "(" << (sixteenBit+1) * 8 << "bits)";

        const float* src  = in.data(0, 0, 0, 0);
        int height        = out.height();

        for (int y = 0 ; y < height ; ++y)
        {
            int n = in.width();

            if (sixteenBit)
            {
                unsigned short* dst = (unsigned short*)out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2ushort_bounded(*src) * 256;
                    dst[1] = float2ushort_bounded(*src) * 256;
                    dst[0] = float2ushort_bounded(*src) * 256;
                    dst[3] = 0xFFFF;
                    src++;
                    dst   += 4;
                }
            }
            else
            {
                unsigned char* dst = out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2uchar_bounded(*src);
                    dst[1] = float2uchar_bounded(*src);
                    dst[0] = float2uchar_bounded(*src);
                    dst[3] = 0xFF;
                    src++;
                    dst   += 4;
                }
            }
        }
    }
}

void convertDImgtoCImg(const DImg& in, cimg_library::CImg<float>& out)
{
    const int w = in.width();
    const int h = in.height();
    out.assign(w, h, 1, in.hasAlpha() ? 4 : 3);

    float* dstR = out.data(0, 0, 0, 0);
    float* dstG = out.data(0, 0, 0, 1);
    float* dstB = out.data(0, 0, 0, 2);
    float* dstA = nullptr;

    if (in.hasAlpha())
    {
        dstA = out.data(0, 0, 0, 3);
    }

    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "GMicQt: convert DImg to CImg:"
                                        << (in.sixteenBit() + 1) * 8 << "bits image"
                                        << "with alpha channel:" << in.hasAlpha();

    for (int y = 0 ; y < h ; ++y)
    {
        if (in.sixteenBit())
        {
            const unsigned short* src = (unsigned short*)in.scanLine(y);
            int n                     = in.width();

            while (n--)
            {
                *dstB++ = static_cast<float>(src[0] / 255.0);
                *dstG++ = static_cast<float>(src[1] / 255.0);
                *dstR++ = static_cast<float>(src[2] / 255.0);

                if (in.hasAlpha())
                {
                    *dstA++ = static_cast<float>(src[3] / 255.0);
                }

                src    += 4;
            }
        }
        else
        {
            const unsigned char* src = in.scanLine(y);
            int n                    = in.width();

            while (n--)
            {
                *dstB++ = static_cast<float>(src[0]);
                *dstG++ = static_cast<float>(src[1]);
                *dstR++ = static_cast<float>(src[2]);

                if (in.hasAlpha())
                {
                    *dstA++ = static_cast<float>(src[3]);
                }

                src    += 4;
            }
        }
    }
}

} // namespace

// --- GMic-Qt plugin functions ----------------------

namespace GmicQtHost
{

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
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << "Calling GmicQt getLayersExtent() : InputMode=" << (int)mode;

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
    const bool entireImage  = ((x < 0.0) && (y < 0.0) && (width < 0.0) && (height < 0.0));

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

    const int ix = static_cast<int>(entireImage ? 0                     : std::floor(x * input_image->width()));
    const int iy = static_cast<int>(entireImage ? 0                     : std::floor(y * input_image->height()));
    const int iw = entireImage                  ? input_image->width()  : std::min(static_cast<int>(input_image->width()  - ix), static_cast<int>(1 + std::ceil(width  * input_image->width())));
    const int ih = entireImage                  ? input_image->height() : std::min(static_cast<int>(input_image->height() - iy), static_cast<int>(1 + std::ceil(height * input_image->height())));

    convertDImgtoCImg(input_image->copy(ix, iy, iw, ih), images[0]);
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

    if (images.size() > 0)
    {
        ImageIface iface;
        DImg dest;
        convertCImgtoDImg(images[0], dest, iface.originalSixteenBit());

        // See bug #462137 : force to save current filter applied
        // to the image to store settings in history.

        if (DigikamEditorGmicQtPlugin::s_mainWindow)
        {
            DigikamEditorGmicQtPlugin::s_mainWindow->saveParameters();
        }

        GmicQt::RunParameters parameters = lastAppliedFilterRunParameters(GmicQt::ReturnedRunParametersFlag::AfterFilterExecution);
        FilterAction action(QLatin1String("G'MIC-Qt"),      1);
        action.addParameter(QLatin1String("Command"),       QString::fromStdString(parameters.command));
        action.addParameter(QLatin1String("FilterPath"),    QString::fromStdString(parameters.filterPath));
        action.addParameter(QLatin1String("InputMode"),     (int)parameters.inputMode);
        action.addParameter(QLatin1String("OutputMode"),    (int)parameters.outputMode);
        action.addParameter(QLatin1String("FilterName"),    QString::fromStdString(parameters.filterName()));
        action.addParameter(QLatin1String("GmicQtVersion"), GmicQt::gmicVersionString());

        iface.setOriginal(QString::fromUtf8("G'MIC-Qt - %1").arg(QString::fromStdString(parameters.filterName())), action, dest);
    }
}

} // namespace GmicQtHost
