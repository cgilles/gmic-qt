/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : common implementations for the digiKam plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2024 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "gmicqtimageconverter.h"

// digiKam includes

#include "digikam_debug.h"

namespace DigikamGmicQtPluginCommon
{

inline unsigned char GMicQtImageConverter::float2ucharBounded(const float& in)
{
    return (
            (in < 0.0f) ? 0
                        : (in > 255.0f) ? 255
                                        : static_cast<unsigned char>(in)
           );
}

inline unsigned short GMicQtImageConverter::float2ushortBounded(const float& in)
{
    return (
            (in < 0.0f) ? 0
                        : (in > 65535.0f) ? 65535
                                          : static_cast<unsigned short>(in)
           );
}

void GMicQtImageConverter::convertCImgtoDImg(const cimg_library::CImg<float>& in,
                                             DImg& out, bool sixteenBit)
{
    Q_ASSERT_X(
               (in.spectrum() <= 4),
               "GMicQtImageConverter::convertCImgtoDImg()",
               QString::fromLatin1("bad input spectrum (%1)").arg(in.spectrum()).toLatin1().data()
              );

    bool alpha = ((in.spectrum() == 4) || (in.spectrum() == 2));
    out        = DImg(in.width(), in.height(), sixteenBit, alpha);

    if      (in.spectrum() == 4) // RGB + Alpha
    {
        qCDebug(DIGIKAM_DPLUGIN_LOG) << "GMicQt: convert CImg to DImg: RGB+Alpha image"
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
                unsigned short* dst = reinterpret_cast<unsigned short*>(out.scanLine(y));

                while (n--)
                {
                    dst[2] = float2ushortBounded(*srcR++) * 256;
                    dst[1] = float2ushortBounded(*srcG++) * 256;
                    dst[0] = float2ushortBounded(*srcB++) * 256;
                    dst[3] = float2ushortBounded(*srcA++) * 256;
                    dst   += 4;
                }
            }
            else
            {
                unsigned char* dst = out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2ucharBounded(*srcR++);
                    dst[1] = float2ucharBounded(*srcG++);
                    dst[0] = float2ucharBounded(*srcB++);
                    dst[3] = float2ucharBounded(*srcA++);
                    dst   += 4;
                }
            }
        }
    }
    else if (in.spectrum() == 3) // RGB
    {
        qCDebug(DIGIKAM_DPLUGIN_LOG) << "GMicQt: convert CImg to DImg: RGB image"
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
                unsigned short* dst = reinterpret_cast<unsigned short*>(out.scanLine(y));

                while (n--)
                {
                    dst[2] = float2ushortBounded(*srcR++) * 256;
                    dst[1] = float2ushortBounded(*srcG++) * 256;
                    dst[0] = float2ushortBounded(*srcB++) * 256;
                    dst[3] = 0xFFFF;
                    dst   += 4;
                }
            }
            else
            {
                unsigned char* dst = out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2ucharBounded(*srcR++);
                    dst[1] = float2ucharBounded(*srcG++);
                    dst[0] = float2ucharBounded(*srcB++);
                    dst[3] = 0xFF;
                    dst   += 4;
                }
            }
        }
    }
    else if (in.spectrum() == 2) // Gray levels + Alpha
    {
        qCDebug(DIGIKAM_DPLUGIN_LOG) << "GMicQt: convert CImg to DImg: Gray+Alpha image"
                                     << "(" << (sixteenBit+1) * 8 << "bits)";

        const float* src  = in.data(0, 0, 0, 0);
        const float* srcA = in.data(0, 0, 0, 1);
        int height        = out.height();

        for (int y = 0 ; y < height ; ++y)
        {
            int n = in.width();

            if (sixteenBit)
            {
                unsigned short* dst = reinterpret_cast<unsigned short*>(out.scanLine(y));

                while (n--)
                {
                    dst[2] = float2ushortBounded(*src) * 256;
                    dst[1] = float2ushortBounded(*src) * 256;
                    dst[0] = float2ushortBounded(*src) * 256;
                    dst[3] = float2ushortBounded(*srcA++) * 256;
                    src++;
                    dst   += 4;
                }
            }
            else
            {
                unsigned char* dst = out.scanLine(y);

                while (n--)
                {
                    dst[2] = float2ucharBounded(*src);
                    dst[1] = float2ucharBounded(*src);
                    dst[0] = float2ucharBounded(*src);
                    dst[3] = float2ucharBounded(*srcA++);
                    src++;
                    dst   += 4;
                }
            }
        }
    }
    else // Gray levels
    {
        qCDebug(DIGIKAM_DPLUGIN_LOG) << "GMicQt: convert CImg to DImg: Gray image"
                                     << "(" << (sixteenBit+1) * 8 << "bits)";

        const float* src  = in.data(0, 0, 0, 0);
        int height        = out.height();

        for (int y = 0 ; y < height ; ++y)
        {
            int n = in.width();

            if (sixteenBit)
            {
                unsigned short* dst = reinterpret_cast<unsigned short*>(out.scanLine(y));

                while (n--)
                {
                    dst[2] = float2ushortBounded(*src) * 256;
                    dst[1] = float2ushortBounded(*src) * 256;
                    dst[0] = float2ushortBounded(*src) * 256;
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
                    dst[2] = float2ucharBounded(*src);
                    dst[1] = float2ucharBounded(*src);
                    dst[0] = float2ucharBounded(*src);
                    dst[3] = 0xFF;
                    src++;
                    dst   += 4;
                }
            }
        }
    }
}

void GMicQtImageConverter::convertDImgtoCImg(const DImg& in,
                                             cimg_library::CImg<float>& out)
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

    qCDebug(DIGIKAM_DPLUGIN_LOG) << "GMicQt: convert DImg to CImg:"
                                 << (in.sixteenBit() + 1) * 8 << "bits image"
                                 << "with alpha channel:" << in.hasAlpha();

    for (int y = 0 ; y < h ; ++y)
    {
        if (in.sixteenBit())
        {
            const unsigned short* src = reinterpret_cast<unsigned short*>(in.scanLine(y));
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

} // namespace DigikamGmicQtPluginCommon
