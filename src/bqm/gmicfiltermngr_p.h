/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam Batch Queue Manager plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

#include "gmicfiltermngr.h"

// Qt includes

#include <QBuffer>
#include <QFile>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QIcon>
#include <QMap>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QDebug>
#include <QApplication>
#include <QFileDialog>

// digiKam includes

#include "digikam_debug.h"
#include "ditemtooltip.h"
#include "dexpanderbox.h"

// Local includes

#include "gmicfilternode.h"
#include "gmicfiltermodel.h"
#include "gmicqtcommon.h"

using namespace Digikam;
using namespace DigikamGmicQtPluginCommon;

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN ChangeGmicFilter::Private
{
public:

    Private() = default;

public:

    GmicFilterManager* manager   = nullptr;
    GmicFilterData     type      = Command;
    QVariant           oldValue;
    QVariant           newValue;
    GmicFilterNode*    node      = nullptr;
};

// --------------------------------------------------------------

class Q_DECL_HIDDEN GmicFilterManager::Private
{
public:

    Private() = default;

public:

    bool             loaded             = false;
    GmicFilterNode*  commandRootNode    = nullptr;
    GmicFilterModel* commandModel       = nullptr;
    QUndoStack       commands;
    QString          commandsFile;
    QString          currentPath;
};

} // namespace DigikamBqmGmicQtPlugin
