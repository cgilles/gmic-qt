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

#include "gmicfiltermngr_p.h"

namespace DigikamBqmGmicQtPlugin
{

RemoveGmicFilter::RemoveGmicFilter(GmicFilterManager* const mngr,
                                   GmicFilterNode* const parent,
                                   int row)
    : QUndoCommand(QObject::tr("Remove Filter")),
      m_row       (row),
      m_manager   (mngr),
      m_node      (parent->children().value(row)),
      m_parent    (parent)
{
}

RemoveGmicFilter::~RemoveGmicFilter()
{
    if (m_done && !m_node->parent())
    {
        delete m_node;
    }
}

void RemoveGmicFilter::undo()
{
    m_parent->add(m_node, m_row);

    Q_EMIT m_manager->signalEntryAdded(m_node);

    m_done = false;
}

void RemoveGmicFilter::redo()
{
    m_parent->remove(m_node);

    Q_EMIT m_manager->signalEntryRemoved(m_parent, m_row, m_node);

    m_done = true;
}

// --------------------------------------------------------------

InsertGmicFilter::InsertGmicFilter(GmicFilterManager* const mngr,
                                   GmicFilterNode* const parent,
                                   GmicFilterNode* const node,
                                   int row)
    : RemoveGmicFilter(mngr, parent, row)
{
    setText(QObject::tr("Insert Filter"));
    m_node = node;
}

void InsertGmicFilter::undo()
{
    RemoveGmicFilter::redo();
}

void InsertGmicFilter::redo()
{
    RemoveGmicFilter::undo();
}

// --------------------------------------------------------------

ChangeGmicFilter::ChangeGmicFilter(GmicFilterManager* const mngr,
                                   GmicFilterNode* const node,
                                   const QVariant& newValue,
                                   GmicFilterData type)
    : QUndoCommand(),
      d           (new Private)
{
    d->manager  = mngr;
    d->type     = type;
    d->newValue = newValue;
    d->node     = node;

    switch (d->type)
    {
        case Title:
        {
            d->oldValue = d->node->title;
            setText(QObject::tr("Title Change"));
            break;
        }

        case Desc:
        {
            d->oldValue = d->node->desc;
            setText(QObject::tr("Comment Change"));
            break;
        }

        default:    // Gmic Commands
        {
            d->oldValue = d->node->commands;
            setText(QObject::tr("Commands Change"));
            break;
        }
    }
}

ChangeGmicFilter::~ChangeGmicFilter()
{
    delete d;
}

void ChangeGmicFilter::undo()
{
    switch (d->type)
    {
        case Title:
        {
            d->node->title    = d->oldValue.toString();
            break;
        }

        case Desc:
        {
            d->node->desc     = d->oldValue.toString();
            break;
        }

        default:    // Gmic Command
        {
            d->node->commands = d->oldValue.toMap();
            break;
        }
    }

    Q_EMIT d->manager->signalEntryChanged(d->node);
}

void ChangeGmicFilter::redo()
{
    switch (d->type)
    {
        case Title:
        {
            d->node->title    = d->newValue.toString();
            break;
        }

        case Desc:
        {
            d->node->desc     = d->newValue.toString();
            break;
        }

        default:    // Gmic Command
        {
            d->node->commands = d->newValue.toMap();
            break;
        }
    }

    Q_EMIT d->manager->signalEntryChanged(d->node);
}

} // namespace DigikamBqmGmicQtPlugin
