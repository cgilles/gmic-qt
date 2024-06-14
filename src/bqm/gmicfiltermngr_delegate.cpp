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

#include "gmicfiltermngr_p.h"

namespace DigikamBqmGmicQtPlugin
{

GmicFilterDelegate::GmicFilterDelegate(TreeProxyModel* const pmodel)
    : QStyledItemDelegate(pmodel)
{
}

void GmicFilterDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                               const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    if (index.isValid())
    {
        const TreeProxyModel* const pmodel  = dynamic_cast<const TreeProxyModel*>(parent());

        if (!pmodel)
        {
            return;
        }

        const GmicFilterModel* const smodel = dynamic_cast<const GmicFilterModel*>(pmodel->sourceModel());

        if (!smodel)
        {
            return;
        }

        QModelIndex idx                     = pmodel->mapToSource(index);

        if (idx.isValid())
        {
            const GmicFilterNode* const commandNode = smodel->node(idx);

            if (commandNode && (commandNode->type() == GmicFilterNode::Separator))
            {
                QStyleOptionFrame frameOption;
                frameOption.rect         = option.rect;
                frameOption.features     = QStyleOptionFrame::Flat;
                frameOption.frameShape   = QFrame::HLine;
                frameOption.lineWidth    = 1;
                frameOption.midLineWidth = 0;

                QApplication::style()->drawControl(QStyle::CE_ShapedFrame,
                                                   &frameOption, painter);
            }
        }
    }
}

} // namespace DigikamBqmGmicQtPlugin
