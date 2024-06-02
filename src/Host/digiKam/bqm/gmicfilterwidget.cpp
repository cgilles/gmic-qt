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

using namespace DigikamEditorGmicQtPlugin;

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
    QAction*              addFilter        = nullptr;
    QAction*              addFolder        = nullptr;
    QAction*              addSeparator     = nullptr;
    QAction*              remove           = nullptr;
    QAction*              edit             = nullptr;
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
    d->addButton->setToolTip(QObject::tr("Add new item."));
    d->addButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->addButton->setPopupMode(QToolButton::InstantPopup);

    QMenu* const menu  = new QMenu(d->addButton);
    d->addFilter       = menu->addAction(QIcon::fromTheme(QLatin1String("process-working-symbolic")),
                                         QObject::tr("Add filter..."));
    d->addFolder       = menu->addAction(QIcon::fromTheme(QLatin1String("folder")),
                                         QObject::tr("Add folder..."));
    d->addSeparator    = menu->addAction(QIcon::fromTheme(QLatin1String("view-more-horizontal-symbolic")),
                                         QObject::tr("Add Separator..."));
    d->addButton->setMenu(menu);

    d->remButton       = new QToolButton(this);
    d->remButton->setToolTip(QObject::tr("Remove current selected item."));
    d->remove          = new QAction(QIcon::fromTheme(QLatin1String("list-remove")),
                                     QObject::tr("Remove..."));
    d->remButton->setDefaultAction(d->remove);

    d->edtButton       = new QToolButton(this);
    d->edtButton->setToolTip(QObject::tr("Edit current selected item."));
    d->edit            = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                     QObject::tr("Edit..."));
    d->edtButton->setDefaultAction(d->edit);

    d->search          = new SearchTextBar(this, QLatin1String("DigikamGmicFilterSearchBar"));
    d->search->setObjectName(QLatin1String("search"));

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->tree,      0, 0, 1, 5);
    grid->addWidget(d->addButton, 1, 0, 1, 1);
    grid->addWidget(d->remButton, 1, 1, 1, 1);
    grid->addWidget(d->edtButton, 1, 2, 1, 1);
    grid->addWidget(d->search,    1, 4, 1, 1);
    grid->setColumnStretch(3, 2);
    grid->setColumnStretch(4, 8);

    d->commandsModel        = d->manager->commandsModel();
    d->proxyModel           = new TreeProxyModel(this);
    d->proxyModel->setSourceModel(d->commandsModel);
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
        QModelIndex idx            = d->proxyModel->mapToSource(index);
        GmicFilterNode* const node = d->manager->commandsModel()->node(idx);

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

            if (QMessageBox::question(this, QObject::tr("G'MIC Filters Management"),
                                      QObject::tr("Do you want to remove \"%1\" "
                                                  "from your G'MIC filters collection?")
                                                  .arg(title),
                                      QMessageBox::Yes | QMessageBox::No
                                     ) == QMessageBox::No)
            {
                return;
            }

            d->manager->removeCommand(node);

            Q_EMIT signalSettingsChanged();
        }
    }
}

void GmicFilterWidget::slotAddFilter()
{
    openCommandDialog(false, true);
}

void GmicFilterWidget::slotAddFolder()
{
    openCommandDialog(false, false);
}

void GmicFilterWidget::slotAddSeparator()
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                        = d->proxyModel->mapToSource(index);
        GmicFilterNode* const parent = d->manager->commandsModel()->node(index);
        GmicFilterNode* const node   = new GmicFilterNode(GmicFilterNode::Separator);

        d->manager->addCommand(parent, node);
        d->manager->save();
    }
}

void GmicFilterWidget::slotEdit()
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                      = d->proxyModel->mapToSource(index);
        GmicFilterNode* const node = d->manager->commandsModel()->node(index);

        if (!node || (node->type() == GmicFilterNode::RootFolder))
        {
            return;
        }

        openCommandDialog(true, (node->type() == GmicFilterNode::Item));
    }
}

void GmicFilterWidget::openCommandDialog(bool edit, bool filter)
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                       = d->proxyModel->mapToSource(index);
        GmicFilterNode* const node  = d->manager->commandsModel()->node(index);

        GmicFilterDialog* const dlg = new GmicFilterDialog(
                                                           node,
                                                           edit,
                                                           filter,
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
}

void GmicFilterWidget::saveSettings()
{
    if (saveExpandedNodes(d->tree->rootIndex()))
    {
        d->manager->changeExpanded();
    }
}

QString GmicFilterWidget::currentGmicFilter() const
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                = d->proxyModel->mapToSource(index);
        GmicFilterNode* node = d->manager->commandsModel()->node(index);

        if (node && (node->type() == GmicFilterNode::Item))
        {
            return node->command;
        }
    }

    return QString();
}

QString GmicFilterWidget::currentPath() const
{
    QStringList hierarchy;
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                = d->proxyModel->mapToSource(index);
        GmicFilterNode* node = d->manager->commandsModel()->node(index);

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

                if (node)
                {
                    if (node->type() == GmicFilterNode::RootFolder)
                    {
                        break;
                    }

                    hierarchy.append(node->title);
                }
            }
        }

        std::reverse(hierarchy.begin(), hierarchy.end());

        return (hierarchy.join(QLatin1Char('/')));
    }

    return QString();
}

void GmicFilterWidget::setCurrentPath(const QString& path)
{
    if (path.isEmpty())
    {
        d->tree->setCurrentIndex(d->commandsModel->index(d->manager->commands()));
        return;
    }

    QStringList hierarchy = path.split(QLatin1Char('/'));
    GmicFilterNode* node  = d->manager->commands();

    // bypass the root folder.

    QList<GmicFilterNode*> children = node->children();

    if (children.isEmpty())
    {
        d->tree->setCurrentIndex(d->commandsModel->index(d->manager->commands()));
        return;
    }

    node         = children[0];
    int branches = 0;
    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Hierarchy:" << hierarchy;

    foreach (const QString& title, hierarchy)
    {
        children = node->children();
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Title:" << title;

        foreach (GmicFilterNode* const child, children)
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
        // Hierarchy is broken. Select root item.

        d->tree->setCurrentIndex(d->commandsModel->index(d->manager->commands()));
        return;
    }

    d->tree->setCurrentIndex(d->commandsModel->index(node));
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfilterwidget.cpp"
