/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : common implementations for the digiKam plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// digiKam includes

#include "dimg.h"

// Local includes

#include "gmic.h"

using namespace Digikam;

namespace DigikamGmicQtPluginCommon
{

/**
 * Helper methods for Digikam::DImg to CImg image data container conversions and vis-versa.
 */
class GMicQtImageConverter
{

public:

    static void convertCImgtoDImg(const cimg_library::CImg<float>& in,
                                  DImg& out, bool sixteenBit);

    static void convertDImgtoCImg(const DImg& in,
                                  cimg_library::CImg<float>& out);

private:

    static unsigned char  float2ucharBounded(const float& in);
    static unsigned short float2ushortBounded(const float& in);

    // Disable
    GMicQtImageConverter()  = delete;
    ~GMicQtImageConverter() = delete;
};

} // namespace DigikamGmicQtPluginCommon
