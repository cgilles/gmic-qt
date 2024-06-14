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

GmicFilterManager::GmicFilterManager(const QString& file, QObject* const parent)
    : QObject(parent),
      d      (new Private)
{
    d->commandsFile = file;
    load();
}

GmicFilterManager::~GmicFilterManager()
{
    delete d->commandRootNode;
    delete d;
}

void GmicFilterManager::changeExpanded()
{
}

void GmicFilterManager::load()
{
    if (d->loaded)
    {
        return;
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Loading G'MIC filters from" << d->commandsFile;
    d->loaded = true;

    GmicXmlReader reader;
    d->commandRootNode = reader.read(d->commandsFile);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(nullptr, QObject::tr("Loading Filters"),
                             QObject::tr("Error when loading G'MIC filters on line %1, column %2:\n%3")
                                  .arg(reader.lineNumber())
                                  .arg(reader.columnNumber())
                                  .arg(reader.errorString()));
    }
}

void GmicFilterManager::save()
{
    if (!d->loaded)
    {
        return;
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Saving G'MIC Filters to" << d->commandsFile;

    GmicXmlWriter writer;

    if (!writer.write(d->commandsFile, d->commandRootNode))
    {
        qCWarning(DIGIKAM_DPLUGIN_BQM_LOG) << "Error saving G'MIC filters to" << d->commandsFile;
    }
}

void GmicFilterManager::addCommand(GmicFilterNode* const parent,
                                   GmicFilterNode* const node, int row)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(parent);

    InsertGmicFilter* const command = new InsertGmicFilter(this, parent, node, row);
    d->commands.push(command);
}

void GmicFilterManager::removeCommand(GmicFilterNode* const node)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    GmicFilterNode* const parent    = node->parent();
    int row                         = parent->children().indexOf(node);
    RemoveGmicFilter* const command = new RemoveGmicFilter(this, parent, row);
    d->commands.push(command);
}

void GmicFilterManager::setTitle(GmicFilterNode* const node, const QString& newTitle)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeGmicFilter* const command = new ChangeGmicFilter(this, node, newTitle,
                                                           ChangeGmicFilter::Title);
    d->commands.push(command);
}

void GmicFilterManager::setCommand(GmicFilterNode* const node, const QString& newCommand)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeGmicFilter* const command = new ChangeGmicFilter(this, node, newCommand,
                                                           ChangeGmicFilter::Command);
    d->commands.push(command);
}

void GmicFilterManager::setComment(GmicFilterNode* const node, const QString& newDesc)
{
    if (!d->loaded)
    {
        return;
    }

    Q_ASSERT(node);

    ChangeGmicFilter* const command = new ChangeGmicFilter(this, node, newDesc,
                                                           ChangeGmicFilter::Desc);
    d->commands.push(command);
}

GmicFilterNode* GmicFilterManager::commands()
{
    if (!d->loaded)
    {
        load();
    }

    return d->commandRootNode;
}

GmicFilterModel* GmicFilterManager::commandsModel()
{
    if (!d->commandModel)
    {
        d->commandModel = new GmicFilterModel(this, this);
    }

    return d->commandModel;
}

QUndoStack* GmicFilterManager::undoRedoStack() const
{
    return &d->commands;
}

void GmicFilterManager::slotImportFilters()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open File"),
                                                    QString(),
                                                    QObject::tr("XML (*.xml)"));
    if (fileName.isEmpty())
    {
        return;
    }

    GmicXmlReader reader;
    GmicFilterNode* const importRootNode = reader.read(fileName);

    if (reader.error() != QXmlStreamReader::NoError)
    {
        QMessageBox::warning(nullptr, QObject::tr("Loading Filters"),
                             QObject::tr("Error when loading G'MIC filters on line %1, column %2:\n%3")
                                  .arg(reader.lineNumber())
                                  .arg(reader.columnNumber())
                                  .arg(reader.errorString()));
    }

    importRootNode->setType(GmicFilterNode::Folder);
    importRootNode->title = QObject::tr("Imported %1")
                                .arg(QLocale().toString(QDate::currentDate(),
                                     QLocale::ShortFormat));

    addCommand(commands(), importRootNode);
}

void GmicFilterManager::slotExportFilters()
{
    QString fileName = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save File"),
                                                    QObject::tr("%1 Gmic Filters.xml")
                                                    .arg(QCoreApplication::applicationName()),
                                                    QObject::tr("XML ( *.xml)"));
    if (fileName.isEmpty())
    {
        return;
    }

    GmicXmlWriter writer;

    if (!writer.write(fileName, d->commandRootNode))
    {
        QMessageBox::critical(nullptr, QObject::tr("Export filters"),
                                       QObject::tr("Error saving G'MIC filters"));
    }
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfiltermngr.cpp"
