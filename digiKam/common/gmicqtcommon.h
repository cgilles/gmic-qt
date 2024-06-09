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

#pragma once

// Qt includes

#include <QString>
#include <QList>
#include <QIcon>
#include <QPushButton>

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

/**
 * Populate the dialog Help Button with about and documentation support.
 */
void s_gmicQtPluginPopulateHelpButton(QWidget* const parent,
                                      DPlugin* const tool,
                                      QPushButton* const help);

} // namespace DigikamGmicQtPluginCommon
