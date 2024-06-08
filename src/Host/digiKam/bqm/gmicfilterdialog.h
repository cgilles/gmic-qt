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
                              bool edit, bool filter,
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
