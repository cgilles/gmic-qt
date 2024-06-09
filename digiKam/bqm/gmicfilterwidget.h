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
