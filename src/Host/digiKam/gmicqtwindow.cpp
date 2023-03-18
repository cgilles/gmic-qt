/*
*  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
*  editors, offering hundreds of filters thanks to the underlying G'MIC
*  image processing framework.
*
*  Copyright (C) 2019-2023 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "gmicqtwindow.h"

// Qt includes

#include <QApplication>

namespace DigikamEditorGmicQtPlugin
{

GMicQtWindow::GMicQtWindow(QWidget* const parent)
    : GmicQt::MainWindow(parent)
{
    m_hostOrg  = QCoreApplication::organizationName();
    m_hostDom  = QCoreApplication::organizationDomain();
    m_hostName = QCoreApplication::applicationName();
}

GMicQtWindow::~GMicQtWindow()
{
}

void GMicQtWindow::saveParameters()
{
    saveSettings();
}

void GMicQtWindow::showEvent(QShowEvent* event)
{
    if (m_plugOrg.isEmpty())
    {
        m_plugOrg  = QCoreApplication::organizationName();
    }

    if (m_plugDom.isEmpty())
    {
        m_plugDom  = QCoreApplication::organizationDomain();
    }

    if (m_plugName.isEmpty())
    {
        m_plugName = QCoreApplication::applicationName();
    }

    QCoreApplication::setOrganizationName(m_plugOrg);
    QCoreApplication::setOrganizationDomain(m_plugDom);
    QCoreApplication::setApplicationName(m_plugName);

    QWidget::showEvent(event);
}

void GMicQtWindow::closeEvent(QCloseEvent* event)
{
    QCoreApplication::setOrganizationName(m_hostOrg);
    QCoreApplication::setOrganizationDomain(m_hostDom);
    QCoreApplication::setApplicationName(m_hostName);
    QWidget::closeEvent(event);
}

} // namespace DigikamEditorGmicQtPlugin
