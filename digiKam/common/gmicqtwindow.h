/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : common implementations for the digiKam plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2024 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QCloseEvent>
#include <QShowEvent>
#include <QWidget>

// digiKam includes

#include "dplugin.h"
#include "dinfointerface.h"

// Local includes

#include "MainWindow.h"
#include "GmicQt.h"

using namespace GmicQt;
using namespace Digikam;

namespace DigikamGmicQtPluginCommon
{

class GMicQtWindow : public MainWindow
{
    Q_OBJECT

public:

    enum HostType
    {
        ImageEditor = 0,
        BQM,
        Showfoto,
        Unknow
    };

public:

    explicit GMicQtWindow(
                          DPlugin* const tool,
                          QWidget* const parent,
                          QString* const filterName
                         );
    ~GMicQtWindow()                     override;

    void saveParameters();
    void setFilterSelectionMode();
    void setHostType(HostType type);

    static QString execWindow(DPlugin* const tool,
                              HostType type,
                              const QString& prm = QString());

protected:

    void showEvent(QShowEvent* event)   override;
    void closeEvent(QCloseEvent* event) override;

private Q_SLOTS:

    void slotOkClicked();
    void slotLayersDialog();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGmicQtPluginCommon
