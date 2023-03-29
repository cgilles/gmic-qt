/** -*- mode: c++ ; c-basic-offset: 2 -*-
 *
 *  @file Bqm_Widget.cpp
 *
 *  Copyright 2017 Sebastien Fourey
 *
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  gmic_qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gmic_qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gmic_qt.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "bqm_widget.h"

#include <cassert>
#include <iostream>
#include <typeinfo>

#include <QAction>
#include <QClipboard>
#include <QCursor>
#include <QDebug>
#include <QEvent>
#include <QGuiApplication>
#include <QKeySequence>
#include <QMessageBox>
#include <QPalette>
#include <QScreen>
#include <QSettings>
#include <QShortcut>
#include <QShowEvent>
#include <QStringList>
#include <QStyleFactory>

#include "Common.h"
#include "CroppedActiveLayerProxy.h"
#include "CroppedImageListProxy.h"
#include "DialogSettings.h"
#include "FilterSelector/FavesModelReader.h"
#include "FilterSelector/FiltersPresenter.h"
#include "FilterTextTranslator.h"
#include "Globals.h"
#include "GmicStdlib.h"
#include "HtmlTranslator.h"
#include "IconLoader.h"
#include "LayersExtentProxy.h"
#include "Logger.h"
#include "Misc.h"
#include "ParametersCache.h"
#include "PersistentMemory.h"
#include "Settings.h"
#include "Updater.h"
#include "Utils.h"
#include "Widgets/VisibleTagSelector.h"
#include "gmic.h"
#include "ui_bqm_widget.h"

namespace
{

QString appendShortcutText(const QString & text, const QKeySequence & key)
{
  if (text.isRightToLeft()) {
    return QString("(%2) %1").arg(text).arg(key.toString());
  } else {
    return QString("%1 (%2)").arg(text).arg(key.toString());
  }
}

} // namespace

namespace GmicQt
{

bool Bqm_Widget::_isAccepted = false;

//
// TODO : Handle window maximization properly (Windows as well as some Linux desktops)
//

Bqm_Widget::Bqm_Widget(QWidget * parent) : QBqm_Widget(parent), ui(new Ui::Bqm_Widget)
{
  TIMING;
  ui->setupUi(this);
  TIMING;
  _messageTimerID = 0;
  _gtkFavesShouldBeImported = false;

  _lastExecutionOK = true; // Overwritten by loadSettings()
  _expandCollapseIcon = nullptr;
  _newSession = true; // Overwritten by loadSettings()

  setWindowTitle(pluginFullName());
  QStringList tsp = QIcon::themeSearchPaths();
  tsp.append(QString("/usr/share/icons/gnome"));
  QIcon::setThemeSearchPaths(tsp);

  _filterUpdateWidgets = {ui->previewWidget, ui->zoomLevelSelector, ui->filtersView,       ui->filterParams,   ui->tbUpdateFilters, ui->pbFullscreen, ui->pbSettings,
                          ui->pbOk,          ui->pbApply,           ui->tbResetParameters, ui->tbCopyCommand,  ui->searchField,     ui->cbPreview,    ui->tbAddFave,
                          ui->tbRemoveFave,  ui->tbRenameFave,      ui->tbExpandCollapse,  ui->tbSelectionMode};

  ui->tbAddFave->setToolTip(tr("Add fave"));

  ui->tbResetParameters->setToolTip(tr("Reset parameters to default values"));
  ui->tbResetParameters->setVisible(false);

  QShortcut * copyShortcut = new QShortcut(QKeySequence::Copy, this);
  copyShortcut->setContext(Qt::ApplicationShortcut);
  connect(copyShortcut, &QShortcut::activated, [this] { ui->tbCopyCommand->animateClick(); });
  ui->tbCopyCommand->setToolTip(appendShortcutText(tr("Copy G'MIC command to clipboard"), copyShortcut->key()));
  ui->tbCopyCommand->setVisible(false);

  QShortcut * closeShortcut = new QShortcut(QKeySequence::Close, this);
  closeShortcut->setContext(Qt::ApplicationShortcut);
  connect(closeShortcut, &QShortcut::activated, this, &Bqm_Widget::close);

  ui->tbRenameFave->setToolTip(tr("Rename fave"));
  ui->tbRenameFave->setEnabled(false);
  ui->tbRemoveFave->setToolTip(tr("Remove fave"));
  ui->tbRemoveFave->setEnabled(false);
  ui->pbFullscreen->setCheckable(true);

  ui->tbExpandCollapse->setToolTip(tr("Expand/Collapse all"));

  ui->logosLabel->setToolTip(tr("G'MIC (https://gmic.eu)<br/>"
                                "GREYC (https://www.greyc.fr)<br/>"
                                "CNRS (https://www.cnrs.fr)<br/>"
                                "Normandy University (https://www.unicaen.fr)<br/>"
                                "Ensicaen (https://www.ensicaen.fr)"));
  ui->logosLabel->setPixmap(QPixmap(":resources/logos.png"));

  ui->tbSelectionMode->setToolTip(tr("Selection mode"));
  ui->tbSelectionMode->setCheckable(true);

  ui->filterName->setTextFormat(Qt::RichText);
  ui->filterName->setVisible(false);

  ui->progressInfoWidget->hide();
  ui->messageLabel->setText(QString());
  ui->messageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  ui->rightMessageLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  ui->filterParams->setNoFilter();
  _pendingActionAfterCurrentProcessing = ProcessingAction::NoAction;
  ui->inOutSelector->disable();
  ui->splitter->setChildrenCollapsible(false);

  ui->zoomLevelSelector->setPreviewWidget(ui->previewWidget);

  auto searchAction = new QAction(this);
  searchAction->setShortcut(QKeySequence::Find);
  searchAction->setShortcutContext(Qt::ApplicationShortcut);
  connect(searchAction, &QAction::triggered, ui->searchField, QOverload<>::of(&SearchFieldWidget::setFocus));
  addAction(searchAction);

  auto togglePreviewAction = new QAction(this);
  togglePreviewAction->setShortcut(QKeySequence("Ctrl+P"));
  togglePreviewAction->setShortcutContext(Qt::ApplicationShortcut);
  connect(togglePreviewAction, &QAction::triggered, ui->cbPreview, &QCheckBox::toggle);
  addAction(togglePreviewAction);

  searchAction = new QAction(this);
  searchAction->setShortcut(QKeySequence("/"));
  searchAction->setShortcutContext(Qt::ApplicationShortcut);
  connect(searchAction, &QAction::triggered, ui->searchField, QOverload<>::of(&SearchFieldWidget::setFocus));
  addAction(searchAction);

  {
    QKeySequence f5("F5");
    QKeySequence ctrlR("Ctrl+R");
    QString updateText = tr("Update filters");
    if (updateText.isRightToLeft()) {
      updateText = QString("(%2 / %3) %1").arg(updateText).arg(f5.toString()).arg(ctrlR.toString());
    } else {
      updateText = QString("%1 (%2 / %3)").arg(updateText).arg(ctrlR.toString()).arg(f5.toString());
    }
    QShortcut * updateShortcutF5 = new QShortcut(QKeySequence("F5"), this);
    updateShortcutF5->setContext(Qt::ApplicationShortcut);
    QShortcut * updateShortcutCtrlR = new QShortcut(QKeySequence("Ctrl+R"), this);
    updateShortcutCtrlR->setContext(Qt::ApplicationShortcut);
    connect(updateShortcutF5, &QShortcut::activated, [this]() { ui->tbUpdateFilters->animateClick(); });
    connect(updateShortcutCtrlR, &QShortcut::activated, [this]() { ui->tbUpdateFilters->animateClick(); });
    ui->tbUpdateFilters->setToolTip(updateText);
  }

  ui->splitter->setHandleWidth(6);
  ui->verticalSplitter->setHandleWidth(6);
  ui->verticalSplitter->setStretchFactor(0, 5);
  ui->verticalSplitter->setStretchFactor(0, 1);

  if (!ui->inOutSelector->hasActiveControls()) {
    ui->vSplitterLine->hide();
    ui->inOutSelector->hide();
  }

  QPalette p = QGuiApplication::palette();
  Settings::UnselectedFilterTextColor = p.color(QPalette::Disabled, QPalette::WindowText);

  _filtersPresenter = new FiltersPresenter(this);
  _filtersPresenter->setFiltersView(ui->filtersView);
  _filtersPresenter->setSearchField(ui->searchField);

  ui->progressInfoWidget->setGmicProcessor(&_processor);

  TIMING;
  loadSettings();
  TIMING;
  ParametersCache::load(!_newSession);
  TIMING;
  setIcons();
  QAction * escAction = new QAction(this);
  escAction->setShortcut(QKeySequence(Qt::Key_Escape));
  escAction->setShortcutContext(Qt::ApplicationShortcut);
  connect(escAction, &QAction::triggered, this, &Bqm_Widget::onEscapeKeyPressed);
  addAction(escAction);

  CroppedImageListProxy::clear();
  CroppedActiveLayerProxy::clear();
  LayersExtentProxy::clear();
  QSize layersExtent = LayersExtentProxy::getExtent(ui->inOutSelector->inputMode());
  ui->previewWidget->setFullImageSize(layersExtent);
  _lastPreviewKeypointBurstUpdateTime = 0;
  _isAccepted = false;

  ui->tbTags->setToolTip(tr("Manage visible tags\n(Right-click on a fave or a filter to set/remove tags)"));
  _visibleTagSelector = new VisibleTagSelector(this);
  _visibleTagSelector->setToolButton(ui->tbTags);
  _visibleTagSelector->updateColors();
  _filtersPresenter->setVisibleTagSelector(_visibleTagSelector);

  TIMING;
  makeConnections();
  TIMING;
}

Bqm_Widget::~Bqm_Widget()
{
  //  QSet<QString> hashes;
  //  FiltersTreeAbstractItem::buildHashesList(_filtersTreeModel.invisibleRootItem(),hashes);
  //  ParametersCache::cleanup(hashes);

  saveCurrentParameters();
  ParametersCache::save();
  saveSettings();
  Logger::setMode(Logger::Mode::StandardOutput); // Close log file, if necessary
  delete ui;
}

void Bqm_Widget::setIcons()
{
  ui->tbTags->setIcon(LOAD_ICON("color-wheel"));
  ui->tbRenameFave->setIcon(LOAD_ICON("rename"));
  ui->pbSettings->setIcon(LOAD_ICON("package_settings"));
  ui->pbFullscreen->setIcon(LOAD_ICON("view-fullscreen"));
  ui->tbUpdateFilters->setIcon(LOAD_ICON_NO_DARKENED("view-refresh"));
  ui->pbApply->setIcon(LOAD_ICON("system-run"));
  ui->pbOk->setIcon(LOAD_ICON("insert-image"));
  ui->tbResetParameters->setIcon(LOAD_ICON("view-refresh"));
  ui->tbCopyCommand->setIcon(LOAD_ICON("edit-copy"));
  ui->pbCancel->setIcon(LOAD_ICON("process-stop"));
  ui->tbAddFave->setIcon(LOAD_ICON("bookmark-add"));
  ui->tbRemoveFave->setIcon(LOAD_ICON("bookmark-remove"));
  ui->tbSelectionMode->setIcon(LOAD_ICON("selection_mode"));
  _expandIcon = LOAD_ICON("draw-arrow-down");
  _collapseIcon = LOAD_ICON("draw-arrow-up");
  _expandCollapseIcon = &_expandIcon;
  ui->tbExpandCollapse->setIcon(_expandIcon);
}

#ifndef _GMIC_QT_DISABLE_THEMING_
void Bqm_Widget::setDarkTheme()
{
  // SHOW(QStyleFactory::keys());
  qApp->setStyle(QStyleFactory::create("Fusion"));
  QPalette p = qApp->palette();
  p.setColor(QPalette::Window, QColor(53, 53, 53));
  p.setColor(QPalette::Button, QColor(73, 73, 73));
  p.setColor(QPalette::Highlight, QColor(110, 110, 110));
  p.setColor(QPalette::Text, QColor(255, 255, 255));
  p.setColor(QPalette::ButtonText, QColor(255, 255, 255));
  p.setColor(QPalette::WindowText, QColor(255, 255, 255));
  QColor linkColor(130, 130, 150);
  linkColor = linkColor.lighter();
  p.setColor(QPalette::Link, linkColor);
  p.setColor(QPalette::LinkVisited, linkColor);

  const QColor disabledGray(40, 40, 40);
  const QColor disabledTextGray(128, 128, 128);
  p.setColor(QPalette::Disabled, QPalette::Window, disabledGray);
  p.setColor(QPalette::Disabled, QPalette::Base, disabledGray);
  p.setColor(QPalette::Disabled, QPalette::AlternateBase, disabledGray);
  p.setColor(QPalette::Disabled, QPalette::Button, disabledGray);
  p.setColor(QPalette::Disabled, QPalette::Text, disabledTextGray);
  p.setColor(QPalette::Disabled, QPalette::ButtonText, disabledTextGray);
  p.setColor(QPalette::Disabled, QPalette::WindowText, disabledTextGray);
  qApp->setPalette(p);

  p = ui->cbInternetUpdate->palette();
  p.setColor(QPalette::Text, Settings::CheckBoxTextColor);
  p.setColor(QPalette::Base, Settings::CheckBoxBaseColor);
  ui->cbInternetUpdate->setPalette(p);
  ui->cbPreview->setPalette(p);

  const QString css = "QTreeView { background: #505050; }"
                      "QLineEdit { background: #505050; }"
                      "QMenu { background: #505050; border: 1px solid rgb(100,100,100); }"
                      "QMenu::item:selected { background: rgb(110,110,110); }"
                      "QTextEdit { background: #505050; }"
                      "QSpinBox  { background: #505050; }"
                      "QListWidget { background: #505050; }"
                      "QDoubleSpinBox { background: #505050; }"
                      "QToolButton:checked { background: #383838; }"
                      "QToolButton:pressed { background: #383838; }"
                      "QComboBox QAbstractItemView { background: #505050; } "
                      "QGroupBox { border: 1px solid #808080; margin-top: 4ex; } "
                      "QFileDialog QAbstractItemView { background: #505050; } "
                      "QComboBox:editable { background: #505050; } "
                      "QProgressBar { background: #505050; }";
  qApp->setStyleSheet(css);
  ui->inOutSelector->setDarkTheme();
  ui->vSplitterLine->setStyleSheet("QFrame{ border-top: 0px none #a0a0a0; border-bottom: 1px solid rgb(160,160,160);}");
  Settings::UnselectedFilterTextColor = Settings::UnselectedFilterTextColor.darker(150);
}
#endif

void Bqm_Widget::setPluginParameters(const RunParameters & parameters)
{
  _pluginParameters = parameters;
}

void Bqm_Widget::updateFiltersFromSources(int ageLimit, bool useNetwork)
{
  if (useNetwork) {
    ui->progressInfoWidget->startFiltersUpdateAnimationAndShow();
  }
  connect(Updater::getInstance(), &Updater::updateIsDone, this, &Bqm_Widget::onUpdateDownloadsFinished, Qt::UniqueConnection);
  Updater::getInstance()->startUpdate(ageLimit, 60, useNetwork);
}

void Bqm_Widget::onUpdateDownloadsFinished(int status)
{
  ui->progressInfoWidget->stopAnimationAndHide();

  if (status == (int)Updater::UpdateStatus::SomeFailed) {
    if (!ui->progressInfoWidget->hasBeenCanceled()) {
      showUpdateErrors();
    }
  } else if (status == (int)Updater::UpdateStatus::Successful) {
    if (ui->cbInternetUpdate->isChecked()) {
      QMessageBox::information(this, tr("Update completed"), tr("Filter definitions have been updated."));
    } else {
      showMessage(tr("Filter definitions have been updated."), 3000);
    }
  } else if (status == (int)Updater::UpdateStatus::NotNecessary) {
    showMessage(tr("No download was needed."), 3000);
  }

  buildFiltersTree();
  ui->tbUpdateFilters->setEnabled(true);
  if (!_filtersPresenter->currentFilter().hash.isEmpty()) {
    ui->previewWidget->sendUpdateRequest();
  }
}

void Bqm_Widget::buildFiltersTree()
{
  saveCurrentParameters();
  GmicStdLib::Array = Updater::getInstance()->buildFullStdlib();
  const bool withVisibility = filtersSelectionMode();

  _filtersPresenter->clear();
  _filtersPresenter->readFilters();
  _filtersPresenter->readFaves();
  _filtersPresenter->restoreFaveHashLinksAfterCaseChange(); // TODO : Remove, some day!
  if (_gtkFavesShouldBeImported) {
    _filtersPresenter->importGmicGTKFaves();
    _filtersPresenter->saveFaves();
    _gtkFavesShouldBeImported = false;
    QSettings().setValue(FAVES_IMPORT_KEY, true);
  }

  _filtersPresenter->toggleSelectionMode(withVisibility);

  if (_filtersPresenter->currentFilter().hash.isEmpty()) {
    setNoFilter();
    ui->previewWidget->sendUpdateRequest();
  } else {
    activateFilter(false);
  }
}

void Bqm_Widget::retrieveFilterAndParametersFromPluginParameters(QString & hash, QList<QString> & parameters)
{
  if (_pluginParameters.command.empty() && _pluginParameters.filterPath.empty()) {
    return;
  }
  hash.clear();
  parameters.clear();
  try {
    QString plainPath = HtmlTranslator::html2txt(QString::fromStdString(_pluginParameters.filterPath), false);
    QString command;
    QString arguments;
    QStringList providedParameters;
    const FiltersPresenter::Filter & filter = _filtersPresenter->currentFilter();
    if (!plainPath.isEmpty()) {
      _filtersPresenter->selectFilterFromAbsolutePathOrPlainName(plainPath);
      if (!filter.isValid()) {
        throw tr("Plugin was called with a filter path with no matching filter:\n\nPath: %1").arg(QString::fromStdString(_pluginParameters.filterPath));
      }
    }
    if (_pluginParameters.command.empty()) {
      if (filter.isValid()) {
        QString error;
        parameters = filter.isAFave ? filter.defaultParameterValues : FilterParametersWidget::defaultParameterList(filter.parameters, &error, nullptr, nullptr);
        if (notEmpty(error)) {
          throw tr("Error parsing filter parameters definition for filter:\n\n%1\n\nCannot retrieve default parameters.\n\n%2").arg(filter.fullPath).arg(error);
        }
        hash = filter.hash;
      }
    } else {
      // A command (and maybe a path) is provided
      if (!parseGmicUniqueFilterCommand(_pluginParameters.command.c_str(), command, arguments) //
          || !parseGmicFilterParameters(arguments, providedParameters)) {
        throw tr("Plugin was called with a command that cannot be parsed:\n\n%1").arg(elided80(_pluginParameters.command));
      }
      if (plainPath.isEmpty()) {
        _filtersPresenter->selectFilterFromCommand(command);
        if (filter.isInvalid()) {
          throw tr("Plugin was called with a command that cannot be recognized as a filter:\n\nCommand: %1").arg(elided80(_pluginParameters.command));
        }
      } else { // Filter has already been selected (above) from its path
        if (filter.command != command) {
          throw tr("Plugin was called with a command that does not match the provided path:\n\nPath: %1\nCommand: %2\nCommand found for this path : %3") //
              .arg(elided80(_pluginParameters.filterPath))
              .arg(QString::fromStdString(_pluginParameters.command))
              .arg(filter.command);
        }
      }
      QString error;
      QVector<int> lengths;
      auto defaults = FilterParametersWidget::defaultParameterList(filter.parameters, &error, nullptr, &lengths);
      if (notEmpty(error)) {
        throw tr("Error parsing filter parameters definition for filter:\n\n%1\n\nCannot retrieve default parameters.\n\n%2").arg(filter.fullPath).arg(error);
      }
      if (filter.isAFave) {
        // lengths have been computed, but we replace 'defaults' with Fave's ones.
        defaults = filter.defaultParameterValues;
      }
      hash = filter.hash;
      auto expandedDefaults = expandParameterList(defaults, lengths);
      auto completed = completePrefixFromFullList(providedParameters, expandedDefaults);
      parameters = mergeSubsequences(completed, lengths);
    }
  } catch (const QString & errorMessage) {
    hash.clear();
    parameters.clear();
    QMessageBox::critical(this, "Error with plugin arguments", errorMessage);
  }
}

QString Bqm_Widget::screenGeometries()
{
  QList<QScreen *> screens = QGuiApplication::screens();
  QStringList geometries;
  for (QScreen * screen : screens) {
    QRect geometry = screen->geometry();
    geometries.push_back(QString("(%1,%2,%3,%4)").arg(geometry.x()).arg(geometry.y()).arg(geometry.width()).arg(geometry.height()));
  }
  return geometries.join(QString());
}

void Bqm_Widget::updateFilters(bool internet)
{
  ui->tbUpdateFilters->setEnabled(false);
  updateFiltersFromSources(0, internet);
}

void Bqm_Widget::onStartupFiltersUpdateFinished(int status)
{
  bool ok = QObject::disconnect(Updater::getInstance(), &Updater::updateIsDone, this, &Bqm_Widget::onStartupFiltersUpdateFinished);
  Q_ASSERT_X(ok, __PRETTY_FUNCTION__, "Cannot disconnect Updater::updateIsDone from Bqm_Widget::onStartupFiltersUpdateFinished");

  ui->progressInfoWidget->stopAnimationAndHide();
  if (status == (int)Updater::UpdateStatus::SomeFailed) {
    if (Settings::notifyFailedStartupUpdate()) {
      showMessage(tr("Filters update could not be achieved"), 3000);
    }
  } else if (status == (int)Updater::UpdateStatus::Successful) {
    if (Updater::getInstance()->someNetworkUpdateAchieved()) {
      showMessage(tr("Filter definitions have been updated."), 4000);
    }
  } else if (status == (int)Updater::UpdateStatus::NotNecessary) {
  }

  if (QSettings().value(FAVES_IMPORT_KEY, false).toBool() || !FavesModelReader::gmicGTKFaveFileAvailable()) {
    _gtkFavesShouldBeImported = false;
  } else {
    _gtkFavesShouldBeImported = askUserForGTKFavesImport();
  }
  buildFiltersTree();
  ui->searchField->setFocus();

  // Let the standalone version load an image, if necessary (not pretty)
  if (GmicQtHost::ApplicationName.isEmpty()) {
    LayersExtentProxy::clear();
    QSize extent = LayersExtentProxy::getExtent(ui->inOutSelector->inputMode());
    ui->previewWidget->setFullImageSize(extent);
    ui->previewWidget->update();
  }

  // Retrieve and select previously selected filter
  QString hash = QSettings().value("SelectedFilter", QString()).toString();
  if (_newSession || !_lastExecutionOK) {
    hash.clear();
  }

  // If plugin was called with parameters
  QList<QString> pluginParametersCommandArguments;
  retrieveFilterAndParametersFromPluginParameters(hash, pluginParametersCommandArguments);

  _filtersPresenter->selectFilterFromHash(hash, false);
  if (_filtersPresenter->currentFilter().hash.isEmpty()) {
    _filtersPresenter->expandFaveFolder();
    _filtersPresenter->adjustViewSize();
    ui->previewWidget->setPreviewFactor(PreviewFactorFullImage, true);
  } else {
    _filtersPresenter->adjustViewSize();
    activateFilter(true, pluginParametersCommandArguments);
    if (ui->cbPreview->isChecked()) {
      ui->previewWidget->sendUpdateRequest();
    }
  }
  // Preview update is triggered when PreviewWidget receives
  // the WindowActivate Event (while pendingResize is true
  // after the very first resize event).
}

void Bqm_Widget::showZoomWarningIfNeeded()
{
  const FiltersPresenter::Filter & currentFilter = _filtersPresenter->currentFilter();
  ui->zoomLevelSelector->showWarning(!currentFilter.hash.isEmpty() && !currentFilter.isAccurateIfZoomed && !ui->previewWidget->isAtDefaultZoom());
}

void Bqm_Widget::updateZoomLabel(double zoom)
{
  ui->zoomLevelSelector->display(zoom);
}

void Bqm_Widget::onFiltersSelectionModeToggled(bool on)
{
  _filtersPresenter->toggleSelectionMode(on);
}

void Bqm_Widget::onPreviewCheckBoxToggled(bool on)
{
  if (!on) {
    _processor.cancel();
  }
  ui->previewWidget->onPreviewToggled(on);
}

void Bqm_Widget::onFilterSelectionChanged()
{
  activateFilter(false);
  ui->previewWidget->sendUpdateRequest();
}

void Bqm_Widget::onEscapeKeyPressed()
{
  ui->searchField->clear();
  if (_processor.isProcessing()) {
    if (_processor.isProcessingFullImage()) {
      ui->progressInfoWidget->onCancelClicked();
    } else {
      _processor.cancel();
      ui->previewWidget->displayOriginalImage();
      ui->tbUpdateFilters->setEnabled(true);
    }
  }
}

void Bqm_Widget::clearMessage()
{
  ui->messageLabel->setText(QString());
  if (!_messageTimerID) {
    return;
  }
  killTimer(_messageTimerID);
  _messageTimerID = 0;
}

void Bqm_Widget::clearRightMessage()
{
  ui->rightMessageLabel->hide();
  ui->rightMessageLabel->clear();
}

void Bqm_Widget::showRightMessage(const QString & text)
{
  ui->rightMessageLabel->setText(text);
  ui->rightMessageLabel->show();
}

void Bqm_Widget::timerEvent(QTimerEvent * e)
{
  if (e->timerId() == _messageTimerID) {
    clearMessage();
    e->accept();
  }
  e->ignore();
}

void Bqm_Widget::showMessage(const QString & text, int ms)
{
  clearMessage();
  if (!text.isEmpty()) {
    ui->messageLabel->setText(text);
    if (ms) {
      _messageTimerID = startTimer(ms);
    }
  }
}

void Bqm_Widget::showUpdateErrors()
{
  QString message(tr("The update could not be achieved<br>"
                     "because of the following errors:<br>"));
  QList<QString> errors = Updater::getInstance()->errorMessages();
  for (const QString & s : errors) {
    message += QString("<br/>%1").arg(s);
  }
  QMessageBox::information(this, tr("Update error"), message);
}

void Bqm_Widget::makeConnections()
{
  connect(ui->zoomLevelSelector, &ZoomLevelSelector::valueChanged, ui->previewWidget, &PreviewWidget::setZoomLevel);

  connect(ui->previewWidget, &PreviewWidget::zoomChanged, this, &Bqm_Widget::showZoomWarningIfNeeded);
  connect(ui->previewWidget, &PreviewWidget::zoomChanged, this, &Bqm_Widget::updateZoomLabel);
  connect(ui->previewWidget, &PreviewWidget::previewVisibleRectIsChanging, &_processor, &GmicProcessor::cancel);
  connect(_filtersPresenter, &FiltersPresenter::filterSelectionChanged, this, &Bqm_Widget::onFilterSelectionChanged);
  connect(ui->pbOk, &QPushButton::clicked, this, &Bqm_Widget::onOkClicked);
  connect(ui->pbCancel, &QPushButton::clicked, this, &Bqm_Widget::onCancelClicked);
  connect(ui->pbApply, &QPushButton::clicked, this, &Bqm_Widget::onApplyClicked);
  connect(ui->tbResetParameters, &QToolButton::clicked, this, &Bqm_Widget::onReset);
  connect(ui->tbCopyCommand, &QToolButton::clicked, this, &Bqm_Widget::onCopyGMICCommand);
  connect(ui->tbUpdateFilters, &QToolButton::clicked, this, &Bqm_Widget::onUpdateFiltersClicked);
  connect(ui->pbSettings, &QPushButton::clicked, this, &Bqm_Widget::onSettingsClicked);
  connect(ui->pbFullscreen, &QPushButton::toggled, this, &Bqm_Widget::onToggleFullScreen);
  connect(ui->filterParams, &FilterParametersWidget::valueChanged, this, &Bqm_Widget::onParametersChanged);
  connect(ui->previewWidget, &PreviewWidget::previewUpdateRequested, this, QOverload<>::of(&Bqm_Widget::onPreviewUpdateRequested));
  connect(ui->previewWidget, &PreviewWidget::keypointPositionsChanged, this, &Bqm_Widget::onPreviewKeypointsEvent);
  connect(ui->zoomLevelSelector, &ZoomLevelSelector::zoomIn, ui->previewWidget, QOverload<>::of(&PreviewWidget::zoomIn));
  connect(ui->zoomLevelSelector, &ZoomLevelSelector::zoomOut, ui->previewWidget, QOverload<>::of(&PreviewWidget::zoomOut));
  connect(ui->zoomLevelSelector, &ZoomLevelSelector::zoomReset, this, &Bqm_Widget::onPreviewZoomReset);
  connect(ui->tbAddFave, &QToolButton::clicked, this, &Bqm_Widget::onAddFave);
  connect(_filtersPresenter, &FiltersPresenter::faveAdditionRequested, this, &Bqm_Widget::onAddFave);
  connect(ui->tbRemoveFave, &QToolButton::clicked, this, &Bqm_Widget::onRemoveFave);
  connect(ui->tbRenameFave, &QToolButton::clicked, this, &Bqm_Widget::onRenameFave);
  connect(ui->inOutSelector, &InOutPanel::inputModeChanged, this, &Bqm_Widget::onInputModeChanged);
  connect(ui->cbPreview, &QCheckBox::toggled, this, &Bqm_Widget::onPreviewCheckBoxToggled);
  connect(ui->searchField, &SearchFieldWidget::textChanged, this, &Bqm_Widget::search);
  connect(ui->tbExpandCollapse, &QToolButton::clicked, this, &Bqm_Widget::expandOrCollapseFolders);
  connect(ui->progressInfoWidget, &ProgressInfoWidget::cancel, this, &Bqm_Widget::onProgressionWidgetCancelClicked);
  connect(ui->tbSelectionMode, &QToolButton::toggled, this, &Bqm_Widget::onFiltersSelectionModeToggled);
  connect(&_processor, &GmicProcessor::previewImageAvailable, this, &Bqm_Widget::onPreviewImageAvailable);
  connect(&_processor, &GmicProcessor::previewCommandFailed, this, &Bqm_Widget::onPreviewError);
  connect(&_processor, &GmicProcessor::fullImageProcessingFailed, this, &Bqm_Widget::onFullImageProcessingError);
  connect(&_processor, &GmicProcessor::fullImageProcessingDone, this, &Bqm_Widget::onFullImageProcessingDone);
  connect(&_processor, &GmicProcessor::aboutToSendImagesToHost, ui->progressInfoWidget, &ProgressInfoWidget::stopAnimationAndHide);
  connect(_filtersPresenter, &FiltersPresenter::faveNameChanged, this, &Bqm_Widget::setFilterName);
}

void Bqm_Widget::onPreviewUpdateRequested()
{
  clearMessage();
  clearRightMessage();
  onPreviewUpdateRequested(false);
}

void Bqm_Widget::onPreviewUpdateRequested(bool synchronous)
{
  if (!ui->cbPreview->isChecked()) {
    ui->previewWidget->invalidateSavedPreview();
    return;
  }
  _processor.init();
  if (_filtersPresenter->currentFilter().isNoPreviewFilter()) {
    ui->previewWidget->displayOriginalImage();
    return;
  }
  ui->tbUpdateFilters->setEnabled(false);

  const FiltersPresenter::Filter currentFilter = _filtersPresenter->currentFilter();
  GmicProcessor::FilterContext context;
  context.requestType = synchronous ? GmicProcessor::FilterContext::RequestType::SynchronousPreview : GmicProcessor::FilterContext::RequestType::Preview;
  GmicProcessor::FilterContext::VisibleRect & rect = context.visibleRect;
  ui->previewWidget->normalizedVisibleRect(rect.x, rect.y, rect.w, rect.h);

  context.inputOutputState = ui->inOutSelector->state();
  ui->previewWidget->getPositionStringCorrection(context.positionStringCorrection.xFactor, context.positionStringCorrection.yFactor);
  context.zoomFactor = ui->previewWidget->currentZoomFactor();
  context.previewWindowWidth = ui->previewWidget->width();
  context.previewWindowHeight = ui->previewWidget->height();
  context.previewTimeout = Settings::previewTimeout();
  // context.filterName = currentFilter.plainTextName; // Unused in this context
  // context.filterHash = currentFilter.hash; // Unused in this context
  context.filterCommand = currentFilter.previewCommand;
  context.filterArguments = ui->filterParams->valueString();
  context.previewFromFullImage = currentFilter.previewFromFullImage;
  _processor.setContext(context);
  _processor.execute();

  ui->filterParams->clearButtonParameters();
  _okButtonShouldApply = true;
}

void Bqm_Widget::onPreviewKeypointsEvent(unsigned int flags, unsigned long time)
{
  if (flags & PreviewWidget::KeypointMouseReleaseEvent) {
    if (flags & PreviewWidget::KeypointBurstEvent) {
      // Notify the filter twice (synchronously) so that it can guess that the button has been released
      ui->filterParams->setKeypoints(ui->previewWidget->keypoints(), false);
      onPreviewUpdateRequested(true);
      onPreviewUpdateRequested(true);
    } else {
      ui->filterParams->setKeypoints(ui->previewWidget->keypoints(), true);
    }
    _lastPreviewKeypointBurstUpdateTime = 0;
  } else {
    ui->filterParams->setKeypoints(ui->previewWidget->keypoints(), false);
    if ((flags & PreviewWidget::KeypointBurstEvent)) {
      const auto t = static_cast<ulong>(_processor.lastPreviewFilterExecutionDurationMS());
      const bool keypointBurstEnabled = (t <= KEYPOINTS_INTERACTIVE_LOWER_DELAY_MS) ||
                                        ((t <= KEYPOINTS_INTERACTIVE_UPPER_DELAY_MS) && ((ulong)_processor.averagePreviewFilterExecutionDuration() <= KEYPOINTS_INTERACTIVE_MIDDLE_DELAY_MS));
      ulong msSinceLastBurstEvent = time - _lastPreviewKeypointBurstUpdateTime;
      if (keypointBurstEnabled && (msSinceLastBurstEvent >= (ulong)_processor.lastPreviewFilterExecutionDurationMS())) {
        onPreviewUpdateRequested(true);
        _lastPreviewKeypointBurstUpdateTime = time;
      }
    }
  }
}

void Bqm_Widget::onPreviewImageAvailable()
{
  ui->filterParams->setValues(_processor.gmicStatus(), false);
  ui->filterParams->setVisibilityStates(_processor.parametersVisibilityStates());
  // Make sure keypoint positions are synchronized with gmic status
  if (ui->filterParams->hasKeypoints()) {
    ui->previewWidget->setKeypoints(ui->filterParams->keypoints());
  }
  ui->previewWidget->setPreviewImage(_processor.previewImage());
  ui->previewWidget->enableRightClick();
  ui->tbUpdateFilters->setEnabled(true);
  if (_pendingActionAfterCurrentProcessing == ProcessingAction::Close) {
    close();
  }
}

void Bqm_Widget::onPreviewError(const QString & message)
{
  ui->previewWidget->setPreviewErrorMessage(message);
  ui->previewWidget->enableRightClick();
  ui->tbUpdateFilters->setEnabled(true);
  if (_pendingActionAfterCurrentProcessing == ProcessingAction::Close) {
    close();
  }
}

void Bqm_Widget::onParametersChanged()
{
  if (ui->filterParams->hasKeypoints()) {
    ui->previewWidget->setKeypoints(ui->filterParams->keypoints());
  }
  ui->previewWidget->sendUpdateRequest();
}

bool Bqm_Widget::isAccepted()
{
  return _isAccepted;
}

void Bqm_Widget::setFilterName(const QString & text)
{
  ui->filterName->setText(QString("<b>%1</b>").arg(text));
}

void Bqm_Widget::processImage()
{
  // Abort any already running thread
  _processor.init();
  const FiltersPresenter::Filter currentFilter = _filtersPresenter->currentFilter();
  if (currentFilter.isNoApplyFilter()) {
    return;
  }

  ui->progressInfoWidget->startFilterThreadAnimationAndShow(true);
  enableWidgetList(false);

  GmicProcessor::FilterContext context;
  context.requestType = GmicProcessor::FilterContext::RequestType::FullImage;
  GmicProcessor::FilterContext::VisibleRect & rect = context.visibleRect;
  rect.x = rect.y = rect.w = rect.h = -1;
  context.inputOutputState = ui->inOutSelector->state();
  context.filterName = currentFilter.plainTextName;
  context.filterFullPath = currentFilter.fullPath;
  context.filterHash = currentFilter.hash;
  context.filterCommand = currentFilter.command;
  ui->filterParams->updateValueString(false); // Required to get up-to-date values of text parameters
  context.filterArguments = ui->filterParams->valueString();
  context.previewFromFullImage = false;
  _processor.setGmicStatusQuotedParameters(ui->filterParams->quotedParameters());
  ui->filterParams->clearButtonParameters();
  _processor.setContext(context);
  _processor.execute();
}

void Bqm_Widget::onFullImageProcessingError(const QString & message)
{
  ui->progressInfoWidget->stopAnimationAndHide();
  QMessageBox::warning(this, tr("Error"), message, QMessageBox::Close);
  enableWidgetList(true);
  if ((_pendingActionAfterCurrentProcessing == ProcessingAction::Ok || _pendingActionAfterCurrentProcessing == ProcessingAction::Close)) {
    close();
  }
}

void Bqm_Widget::onInputModeChanged(InputMode mode)
{
  PersistentMemory::clear();
  ui->previewWidget->setFullImageSize(LayersExtentProxy::getExtent(mode));
  ui->previewWidget->sendUpdateRequest();
}

void Bqm_Widget::onVeryFirstShowEvent()
{
  adjustVerticalSplitter();
  if (_newSession) {
    Logger::clear();
  }
  QObject::connect(Updater::getInstance(), &Updater::updateIsDone, this, &Bqm_Widget::onStartupFiltersUpdateFinished);
  Logger::setMode(Settings::outputMessageMode());
  Updater::setOutputMessageMode(Settings::outputMessageMode());
  int ageLimit;
  {
    QSettings settings;
    ageLimit = settings.value(INTERNET_UPDATE_PERIODICITY_KEY, INTERNET_DEFAULT_PERIODICITY).toInt();
  }
  const bool useNetwork = (ageLimit != INTERNET_NEVER_UPDATE_PERIODICITY);
  ui->progressInfoWidget->startFiltersUpdateAnimationAndShow();
  Updater::getInstance()->startUpdate(ageLimit, 4, useNetwork);
}

void Bqm_Widget::setZoomConstraint()
{
  const FiltersPresenter::Filter & currentFilter = _filtersPresenter->currentFilter();
  ZoomConstraint constraint;
  if (currentFilter.hash.isEmpty() || currentFilter.isAccurateIfZoomed || Settings::previewZoomAlwaysEnabled() || (currentFilter.previewFactor == PreviewFactorAny)) {
    constraint = ZoomConstraint::Any;
  } else if (currentFilter.previewFactor == PreviewFactorActualSize) {
    constraint = ZoomConstraint::OneOrMore;
  } else {
    constraint = ZoomConstraint::Fixed;
  }
  showZoomWarningIfNeeded();
  ui->zoomLevelSelector->setZoomConstraint(constraint);
  ui->previewWidget->setZoomConstraint(constraint);
}

void Bqm_Widget::onFullImageProcessingDone()
{
  ui->progressInfoWidget->stopAnimationAndHide();
  enableWidgetList(true);
  ui->previewWidget->update();
  ui->filterParams->setValues(_processor.gmicStatus(), false);
  ui->filterParams->setVisibilityStates(_processor.parametersVisibilityStates());
  if ((_pendingActionAfterCurrentProcessing == ProcessingAction::Ok || _pendingActionAfterCurrentProcessing == ProcessingAction::Close)) {
    _isAccepted = (_pendingActionAfterCurrentProcessing == ProcessingAction::Ok);
    close();
  } else {
    // Extent cache has been cleared by the GmicProcessor
    QSize extent = LayersExtentProxy::getExtent(ui->inOutSelector->inputMode());
    ui->previewWidget->updateFullImageSizeIfDifferent(extent);
    ui->previewWidget->sendUpdateRequest();
    _okButtonShouldApply = false;
    if (_pendingActionAfterCurrentProcessing == ProcessingAction::Apply) {
      showRightMessage(QString(tr("[Elapsed time: %1]")).arg(readableDuration(_processor.lastCompletedExecutionTime())));
    }
  }
}

void Bqm_Widget::expandOrCollapseFolders()
{
  if (_expandCollapseIcon == &_expandIcon) {
    _filtersPresenter->expandAll();
    ui->tbExpandCollapse->setIcon(_collapseIcon);
    _expandCollapseIcon = &_collapseIcon;
  } else {
    ui->tbExpandCollapse->setIcon(_expandIcon);
    _filtersPresenter->collapseAll();
    _expandCollapseIcon = &_expandIcon;
  }
}

void Bqm_Widget::search(const QString & text)
{
  _filtersPresenter->applySearchCriterion(text);
}

void Bqm_Widget::onApplyClicked()
{
  clearMessage();
  clearRightMessage();
  _pendingActionAfterCurrentProcessing = ProcessingAction::Apply;
  processImage();
}

void Bqm_Widget::onOkClicked()
{
  if (_filtersPresenter->currentFilter().isNoApplyFilter()) {
    _isAccepted = _processor.completedFullImageProcessingCount();
    close();
    return;
  }
  if (_okButtonShouldApply) {
    clearMessage();
    clearRightMessage();
    _pendingActionAfterCurrentProcessing = ProcessingAction::Ok;
    processImage();
  } else {
    _isAccepted = _processor.completedFullImageProcessingCount();
    close();
  }
}

void Bqm_Widget::onCancelClicked()
{
  if (_processor.isProcessing() && confirmAbortProcessingOnCloseRequest()) {
    if (_processor.isProcessing()) {
      _pendingActionAfterCurrentProcessing = ProcessingAction::Close;
      connect(&_processor, &GmicProcessor::noMoreUnfinishedJobs, this, &Bqm_Widget::close);
      ui->progressInfoWidget->showBusyIndicator();
      ui->previewWidget->setOverlayMessage(tr("Waiting for cancelled jobs..."));
      _processor.cancel();
    } else {
      close();
    }
  } else {
    close();
  }
}

void Bqm_Widget::onProgressionWidgetCancelClicked()
{
  if (ui->progressInfoWidget->mode() == ProgressInfoWidget::Mode::GmicProcessing) {
    if (_processor.isProcessing()) {
      _pendingActionAfterCurrentProcessing = ProcessingAction::NoAction;
      _processor.cancel();
      ui->progressInfoWidget->stopAnimationAndHide();
      enableWidgetList(true);
    }
  }
  if (ui->progressInfoWidget->mode() == ProgressInfoWidget::Mode::FiltersUpdate) {
    Updater::getInstance()->cancelAllPendingDownloads();
  }
}

void Bqm_Widget::onReset()
{
  if (!_filtersPresenter->currentFilter().hash.isEmpty() && _filtersPresenter->currentFilter().isAFave) {
    PersistentMemory::clear();
    ui->filterParams->setVisibilityStates(_filtersPresenter->currentFilter().defaultVisibilityStates);
    ui->filterParams->setValues(_filtersPresenter->currentFilter().defaultParameterValues, true);
    return;
  }
  if (!_filtersPresenter->currentFilter().isNoPreviewFilter()) {
    PersistentMemory::clear();
    ui->filterParams->reset(true);
  }
}

void Bqm_Widget::onCopyGMICCommand()
{
  QClipboard * clipboard = QGuiApplication::clipboard();
  QString fullCommand = _filtersPresenter->currentFilter().command;
  fullCommand += " ";
  fullCommand += ui->filterParams->valueString();
  clipboard->setText(fullCommand, QClipboard::Clipboard);
}

void Bqm_Widget::onPreviewZoomReset()
{
  if (!_filtersPresenter->currentFilter().hash.isEmpty()) {
    ui->previewWidget->setPreviewFactor(_filtersPresenter->currentFilter().previewFactor, true);
    ui->previewWidget->sendUpdateRequest();
    ui->zoomLevelSelector->showWarning(false);
  }
}

void Bqm_Widget::onUpdateFiltersClicked()
{
  updateFilters(ui->cbInternetUpdate->isChecked());
}

void Bqm_Widget::saveCurrentParameters()
{
  QString hash = ui->filterParams->filterHash();
  if (!hash.isEmpty()) {
    ParametersCache::setValues(hash, ui->filterParams->valueStringList());
    ParametersCache::setVisibilityStates(hash, ui->filterParams->visibilityStates());
    ParametersCache::setInputOutputState(hash, ui->inOutSelector->state(), _filtersPresenter->currentFilter().defaultInputMode);
  }
}

void Bqm_Widget::saveSettings()
{
  QSettings settings;

  _filtersPresenter->saveSettings(settings);

  // Cleanup obsolete keys
  settings.remove("OutputMessageModeIndex");
  settings.remove("OutputMessageModeValue");
  settings.remove("InputLayers");
  settings.remove("OutputMode");
  settings.remove("PreviewMode");
  settings.remove("Config/VerticalSplitterSize0");
  settings.remove("Config/VerticalSplitterSize1");
  settings.remove("Config/VerticalSplitterSizeTop");
  settings.remove("Config/VerticalSplitterSizeBottom");

  // Save all settings

  Settings::save(settings);
  settings.setValue("LastExecution/gmic_version", gmic_version);
  _processor.saveSettings(settings);
  settings.setValue("SelectedFilter", _filtersPresenter->currentFilter().hash);
  settings.setValue("Config/Bqm_WidgetPosition", frameGeometry().topLeft());
  settings.setValue("Config/Bqm_WidgetRect", rect());
  settings.setValue("Config/Bqm_WidgetMaximized", isMaximized());
  settings.setValue("Config/ScreenGeometries", screenGeometries());
  settings.setValue("Config/PreviewEnabled", ui->cbPreview->isChecked());
  settings.setValue("LastExecution/ExitedNormally", true);
  settings.setValue("LastExecution/HostApplicationID", host_app_pid());
  QList<int> splitterSizes = ui->splitter->sizes();
  for (int i = 0; i < splitterSizes.size(); ++i) {
    settings.setValue(QString("Config/PanelSize%1").arg(i), splitterSizes.at(i));
  }
  splitterSizes = ui->verticalSplitter->sizes();
  if (!_filtersPresenter->currentFilter().hash.isEmpty() && !_filtersPresenter->currentFilter().isInvalid()) {
    settings.setValue(QString("Config/ParamsVerticalSplitterSizeTop"), splitterSizes.at(0));
    settings.setValue(QString("Config/ParamsVerticalSplitterSizeBottom"), splitterSizes.at(1));
  }
  settings.setValue(REFRESH_USING_INTERNET_KEY, ui->cbInternetUpdate->isChecked());
}

void Bqm_Widget::loadSettings()
{
  QSettings settings;
  _filtersPresenter->loadSettings(settings);
  _lastExecutionOK = settings.value("LastExecution/ExitedNormally", true).toBool();
  _newSession = host_app_pid() != settings.value("LastExecution/HostApplicationID", 0).toUInt();
  settings.setValue("LastExecution/ExitedNormally", false);
  ui->inOutSelector->reset();

  bool previewEnabled = settings.value("Config/PreviewEnabled", true).toBool();
  ui->cbPreview->setChecked(previewEnabled);
  ui->previewWidget->setPreviewEnabled(previewEnabled);

  // Preview position
  if (settings.value("Config/PreviewPosition", "Left").toString() == "Left") {
    setPreviewPosition(PreviewPosition::Left);
  }
#ifndef _GMIC_QT_DISABLE_THEMING_
  if (Settings::darkThemeEnabled()) {
    setDarkTheme();
  }
#endif
  if (!Settings::visibleLogos()) {
    ui->logosLabel->hide();
  }

  // Mainwindow geometry
  QPoint position = settings.value("Config/Bqm_WidgetPosition", QPoint(20, 20)).toPoint();
  QRect r = settings.value("Config/Bqm_WidgetRect", QRect()).toRect();
  const bool sameScreenGeometries = (settings.value("Config/ScreenGeometries", QString()).toString() == screenGeometries());
  if (settings.value("Config/Bqm_WidgetMaximized", false).toBool()) {
    ui->pbFullscreen->setChecked(true);
  } else {
    if (r.isValid() && sameScreenGeometries) {
      if ((r.width() < 640) || (r.height() < 400)) {
        r.setSize(QSize(640, 400));
      }
      setGeometry(r);
      move(position);
    } else {
      QList<QScreen *> screens = QGuiApplication::screens();
      if (!screens.isEmpty()) {
        QRect screenSize = screens.front()->geometry();
        screenSize.setWidth(static_cast<int>(screenSize.width() * 0.66));
        screenSize.setHeight(static_cast<int>(screenSize.height() * 0.66));
        screenSize.moveCenter(screens.front()->geometry().center());
        setGeometry(screenSize);
        int w = screenSize.width();
        ui->splitter->setSizes(QList<int>() << static_cast<int>(w * 0.4) << static_cast<int>(w * 0.2) << static_cast<int>(w * 0.4));
      }
    }
  }

  // Splitter sizes
  QList<int> sizes;
  for (int i = 0; i < 3; ++i) {
    int s = settings.value(QString("Config/PanelSize%1").arg(i), 0).toInt();
    if (s) {
      sizes.push_back(s);
    }
  }
  if (sizes.size() == 3) {
    ui->splitter->setSizes(sizes);
  }

  ui->cbInternetUpdate->setChecked(settings.value("Config/RefreshInternetUpdate", true).toBool());
}

void Bqm_Widget::setPreviewPosition(Bqm_Widget::PreviewPosition position)
{
  if (position == _previewPosition) {
    return;
  }
  _previewPosition = position;

  auto layout = dynamic_cast<QHBoxLayout *>(ui->belowPreviewWidget->layout());
  if (layout) {
    layout->removeWidget(ui->belowPreviewPadding);
    layout->removeWidget(ui->logosLabel);
    if (position == Bqm_Widget::PreviewPosition::Left) {
      layout->addWidget(ui->logosLabel);
      layout->addWidget(ui->belowPreviewPadding);
    } else {
      layout->addWidget(ui->belowPreviewPadding);
      layout->addWidget(ui->logosLabel);
    }
  }

  // Swap splitter widgets
  QWidget * preview;
  QWidget * list;
  QWidget * params;
  if (position == Bqm_Widget::PreviewPosition::Right) {
    ui->messageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    preview = ui->splitter->widget(0);
    list = ui->splitter->widget(1);
    params = ui->splitter->widget(2);
  } else {
    ui->messageLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    list = ui->splitter->widget(0);
    params = ui->splitter->widget(1);
    preview = ui->splitter->widget(2);
  }
  preview->hide();
  list->hide();
  params->hide();
  preview->setParent(this);
  list->setParent(this);
  params->setParent(this);
  if (position == Bqm_Widget::PreviewPosition::Right) {
    ui->splitter->addWidget(list);
    ui->splitter->addWidget(params);
    ui->splitter->addWidget(preview);
  } else {
    ui->splitter->addWidget(preview);
    ui->splitter->addWidget(list);
    ui->splitter->addWidget(params);
  }
  preview->show();
  list->show();
  params->show();
  ui->logosLabel->setAlignment(Qt::AlignVCenter | ((_previewPosition == PreviewPosition::Right) ? Qt::AlignRight : Qt::AlignLeft));
}

void Bqm_Widget::adjustVerticalSplitter()
{
  QList<int> sizes;
  QSettings settings;
  sizes.push_back(settings.value(QString("Config/ParamsVerticalSplitterSizeTop"), -1).toInt());
  sizes.push_back(settings.value(QString("Config/ParamsVerticalSplitterSizeBottom"), -1).toInt());
  const int splitterHeight = ui->verticalSplitter->height();
  if ((sizes.front() != -1) && (sizes.back() != -1) && (sizes.front() + sizes.back() <= splitterHeight)) {
    ui->verticalSplitter->setSizes(sizes);
  } else {
    const int inOutHeight = std::max(ui->inOutSelector->sizeHint().height(), 75);
    if (splitterHeight > inOutHeight) {
      sizes.clear();
      sizes.push_back(splitterHeight - inOutHeight);
      sizes.push_back(inOutHeight);
      ui->verticalSplitter->setSizes(sizes);
    }
  }
}

bool Bqm_Widget::filtersSelectionMode()
{
  return ui->tbSelectionMode->isChecked();
}

void Bqm_Widget::activateFilter(bool resetZoom, const QList<QString> & values)
{
  saveCurrentParameters();
  const FiltersPresenter::Filter & filter = _filtersPresenter->currentFilter();
  _processor.resetLastPreviewFilterExecutionDurations();

  if (filter.hash.isEmpty()) {
    setNoFilter();
  } else {
    QList<QString> savedValues = values.isEmpty() ? ParametersCache::getValues(filter.hash) : values;
    if (savedValues.isEmpty() && filter.isAFave) {
      savedValues = filter.defaultParameterValues;
    }
    QList<int> savedVisibilityStates = ParametersCache::getVisibilityStates(filter.hash);
    if (savedVisibilityStates.isEmpty() && filter.isAFave) {
      savedVisibilityStates = filter.defaultVisibilityStates;
    }
    if (!ui->filterParams->build(filter.name, filter.hash, filter.parameters, savedValues, savedVisibilityStates)) {
      _filtersPresenter->setInvalidFilter();
      ui->previewWidget->setKeypoints(KeypointList());
    } else {
      ui->previewWidget->setKeypoints(ui->filterParams->keypoints());
    }
    setFilterName(FilterTextTranslator::translate(filter.name));
    ui->inOutSelector->enable();
    if (ui->inOutSelector->hasActiveControls()) {
      ui->inOutSelector->show();
    } else {
      ui->inOutSelector->hide();
    }

    InputOutputState inOutState = ParametersCache::getInputOutputState(filter.hash);
    if (inOutState.inputMode == InputMode::Unspecified) {
      if ((filter.defaultInputMode != InputMode::Unspecified)) {
        inOutState.inputMode = filter.defaultInputMode;
      } else {
        inOutState.inputMode = DefaultInputMode;
      }
    }

    // Take plugin parameters into account
    if (_pluginParameters.inputMode != InputMode::Unspecified) {
      inOutState.inputMode = _pluginParameters.inputMode;
      _pluginParameters.inputMode = InputMode::Unspecified;
    }
    if (_pluginParameters.outputMode != OutputMode::Unspecified) {
      inOutState.outputMode = _pluginParameters.outputMode;
      _pluginParameters.outputMode = OutputMode::Unspecified;
    }

    ui->inOutSelector->setState(inOutState, false);

    ui->previewWidget->updateFullImageSizeIfDifferent(LayersExtentProxy::getExtent(ui->inOutSelector->inputMode()));
    ui->filterName->setVisible(true);
    ui->tbAddFave->setEnabled(true);
    ui->previewWidget->setPreviewFactor(filter.previewFactor, resetZoom);
    setZoomConstraint();
    _okButtonShouldApply = true;
    ui->tbResetParameters->setVisible(true);
    ui->tbCopyCommand->setVisible(true);
    ui->tbRemoveFave->setEnabled(filter.isAFave);
    ui->tbRenameFave->setEnabled(filter.isAFave);
  }
}

void Bqm_Widget::setNoFilter()
{
  PersistentMemory::clear();
  ui->filterParams->setNoFilter(_filtersPresenter->errorMessage());
  ui->previewWidget->disableRightClick();
  ui->previewWidget->setKeypoints(KeypointList());
  ui->inOutSelector->hide();
  ui->inOutSelector->setState(InputOutputState::Default, false);
  ui->filterName->setVisible(false);
  ui->tbAddFave->setEnabled(false);
  ui->tbCopyCommand->setVisible(false);
  ui->tbResetParameters->setVisible(false);
  ui->zoomLevelSelector->showWarning(false);
  _okButtonShouldApply = false;
  ui->tbRemoveFave->setEnabled(_filtersPresenter->danglingFaveIsSelected());
  ui->tbRenameFave->setEnabled(false);
}

void Bqm_Widget::showEvent(QShowEvent * event)
{
  TIMING;
  event->accept();
  if (!_showEventReceived) {
    _showEventReceived = true;
    onVeryFirstShowEvent();
  }
}

void Bqm_Widget::resizeEvent(QResizeEvent * e)
{
  // Check if size is reducing
  if ((e->size().width() < e->oldSize().width() || e->size().height() < e->oldSize().height()) && ui->pbFullscreen->isChecked() && (windowState() & Qt::WindowMaximized)) {
    ui->pbFullscreen->toggle();
  }
}

bool Bqm_Widget::askUserForGTKFavesImport()
{
  QMessageBox messageBox(QMessageBox::Question, tr("Import faves"), QString(tr("Do you want to import faves from file below?<br/>%1")).arg(FavesModelReader::gmicGTKFavesFilename()),
                         QMessageBox::Yes | QMessageBox::No, this);
  messageBox.setDefaultButton(QMessageBox::Yes);
  QCheckBox * cb = new QCheckBox(tr("Don't ask again"));
#ifndef _GMIC_QT_DISABLE_THEMING_
  if (Settings::darkThemeEnabled()) {
    QPalette p = cb->palette();
    p.setColor(QPalette::Text, Settings::CheckBoxTextColor);
    p.setColor(QPalette::Base, Settings::CheckBoxBaseColor);
    cb->setPalette(p);
  }
#endif
  messageBox.setCheckBox(cb);
  int choice = messageBox.exec();
  if (choice != QMessageBox::Yes) {
    if (cb->isChecked()) {
      QSettings().setValue(FAVES_IMPORT_KEY, true);
    }
    return false;
  }
  return true;
}

void Bqm_Widget::onAddFave()
{
  if (_filtersPresenter->currentFilter().hash.isEmpty()) {
    return;
  }
  saveCurrentParameters();
  _filtersPresenter->addSelectedFilterAsNewFave(ui->filterParams->valueStringList(), ui->filterParams->visibilityStates(), ui->inOutSelector->state());
}
void Bqm_Widget::onRemoveFave()
{
  _filtersPresenter->removeSelectedFave();
}

void Bqm_Widget::onRenameFave()
{
  _filtersPresenter->editSelectedFaveName();
}

void Bqm_Widget::onToggleFullScreen(bool on)
{
  if (on && !(windowState() & Qt::WindowMaximized)) {
    showMaximized();
  }
  if (!on && (windowState() & Qt::WindowMaximized)) {
    showNormal();
  }
}

void Bqm_Widget::onSettingsClicked()
{
  QList<int> splitterSizes = ui->splitter->sizes();

  int previewWidth;
  int paramsWidth;
  int treeWidth;
  if (_previewPosition == PreviewPosition::Left) {
    previewWidth = splitterSizes.at(0);
    paramsWidth = splitterSizes.at(2);
    treeWidth = splitterSizes.at(1);
  } else {
    previewWidth = splitterSizes.at(2);
    paramsWidth = splitterSizes.at(1);
    treeWidth = splitterSizes.at(0);
  }

  DialogSettings dialog(this);
  dialog.exec();
  bool previewPositionChanged = (_previewPosition != Settings::previewPosition());
  setPreviewPosition(Settings::previewPosition());
  if (previewPositionChanged) {
    splitterSizes.clear();
    if (_previewPosition == PreviewPosition::Left) {
      splitterSizes.push_back(previewWidth);
      splitterSizes.push_back(treeWidth);
      splitterSizes.push_back(paramsWidth);
    } else {
      splitterSizes.push_back(treeWidth);
      splitterSizes.push_back(paramsWidth);
      splitterSizes.push_back(previewWidth);
    }
    ui->splitter->setSizes(splitterSizes);
  }
  bool shouldUpdatePreview = false;
  if (Settings::visibleLogos()) {
    if (!ui->logosLabel->isVisible()) {
      shouldUpdatePreview = true;
      ui->logosLabel->show();
    }
  } else {
    if (ui->logosLabel->isVisible()) {
      shouldUpdatePreview = true;
      ui->logosLabel->hide();
    }
  }
  if (shouldUpdatePreview) {
    ui->previewWidget->sendUpdateRequest();
  }
  // Manage zoom constraints
  setZoomConstraint();
  if (!Settings::previewZoomAlwaysEnabled()) {
    const FiltersPresenter::Filter & filter = _filtersPresenter->currentFilter();
    if (((ui->previewWidget->zoomConstraint() == ZoomConstraint::Fixed) && (ui->previewWidget->defaultZoomFactor() != ui->previewWidget->currentZoomFactor())) ||
        ((ui->previewWidget->zoomConstraint() == ZoomConstraint::OneOrMore) && (ui->previewWidget->currentZoomFactor() < 1.0))) {
      ui->previewWidget->setPreviewFactor(filter.previewFactor, true);
      if (ui->cbPreview->isChecked()) {
        ui->previewWidget->sendUpdateRequest();
      }
    }
  }
  showZoomWarningIfNeeded();
  // Sources modification may require an update
  bool sourcesModified = false;
  bool sourcesRequireInternetUpdate = false;
  dialog.sourcesStatus(sourcesModified, sourcesRequireInternetUpdate);
  if (sourcesModified) {
    updateFilters(sourcesRequireInternetUpdate && ui->cbInternetUpdate->isChecked());
  }
}

bool Bqm_Widget::confirmAbortProcessingOnCloseRequest()
{
  int button = QMessageBox::question(this, tr("Confirmation"), tr("A gmic command is running.<br>Do you really want to close the plugin?"), QMessageBox::Yes, QMessageBox::No);
  return (button == QMessageBox::Yes);
}

void Bqm_Widget::enableWidgetList(bool on)
{
  for (QWidget * w : _filterUpdateWidgets) {
    w->setEnabled(on);
  }
  ui->inOutSelector->setEnabled(on);
}

void Bqm_Widget::closeEvent(QCloseEvent * e)
{
  if (_processor.isProcessing() && _pendingActionAfterCurrentProcessing != ProcessingAction::Close) {
    if (confirmAbortProcessingOnCloseRequest()) {
      _pendingActionAfterCurrentProcessing = ProcessingAction::Close;
      _processor.cancel();
    }
    e->ignore();
  } else {
    e->accept();
  }
}

} // namespace GmicQt
