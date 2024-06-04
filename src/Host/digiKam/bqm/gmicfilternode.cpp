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

#include "gmicfilternode.h"

// Qt includes

#include <QPointer>
#include <QFile>
#include <QTranslator>

// Local includes

#include "digikam_debug.h"

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicFilterNode::Private
{
public:

    Private() = default;

    GmicFilterNode*        parent    = nullptr;
    Type                   type      = GmicFilterNode::Root;
    QList<GmicFilterNode*> children;
};

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

// -------------------------------------------------------

GmicFilterNode* GmicXmlReader::read(const QString& fileName)
{
    QFile file(fileName);

    if (!file.exists() || !file.open(QFile::ReadOnly))
    {
        GmicFilterNode* const root   = new GmicFilterNode(GmicFilterNode::Root);
        GmicFilterNode* const folder = new GmicFilterNode(GmicFilterNode::RootFolder, root);
        folder->title                = QObject::tr("My G'MIC Filters");

        return root;
    }

    return read(&file, true);
}

GmicFilterNode* GmicXmlReader::read(QIODevice* const device, bool addRootFolder)
{
    GmicFilterNode* const root = new GmicFilterNode(GmicFilterNode::Root);
    setDevice(device);

    if (readNextStartElement())
    {
        QString version = attributes().value(QLatin1String("version")).toString();

        if (
            (name() == QLatin1String("gmic")) &&
            (version.isEmpty() || (version == QLatin1String("2.0")))
           )
        {
            if (addRootFolder)
            {
                GmicFilterNode* const folder = new GmicFilterNode(GmicFilterNode::RootFolder, root);
                folder->title                = QObject::tr("My G'MIC Filters");
                readXml(folder);
            }
            else
            {
                readXml(root);
            }
        }
        else
        {
            raiseError(QObject::tr("The file is not an G'MIC filters database version 2.0 file."));
        }
    }

    return root;
}

void GmicXmlReader::readXml(GmicFilterNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("gmic")));

    while (readNextStartElement())
    {
        if      (name() == QLatin1String("folder"))
        {
            readFolder(parent);
        }
        else if (name() == QLatin1String("item"))
        {
            readGmicFilterNode(parent);
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

void GmicXmlReader::readFolder(GmicFilterNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("folder")));

    QPointer<GmicFilterNode> folder = new GmicFilterNode(GmicFilterNode::Folder, parent);
    folder->expanded                = (attributes().value(QLatin1String("folded")) == QLatin1String("no"));

    while (readNextStartElement())
    {
        if      (name() == QLatin1String("title"))
        {
            readTitle(folder);
        }
        else if (name() == QLatin1String("folder"))
        {
            readFolder(folder);
        }
        else if (name() == QLatin1String("item"))
        {
            readGmicFilterNode(folder);
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

void GmicXmlReader::readTitle(GmicFilterNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("title")));

    parent->title = readElementText();
}

void GmicXmlReader::readSeparator(GmicFilterNode* const parent)
{
    new GmicFilterNode(GmicFilterNode::Separator, parent);

    // empty elements have a start and end element

    readNext();
}

void GmicXmlReader::readGmicFilterNode(GmicFilterNode* const parent)
{
    Q_ASSERT(isStartElement() && (name() == QLatin1String("item")));

    GmicFilterNode* const item = new GmicFilterNode(GmicFilterNode::Item, parent);

    QStringList names          = attributes().value(QLatin1String("names")).toString().split(QLatin1Char(';'));
    QStringList filters        = attributes().value(QLatin1String("filters")).toString().split(QLatin1Char(';'));

    if (names.size() == filters.size())
    {
        QMap<QString, QVariant> map;
        int index = 0;

        foreach (const QString& cmd, filters)
        {
            map.insert(names[index], cmd);
            index++;
        }

        item->commands = map;
    }

    QString date               = attributes().value(QLatin1String("added")).toString();
    item->dateAdded            = QDateTime::fromString(date, Qt::ISODate);
    item->desc                 = attributes().value(QLatin1String("desc")).toString();

    while (readNextStartElement())
    {
        if (name() == QLatin1String("title"))
        {
            readTitle(item);
        }
        else
        {
            skipCurrentElement();
        }
    }

    if (item->title.isEmpty())
    {
        item->title = QObject::tr("Unknown item");
    }
}

// -------------------------------------------------------

GmicXmlWriter::GmicXmlWriter()
{
    setAutoFormatting(true);
}

bool GmicXmlWriter::write(const QString& fileName, const GmicFilterNode* const root)
{
    QFile file(fileName);

    if (!root || !file.open(QFile::WriteOnly))
    {
        return false;
    }

    return write(&file, root);
}

bool GmicXmlWriter::write(QIODevice* const device, const GmicFilterNode* const root)
{
    setDevice(device);

    writeStartDocument();
    writeDTD(QLatin1String("<!DOCTYPE gmic>"));
    writeStartElement(QLatin1String("gmic"));
    writeAttribute(QLatin1String("version"), QLatin1String("2.0"));

    if ((root->type() == GmicFilterNode::Root) && !root->children().isEmpty())
    {
        GmicFilterNode* const rootFolder = root->children().constFirst();

        for (int i = 0 ; i < rootFolder->children().count() ; ++i)
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

void GmicXmlWriter::writeItem(const GmicFilterNode* const parent)
{
    switch (parent->type())
    {
        case GmicFilterNode::Folder:
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

        case GmicFilterNode::Item:
        {
            writeStartElement(QLatin1String("item"));

            if (!parent->commands.isEmpty())
            {
                QStringList names = parent->commands.keys();
                QVariantList vals = parent->commands.values();
                QStringList filters;

                foreach (const QVariant& v, vals)
                {
                    filters.append(v.toString());
                }

                writeAttribute(QLatin1String("names"),   names.join(QLatin1Char(';')));
                writeAttribute(QLatin1String("filters"), filters.join(QLatin1Char(';')));
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

        case GmicFilterNode::Separator:
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

#include "moc_gmicfilternode.cpp"
