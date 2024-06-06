/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 *  Description: digiKam plugin for GmicQt.
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

#include "gmicqtwindow.h"

// Qt includes

#include <QApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QIcon>
#include <QUrl>
#include <QLabel>
#include <QAction>
#include <QPointer>
#include <QDesktopServices>
#include <QSettings>

// digiKam includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dpluginaboutdlg.h"

// Local includes

#include "LanguageSettings.h"
#include "Settings.h"
#include "GmicQt.h"
#include "Widgets/InOutPanel.h"

using namespace GmicQt;

namespace DigikamEditorGmicQtPlugin
{

GMicQtWindow* s_mainWindow = nullptr;
QString       s_filterName;

class Q_DECL_HIDDEN GMicQtWindow::Private
{
public:

    explicit Private(DPlugin* const tool)
        : plugTool(tool)
    {
    }

    QString         hostOrg     = QCoreApplication::organizationName();
    QString         hostDom     = QCoreApplication::organizationDomain();
    QString         hostName    = QCoreApplication::applicationName();

    QString         plugName;
    QString         plugOrg;
    QString         plugDom;

    DPlugin*        plugTool    = nullptr;
    QString         dkModule;
    QLabel*         filterLbl   = nullptr;
    QString*        filterName  = nullptr;
};

GMicQtWindow::GMicQtWindow(
                           DPlugin* const tool,
                           QWidget* const parent,
                           QString* const filterName
                          )
    : MainWindow(parent),
      d         (new Private(tool))
{
    d->filterName = filterName;
    d->filterLbl  = findChild<QLabel*>("filterName");

    if (!d->filterLbl)
    {
        qCWarning(DIGIKAM_DPLUGIN_LOG) << "G'MIC-Qt: Cannot found \"filterName\" "
                                          "label from plugin dialog!";
    }

    QHBoxLayout* const hlay = findChild<QHBoxLayout*>("horizontalLayout");

    if (hlay)
    {
/*
        QPushButton* const layers  = new QPushButton(this);
        layers->setText(tr("Layers..."));
        layers->setIcon(QIcon::fromTheme(QLatin1String("dialog-layers")));
        layers->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        connect(layers, SIGNAL(released()),
                this, SLOT(slotLayersDialog()));

        hlay->insertWidget(0, layers);
*/
        // ---

        QPushButton* const help    = new QPushButton(this);
        help->setText(tr("Help"));
        help->setIcon(QIcon::fromTheme(QLatin1String("help-browser")));
        help->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        QMenu* const menu          = new QMenu(help);
        QAction* const webAction   = menu->addAction(QIcon::fromTheme(QLatin1String("globe")),
                                                     tr("Online Handbook..."));
        QAction* const aboutAction = menu->addAction(QIcon::fromTheme(QLatin1String("help-about")),
                                                     tr("About..."));
        help->setMenu(menu);

        if (!d->plugTool)
        {
            help->setEnabled(false);
        }

        connect(webAction, SIGNAL(triggered()),
                this, SLOT(slotOnlineHandbook()));

        connect(aboutAction, SIGNAL(triggered()),
                this, SLOT(slotAboutPlugin()));

        hlay->insertWidget(0, help);

        QLabel* const lbl          = findChild<QLabel*>("messageLabel");

        if (lbl)
        {
            hlay->setStretchFactor(lbl, 10);
        }
        else
        {
            qCWarning(DIGIKAM_DPLUGIN_LOG) << "G'MIC-Qt: Cannot found \"messageLabel\" "
                                              "label from plugin dialog!";
        }
    }
    else
    {
        qCWarning(DIGIKAM_DPLUGIN_LOG) << "G'MIC-Qt: Cannot found \"horizontalLayout\" "
                                          "layout from plugin dialog!";
    }
}

GMicQtWindow::~GMicQtWindow()
{
    delete d;
}

void GMicQtWindow::setHostType(HostType type)
{
    // Customize the rc settings file-name depending of the host application running Gmic-Qt.

    switch (type)
    {
        case BQM:
        {
            d->dkModule = QLatin1String("digikam-bqm-");
            break;
        }

        case ImageEditor:
        {
            d->dkModule = QLatin1String("digikam-editor-");
            break;
        }

        case Showfoto:
        {
            d->dkModule = QLatin1String("showfoto-");
            break;
        }

        default:
        {
            break;
        }
    }
}

void GMicQtWindow::setFilterSelectionMode()
{
    QPushButton* const pbOk     = findChild<QPushButton*>("pbOk");

    if (pbOk)
    {
        pbOk->setText(QObject::tr("Select Filter"));

        disconnect(pbOk, &QPushButton::clicked,
                   static_cast<MainWindow*>(this), &MainWindow::onOkClicked);

        connect(pbOk, &QPushButton::clicked,
                this, &GMicQtWindow::slotOkClicked);
    }
    else
    {
        qCWarning(DIGIKAM_DPLUGIN_LOG) << "G'MIC-Qt: Cannot found \"pbOk\" "
                                          "button from plugin dialog!";
    }

    // ---

    QPushButton* const pbApply  = findChild<QPushButton*>("pbApply");

    if (pbApply)
    {
        pbApply->setVisible(false);
    }
    else
    {
        qCWarning(DIGIKAM_DPLUGIN_LOG) << "G'MIC-Qt: Cannot found \"pbApply\" "
                                          "button from plugin dialog!";
    }

    // ---

    QPushButton* const pbCancel = findChild<QPushButton*>("pbCancel");

    if (pbCancel)
    {
        pbCancel->setVisible(false);
    }
    else
    {
        qCWarning(DIGIKAM_DPLUGIN_LOG) << "G'MIC-Qt: Cannot found \"pbCancel\" "
                                          "button from plugin dialog!";
    }
}

void GMicQtWindow::slotAboutPlugin()
{
    QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(d->plugTool);
    dlg->exec();
    delete dlg;
}

void GMicQtWindow::slotOnlineHandbook()
{
    openOnlineDocumentation(
                            d->plugTool->handbookSection(),
                            d->plugTool->handbookChapter(),
                            d->plugTool->handbookReference()
                           );
}

void GMicQtWindow::slotOkClicked()
{
    // Return current filter name.

    if (d->filterName && d->filterLbl)
    {
        *d->filterName = d->filterLbl->text()
                         .remove(QLatin1String("<b>"))
                         .remove(QLatin1String("</b>"));
    }

    // Copy the current G'MIC command on the clipboard.

    s_mainWindow->onCopyGMICCommand();
    close();
}

void GMicQtWindow::slotLayersDialog()
{
    // TODO
}

void GMicQtWindow::saveParameters()
{
    saveSettings();
}

void GMicQtWindow::showEvent(QShowEvent* event)
{
    if (d->plugOrg.isEmpty())
    {
        d->plugOrg  = QCoreApplication::organizationName();
    }

    if (d->plugDom.isEmpty())
    {
        d->plugDom  = QCoreApplication::organizationDomain();
    }

    if (d->plugName.isEmpty())
    {

        d->plugName = d->dkModule + QCoreApplication::applicationName();
    }

    QCoreApplication::setOrganizationName(d->plugOrg);
    QCoreApplication::setOrganizationDomain(d->plugDom);
    QCoreApplication::setApplicationName(d->plugName);

    MainWindow::showEvent(event);
}

void GMicQtWindow::closeEvent(QCloseEvent* event)
{
    QCoreApplication::setOrganizationName(d->hostOrg);
    QCoreApplication::setOrganizationDomain(d->hostDom);
    QCoreApplication::setApplicationName(d->hostName);

    MainWindow::closeEvent(event);
}

// --- Static method ---

QString GMicQtWindow::execWindow(DPlugin* const tool,
                                 HostType type,
                                 const QString& command)
{
    // Code inspired from GmicQt.cpp::run() and host_none.cpp::main()

    Settings::load(GmicQt::UserInterfaceMode::Full);
    LanguageSettings::installTranslators();

    // ---

    std::list<GmicQt::InputMode> disabledInputModes;
    disabledInputModes.push_back(GmicQt::InputMode::NoInput);
    // disabledInputModes.push_back(InputMode::Active);
    disabledInputModes.push_back(GmicQt::InputMode::All);
    disabledInputModes.push_back(GmicQt::InputMode::ActiveAndBelow);
    disabledInputModes.push_back(GmicQt::InputMode::ActiveAndAbove);
    disabledInputModes.push_back(GmicQt::InputMode::AllVisible);
    disabledInputModes.push_back(GmicQt::InputMode::AllInvisible);

    std::list<GmicQt::OutputMode> disabledOutputModes;
    // disabledOutputModes.push_back(GmicQt::OutputMode::InPlace);
    disabledOutputModes.push_back(GmicQt::OutputMode::NewImage);
    disabledOutputModes.push_back(GmicQt::OutputMode::NewLayers);
    disabledOutputModes.push_back(GmicQt::OutputMode::NewActiveLayers);

    for (const GmicQt::InputMode& mode : disabledInputModes)
    {
        GmicQt::InOutPanel::disableInputMode(mode);
    }

    for (const GmicQt::OutputMode& mode : disabledOutputModes)
    {
        GmicQt::InOutPanel::disableOutputMode(mode);
    }

    // ---

    /**
     * We need to backup QApplication instance properties between plugin sessions else we can
     * seen side effects, for example with the settings to host in RC file.
     */

    s_mainWindow = new GMicQtWindow(tool, qApp->activeWindow(), &s_filterName);

    if (type == GMicQtWindow::BQM)
    {
        s_mainWindow->setFilterSelectionMode();
    }

    s_mainWindow->setHostType(type);

    RunParameters parameters;

    if (!command.isEmpty())
    {
        parameters.command = command.toStdString();
    }
    else
    {
        parameters = lastAppliedFilterRunParameters(GmicQt::ReturnedRunParametersFlag::BeforeFilterExecution);
    }

    qCDebug(DIGIKAM_DPLUGIN_LOG) << "Start G'MIC-Qt dialog with parameters:";
    qCDebug(DIGIKAM_DPLUGIN_LOG) << "Command:"     << QString::fromStdString(parameters.command);
    qCDebug(DIGIKAM_DPLUGIN_LOG) << "Path:"        << QString::fromStdString(parameters.filterPath);
    qCDebug(DIGIKAM_DPLUGIN_LOG) << "Input Mode:"  << (int)parameters.inputMode;
    qCDebug(DIGIKAM_DPLUGIN_LOG) << "Output Mode:" << (int)parameters.outputMode;
    qCDebug(DIGIKAM_DPLUGIN_LOG) << "Filter name:" << QString::fromStdString(parameters.filterName());

    s_mainWindow->setPluginParameters(parameters);

    // We want a non modal dialog here.

#ifdef Q_OS_MACOS

    s_mainWindow->setWindowFlags(Qt::Tool | Qt::Dialog);

#else

    s_mainWindow->setWindowFlags(Qt::Dialog);

#endif

    s_mainWindow->setWindowModality(Qt::ApplicationModal);

    if (QSettings().value("Config/MainWindowMaximized", false).toBool())
    {
        s_mainWindow->showMaximized();  // krazy:exclude=qmethods
    }
    else
    {
        s_mainWindow->show();
    }

    // Make it destroy itself on close (signaling the event loop)

    s_mainWindow->setAttribute(Qt::WA_DeleteOnClose);

    // Wait than main widget is closed.

    QEventLoop loop;

    connect(s_mainWindow, SIGNAL(destroyed()),
            &loop, SLOT(quit()));

    loop.exec();

    return s_filterName;
}

} // namespace DigikamEditorGmicQtPlugin

#include "moc_gmicqtwindow.cpp"
