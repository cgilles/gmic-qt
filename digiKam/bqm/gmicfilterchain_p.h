/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam Batch Queue Manager plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2024 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

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
