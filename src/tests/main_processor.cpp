/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam GmicQt tests.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

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
        QString path = parser.positionalArguments().constFirst();
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

            return (-1);
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
