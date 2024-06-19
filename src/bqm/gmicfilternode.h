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

#pragma once

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
    QString                 title;            ///< Node title
    QString                 desc;             ///< Node description
    QDateTime               dateAdded;        ///< Node creation date
    bool                    expanded  = true; ///< Node expanded or not in tree-view 

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

    GmicFilterNode* read(const QString& fileName, QString& currentPath);
    GmicFilterNode* read(QIODevice* const device, bool addRootFolder, QString& currentPath);

private:

    void readXml(GmicFilterNode* const parent);
    void readTitle(GmicFilterNode* const parent);
    void readDescription(GmicFilterNode* const parent);
    void readSeparator(GmicFilterNode* const parent);
    void readFolder(GmicFilterNode* const parent);
    void readItem(GmicFilterNode* const parent);
};

// -----------------------------------------------------------

class GmicXmlWriter : public QXmlStreamWriter
{
public:

    GmicXmlWriter();

    bool write(const QString& fileName, const GmicFilterNode* const root,
               const QString& currentPath);

    bool write(QIODevice* const device, const GmicFilterNode* const root,
               const QString& currentPath);

private:

    void writeItem(const GmicFilterNode* const parent);
};

} // namespace DigikamBqmGmicQtPlugin
