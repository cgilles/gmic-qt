/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file LanguageSelectionWidget.cpp
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
#include "Widgets/LanguageSelectionWidget.h"
#include <QDebug>
#include <QSettings>
#include <QStringList>
#include "Common.h"
#include "LanguageSettings.h"
#include "ui_languageselectionwidget.h"

LanguageSelectionWidget::LanguageSelectionWidget(QWidget * parent) //
    : QWidget(parent),                                             //
      ui(new Ui::LanguageSelectionWidget),                         //
      _code2name(LanguageSettings::availableLanguages())
{
  ui->setupUi(this);
  QMap<QString, QString>::const_iterator it = _code2name.begin();
  while (it != _code2name.end()) {
    ui->comboBox->addItem(*it, QVariant(it.key()));
    ++it;
  }

  QString lang = LanguageSettings::systemDefaultAndAvailableLanguageCode();
  _systemDefaultIsAvailable = !lang.isEmpty();
  if (_systemDefaultIsAvailable) {
    ui->comboBox->insertItem(0, QString(tr("System default (%1)")).arg(_code2name[lang]), QVariant(QString()));
  }
}

LanguageSelectionWidget::~LanguageSelectionWidget()
{
  delete ui;
}

QString LanguageSelectionWidget::selectedLanguageCode()
{
  return ui->comboBox->currentData().toString();
}

void LanguageSelectionWidget::selectLanguage(const QString & code)
{
  int count = ui->comboBox->count();
  QString lang;
  // Empty code means "system default"
  if (code.isEmpty()) {
    if (_systemDefaultIsAvailable) {
      ui->comboBox->setCurrentIndex(0);
      return;
    }
    lang = "en";
  } else {
    if (_code2name.find(code) != _code2name.end()) {
      lang = code;
    } else {
      lang = "en";
    }
  }
  for (int i = _systemDefaultIsAvailable ? 1 : 0; i < count; ++i) {
    if (ui->comboBox->itemData(i).toString() == lang) {
      ui->comboBox->setCurrentIndex(i);
      return;
    }
  }
}
