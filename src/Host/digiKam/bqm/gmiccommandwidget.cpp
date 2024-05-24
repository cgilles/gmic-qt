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

#include "gmiccommandwidget.h"

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

#include "gmiccommandnode.h"

using namespace Digikam;

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN GmicCommandDialog::Private
{
public:

    Private() = default;

    bool                      edit            = false;
    GmicCommandNode*          currentItem     = nullptr;
    GmicCommandManager*       manager         = nullptr;
    AddGmicCommandProxyModel* proxyModel      = nullptr;
    QLineEdit*                title           = nullptr;
    DTextEdit*                desc            = nullptr;
    QTextEdit*                command         = nullptr;
};

GmicCommandDialog::GmicCommandDialog(GmicCommandNode* const citem,
                                     bool edit,
                                     QWidget* const parent,
                                     GmicCommandManager* const mngr)
    : QDialog(parent),
      d      (new Private)
{
    d->edit        = edit;
    d->manager     = mngr;
    d->currentItem = citem;

    setObjectName(QLatin1String("GmicCommandDialog"));
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

    QLabel* const titleLbl   = new QLabel(QObject::tr("Filter Title:"), this);
    d->title                 = new QLineEdit(this);
    d->title->setPlaceholderText(QObject::tr("Enter here the filter title"));

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

    QTreeView* const view       = new QTreeView(this);
    d->proxyModel               = new AddGmicCommandProxyModel(this);
    GmicCommandModel* const model = d->manager->commandsModel();
    d->proxyModel->setSourceModel(model);
    view->setModel(d->proxyModel);
    view->expandAll();
    view->header()->setStretchLastSection(true);
    view->header()->hide();
    view->setItemsExpandable(false);
    view->setRootIsDecorated(false);
    view->setIndentation(10);
    view->show();

    GmicCommandNode* const menu = d->manager->commands();
    QModelIndex idx             = d->proxyModel->mapFromSource(model->index(menu));
    view->setCurrentIndex(idx);

    if (d->edit)
    {
        d->command->setText(d->currentItem->command);
        d->title->setText(d->currentItem->title);
        d->desc->setText(d->currentItem->desc);
        setWindowTitle(QObject::tr("Edit G'MIC Filter"));
        view->setCurrentIndex(model->index(d->currentItem->parent()));
    }
    else
    {
        d->command->setText(QString());     // TODO use Clipboard
        d->title->setText(QObject::tr("My new G'MIC filter title"));
        setWindowTitle(QObject::tr("Add G'MIC Filter"));
        view->setCurrentIndex(idx);
    }

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    adjustSize();
}

GmicCommandDialog::~GmicCommandDialog()
{
    delete d;
}

void GmicCommandDialog::accept()
{
    if (d->edit)
    {
        d->currentItem->command   = d->command->toPlainText();
        d->currentItem->title     = d->title->text();
        d->currentItem->desc      = d->desc->text();
        d->currentItem->dateAdded = QDateTime::currentDateTime();
        d->manager->save();
    }
    else
    {
        GmicCommandNode* const node = new GmicCommandNode(GmicCommandNode::Item);
        node->command               = d->command->toPlainText();
        node->title                 = d->title->text();
        node->desc                  = d->desc->text();
        node->dateAdded             = QDateTime::currentDateTime();
        d->manager->addCommand(d->currentItem, node);
        d->manager->save();
    }

    QDialog::accept();
}

// ----------------------------------------------------------------

class Q_DECL_HIDDEN GmicCommandWidget::Private
{
public:

    Private() = default;

    GmicCommandManager*    manager          = nullptr;
    GmicCommandModel*      commandsModel    = nullptr;
    TreeProxyModel*        proxyModel       = nullptr;
    SearchTextBar*         search           = nullptr;
    QTreeView*             tree             = nullptr;
    QPushButton*           addButton        = nullptr;
    QPushButton*           remButton        = nullptr;
    QPushButton*           edtButton        = nullptr;
    QPushButton*           addFolderButton  = nullptr;
};

GmicCommandWidget::GmicCommandWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setObjectName(QLatin1String("GmicCommandEditDialog"));

    const QString db = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                                        QLatin1String("/gmiccommands.xml");
    d->manager       = new GmicCommandManager(db, this);
    d->manager->load();

    d->search        = new SearchTextBar(this, QLatin1String("DigikamGmicCommandSearchBar"));
    d->search->setObjectName(QLatin1String("search"));

    d->tree          = new QTreeView(this);
    d->tree->setUniformRowHeights(true);
    d->tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    d->tree->setSelectionMode(QAbstractItemView::SingleSelection);
    d->tree->setTextElideMode(Qt::ElideMiddle);
    d->tree->setDragDropMode(QAbstractItemView::InternalMove);
    d->tree->setAlternatingRowColors(true);
    d->tree->setContextMenuPolicy(Qt::CustomContextMenu);

    d->addButton       = new QPushButton(this);
    d->addButton->setText(QObject::tr("&Add..."));

    d->remButton       = new QPushButton(this);
    d->remButton->setText(QObject::tr("&Remove..."));

    d->edtButton       = new QPushButton(this);
    d->edtButton->setText(QObject::tr("&Edit..."));

    d->addFolderButton = new QPushButton(this);
    d->addFolderButton->setText(QObject::tr("Add Folder..."));

    QSpacerItem* const spacerItem1     = new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                                                 QSizePolicy::Minimum);

    QHBoxLayout* const hbox = new QHBoxLayout();
    hbox->addWidget(d->addButton);
    hbox->addWidget(d->remButton);
    hbox->addWidget(d->edtButton);
    hbox->addWidget(d->addFolderButton);
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
            this, SLOT(slotRemoveOne()));

    connect(d->edtButton, SIGNAL(clicked()),
            this, SLOT(slotEditOne()));

    connect(d->addButton, SIGNAL(clicked()),
            this, SLOT(slotAddOne()));

    connect(d->addFolderButton, SIGNAL(clicked()),
            this, SLOT(slotNewFolder()));

    connect(d->tree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotTreeViewItemActivated(QModelIndex)));

    connect(d->tree, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(slotAddOne()));

    connect(d->tree, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenuRequested(QPoint)));

    readSettings();
}

GmicCommandWidget::~GmicCommandWidget()
{
    saveSettings();
    d->manager->save();

    delete d;
}


bool GmicCommandWidget::saveExpandedNodes(const QModelIndex& parent)
{
    bool changed = false;

    for (int i = 0 ; i < d->proxyModel->rowCount(parent) ; ++i)
    {
        QModelIndex child                = d->proxyModel->index(i, 0, parent);
        QModelIndex sourceIndex          = d->proxyModel->mapToSource(child);
        GmicCommandNode* const childNode = d->commandsModel->node(sourceIndex);
        bool wasExpanded                 = childNode->expanded;

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

void GmicCommandWidget::expandNodes(GmicCommandNode* const node)
{
    for (int i = 0 ; i < node->children().count() ; ++i)
    {
        GmicCommandNode* const childNode = node->children().value(i);

        if (childNode->expanded)
        {
            QModelIndex idx = d->commandsModel->index(childNode);
            idx             = d->proxyModel->mapFromSource(idx);
            d->tree->setExpanded(idx, true);
            expandNodes(childNode);
        }
    }
}

void GmicCommandWidget::slotTreeViewItemActivated(const QModelIndex& index)
{
    if (index.isValid())
    {
        QModelIndex idx             = d->proxyModel->mapToSource(index);
        GmicCommandNode* const node = d->manager->commandsModel()->node(idx);

        switch (node->type())
        {
            case GmicCommandNode::Root:
            case GmicCommandNode::RootFolder:
            {
                d->addFolderButton->setEnabled(true);
                d->remButton->setEnabled(false);
                d->addButton->setEnabled(true);
                d->edtButton->setEnabled(false);
                break;
            }

            case GmicCommandNode::Folder:
            {
                d->addFolderButton->setEnabled(true);
                d->remButton->setEnabled(true);
                d->addButton->setEnabled(true);
                d->edtButton->setEnabled(false);
                break;
            }

            case GmicCommandNode::Item:
            {
                d->addFolderButton->setEnabled(false);
                d->remButton->setEnabled(true);
                d->addButton->setEnabled(false);
                d->edtButton->setEnabled(true);

                Q_EMIT signalSettingsChanged();

                break;
            }

            case GmicCommandNode::Separator:
            {
                d->addFolderButton->setEnabled(false);
                d->remButton->setEnabled(true);
                d->addButton->setEnabled(false);
                d->edtButton->setEnabled(false);
                break;
            }

            default:
            {
                d->addFolderButton->setEnabled(false);
                d->remButton->setEnabled(false);
                d->addButton->setEnabled(false);
                d->edtButton->setEnabled(false);
                break;
            }
        }
    }

    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << currentPath();
}

void GmicCommandWidget::slotCustomContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = d->tree->indexAt(pos);
    index             = index.sibling(index.row(), 0);

    if (index.isValid())
    {
        index                       = d->proxyModel->mapToSource(index);
        GmicCommandNode* const node = d->manager->commandsModel()->node(index);

        if (node && (node->type() != GmicCommandNode::RootFolder))
        {
            QMenu menu;
            menu.addAction(QObject::tr("Remove"), this, SLOT(slotRemoveOne()));
            menu.exec(QCursor::pos());
        }
    }
}

void GmicCommandWidget::slotNewFolder()
{
    QModelIndex currentIndex = d->tree->currentIndex();
    QModelIndex idx          = currentIndex;

    if (idx.isValid() && !idx.model()->hasChildren(idx))
    {
        idx = idx.parent();
    }

    if (!idx.isValid())
    {
        idx = d->tree->rootIndex();
    }

    idx                           = d->proxyModel->mapToSource(idx);
    GmicCommandNode* const parent = d->manager->commandsModel()->node(idx);
    GmicCommandNode* const node   = new GmicCommandNode(GmicCommandNode::Folder);
    node->title                   = QObject::tr("New Folder");
    d->manager->addCommand(parent, node, currentIndex.row() + 1);
}

void GmicCommandWidget::slotRemoveOne()
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                       = d->proxyModel->mapToSource(index);
        GmicCommandNode* const node = d->manager->commandsModel()->node(index);

        if (!node || (node->type() == GmicCommandNode::RootFolder))
        {
            return;
        }

        if (QMessageBox::question(this, QObject::tr("G'MIC Commands Management"),
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

void GmicCommandWidget::slotAddOne()
{
    openCommandDialog(false);
}

void GmicCommandWidget::slotEditOne()
{
    openCommandDialog(true);
}

void GmicCommandWidget::openCommandDialog(bool edit)
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                       = d->proxyModel->mapToSource(index);
        GmicCommandNode* const node = d->manager->commandsModel()->node(index);

        GmicCommandDialog* const dlg = new GmicCommandDialog(
                                                             node,
                                                             edit,
                                                             this,
                                                             d->manager
                                                            );
        dlg->exec();
        delete dlg;

        Q_EMIT signalSettingsChanged();
    }
}

void GmicCommandWidget::readSettings()
{
    expandNodes(d->manager->commands());
}

void GmicCommandWidget::saveSettings()
{
    if (saveExpandedNodes(d->tree->rootIndex()))
    {
        d->manager->changeExpanded();
    }
}

QString GmicCommandWidget::currentGmicCommand() const
{
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                 = d->proxyModel->mapToSource(index);
        GmicCommandNode* node = d->manager->commandsModel()->node(index);

        if (node && (node->type() == GmicCommandNode::Item))
        {
            return node->command;
        }
    }

    return QString();
}

QString GmicCommandWidget::currentPath() const
{
    QStringList hierarchy;
    QModelIndex index = d->tree->currentIndex();

    if (index.isValid())
    {
        index                 = d->proxyModel->mapToSource(index);
        GmicCommandNode* node = d->manager->commandsModel()->node(index);

        if (node)
        {
            if (node->type() == GmicCommandNode::RootFolder)
            {
                return QString();
            }

            hierarchy.append(node->title);

            while (node->parent())
            {
                node  = node->parent();

                if (node)
                {
                    if (node->type() == GmicCommandNode::RootFolder)
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

void GmicCommandWidget::setCurrentPath(const QString& path)
{
    QStringList hierarchy = path.split(QLatin1Char('/'));
    GmicCommandNode* node = d->manager->commands();

    foreach (const QString& title, hierarchy)
    {
        QList<GmicCommandNode*> children = node->children();

        foreach (GmicCommandNode* const child, children)
        {
            if (child->title == title)
            {
                node = child;
            }
            else
            {
                // Hierarchy is broken. Select root item.

                d->tree->setCurrentIndex(d->commandsModel->index(d->manager->commands()));
                return;
            }
        }
    }

    d->tree->setCurrentIndex(d->commandsModel->index(node));
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmiccommandwidget.cpp"
