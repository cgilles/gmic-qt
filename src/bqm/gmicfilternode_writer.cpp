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

#include "gmicfilternode_p.h"

namespace DigikamBqmGmicQtPlugin
{

GmicXmlWriter::GmicXmlWriter()
{
    setAutoFormatting(true);
}

bool GmicXmlWriter::write(const QString& fileName, const GmicFilterNode* const root,
                          const QString& currentPath)
{
    QFile file(fileName);

    if (!root || !file.open(QFile::WriteOnly))
    {
        return false;
    }

    return write(&file, root, currentPath);
}

bool GmicXmlWriter::write(QIODevice* const device, const GmicFilterNode* const root,
                          const QString& currentPath)
{
    QString cpath = currentPath;

    setDevice(device);

    writeStartDocument();
    writeDTD(QLatin1String("<!DOCTYPE gmic>"));
    writeStartElement(QLatin1String("gmic"));
    writeAttribute(QLatin1String("version"),     QLatin1String("2.0"));
    writeAttribute(QLatin1String("currentpath"), cpath.replace(QLatin1Char('/'), QLatin1Char('|')));

    if ((root->type() == GmicFilterNode::Root) && !root->children().isEmpty())
    {
        const GmicFilterNode* const rootFolder = root->children().constFirst();

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

                for (const QVariant& v : qAsConst(vals))
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
