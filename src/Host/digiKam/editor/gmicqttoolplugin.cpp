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

#include "gmicqttoolplugin.h"

// Qt includes

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QEventLoop>
#include <QPointer>
#include <QImage>
#include <QBuffer>
#include <QByteArray>

// Libfftw includes

#ifdef cimg_use_fftw3
#   include <fftw3.h>
#endif

// Local includes

#include "gmicqtcommon.h"
#include "gmicqtwindow.h"
#include "gmic.h"

namespace DigikamEditorGmicQtPlugin
{

GmicQtToolPlugin::GmicQtToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

QString GmicQtToolPlugin::name() const
{
    return QString::fromUtf8("GmicQt");
}

QString GmicQtToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon GmicQtToolPlugin::icon() const
{
    return s_gmicQtPluginIcon();
}

QString GmicQtToolPlugin::description() const
{
    return tr("A tool for G'MIC-Qt");
}

QString GmicQtToolPlugin::details() const
{
    return s_gmicQtPluginDetails(tr("An Image Editor tool for G'MIC-Qt."));
}

QList<DPluginAuthor> GmicQtToolPlugin::authors() const
{
    return s_gmicQtPluginAuthors();
}

QString GmicQtToolPlugin::handbookSection() const
{
    return QLatin1String("image_editor");
}

QString GmicQtToolPlugin::handbookChapter() const
{
    return QLatin1String("enhancement_tools");
}

QString GmicQtToolPlugin::handbookReference() const
{
    return QLatin1String("enhance-gmicqt");
}

void GmicQtToolPlugin::setup(QObject* const parent)
{
    m_action = new DPluginAction(parent);
    m_action->setIcon(icon());
    m_action->setText(tr("G'MIC-Qt..."));
    m_action->setObjectName(QLatin1String("editorwindow_gmicqt"));
    m_action->setActionCategory(DPluginAction::EditorEnhance);

    connect(m_action, SIGNAL(triggered(bool)),
            this, SLOT(slotGmicQt()));

    addAction(m_action);
}

void GmicQtToolPlugin::slotGmicQt()
{
    GMicQtWindow::HostType type = GMicQtWindow::ImageEditor;

    if (qApp->applicationName() == QLatin1String("showfoto"))
    {
        type = GMicQtWindow::Showfoto;
    }

    GMicQtWindow::execWindow(this, type);
}

} // namespace DigikamEditorGmicQtPlugin

#include "moc_gmicqttoolplugin.cpp"
