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

#include <QEventLoop>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

// digiKam includes

#include "digikam_debug.h"
#include "dpluginloader.h"
#include "dimg.h"

// local includes

#include "gmicbqmprocessor.h"

namespace DigikamBqmGmicQtPlugin
{

QString s_imagePath;

} // namespace DigikamBqmGmicQtPlugin

using namespace Digikam;
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

    DImg img;

    if (!parser.positionalArguments().isEmpty())
    {
        QString path = parser.positionalArguments().first();
        qCDebug(DIGIKAM_TESTS_LOG) << "Image to Process:" << path;

        GmicBqmProcessor* const gmicProcessor = new GmicBqmProcessor();
        img.load(path);
        gmicProcessor->setInputImage(img);

        QStringList chainedCommands;
        chainedCommands << QLatin1String("gcd_aurora 6,1,0");               // Apply Aurora FX.
        chainedCommands << QLatin1String("gcd_auto_balance 30,0,0,1,0");    // Apply auto color balance.
        chainedCommands << QLatin1String("fx_old_photo 200,50,85");         // Add old photo frame.

        if (!gmicProcessor->setProcessingCommand(chainedCommands.join(QLatin1Char(' '))))
        {
            delete gmicProcessor;
            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: cannot setup G'MIC filter!";

            return false;
        }

        gmicProcessor->startProcessing();

        QEventLoop loop;

        QObject::connect(gmicProcessor, SIGNAL(signalDone(QString)),
                         &loop, SLOT(quit()));

        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: started G'MIC filter...";

        loop.exec();

        bool b = gmicProcessor->processingComplete();

        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "GmicBqmTool: G'MIC filter completed:" << b;

        gmicProcessor->outputImage().save(path + QLatin1String("_gmic.jpg"), "JPG");

        delete gmicProcessor;
    }
    else
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Image path is missing...";
    }

    return 0;
}
