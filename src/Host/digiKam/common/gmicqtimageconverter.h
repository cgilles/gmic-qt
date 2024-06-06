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

#pragma once

// digiKam includes

#include "dimg.h"

// Local includes

#include "gmic.h"

using namespace Digikam;

namespace DigikamEditorGmicQtPlugin
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

} // namespace DigikamEditorGmicQtPlugin
