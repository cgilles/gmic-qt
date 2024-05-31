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

#include "gmicfilterdialog.h"

// Qt includes

#include <QMenu>
#include <QFile>
#include <QIcon>
#include <QStandardPaths>
#include <QHeaderView>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>
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
#include <QSpacerItem>
#include <QLabel>
#include <QItemDelegate>
#include <QValidator>
#include <QClipboard>

// digiKam includes

#include "digikam_debug.h"
#include "searchtextbar.h"
#include "dtextedit.h"
#include "bqminfoiface.h"
#include "dpluginaboutdlg.h"

// Local includes

#include "gmicfilternode.h"
#include "gmicqtwindow.h"

using namespace DigikamEditorGmicQtPlugin;

extern BqmInfoIface* s_infoIface;

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
    QPushButton*             commandBtn      = nullptr;
    DPluginBqm*              plugin          = nullptr;

};

GmicFilterDialog::GmicFilterDialog(GmicFilterNode* const citem,
                                   bool edit, bool filter,
                                   QWidget* const parent,
                                   GmicFilterManager* const mngr,
                                   DPluginBqm* const plugin)
    : QDialog(parent),
      d      (new Private)
{
    d->edit        = edit;
    d->filter      = filter;
    d->manager     = mngr;
    d->currentItem = citem;
    d->plugin      = plugin;

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
    d->commandBtn            = new QPushButton(this);
    d->command               = new QTextEdit(this);

    QLabel* const titleLbl   = new QLabel(d->filter ? QObject::tr("Filter Title:")
                                                    : QObject::tr("Folder Title:"), this);
    d->title                 = new QLineEdit(this);
    d->title->setPlaceholderText(d->filter ? QObject::tr("Enter here the filter title")
                                           : QObject::tr("Enter here the folder title"));

    /*
     * Accepts all UTF-8 Characters.
     * Excludes the "/" symbol (for the absolute filter title path support).
     */
    QRegularExpression utf8Rx(QLatin1String("[^/]*"));
    QValidator* const utf8Validator   = new QRegularExpressionValidator(utf8Rx, this);
    d->title->setValidator(utf8Validator);

    QLabel* const descLbl             = new QLabel(QObject::tr("Filter Description:"), this);
    d->desc                           = new DTextEdit(this);
    d->desc->setLinesVisible(3);
    d->desc->setPlaceholderText(QObject::tr("Enter here the filter description"));

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(
                                  QDialogButtonBox::Cancel |
                                  QDialogButtonBox::Ok
                                 );

    buttonBox->setCenterButtons(false);

    QGridLayout* const grid           = new QGridLayout(this);
    grid->addWidget(frontLbl,       0, 0, 1, 3);
    grid->addWidget(commandLbl,     1, 0, 1, 1);
    grid->addWidget(d->commandBtn,  1, 2, 1, 1);
    grid->addWidget(d->command,     2, 0, 1, 3);
    grid->addWidget(titleLbl,       3, 0, 1, 1);
    grid->addWidget(d->title,       3, 1, 1, 2);
    grid->addWidget(descLbl,        4, 0, 1, 3);
    grid->addWidget(d->desc,        5, 0, 1, 3);
    grid->addWidget(buttonBox,      6, 0, 1, 3);

    if (d->edit)
    {
        d->title->setText(d->currentItem->title);

        if (d->filter)
        {
            d->command->setText(d->currentItem->command);
            d->command->setFocus();
            d->desc->setText(d->currentItem->desc);
            setWindowTitle(QObject::tr("Edit G'MIC Filter"));
            d->commandBtn->setText(QObject::tr("Edit Filter..."));
        }
        else
        {
            d->title->setFocus();
            frontLbl->setVisible(false);
            commandLbl->setVisible(false);
            d->command->setVisible(false);
            d->commandBtn->setVisible(false);
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
            d->command->setFocus();
            setWindowTitle(QObject::tr("Add G'MIC Filter"));
            d->commandBtn->setText(QObject::tr("Select Filter..."));
        }
        else
        {
            d->title->setFocus();
            frontLbl->setVisible(false);
            commandLbl->setVisible(false);
            d->command->setVisible(false);
            d->commandBtn->setVisible(false);
            descLbl->setVisible(false);
            d->desc->setVisible(false);
            setWindowTitle(QObject::tr("Add G'MIC Folder"));
        }
    }

    // ---

    QPushButton* const help       = buttonBox->addButton(QDialogButtonBox::Help);
    help->setIcon(QIcon::fromTheme(QLatin1String("help-browser")));
    help->setText(QObject::tr("Help"));
    help->setAutoDefault(false);
    QMenu* const menu             = new QMenu(help);
    QAction* const handbookAction = menu->addAction(QIcon::fromTheme(QLatin1String("globe")),
                                                    QObject::tr("Online Handbook..."));
    QAction* const aboutAction    = menu->addAction(QIcon::fromTheme(QLatin1String("help-about")),
                                                    QObject::tr("About..."));
    help->setMenu(menu);

    connect(handbookAction, SIGNAL(triggered()),
            this, SLOT(slotOnlineHandbook()));

    connect(aboutAction, SIGNAL(triggered()),
            this, SLOT(slotAboutPlugin()));

    // ---

    connect(d->commandBtn, SIGNAL(pressed()),
            this, SLOT(slotGmicQt()));

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

void GmicFilterDialog::slotAboutPlugin()
{
    QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(d->plugin);
    dlg->exec();
    delete dlg;
}

void GmicFilterDialog::slotOnlineHandbook()
{
    openOnlineDocumentation(
                            d->plugin->handbookSection(),
                            d->plugin->handbookChapter(),
                            d->plugin->handbookReference()
                           );
}

void GmicFilterDialog::slotGmicQt()
{
    GMicQtWindow::execWindow(
                             d->plugin,                     // BQM plugin instance.
                             s_infoIface,                   // Host interface.
                             d->command->toPlainText(),     // The G'MIC command.
                             true                           // Viewver mode.
                            );

    QClipboard* const clipboard = QGuiApplication::clipboard();

    if (!clipboard->text().isEmpty())
    {
        d->command->setText(clipboard->text());
    }
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

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfilterdialog.cpp"
