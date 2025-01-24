/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam image editor plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

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

using namespace DigikamGmicQtPluginCommon;

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
