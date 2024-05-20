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

#include "gmicqtwindow.h"

// Qt includes

#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QIcon>
#include <QUrl>
#include <QLabel>
#include <QAction>
#include <QPointer>
#include <QDesktopServices>

// digiKam includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dpluginaboutdlg.h"

namespace DigikamEditorGmicQtPlugin
{

GMicQtWindow::GMicQtWindow(DPlugin*const tool, QWidget* const parent)
    : GmicQt::MainWindow(parent),
      m_tool            (tool)
{
    m_hostOrg               = QCoreApplication::organizationName();
    m_hostDom               = QCoreApplication::organizationDomain();
    m_hostName              = QCoreApplication::applicationName();

    QHBoxLayout* const hlay = findChild<QHBoxLayout*>("horizontalLayout");

    if (hlay)
    {
        QPushButton* const help    = new QPushButton(this);
        help->setText(tr("Help"));
        help->setIcon(QIcon::fromTheme(QLatin1String("help-browser")));
        help->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        QMenu* const menu          = new QMenu(help);
        QAction* const webAction   = menu->addAction(QIcon::fromTheme(QLatin1String("globe")),
                                                     tr("Online Handbook..."));
        QAction* const aboutAction = menu->addAction(QIcon::fromTheme(QLatin1String("help-about")),
                                                     tr("About..."));
        help->setMenu(menu);

        connect(webAction, SIGNAL(triggered()),
                this, SLOT(slotOpenWebSite()));

        connect(aboutAction, SIGNAL(triggered()),
                this, SLOT(slotAboutPlugin()));

        hlay->insertWidget(0, help);

        QLabel* const lbl          = findChild<QLabel*>("messageLabel");

        if (lbl)
        {
            hlay->setStretchFactor(lbl, 10);
        }
    }
}

void GMicQtWindow::slotAboutPlugin()
{
    QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(m_tool);
    dlg->exec();
    delete dlg;
}

void GMicQtWindow::slotOpenWebSite()
{
    openOnlineDocumentation(
                            m_tool->handbookSection(),
                            m_tool->handbookChapter(),
                            m_tool->handbookReference()
                           );
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
