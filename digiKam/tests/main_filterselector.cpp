/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam GmicQt tests.
 *
 * SPDX-FileCopyrightText: 2019-2024 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

// digiKam includes

#include "digikam_debug.h"
#include "dpluginloader.h"

// local includes

#include "gmicqtwindow.h"

using namespace DigikamGmicQtPluginCommon;
using namespace Digikam;

namespace DigikamBqmGmicQtPlugin
{

QString s_imagePath;

} // namespace DigikamBqmGmicQtPlugin

using namespace DigikamBqmGmicQtPlugin;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    DPluginLoader::instance()->init();

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addPositionalArgument(QString::fromLatin1("image"), QLatin1String("Image file path"), QString::fromLatin1("[image]"));
    parser.process(app);

    if (!parser.positionalArguments().isEmpty())
    {
        s_imagePath   = parser.positionalArguments().constFirst();
        qCDebug(DIGIKAM_TESTS_LOG) << "Image to Process:" << s_imagePath;

        QString fname = GMicQtWindow::execWindow(
                                                 nullptr,
                                                 GMicQtWindow::BQM,
                                                 QLatin1String("samj_Barbouillage_Paint_Daub 2,2,100,0.2,1,4,1,0,8")
                                                );

        qCDebug(DIGIKAM_TESTS_LOG) << "Selected Filter:" << fname;
    }
    else
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Image path is missing...";
    }

    return 0;
}
