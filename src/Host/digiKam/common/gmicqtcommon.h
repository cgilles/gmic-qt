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

// Qt includes

#include <QString>
#include <QList>
#include <QIcon>

// digiKam includes

#include "dplugin.h"
#include "filteraction.h"

using namespace Digikam;

namespace DigikamGmicQtPluginCommon
{

/**
 * Return the G'MIC plugin description and details.
 */
QString              s_gmicQtPluginDetails(const QString& title);

/**
 * Return the G'MIC plugin authors list.
 */
QList<DPluginAuthor> s_gmicQtPluginAuthors();

/**
 * Return the G'MIC plugin icon.
 */
QIcon                s_gmicQtPluginIcon();

/**
 * Return the digiKam image versioning container populated with the G'MIC filter properties.
 */
FilterAction         s_gmicQtFilterAction(const QString& gmicCommand,
                                          const QString& filterPath,
                                          int            inMode,
                                          int            outMode,
                                          const QString& filterName);

} // namespace DigikamGmicQtPluginCommon
