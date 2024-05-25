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
#include <QFile>
#include <QIcon>
#include <QStandardPaths>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QAction>
#include <QCloseEvent>
#include <QObject>
#include <QUndoCommand>
#include <QVariant>
#include <QApplication>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QLabel>
#include <QItemDelegate>
#include <QValidator>

// digiKam includes

#include "digikam_debug.h"
#include "searchtextbar.h"
#include "dtextedit.h"

// Local includes

#include "gmicfilternode.h"

using namespace Digikam;

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicFilterDialog::Private
{
public:

    Private() = default;

    bool                     edit            = false;
    bool                     filter          = true;
    GmicFilterNode*          currentItem     = nullptr;
    GmicFilterManager*       manager         = nullptr;
    AddGmicFilterProxyModel* proxyModel      = nullptr;
    QLineEdit*               title           = nullptr;
    DTextEdit*               desc            = nullptr;
    QTextEdit*               command         = nullptr;
};

GmicFilterDialog::GmicFilterDialog(GmicFilterNode* const citem,
                                   bool edit, bool filter,
                                   QWidget* const parent,
                                   GmicFilterManager* const mngr)
    : QDialog(parent),
      d      (new Private)
{
    d->edit        = edit;
    d->filter      = filter;
    d->manager     = mngr;
    d->currentItem = citem;

    setObjectName(QLatin1String("GmicFilterDialog"));
    setModal(true);
    setWindowFlags((windowFlags() & ~Qt::Dialog) |
                   Qt::Window                    |
                   Qt::WindowCloseButtonHint     |
                   Qt::WindowMinMaxButtonsHint);

    QLabel* const frontLbl = new QLabel(this);
    frontLbl->setText(QObject::tr("This dialog allow to customize the G'MIC Command string corresponding "
                                  "to this new filter. "
                                  "Don't forget to assign at least a name and optionally a comment "
                                  "to describe the filter. Finaly you can choose where to keep it in your "
                                  "filters collection."));
    frontLbl->setTextFormat(Qt::PlainText);
    frontLbl->setWordWrap(true);

    QLabel* const commandLbl = new QLabel(QObject::tr("Filter Command:"), this);
    d->command               = new QTextEdit(this);

    QLabel* const titleLbl   = new QLabel(d->filter ? QObject::tr("Filter Title:")
                                                    : QObject::tr("Folder Title:"), this);
    d->title                 = new QLineEdit(this);
    d->title->setPlaceholderText(d->filter ? QObject::tr("Enter here the filter title")
                                           : QObject::tr("Enter here the folder title"));

    /*
     * Accepts all UTF-8 Characters.
     * Excludes the "/" symbol (for the absolute title path support).
     */
    QRegularExpression utf8Rx(QLatin1String("[^/]*"));
    QValidator* const utf8Validator = new QRegularExpressionValidator(utf8Rx, this);
    d->title->setValidator(utf8Validator);

    QLabel* const descLbl    = new QLabel(QObject::tr("Filter Description:"), this);
    d->desc                  = new DTextEdit(this);
    d->desc->setLinesVisible(3);
    d->desc->setPlaceholderText(QObject::tr("Enter here the filter description"));

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    buttonBox->setCenterButtons(false);

    QGridLayout* const grid     = new QGridLayout(this);
    grid->addWidget(frontLbl,           0, 0, 1, 2);
    grid->addWidget(commandLbl,         1, 0, 1, 2);
    grid->addWidget(d->command,         2, 0, 1, 2);
    grid->addWidget(titleLbl,           3, 0, 1, 1);
    grid->addWidget(d->title,           3, 1, 1, 1);
    grid->addWidget(descLbl,            4, 0, 1, 2);
    grid->addWidget(d->desc,            5, 0, 1, 2);
    grid->addWidget(buttonBox);

    if (d->edit)
    {
        d->title->setText(d->currentItem->title);

        if (d->filter)
        {
            d->command->setText(d->currentItem->command);
            d->desc->setText(d->currentItem->desc);
            setWindowTitle(QObject::tr("Edit G'MIC Filter"));
        }
        else
        {
            frontLbl->setVisible(false);
            commandLbl->setVisible(false);
            d->command->setVisible(false);
            descLbl->setVisible(false);
            d->desc->setVisible(false);
            setWindowTitle(QObject::tr("Edit G'MIC Folder"));
        }
    }
    else
    {
        if (d->filter)
        {
            d->command->setText(QString());     // TODO use Clipboard
            d->title->setText(QObject::tr("My new G'MIC filter"));
            setWindowTitle(QObject::tr("Add G'MIC Filter"));
        }
        else
        {
            frontLbl->setVisible(false);
            commandLbl->setVisible(false);
            d->command->setVisible(false);
            descLbl->setVisible(false);
            d->desc->setVisible(false);
            d->title->setText(QObject::tr("My new G'MIC folder"));
            setWindowTitle(QObject::tr("Add G'MIC Folder"));
        }
    }

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    adjustSize();
}

GmicFilterDialog::~GmicFilterDialog()
{
    delete d;
}

void GmicFilterDialog::accept()
{
    if (d->edit)
    {
        d->currentItem->command   = d->command->toPlainText();
        d->currentItem->title     = d->title->text();
        d->currentItem->desc      = d->desc->text();
        d->currentItem->dateAdded = QDateTime::currentDateTime();
    }
    else
    {
        GmicFilterNode* node = nullptr;

        if (d->filter)
        {
            node          = new GmicFilterNode(GmicFilterNode::Item);
            node->command = d->command->toPlainText();
            node->desc    = d->desc->text();
        }
        else
        {
            node          = new GmicFilterNode(GmicFilterNode::Folder);
        }

        node->title       = d->title->text();
        node->dateAdded   = QDateTime::currentDateTime();
        d->manager->addCommand(d->currentItem, node);
    }

    d->manager->save();

    QDialog::accept();
}

// ----------------------------------------------------------------

class Q_DECL_HIDDEN GmicFilterWidget::Private
{
public:

    Private() = default;

    GmicFilterManager*    manager          = nullptr;
    GmicFilterModel*      commandsModel    = nullptr;
    TreeProxyModel*       proxyModel       = nullptr;
    SearchTextBar*        search           = nullptr;
    QTreeView*            tree             = nullptr;
    QPushButton*          addButton        = nullptr;
    QPushButton*          remButton        = nullptr;
    QPushButton*          edtButton        = nullptr;
    QAction*              addFilter        = nullptr;
    QAction*              addFolder        = nullptr;
    QAction*              addSeparator     = nullptr;
};

GmicFilterWidget::GmicFilterWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setObjectName(QLatin1String("GmicFilterWidget"));

    const QString db = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                                        QLatin1String("/gmicfilters.xml");
    d->manager       = new GmicFilterManager(db, this);
    d->manager->load();

    d->search        = new SearchTextBar(this, QLatin1String("DigikamGmicFilterSearchBar"));
    d->search->setObjectName(QLatin1String("search"));

    d->tree            = new QTreeView(this);
    d->tree->setUniformRowHeights(true);
    d->tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->tree->setSelectionMode(QAbstractItemView::SingleSelection);
    d->tree->setTextElideMode(Qt::ElideMiddle);
    d->tree->setDragDropMode(QAbstractItemView::InternalMove);
    d->tree->setAlternatingRowColors(true);
    d->tree->setContextMenuPolicy(Qt::CustomContextMenu);

    d->addButton       = new QPushButton(this);
    d->addButton->setToolTip(QObject::tr("Add new item."));
    d->addButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    d->addButton->setAutoDefault(false);
    QMenu* const menu  = new QMenu(d->addButton);
    d->addFilter       = menu->addAction(QIcon::fromTheme(QLatin1String("process-working-symbolic")),
                                         QObject::tr("Add filter..."));
    d->addFolder       = menu->addAction(QIcon::fromTheme(QLatin1String("folder")),
                                         QObject::tr("Add folder..."));
    d->addSeparator    = menu->addAction(QIcon::fromTheme(QLatin1String("view-more-horizontal-symbolic")),
                                         QObject::tr("Add Separator..."));
    d->addButton->setMenu(menu);

    d->remButton       = new QPushButton(this);
    d->remButton->setToolTip(QObject::tr("Remove current selected item."));
    d->remButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));

    d->edtButton       = new QPushButton(this);
    d->edtButton->setToolTip(QObject::tr("Edit current selected item."));
    d->edtButton->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));

    QSpacerItem* const spacerItem1     = new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                                                 QSizePolicy::Minimum);

    QHBoxLayout* const hbox = new QHBoxLayout();
    hbox->addWidget(d->addButton);
    hbox->addWidget(d->remButton);
    hbox->addWidget(d->edtButton);
    hbox->addItem(spacerItem1);

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->search,  0, 0, 1, 2);
    grid->addWidget(d->tree,    1, 0, 1, 2);
    grid->addLayout(hbox,       2, 0, 1, 3);
    grid->setColumnStretch(1, 10);

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

    connect(d->remButton, SIGNAL(clicked()),
            this, SLOT(slotRemove()));

    connect(d->edtButton, SIGNAL(clicked()),
            this, SLOT(slotEdit()));

    connect(d->addFilter, SIGNAL(triggered()),
            this, SLOT(slotAddFilter()));

    connect(d->addFolder, SIGNAL(triggered()),
            this, SLOT(slotAddFolder()));

    connect(d->addSeparator, SIGNAL(triggered()),
            this, SLOT(slotAddSeparator()));

    connect(d->tree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotTreeViewItemActivated(QModelIndex)));

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

void GmicFilterWidget::slotTreeViewItemActivated(const QModelIndex& index)
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
                d->remButton->setEnabled(false);
                d->addFilter->setEnabled(true);
                d->edtButton->setEnabled(false);
                break;
            }

            case GmicFilterNode::Folder:
            {
                d->addSeparator->setEnabled(true);
                d->addFolder->setEnabled(true);
                d->remButton->setEnabled(true);
                d->addFilter->setEnabled(true);
                d->edtButton->setEnabled(true);
                break;
            }

            case GmicFilterNode::Item:
            {
                d->addSeparator->setEnabled(false);
                d->addFolder->setEnabled(false);
                d->remButton->setEnabled(true);
                d->addFilter->setEnabled(false);
                d->edtButton->setEnabled(true);

                Q_EMIT signalSettingsChanged();

                break;
            }

            case GmicFilterNode::Separator:
            {
                d->addSeparator->setEnabled(false);
                d->addFolder->setEnabled(false);
                d->remButton->setEnabled(true);
                d->addFilter->setEnabled(false);
                d->edtButton->setEnabled(false);
                break;
            }

            default:
            {
                d->addFolder->setEnabled(false);
                d->addSeparator->setEnabled(false);
                d->remButton->setEnabled(false);
                d->addFilter->setEnabled(false);
                d->edtButton->setEnabled(false);
                break;
            }
        }
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << currentPath();
}

void GmicFilterWidget::slotCustomContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = d->tree->indexAt(pos);
    index             = index.sibling(index.row(), 0);

    if (index.isValid())
    {
        index                      = d->proxyModel->mapToSource(index);
        GmicFilterNode* const node = d->manager->commandsModel()->node(index);

        if (node && (node->type() != GmicFilterNode::RootFolder))
        {
            QMenu menu;
            menu.addAction(QObject::tr("Remove"), this, SLOT(slotRemoveOne()));
            menu.exec(QCursor::pos());
        }
    }
}

void GmicFilterWidget::slotRemove()
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

        if (QMessageBox::question(this, QObject::tr("G'MIC Filters Management"),
                                  QObject::tr("Do you want to remove \"%1\" "
                                        "from your G'MIC filters collection?")
                                  .arg(node->title),
                                  QMessageBox::Yes | QMessageBox::No
                                 ) == QMessageBox::No)
        {
            return;
        }

        d->manager->removeCommand(node);
    }

    Q_EMIT signalSettingsChanged();
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
                                                           d->manager
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
        index                 = d->proxyModel->mapToSource(index);
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

            while (node->parent())
            {
                node  = node->parent();

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
