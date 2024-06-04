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

#ifndef DIGIKAM_GMIC_FILTER_NODE_H
#define DIGIKAM_GMIC_FILTER_NODE_H

// Qt includes

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QDateTime>
#include <QIODevice>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace DigikamBqmGmicQtPlugin
{

class GmicFilterNode : public QObject
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

    explicit GmicFilterNode(Type type = Root, GmicFilterNode* const parent = nullptr);
    ~GmicFilterNode()                                  override;

    bool operator==(const GmicFilterNode& other) const;

    Type type()                                  const;
    void setType(Type type);

    QList<GmicFilterNode*> children()            const;
    GmicFilterNode*        parent()              const;

    void add(GmicFilterNode* const child, int offset = -1);
    void remove(GmicFilterNode* const child);

public:

    QMap<QString, QVariant> commands;         ///< Map of filter name and filter command
    QString                title;            ///< Node title 
    QString                desc;             ///< Node description 
    QDateTime              dateAdded;        ///< Node creation date 
    bool                   expanded  = true; ///< Node expanded or not in tree-view 

private:

    // Disable
    GmicFilterNode(const GmicFilterNode&)            = delete;
    GmicFilterNode& operator=(const GmicFilterNode&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

// -----------------------------------------------------------

class GmicXmlReader : public QXmlStreamReader
{
public:

    GmicXmlReader() = default;

    GmicFilterNode* read(const QString& fileName);
    GmicFilterNode* read(QIODevice* const device, bool addRootFolder = false);

private:

    void readXml(GmicFilterNode* const parent);
    void readTitle(GmicFilterNode* const parent);
    void readDescription(GmicFilterNode* const parent);
    void readSeparator(GmicFilterNode* const parent);
    void readFolder(GmicFilterNode* const parent);
    void readGmicFilterNode(GmicFilterNode* const parent);
};

// -----------------------------------------------------------

class GmicXmlWriter : public QXmlStreamWriter
{
public:

    GmicXmlWriter();

    bool write(const QString& fileName, const GmicFilterNode* const root);
    bool write(QIODevice* const device, const GmicFilterNode* const root);

private:

    void writeItem(const GmicFilterNode* const parent);
};

} // namespace DigikamBqmGmicQtPlugin

#endif // DIGIKAM_GMIC_FILTER_NODE_H
