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

#ifndef DIGIKAM_GMIC_COMMAND_NODE_H
#define DIGIKAM_GMIC_COMMAND_NODE_H

// Qt includes

#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QIODevice>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace DigikamBqmGmicQtPlugin
{

class GmicCommandNode : public QObject
{
    Q_OBJECT

public:

    enum Type
    {
        Root,
        Folder,
        Item,
        Separator,
        RootFolder
    };

public:

    explicit GmicCommandNode(Type type = Root, GmicCommandNode* const parent = nullptr);
    ~GmicCommandNode() override;

    bool operator==(const GmicCommandNode& other) const;

    Type type()                                   const;
    void setType(Type type);

    QList<GmicCommandNode*> children()            const;
    GmicCommandNode*        parent()              const;

    void add(GmicCommandNode* const child, int offset = -1);
    void remove(GmicCommandNode* const child);

public:

    QString   command;
    QString   title;
    QString   desc;
    QDateTime dateAdded;
    bool      expanded  = false;

private:

    // Disable
    GmicCommandNode(const GmicCommandNode&)            = delete;
    GmicCommandNode& operator=(const GmicCommandNode&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------

class XbelReader : public QXmlStreamReader
{
public:

    XbelReader() = default;

    GmicCommandNode* read(const QString& fileName);
    GmicCommandNode* read(QIODevice* const device, bool addRootFolder = false);

private:

    void readXBEL(GmicCommandNode* const parent);
    void readTitle(GmicCommandNode* const parent);
    void readDescription(GmicCommandNode* const parent);
    void readSeparator(GmicCommandNode* const parent);
    void readFolder(GmicCommandNode* const parent);
    void readGmicCommandNode(GmicCommandNode* const parent);
};

// -----------------------------------------------------------

class XbelWriter : public QXmlStreamWriter
{
public:

    XbelWriter();

    bool write(const QString& fileName, const GmicCommandNode* const root);
    bool write(QIODevice* const device, const GmicCommandNode* const root);

private:

    void writeItem(const GmicCommandNode* const parent);
};

} // namespace DigikamBqmGmicQtPlugin

#endif // DIGIKAM_GMIC_COMMAND_NODE_H
