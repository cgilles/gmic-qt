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

// Qt includes

#include <QWidget>
#include <QDialog>
#include <QTreeView>
#include <QComboBox>
#include <QAbstractItemModel>

// digiKam includes

#include "dpluginbqm.h"

// Local includes

#include "gmicfiltermngr.h"

using namespace Digikam;

namespace DigikamBqmGmicQtPlugin
{

class GmicFilterDialog : public QDialog
{
    Q_OBJECT

public:

    explicit GmicFilterDialog(GmicFilterNode* const citem,
                              bool editMode, bool isFilter,
                              QWidget* const parent,
                              GmicFilterManager* const mngr,
                              DPluginBqm* const plugin);
    ~GmicFilterDialog()                                     override;

private Q_SLOTS:

    void accept()                                           override;
    void slotGmicQt(const QString& command = QString());

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin
