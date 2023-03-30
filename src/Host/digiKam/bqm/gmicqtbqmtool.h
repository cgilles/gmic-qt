/*
*  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
*  editors, offering hundreds of filters thanks to the underlying G'MIC
*  image processing framework.
*
*  Copyright (C) 2019-2023 Gilles Caulier <caulier dot gilles at gmail dot com>
*
*  Description: digiKam Batch Queue Manager plugin for GmicQt.
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

#ifndef DIGIKAM_BQM_GMICQT_TOOL_H
#define DIGIKAM_BQM_GMICQT_TOOL_H

// Local includes

#include "batchtool.h"

using namespace Digikam;

namespace GmicQt
{
    class HeadlessProcessor;
}

namespace DigikamBqmGmicQtPlugin
{

class Bqm_Widget;

class GmicQtBqmTool : public BatchTool
{
    Q_OBJECT

public:

    explicit GmicQtBqmTool(QObject* const parent = nullptr);
    ~GmicQtBqmTool()                                        override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

    void cancel()                                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    Bqm_Widget* m_gmicWidget         = nullptr;
    HeadlessProcess* m_gmicProcessor = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin

#endif // DIGIKAM_BQM_GMICQT_TOOL_H
