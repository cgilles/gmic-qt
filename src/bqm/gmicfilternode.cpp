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

#include "gmicfilternode_p.h"

namespace DigikamBqmGmicQtPlugin
{

GmicFilterNode::GmicFilterNode(GmicFilterNode::Type type, GmicFilterNode* const parent)
    : QObject(nullptr),
      d      (new Private)
{
    d->parent = parent;
    d->type   = type;

    if (parent)
    {
        parent->add(this);
    }
}

GmicFilterNode::~GmicFilterNode()
{
    if (d->parent)
    {
        d->parent->remove(this);
    }

    while (d->children.size())
    {
        delete d->children.takeFirst();
    }

    delete d;
}

bool GmicFilterNode::operator==(const GmicFilterNode& other) const
{
    if (
        (commands            != other.commands)      ||
        (title               != other.title)         ||
        (desc                != other.desc)          ||
        (expanded            != other.expanded)      ||
        (dateAdded           != other.dateAdded)     ||
        (d->type             != other.d->type)       ||
        (d->children.count() != other.d->children.count())
       )
    {
        return false;
    }

    for (int i = 0 ; i < d->children.count() ; ++i)
    {
        if (!((*(d->children[i])) == (*(other.d->children[i]))))
        {
            return false;
        }
    }

    return true;
}

GmicFilterNode::Type GmicFilterNode::type() const
{
    return d->type;
}

void GmicFilterNode::setType(Type type)
{
    d->type = type;
}

QList<GmicFilterNode*> GmicFilterNode::children() const
{
    return d->children;
}

GmicFilterNode* GmicFilterNode::parent() const
{
    return d->parent;
}

void GmicFilterNode::add(GmicFilterNode* const child, int offset)
{
    Q_ASSERT(child->d->type != Root);

    if (child->d->parent)
    {
        child->d->parent->remove(child);
    }

    child->d->parent = this;

    if (offset == -1)
    {
        offset = d->children.size();
    }

    d->children.insert(offset, child);
}

void GmicFilterNode::remove(GmicFilterNode* const child)
{
    child->d->parent = nullptr;
    d->children.removeAll(child);
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfilternode.cpp"
