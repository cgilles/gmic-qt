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

#include "gmicfilternode.h"

// Qt includes

#include <QPointer>
#include <QFile>
#include <QTranslator>

// Local includes

#include "digikam_debug.h"

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicFilterNode::Private
{
public:

    Private() = default;

    GmicFilterNode*        parent    = nullptr;
    Type                   type      = GmicFilterNode::Root;
    QList<GmicFilterNode*> children;
};

} // namespace DigikamBqmGmicQtPlugin
