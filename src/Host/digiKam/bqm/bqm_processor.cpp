/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file Bqm_Processor.cpp
 *
 *  Copyright 2017 Sebastien Fourey
 *
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  gmic_qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gmic_qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gmic_qt.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bqm_processor.h"

#include "digikam_debug.h"

#include "Common.h"
#include "FilterThread.h"
#include "GmicStdlib.h"
#include "Misc.h"
#include "Updater.h"
#include "GmicQt.h"

#include "gmicqtimageconverter.h"

namespace GmicQtHost
{

const QString ApplicationName          = QString("digiKam");
const char* const ApplicationShortname = GMIC_QT_XSTRINGIFY(GMIC_HOST);
const bool DarkThemeIsDefault          = false;

} // namespace GmicQtHost

namespace DigikamBqmGmicQtPlugin
{

Bqm_Processor::Bqm_Processor(QObject* const parent)
    : QObject      (parent),
      _filterThread(nullptr),
      _gmicImages  (new gmic_library::gmic_list<gmic_pixel_type>)
{
  _processingCompletedProperly = false;
  GmicStdLib::Array            = Updater::getInstance()->buildFullStdlib();
}

Bqm_Processor::~Bqm_Processor()
{
  delete _gmicImages;
}

bool Bqm_Processor::setPluginParameters(const QString& command, const DImg& inImage)
{
    _inImage = inImage;

    if (command.isEmpty())
    {
        _errorMessage = tr("At least a filter command must be provided.");
        return false;
    }
    else
    {
        _filterName = tr("Custom command (%1)").arg(elided(command, 35));
        _command    = "skip 0";
        _arguments  = command;
    }

    return true;
}

const QString& Bqm_Processor::error() const
{
  return _errorMessage;
}

void Bqm_Processor::startProcessing()
{
  if (!_errorMessage.isEmpty())
  {
    endApplication(_errorMessage);
  }

  _singleShotTimer.setInterval(750);
  _singleShotTimer.setSingleShot(true);

  connect(&_singleShotTimer, &QTimer::timeout,
          this, &Bqm_Processor::progressWindowShouldShow);

  _singleShotTimer.start();

  gmic_list<char> imageNames;

  _gmicImages->assign(1);
  imageNames.assign(1);

  QString name  = QString("pos(0,0),name(%1)").arg("Batch Queue Manager");
  QByteArray ba = name.toUtf8();
  gmic_image<char>::string(ba.constData()).move_to(imageNames[0]);

  DigikamEditorGmicQtPlugin::GMicQtImageConverter::convertDImgtoCImg(_inImage.copy(0, 0,
                                                                                   _inImage.width(),
                                                                                   _inImage.height()),
                                                                     *_gmicImages[0]);

  qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << QString::fromUtf8("G'MIC: %1 %2")
                                         .arg(_command)
                                         .arg(_arguments);

  QString env = QString::fromLatin1("_input_layers=%1").arg((int)DefaultInputMode);
  env        += QString::fromLatin1(" _output_mode=%1").arg((int)DefaultInputMode);
  env        += QString::fromLatin1(" _output_messages=%1").arg((int)OutputMessageMode::VerboseConsole);

  _filterThread = new FilterThread(this, _command, _arguments, env);
  _filterThread->swapImages(*_gmicImages);
  _filterThread->setImageNames(imageNames);

  _processingCompletedProperly = false;

  connect(_filterThread, &FilterThread::finished,
          this, &Bqm_Processor::onProcessingFinished);

  _timer.setInterval(250);

  connect(&_timer, &QTimer::timeout,
          this, &Bqm_Processor::sendProgressInformation);

  _timer.start();
  _filterThread->start();
}

QString Bqm_Processor::command() const
{
  return _command;
}

QString Bqm_Processor::filterName() const
{
  return _filterName;
}

bool Bqm_Processor::processingCompletedProperly()
{
  return _processingCompletedProperly;
}

void Bqm_Processor::sendProgressInformation()
{
  if (!_filterThread)
  {
    return;
  }

  float progress = _filterThread->progress();

  Q_EMIT progression(progress);
}

void Bqm_Processor::onProcessingFinished()
{
  _timer.stop();
  QString errorMessage;
  QStringList status = _filterThread->gmicStatus();

  if (_filterThread->failed())
  {
    errorMessage = _filterThread->errorMessage();

    if (errorMessage.isEmpty())
    {
      errorMessage = tr("Filter execution failed, but with no error message.");
    }
  }
  else
  {
    gmic_list<gmic_pixel_type> images = _filterThread->images();

    if (!_filterThread->aborted())
    {
      DigikamEditorGmicQtPlugin::GMicQtImageConverter::convertCImgtoDImg(images[0],
                                                                         _outImage,
                                                                         _inImage.sixteenBit());

      _processingCompletedProperly = true;
    }
  }

  _filterThread->deleteLater();
  _filterThread = nullptr;

  endApplication(errorMessage);
}

DImg Bqm_Processor::outputImage() const
{
    return _outImage;
}

void Bqm_Processor::cancel()
{
  if (_filterThread)
  {
    _filterThread->abortGmic();
  }
}

void Bqm_Processor::endApplication(const QString& errorMessage)
{
  _singleShotTimer.stop();

  Q_EMIT done(errorMessage);

  if (!errorMessage.isEmpty())
  {
    qCDebug(DIGIKAM_DPLUGIN_EDITOR_LOG) << errorMessage;
  }
}

} // namespace DigikamBqmGmicQtPlugin
