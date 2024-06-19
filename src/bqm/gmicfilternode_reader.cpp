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

GmicFilterNode* GmicXmlReader::read(const QString& fileName, QString& currentPath)
{
    QFile file(fileName);

    if (!file.exists() || !file.open(QFile::ReadOnly))
    {
        GmicFilterNode* const root   = new GmicFilterNode(GmicFilterNode::Root);
        GmicFilterNode* const folder = new GmicFilterNode(GmicFilterNode::RootFolder, root);
        folder->title                = QObject::tr("My G'MIC Filters");

        return root;
    }

    return read(&file, true, currentPath);
}

GmicFilterNode* GmicXmlReader::read(QIODevice* const device, bool addRootFolder,
                                    QString& currentPath)
{
    GmicFilterNode* const root = new GmicFilterNode(GmicFilterNode::Root);
    setDevice(device);

    if (readNextStartElement())
    {
        QString version = attributes().value(QLatin1String("version")).toString();
        currentPath     = attributes().value(QLatin1String("currentpath")).toString()
                                      .replace(QLatin1Char('|'), QLatin1Char('/'));

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
            readItem(parent);
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
            readItem(folder);
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

void GmicXmlReader::readItem(GmicFilterNode* const parent)
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

} // namespace DigikamBqmGmicQtPlugin
