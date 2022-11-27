/*
*  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
*  editors, offering hundreds of filters thanks to the underlying G'MIC
*  image processing framework.
*
*  Copyright (C) 2019-2022 Gilles Caulier <caulier dot gilles at gmail dot com>
*
*  Description: digiKam image editor plugin for GmicQt.
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

#ifndef DIGIKAM_GMICQT_WINDOW_H
#define DIGIKAM_GMICQT_WINDOW_H

// Qt includes

#include <QString>
#include <QCloseEvent>
#include <QShowEvent>
#include <QWidget>

// Local includes

#include "MainWindow.h"

namespace DigikamEditorGmicQtPlugin
{

class GMicQtWindow : public GmicQt::MainWindow
{
public:

    GMicQtWindow(QWidget* const parent);
    ~GMicQtWindow();

    void saveParameters();

protected:

    void showEvent(QShowEvent* event)   override;
    void closeEvent(QCloseEvent* event) override;

private:

    QString m_hostName;
    QString m_hostOrg;
    QString m_hostDom;
    QString m_plugName;
    QString m_plugOrg;
    QString m_plugDom;
};

} // namespace DigikamEditorGmicQtPlugin

#endif // DIGIKAM_GMICQT_WINDOW_H
