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

// Qt includes

#include <QMap>
#include <QString>
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

class GmicFilterWidget : public QWidget
{
    Q_OBJECT

public:

    explicit GmicFilterWidget(QWidget* const parent = nullptr);
    ~GmicFilterWidget()                                   override;

    void setPlugin(DPluginBqm* const plugin);

    QString currentPath()                           const;
    void setCurrentPath(const QString& path);

    QString currentGmicChainedCommands()            const;

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotCustomContextMenuRequested(const QPoint&);
    void slotTreeViewItemClicked(const QModelIndex&);
    void slotRemove();
    void slotAddFilter();
    void slotAddFolder();
    void slotAddSeparator();
    void slotEdit();

private:

    void expandNodes(GmicFilterNode* const node);
    bool saveExpandedNodes(const QModelIndex& parent);
    void readSettings();
    void saveSettings();
    void openPropertiesDialog(bool editMode, bool isFilter);
    QMap<QString, QVariant> currentGmicFilters()    const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin
