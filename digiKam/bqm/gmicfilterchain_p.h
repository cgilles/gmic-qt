/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 *  Description: digiKam Batch Queue Manager plugin for Gmic-Qt.
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

#pragma once

#include "gmicfilterchain.h"

// Qt includes

#include <QMetaMethod>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QMimeData>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QToolButton>
#include <QUrl>
#include <QTimer>
#include <QFile>
#include <QPointer>
#include <QXmlStreamAttributes>
#include <QString>
#include <QStandardPaths>
#include <QIcon>
#include <QApplication>
#include <QStyle>
#include <QAction>
#include <QMessageBox>

// Local includes

#include "digikam_debug.h"

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicFilterChainViewItem::Private
{
public:

    Private() = default;

    int                         index    = -1;
    QString                     title;
    QString                     command;
    GmicFilterChainView*        view     = nullptr;
};

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN CtrlButton : public QToolButton
{
    Q_OBJECT

public:

    explicit CtrlButton(
                        const QIcon& icon,
                        const QString& tip,
                        QWidget* const parent,
                        const char* method
                       )
       : QToolButton(parent)
    {
        setDefaultAction(new QAction(icon, tip));

        connect(this, SIGNAL(triggered(QAction*)),
                parent, method);
    }

    ~CtrlButton() override = default;
};

// ------------------------------------------------------------------------

class Q_DECL_HIDDEN GmicFilterChain::Private
{
public:

    Private() = default;

    CtrlButton*                      editButton              = nullptr;
    CtrlButton*                      moveUpButton            = nullptr;
    CtrlButton*                      moveDownButton          = nullptr;
    CtrlButton*                      addButton               = nullptr;
    CtrlButton*                      removeButton            = nullptr;
    CtrlButton*                      clearButton             = nullptr;

    GmicFilterChainView*             listView                = nullptr;

    GmicFilterChainIsLessThanHandler isLessThan;
};

} // namespace DigikamBqmGmicQtPlugin