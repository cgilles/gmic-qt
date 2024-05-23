/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "gmiccommandnode.h"

// Qt includes

#include <QPointer>
#include <QFile>
#include <QTranslator>

// Local includes

#include "digikam_debug.h"

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicCommandNode::Private
{
public:

    Private() = default;

    GmicCommandNode*        parent     = nullptr;
    Type                    type       = GmicCommandNode::Root;
    QList<GmicCommandNode*> children;
};

GmicCommandNode::GmicCommandNode(GmicCommandNode::Type type, GmicCommandNode* const parent)
    : QObject(nullptr),
      d      (new Private)
{
    expanded  = false;
    d->parent = parent;
    d->type   = type;

    if (parent)
    {
        parent->add(this);
    }
}

GmicCommandNode::~GmicCommandNode()
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

bool GmicCommandNode::operator==(const GmicCommandNode& other) const
{
    if (
        (command             != other.command)       ||
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

GmicCommandNode::Type GmicCommandNode::type() const
{
    return d->type;
}

void GmicCommandNode::setType(Type type)
{
    d->type = type;
}

QList<GmicCommandNode*> GmicCommandNode::children() const
{
    return d->children;
}

GmicCommandNode* GmicCommandNode::parent() const
{
    return d->parent;
}

void GmicCommandNode::add(GmicCommandNode* const child, int offset)
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

void GmicCommandNode::remove(GmicCommandNode* const child)
{
    child->d->parent = nullptr;
    d->children.removeAll(child);
}

// -------------------------------------------------------

GmicCommandNode* XbelReader::read(const QString& fileName)
{
    QFile file(fileName);

    if (!file.exists() || !file.open(QFile::ReadOnly))
    {
        GmicCommandNode* const root   = new GmicCommandNode(GmicCommandNode::Root);
        GmicCommandNode* const folder = new GmicCommandNode(GmicCommandNode::RootFolder, root);
        folder->title                 = QObject::tr("My Gmic Filters");

        return root;
    }

    return read(&file, true);
}

GmicCommandNode* XbelReader::read(QIODevice* const device, bool addRootFolder)
{
    GmicCommandNode* const root = new GmicCommandNode(GmicCommandNode::Root);
    setDevice(device);

    if (readNextStartElement())
    {
        QString version = attributes().value(QLatin1String("version")).toString();

        if (
            (name() == QLatin1String("xbel")) &&
            (version.isEmpty() || (version == QLatin1String("1.0")))
           )
        {
            if (addRootFolder)
            {
                GmicCommandNode* const folder = new GmicCommandNode(GmicCommandNode::RootFolder, root);
                folder->title                 = QObject::tr("My Gmic Filters");
                readXBEL(folder);
            }
            else
            {
                readXBEL(root);
            }
        }
        else
        {
            raiseError(QObject::tr("The file is not an XBEL version 1.0 file."));
        }
    }

    return root;
}

void XbelReader::readXBEL(GmicCommandNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("xbel")));

    while (readNextStartElement())
    {
        if      (name() == QLatin1String("folder"))
        {
            readFolder(parent);
        }
        else if (name() == QLatin1String("item"))
        {
            readGmicCommandNode(parent);
        }
        else if (name() == QLatin1String("separator"))
        {
            readSeparator(parent);
        }
        else
        {
            skipCurrentElement();
        }
    }
}

void XbelReader::readFolder(GmicCommandNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("folder")));

    QPointer<GmicCommandNode> folder = new GmicCommandNode(GmicCommandNode::Folder, parent);
    folder->expanded                 = (attributes().value(QLatin1String("folded")) == QLatin1String("no"));

    while (readNextStartElement())
    {
        if      (name() == QLatin1String("title"))
        {
            readTitle(folder);
        }
        else if (name() == QLatin1String("desc"))
        {
            readDescription(folder);
        }
        else if (name() == QLatin1String("folder"))
        {
            readFolder(folder);
        }
        else if (name() == QLatin1String("item"))
        {
            readGmicCommandNode(folder);
        }
        else if (name() == QLatin1String("separator"))
        {
            readSeparator(folder);
        }
        else
        {
            skipCurrentElement();
        }
    }
}

void XbelReader::readTitle(GmicCommandNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("title")));

    parent->title = readElementText();
}

void XbelReader::readDescription(GmicCommandNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("desc")));

    parent->desc = readElementText();
}

void XbelReader::readSeparator(GmicCommandNode* const parent)
{
    new GmicCommandNode(GmicCommandNode::Separator, parent);

    // empty elements have a start and end element

    readNext();
}

void XbelReader::readGmicCommandNode(GmicCommandNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("item")));

    GmicCommandNode* const item = new GmicCommandNode(GmicCommandNode::Item, parent);
    item->command               = attributes().value(QLatin1String("command")).toString();
    QString date                = attributes().value(QLatin1String("added")).toString();
    item->dateAdded             = QDateTime::fromString(date, Qt::ISODate);

    while (readNextStartElement())
    {
        if      (name() == QLatin1String("title"))
        {
            readTitle(item);
        }
        else if (name() == QLatin1String("desc"))
        {
            readDescription(item);
        }
        else
        {
            skipCurrentElement();
        }
    }

    if (item->title.isEmpty())
    {
        item->title = QObject::tr("Unknown title");
    }
}

// -------------------------------------------------------

XbelWriter::XbelWriter()
{
    setAutoFormatting(true);
}

bool XbelWriter::write(const QString& fileName, const GmicCommandNode* const root)
{
    QFile file(fileName);

    if (!root || !file.open(QFile::WriteOnly))
    {
        return false;
    }

    return write(&file, root);
}

bool XbelWriter::write(QIODevice* const device, const GmicCommandNode* const root)
{
    setDevice(device);

    writeStartDocument();
    writeDTD(QLatin1String("<!DOCTYPE xbel>"));
    writeStartElement(QLatin1String("xbel"));
    writeAttribute(QLatin1String("version"), QLatin1String("1.0"));

    if (root->type() == GmicCommandNode::Root)
    {
        GmicCommandNode* const rootFolder = root->children().constFirst();

        for (int i = 0  ; i < rootFolder->children().count() ; ++i)
        {
            writeItem(rootFolder->children().at(i));
        }
    }
    else
    {
        writeItem(root);
    }

    writeEndDocument();

    return true;
}

void XbelWriter::writeItem(const GmicCommandNode* const parent)
{
    switch (parent->type())
    {
        case GmicCommandNode::Folder:
        {
            writeStartElement(QLatin1String("folder"));
            writeAttribute(QLatin1String("folded"), parent->expanded ? QLatin1String("no") : QLatin1String("yes"));
            writeTextElement(QLatin1String("title"), parent->title);

            for (int i = 0 ; i < parent->children().count() ; ++i)
            {
                writeItem(parent->children().at(i));
            }

            writeEndElement();
            break;
        }

        case GmicCommandNode::Item:
        {
            writeStartElement(QLatin1String("item"));

            if (!parent->command.isEmpty())
            {
                writeAttribute(QLatin1String("command"), parent->command);
            }

            if (parent->dateAdded.isValid())
            {
                writeAttribute(QLatin1String("added"), parent->dateAdded.toString(Qt::ISODate));
            }

            if (!parent->desc.isEmpty())
            {
                writeAttribute(QLatin1String("desc"), parent->desc);
            }

            writeTextElement(QLatin1String("title"), parent->title);

            writeEndElement();
            break;
        }

        case GmicCommandNode::Separator:
        {
            writeEmptyElement(QLatin1String("separator"));
            break;
        }

        default:
        {
            break;
        }
    }
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmiccommandnode.cpp"
