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
#include <QTextEdit>

// digiKam includes

#include "searchtextbar.h"
#include "dtextedit.h"

// Local includes

#include "gmiccommandnode.h"

using namespace Digikam;

namespace DigikamBqmGmicQtPlugin
{

class Q_DECL_HIDDEN AddGmicCommandDialog::Private
{
public:

    Private() = default;

    GmicCommandManager*       manager         = nullptr;
    AddGmicCommandProxyModel* proxyModel      = nullptr;
    QComboBox*                collectionPlace = nullptr;
    DTextEdit*                title           = nullptr;
    DTextEdit*                desc            = nullptr;
    QTextEdit*                command         = nullptr;
};

AddGmicCommandDialog::AddGmicCommandDialog(const QString& command,
                                           const QString& title,
                                           QWidget* const parent,
                                           GmicCommandManager* const mngr)
    : QDialog(parent),
      d      (new Private)
{
    d->manager     = mngr;

    setWindowTitle(QObject::tr("Add G'MIC Command"));
    setObjectName(QLatin1String("AddGmicCommandDialog"));
    setModal(true);
    setWindowFlags((windowFlags() & ~Qt::Dialog) |
                   Qt::Window                    |
                   Qt::WindowCloseButtonHint     |
                   Qt::WindowMinMaxButtonsHint);

    QLabel* const frontLbl = new QLabel(this);
    frontLbl->setText(QObject::tr("Enter the G'MIC Command string corresponding to this new filter. "
                                  "Don't forget to assign at least a name and optionally a comment "
                                  "to describe the filter. Finaly choose where to keep it in your "
                                  "filters collection."));
    frontLbl->setTextFormat(Qt::PlainText);
    frontLbl->setWordWrap(true);

    QLabel* const commandLbl = new QLabel(QObject::tr("Filter Command:"), this);
    d->command               = new QTextEdit(this);
    d->command->setText(command);

    QLabel* const titleLbl   = new QLabel(QObject::tr("Filter Title:"), this);
    d->title                 = new DTextEdit(this);
    d->title->setLinesVisible(1);
    d->title->setPlaceholderText(QObject::tr("Enter here the filter title"));
    d->title->setText(title);

    QLabel* const descLbl    = new QLabel(QObject::tr("Filter Description:"), this);
    d->desc                  = new DTextEdit(this);
    d->desc->setLinesVisible(3);
    d->desc->setPlaceholderText(QObject::tr("Enter here the filter description"));

    QLabel* const placeLbl   = new QLabel(QObject::tr("Place in Collection:"), this);
    d->collectionPlace       = new QComboBox(this);

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
    grid->addWidget(placeLbl,           6, 0, 1, 1);
    grid->addWidget(d->collectionPlace, 6, 1, 1, 1);
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

    d->collectionPlace->setModel(d->proxyModel);
    d->collectionPlace->setView(view);

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));

    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(reject()));

    adjustSize();
}

AddGmicCommandDialog::~AddGmicCommandDialog()
{
    delete d;
}

void AddGmicCommandDialog::accept()
{
    QModelIndex index = d->collectionPlace->view()->currentIndex();
    index             = d->proxyModel->mapToSource(index);

    if (!index.isValid())
    {
        index = d->manager->commandsModel()->index(0, 0);
    }

    GmicCommandNode* const parent = d->manager->commandsModel()->node(index);
    GmicCommandNode* const node   = new GmicCommandNode(GmicCommandNode::Item);
    node->command                 = d->command->toPlainText();
    node->title                   = d->title->text();
    node->desc                    = d->desc->text();
    node->dateAdded               = QDateTime::currentDateTime();
    d->manager->addCommand(parent, node);
    d->manager->save();

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
    d->tree->setSelectionMode(QAbstractItemView::ContiguousSelection);
    d->tree->setTextElideMode(Qt::ElideMiddle);
    d->tree->setDragDropMode(QAbstractItemView::InternalMove);
    d->tree->setAlternatingRowColors(true);
    d->tree->setContextMenuPolicy(Qt::CustomContextMenu);

    QPushButton* const addButton       = new QPushButton(this);
    addButton->setText(QObject::tr("&Add..."));

    QPushButton* const removeButton    = new QPushButton(this);
    removeButton->setText(QObject::tr("&Remove"));

    QPushButton* const addFolderButton = new QPushButton(this);
    addFolderButton->setText(QObject::tr("Add Folder"));

    QSpacerItem* const spacerItem1     = new QSpacerItem(40, 20, QSizePolicy::Expanding,
                                                                 QSizePolicy::Minimum);

    QHBoxLayout* const hbox = new QHBoxLayout();
    hbox->addWidget(addButton);
    hbox->addWidget(removeButton);
    hbox->addWidget(addFolderButton);
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

    connect(removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveOne()));

    connect(addButton, SIGNAL(clicked()),
            this, SLOT(slotAddOne()));

    connect(d->tree, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomContextMenuRequested(QPoint)));

    connect(addFolderButton, SIGNAL(clicked()),
            this, SLOT(slotNewFolder()));

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

void GmicCommandWidget::slotCustomContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = d->tree->indexAt(pos);
    index             = index.sibling(index.row(), 0);

    if (index.isValid())
    {
        index                       = d->proxyModel->mapToSource(index);
        GmicCommandNode* const node = d->manager->commandsModel()->node(index);

        if (node && node->type() != GmicCommandNode::RootFolder)
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

        if (node->type() == GmicCommandNode::RootFolder)
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
}

void GmicCommandWidget::slotAddOne()
{
    AddGmicCommandDialog* const dlg = new AddGmicCommandDialog(
                                                               QString(),  // TODO: use clipboard
                                                               QString(),
                                                               this,
                                                               d->manager
                                                              );
    dlg->exec();
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

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmiccommandwidget.cpp"
