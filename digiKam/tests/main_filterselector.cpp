/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 *  Description: digiKam GmicQt tests.
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
