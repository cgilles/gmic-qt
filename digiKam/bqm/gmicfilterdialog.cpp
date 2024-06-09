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
#include <QIcon>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QCloseEvent>
#include <QObject>
#include <QTextBrowser>
#include <QApplication>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QValidator>
#include <QClipboard>

// digiKam includes

#include "digikam_debug.h"
#include "searchtextbar.h"
#include "dtextedit.h"
#include "bqminfoiface.h"

// Local includes

#include "gmicfilternode.h"
#include "gmicfilterchain.h"
#include "gmicqtwindow.h"
#include "gmicqtcommon.h"

using namespace DigikamGmicQtPluginCommon;

namespace DigikamBqmGmicQtPlugin
{

extern BqmInfoIface* s_infoIface;

class Q_DECL_HIDDEN GmicFilterDialog::Private
{
public:

    Private() = default;

    bool                     edit            = false;   ///< True: editing mode ; false: adding mode.
    bool                     filter          = true;    ///< True: filtre       ; false: folder.

    GmicFilterNode*          currentItem     = nullptr;
    GmicFilterManager*       manager         = nullptr;
    AddGmicFilterProxyModel* proxyModel      = nullptr;
    QLineEdit*               title           = nullptr;
    DTextEdit*               desc            = nullptr;
    GmicFilterChain*         filterChain     = nullptr;
    DPluginBqm*              plugin          = nullptr;
};

GmicFilterDialog::GmicFilterDialog(GmicFilterNode* const citem,
                                   bool editMode, bool isFilter,
                                   QWidget* const parent,
                                   GmicFilterManager* const mngr,
                                   DPluginBqm* const plugin)
    : QDialog(parent),
      d      (new Private)
{
    d->edit        = editMode;
    d->filter      = isFilter;
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
    frontLbl->setText(tr("This dialog allow to customize the G'MIC chained command strings corresponding "
                         "to this filter. "
                         "Don't forget to assign at least a title and optionally a comment "
                         "to describe the filter."));
    frontLbl->setTextFormat(Qt::PlainText);
    frontLbl->setWordWrap(true);

    d->filterChain          = new GmicFilterChain(this);

    QLabel* const titleLbl  = new QLabel(d->filter ? tr("Title:")
                                                   : tr("Name:"), this);
    d->title                = new QLineEdit(this);
    d->title->setPlaceholderText(d->filter ? tr("Enter here the title")
                                           : tr("Enter here the folder name"));

    /*
     * Accepts all UTF-8 Characters.
     * Excludes the "/" symbol (for the absolute filter title path support).
     */
    QRegularExpression utf8Rx(QLatin1String("[^/]*"));
    QValidator* const utf8Validator   = new QRegularExpressionValidator(utf8Rx, this);
    d->title->setValidator(utf8Validator);

    QLabel* const descLbl             = new QLabel(tr("Description:"), this);
    d->desc                           = new DTextEdit(this);
    d->desc->setLinesVisible(3);
    d->desc->setPlaceholderText(tr("Enter here the description"));

    QDialogButtonBox* const buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(
                                  QDialogButtonBox::Cancel |
                                  QDialogButtonBox::Ok
                                 );

    buttonBox->setCenterButtons(false);

    QGridLayout* const grid           = new QGridLayout(this);
    grid->addWidget(frontLbl,       0, 0, 1, 3);
    grid->addWidget(d->filterChain, 1, 0, 1, 3);
    grid->addWidget(titleLbl,       2, 0, 1, 1);
    grid->addWidget(d->title,       2, 1, 1, 2);
    grid->addWidget(descLbl,        3, 0, 1, 3);
    grid->addWidget(d->desc,        4, 0, 1, 3);
    grid->addWidget(buttonBox,      5, 0, 1, 3);

    if (d->edit)
    {
        d->title->setText(d->currentItem->title);

        if (d->filter)
        {
            d->filterChain->setChainedFilters(d->currentItem->commands);
            d->title->setFocus();
            d->desc->setText(d->currentItem->desc);
            setWindowTitle(tr("Edit G'MIC Filter"));
        }
        else
        {
            d->title->setFocus();
            frontLbl->setVisible(false);
            d->filterChain->setVisible(false);
            descLbl->setVisible(false);
            d->desc->setVisible(false);
            setWindowTitle(tr("Edit G'MIC Folder"));
        }
    }
    else
    {
        if (d->filter)
        {
            d->title->setFocus();
            setWindowTitle(tr("Add G'MIC Filter"));
        }
        else
        {
            d->title->setFocus();
            frontLbl->setVisible(false);
            d->filterChain->setVisible(false);
            descLbl->setVisible(false);
            d->desc->setVisible(false);
            setWindowTitle(tr("Add G'MIC Folder"));
        }
    }

    // ---

    QPushButton* const help = buttonBox->addButton(QDialogButtonBox::Help);
    s_gmicQtPluginPopulateHelpButton(this, d->plugin, help);

    // ---

    connect(d->filterChain, SIGNAL(signalAddItem()),
            this, SLOT(slotGmicQt()));

    connect(d->filterChain, SIGNAL(signalEditItem(QString)),
            this, SLOT(slotGmicQt(QString)));

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

void GmicFilterDialog::slotGmicQt(const QString& command)
{
    QClipboard* const clipboard = QGuiApplication::clipboard();
    clipboard->clear();

    QString fname = GMicQtWindow::execWindow(
                                             d->plugin,                       // BQM plugin instance.
                                             GMicQtWindow::BQM,               // Host type.
                                             command                          // The G'MIC command in Edit mode, else empty in ADD mode.
                                            );

    if (!clipboard->text().isEmpty() && !fname.isEmpty())
    {
        if (command.isEmpty())
        {
            d->filterChain->createNewFilter(fname, clipboard->text());
        }
        else
        {
            d->filterChain->updateCurrentFilter(fname, clipboard->text());
        }
    }
}

void GmicFilterDialog::accept()
{
    if (d->title->text().isEmpty())
    {
        QMessageBox::information(this, tr("Error"),
                                 tr("Title cannot be empty..."));
        return;
    }

    if (d->edit)
    {
        d->currentItem->commands  = d->filterChain->chainedFilters();
        d->currentItem->title     = d->title->text();
        d->currentItem->desc      = d->desc->text();
        d->currentItem->dateAdded = QDateTime::currentDateTime();
    }
    else
    {
        GmicFilterNode* node = nullptr;

        if (d->filter)
        {
            node           = new GmicFilterNode(GmicFilterNode::Item);
            node->commands = d->filterChain->chainedFilters();
            node->desc     = d->desc->text();
        }
        else
        {
            node           = new GmicFilterNode(GmicFilterNode::Folder);
        }

        node->title        = d->title->text();
        node->dateAdded    = QDateTime::currentDateTime();
        d->manager->addCommand(d->currentItem, node);
    }

    d->manager->save();

    QDialog::accept();
}

} // namespace DigikamBqmGmicQtPlugin

#include "moc_gmicfilterdialog.cpp"
