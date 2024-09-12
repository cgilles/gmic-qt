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

#include "gmicfilterwidget.h"

// Qt includes

#include <QMenu>
#include <QIcon>
#include <QStandardPaths>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QAction>
#include <QObject>
#include <QApplication>
#include <QGridLayout>

// digiKam includes

#include "digikam_debug.h"
#include "searchtextbar.h"
#include "dtextedit.h"
#include "bqminfoiface.h"

// Local includes

#include "gmicfilternode.h"
#include "gmicfilterdialog.h"
#include "gmicqtwindow.h"
#include "gmicfiltermodel.h"

using namespace DigikamGmicQtPluginCommon;

namespace DigikamBqmGmicQtPlugin
{

BqmInfoIface* s_infoIface = nullptr;

class Q_DECL_HIDDEN GmicFilterWidget::Private
{
public:

    Private() = default;

    GmicFilterManager*    manager          = nullptr;
    GmicFilterModel*      commandsModel    = nullptr;
    TreeProxyModel*       proxyModel       = nullptr;
    SearchTextBar*        search           = nullptr;
    QTreeView*            tree             = nullptr;
    QToolButton*          addButton        = nullptr;
    QToolButton*          remButton        = nullptr;
    QToolButton*          edtButton        = nullptr;
    QToolButton*          dbButton         = nullptr;
    QAction*              addFilter        = nullptr;
    QAction*              addFolder        = nullptr;
    QAction*              addSeparator     = nullptr;
    QAction*              remove           = nullptr;
    QAction*              edit             = nullptr;
    QAction*              importdb         = nullptr;
    QAction*              exportdb         = nullptr;
    DPluginBqm*           plugin           = nullptr;
};

GmicFilterWidget::GmicFilterWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setObjectName(QLatin1String("GmicFilterWidget"));

    const QString db   = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                                          QLatin1String("/gmicfilters.xml");
    d->manager         = new GmicFilterManager(db, this);
    d->manager->load();


    d->tree            = new QTreeView(this);
    d->tree->setUniformRowHeights(true);
    d->tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->tree->setSelectionMode(QAbstractItemView::SingleSelection);
    d->tree->setTextElideMode(Qt::ElideMiddle);
    d->tree->setDragDropMode(QAbstractItemView::InternalMove);
    d->tree->setAlternatingRowColors(true);
    d->tree->setContextMenuPolicy(Qt::CustomContextMenu);
    d->tree->setHeaderHidden(true);

    d->addButton       = new QToolButton(this);
    d->addButton->setToolTip(tr("Add new item."));
    d->addButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->addButton->setPopupMode(QToolButton::InstantPopup);

    QMenu* const menu  = new QMenu(d->addButton);
    d->addFilter       = menu->addAction(QIcon::fromTheme(QLatin1String("process-working-symbolic")),
                                         tr("Add filter..."));
    d->addFolder       = menu->addAction(QIcon::fromTheme(QLatin1String("folder")),
                                         tr("Add folder..."));
    d->addSeparator    = menu->addAction(QIcon::fromTheme(QLatin1String("view-more-horizontal-symbolic")),
                                         tr("Add Separator..."));
    d->addButton->setMenu(menu);

    // ---

    d->remButton       = new QToolButton(this);
    d->remButton->setToolTip(tr("Remove current selected item."));
    d->remove          = new QAction(QIcon::fromTheme(QLatin1String("list-remove")),
                                     tr("Remove..."));
    d->remButton->setDefaultAction(d->remove);

    // ---

    d->edtButton       = new QToolButton(this);
    d->edtButton->setToolTip(tr("Edit current selected item."));
    d->edit            = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                     tr("Edit..."));
    d->edtButton->setDefaultAction(d->edit);

    // ---

    d->dbButton         = new QToolButton(this);
    d->dbButton->setToolTip(tr("Import and export hierarchy."));
    d->dbButton->setIcon(QIcon::fromTheme(QLatin1String("server-database")));
    d->dbButton->setPopupMode(QToolButton::InstantPopup);

    QMenu* const menudb = new QMenu(d->dbButton);
    d->importdb         = menudb->addAction(QIcon::fromTheme(QLatin1String("document-import")),
                                            tr("Import..."));
    d->exportdb         = menudb->addAction(QIcon::fromTheme(QLatin1String("document-export")),
                                            tr("Export..."));
    d->dbButton->setMenu(menudb);

    // ---

    d->search           = new SearchTextBar(this, QLatin1String("DigikamGmicFilterSearchBar"));
    d->search->setObjectName(QLatin1String("search"));

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->tree,      0, 0, 1, 6);
    grid->addWidget(d->addButton, 1, 0, 1, 1);
    grid->addWidget(d->remButton, 1, 1, 1, 1);
    grid->addWidget(d->edtButton, 1, 2, 1, 1);
    grid->addWidget(d->dbButton,  1, 3, 1, 1);
    grid->addWidget(d->search,    1, 5, 1, 1);
    grid->setColumnStretch(4, 2);
    grid->setColumnStretch(5, 8);

    d->commandsModel        = d->manager->commandsModel();
    d->proxyModel           = new TreeProxyModel(this);
    d->proxyModel->setSourceModel(d->commandsModel);
    d->tree->setItemDelegate(new GmicFilterDelegate(d->proxyModel));
    d->tree->setModel(d->proxyModel);
    d->tree->setExpanded(d->proxyModel->index(0, 0), true);
    d->tree->header()->setSectionResizeMode(QHeaderView::Stretch);

    connect(d->search, SIGNAL(textChanged(QString)),
            d->proxyModel, SLOT(setFilterFixedString(QString)));

    connect(d->proxyModel, SIGNAL(signalFilterAccepts(bool)),
            d->search, SLOT(slotSearchResult(bool)));

    connect(d->remove, SIGNAL(triggered()),
            this, SLOT(slotRemove()));

    connect(d->edit, SIGNAL(triggered()),
            this, SLOT(slotEdit()));

    connect(d->addFilter, SIGNAL(triggered()),
            this, SLOT(slotAddFilter()));

    connect(d->addFolder, SIGNAL(triggered()),
            this, SLOT(slotAddFolder()));

    connect(d->addSeparator, SIGNAL(triggered()),
            this, SLOT(slotAddSeparator()));

    connect(d->importdb, SIGNAL(triggered()),
            d->manager, SLOT(slotImportFilters()));

    connect(d->exportdb, SIGNAL(triggered()),
            d->manager, SLOT(slotExportFilters()));

    connect(d->tree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotTreeViewItemClicked(QModelIndex)));

    connect(d->tree, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(slotEdit()));

    connect(d->tree, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenuRequested(QPoint)));

    readSettings();
}

GmicFilterWidget::~GmicFilterWidget()
{
    saveSettings();
    d->manager->save();

    delete d;
}

void GmicFilterWidget::setPlugin(DPluginBqm* const plugin)
{
    d->plugin   = plugin;
    s_infoIface = d->plugin->infoIface();
}

bool GmicFilterWidget::saveExpandedNodes(const QModelIndex& parent)
{
    bool changed = false;

    for (int i = 0 ; i < d->proxyModel->rowCount(parent) ; ++i)
    {
        QModelIndex child               = d->proxyModel->index(i, 0, parent);
        QModelIndex sourceIndex         = d->proxyModel->mapToSource(child);
        GmicFilterNode* const childNode = d->commandsModel->node(sourceIndex);
        bool wasExpanded                = childNode->expanded;

        if (d->tree->isExpanded(child))
        {
            childNode->expanded = true;
            changed            |= saveExpandedNodes(child);
        }
        else
        {
            childNode->expanded = false;
        }

        changed |= (wasExpanded != childNode->expanded);
    }

    return changed;
}

void GmicFilterWidget::expandNodes(GmicFilterNode* const node)
{
    for (int i = 0 ; i < node->children().count() ; ++i)
    {
        GmicFilterNode* const childNode = node->children().value(i);

        if (childNode->expanded)
        {
            QModelIndex idx = d->commandsModel->index(childNode);
            idx             = d->proxyModel->mapFromSource(idx);
            d->tree->setExpanded(idx, true);
            expandNodes(childNode);
        }
    }
}

void GmicFilterWidget::slotTreeViewItemClicked(const QModelIndex& index)
{
    if (index.isValid())
    {
        QModelIndex idx                  = d->proxyModel->mapToSource(index);
        const GmicFilterNode* const node = d->manager->commandsModel()->node(idx);

        switch (node->type())
        {
            case GmicFilterNode::Root:
            case GmicFilterNode::RootFolder:
            {
                d->addSeparator->setEnabled(true);
                d->addFolder->setEnabled(true);
                d->remove->setEnabled(false);
                d->addFilter->setEnabled(true);
                d->edit->setEnabled(false);
                break;
            }

            case GmicFilterNode::Folder:
            {
                d->addSeparator->setEnabled(true);
                d->addFolder->setEnabled(true);
                d->remove->setEnabled(true);
                d->addFilter->setEnabled(true);
                d->edit->setEnabled(true);
                break;
            }

            case GmicFilterNode::Item:
            {
                d->addSeparator->setEnabled(false);
                d->addFolder->setEnabled(false);
                d->remove->setEnabled(true);
                d->addFilter->setEnabled(false);
                d->edit->setEnabled(true);

                Q_EMIT signalSettingsChanged();

                break;
            }

            case GmicFilterNode::Separator:
            {
                d->addSeparator->setEnabled(false);
                d->addFolder->setEnabled(false);
                d->remove->setEnabled(true);
                d->addFilter->setEnabled(false);
                d->edit->setEnabled(false);
                break;
            }

            default:
            {
                d->addFolder->setEnabled(false);
                d->addSeparator->setEnabled(false);
                d->remove->setEnabled(false);
                d->addFilter->setEnabled(false);
                d->edit->setEnabled(false);
                break;
            }
        }
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << currentPath();
}

void GmicFilterWidget::slotCustomContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = d->tree->indexAt(pos);
    slotTreeViewItemClicked(index);

    QMenu menu;
    menu.addAction(d->addFilter);
    menu.addAction(d->addFolder);
    menu.addAction(d->addSeparator);
    menu.addSeparator();
    menu.addAction(d->remove);
    menu.addSeparator();
    menu.addAction(d->edit);
    menu.exec(QCursor::pos());
}

void GmicFilterWidget::slotRemove()
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                      = d->proxyModel->mapToSource(index);
        GmicFilterNode* const node = d->manager->commandsModel()->node(index);

        if (node)
        {
            QString title;

            switch (node->type())
            {
                case GmicFilterNode::Item:
                case GmicFilterNode::Folder:
                {
                    title = node->title;
                    break;
                }

                case GmicFilterNode::Separator:
                {
                    title = QObject::tr("separator");
                    break;
                }

                case GmicFilterNode::Root:
                case GmicFilterNode::RootFolder:
                default:
                {
                    return;
                }
            }

            if (QMessageBox::question(this, tr("G'MIC Filters Management"),
                                      tr("Do you want to remove \"%1\" "
                                         "from your G'MIC filters collection?")
                                         .arg(title),
                                      QMessageBox::Yes | QMessageBox::No
                                     ) == QMessageBox::No)
            {
                return;
            }

            d->manager->removeEntry(node);

            Q_EMIT signalSettingsChanged();
        }
    }
}

void GmicFilterWidget::slotAddFilter()
{
    openPropertiesDialog(false, true);
}

void GmicFilterWidget::slotAddFolder()
{
    openPropertiesDialog(false, false);
}

void GmicFilterWidget::slotAddSeparator()
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                        = d->proxyModel->mapToSource(index);
        GmicFilterNode* const parent = d->manager->commandsModel()->node(index);
        GmicFilterNode* const node   = new GmicFilterNode(GmicFilterNode::Separator);

        d->manager->addEntry(parent, node);
        d->manager->save();
    }
}

void GmicFilterWidget::slotEdit()
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                            = d->proxyModel->mapToSource(index);
        const GmicFilterNode* const node = d->manager->commandsModel()->node(index);

        if (
            !node                                           ||
            (node->type() == GmicFilterNode::RootFolder)    ||
            (node->type() == GmicFilterNode::Separator)
           )
        {
            return;
        }

        openPropertiesDialog(true, (node->type() == GmicFilterNode::Item));
    }
}

void GmicFilterWidget::openPropertiesDialog(bool editMode, bool isFilter)
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                       = d->proxyModel->mapToSource(index);
        GmicFilterNode* const node  = d->manager->commandsModel()->node(index);

        GmicFilterDialog* const dlg = new GmicFilterDialog(
                                                           node,
                                                           editMode,
                                                           isFilter,
                                                           this,
                                                           d->manager,
                                                           d->plugin
                                                          );
        dlg->exec();
        delete dlg;

        Q_EMIT signalSettingsChanged();
    }
}

void GmicFilterWidget::readSettings()
{
    expandNodes(d->manager->commands());

    setCurrentPath(d->manager->currentPath());
}

void GmicFilterWidget::saveSettings()
{
    d->manager->setCurrentPath(currentPath());

    if (saveExpandedNodes(d->tree->rootIndex()))
    {
        d->manager->changeExpanded();
    }
}

QMap<QString, QVariant> GmicFilterWidget::currentGmicFilters() const
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                = d->proxyModel->mapToSource(index);
        GmicFilterNode* node = d->manager->commandsModel()->node(index);

        if (node && (node->type() == GmicFilterNode::Item))
        {
            return node->commands;
        }
    }

    return QMap<QString, QVariant>();
}

QString GmicFilterWidget::currentGmicChainedCommands() const
{
    QString chained;
    QMap<QString, QVariant> filters = currentGmicFilters();

    if (!filters.isEmpty())
    {
        const QList<QVariant> lst = filters.values();

        for (const QVariant& v : qAsConst(lst))
        {
            chained.append(QLatin1Char(' '));
            chained.append(v.toString());
        }
    }

    return chained.trimmed();
}

QString GmicFilterWidget::currentPath() const
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        QStringList hierarchy;
        index                      = d->proxyModel->mapToSource(index);
        const GmicFilterNode* node = d->manager->commandsModel()->node(index);

        if (node)
        {
            if (node->type() == GmicFilterNode::RootFolder)
            {
                return QString();
            }

            hierarchy.append(node->title);

            while (node && node->parent())
            {
                node = node->parent();

                if (node->type() == GmicFilterNode::RootFolder)
                {
                    break;
                }

                hierarchy.append(node->title);
            }
        }

        std::reverse(hierarchy.begin(), hierarchy.end());

        return (hierarchy.join(QLatin1Char('/')));
    }

    return QString();
}

void GmicFilterWidget::setCurrentPath(const QString& path)
{
    GmicFilterNode* node  = d->manager->commands();
    QModelIndex idx;

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Settings current path:" << path;

    // bypass the root folder.

    QList<GmicFilterNode*> children = node->children();

    if (children.isEmpty())
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Tree-view is empty.";

        return;
    }

    GmicFilterNode* const root = children[0]; // Root node
    node                       = root;

    if (path.isEmpty())
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Path is empty.";

        return;
    }

    QStringList hierarchy = path.split(QLatin1Char('/'));

    int branches = 0;

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Hierarchy:" << hierarchy;

    for (const QString& title : qAsConst(hierarchy))
    {
        children = node->children();
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Title:" << title;

        for (GmicFilterNode* const child : qAsConst(children))
        {
            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Child node:" << child->title;

            if (child->title == title)
            {
                qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Found node:" << title;
                node = child;
                branches++;
                break;
            }
        }
    }

    if (branches != hierarchy.size())
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "branches:" << branches << "hierarchy:" << hierarchy;

        idx  = d->proxyModel->mapToSource(d->commandsModel->index(root));
        d->tree->setCurrentIndex(idx);

        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Hierarchy is broken. Select Root Item:" << root->title;

        return;
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Select Item:" << node->title;
    idx = d->commandsModel->index(node);
    d->tree->setCurrentIndex(d->proxyModel->mapFromSource(idx));
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfilterwidget.cpp"
