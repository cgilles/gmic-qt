/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file ConstParameter.cpp
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
#include "FilterParameters/ConstParameter.h"
#include <QString>
#include <QStringList>
#include "Common.h"
#include "FilterTextTranslator.h"
#include "HtmlTranslator.h"
#include "Misc.h"

namespace GmicQt
{

ConstParameter::ConstParameter(QObject * parent) : AbstractParameter(parent) {}

int ConstParameter::size() const
{
  return 1;
}

ConstParameter::~ConstParameter() = default;

bool ConstParameter::addTo(QWidget *, int)
{
  return false;
}

bool ConstParameter::isQuoted() const
{
  return true;
}

QString ConstParameter::value() const
{
  return _value;
}

QString ConstParameter::defaultValue() const
{
  return _default;
}

void ConstParameter::setValue(const QString & value)
{
  _value = value;
}

void ConstParameter::reset()
{
  _value = _default;
}

bool ConstParameter::initFromText(const QString & filterName, const char * text, int & textLength)
{
  QStringList list = parseText("value", text, textLength);
  if (list.isEmpty()) {
    return false;
  }
  _name = HtmlTranslator::html2txt(FilterTextTranslator::translate(list[0], filterName));
  _value = _default = unescaped(unquoted(list[1]));
  return true;
}

} // namespace GmicQt
