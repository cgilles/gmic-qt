/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
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

// digiKam includes

#include "dplugin.h"

// Local includes

#include "MainWindow.h"
#include "GmicQt.h"

using namespace GmicQt;
using namespace Digikam;

namespace DigikamEditorGmicQtPlugin
{

class GMicQtWindow : public MainWindow
{
    Q_OBJECT

public:

    explicit GMicQtWindow(DPlugin* const tool, QWidget* const parent);
    ~GMicQtWindow()                     override;

    void saveParameters();
    void setCommandSelector();

    static void execWindow(DPlugin* const tool,
                           const QString& prm = QString(),
                           bool  viewer = false);

protected:

    void showEvent(QShowEvent* event)   override;
    void closeEvent(QCloseEvent* event) override;

private Q_SLOTS:

    void slotAboutPlugin();
    void slotOnlineHandbook();
    void slotOkClicked();
    void slotLayersDialog();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorGmicQtPlugin

#endif // DIGIKAM_GMICQT_WINDOW_H
