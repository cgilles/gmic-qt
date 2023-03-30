/*
*  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
*  editors, offering hundreds of filters thanks to the underlying G'MIC
*  image processing framework.
*
*  Copyright (C) 2019-2023 Gilles Caulier <caulier dot gilles at gmail dot com>
*
*  Description: digiKam image editor plugin for GmicQt.
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

#ifndef GMIC_QT_BQM_WIDGET_H
#define GMIC_QT_BQM_WIDGET_H

#include <QIcon>
#include <QList>
#include <QString>
#include <QTimer>
#include <QWidget>

#include "Common.h"
#include "GmicProcessor.h"
#include "Updater.h"

class QResizeEvent;

using namespace GmicQt;

namespace Ui
{
class Bqm_Widget;
}

namespace gmic_library
{
template <typename T> struct gmic_list;
}

namespace GmicQt
{

class FiltersTreeFolderItem;
class FiltersTreeFilterItem;
class FiltersTreeAbstractFilterItem;
class FiltersTreeFaveItem;
class Updater;
class FilterThread;
class FiltersPresenter;
class VisibleTagSelector;
}

namespace DigikamBqmGmicQtPlugin
{

class Bqm_Widget : public QWidget
{
  Q_OBJECT

public:

  explicit Bqm_Widget(QWidget * parent = nullptr);
  ~Bqm_Widget() override;

  void updateFiltersFromSources(int ageLimit, bool useNetwork);

  void setPluginParameters(const RunParameters & parameters);
  RunParameters pluginParameters() const;

public Q_SLOTS:

  void onUpdateDownloadsFinished(int status);
  void onApplyClicked();
  void onPreviewUpdateRequested(bool synchronous);
  void onPreviewUpdateRequested();
  void onPreviewKeypointsEvent(unsigned int flags, unsigned long time);
  void onFullImageProcessingDone();
  void expandOrCollapseFolders();
  void search(const QString &);
  void onOkClicked();
  void onCancelClicked();
  void onProgressionWidgetCancelClicked();
  void onReset();
  void onCopyGMICCommand();
  void onPreviewZoomReset();
  void onUpdateFiltersClicked();
  void saveCurrentParameters();
  void onAddFave();
  void onRemoveFave();
  void onRenameFave();
  void onToggleFullScreen(bool on);
  void onSettingsClicked();
  void onStartupFiltersUpdateFinished(int);
  void showZoomWarningIfNeeded();
  void updateZoomLabel(double);
  void onFiltersSelectionModeToggled(bool);
  void onPreviewCheckBoxToggled(bool);
  void onFilterSelectionChanged();
  void onEscapeKeyPressed();
  void onPreviewImageAvailable();
  void onPreviewError(const QString & message);
  void onParametersChanged();
  static bool isAccepted();
  void setFilterName(const QString & text);

protected:

  void timerEvent(QTimerEvent *) override;
  void closeEvent(QCloseEvent * e) override;
  void showEvent(QShowEvent *) override;
  void resizeEvent(QResizeEvent *) override;
  void saveSettings();
  void loadSettings();
  void showUpdateErrors();
  void makeConnections();
  void processImage();
  void activateFilter(bool resetZoom, const QList<QString> & values = QList<QString>());
  void setNoFilter();
  void adjustVerticalSplitter();

private Q_SLOTS:

  void onFullImageProcessingError(const QString & message);
  void onInputModeChanged(InputMode);

private:

  void onVeryFirstShowEvent();
  void setZoomConstraint();
  bool filtersSelectionMode();
  void clearMessage();
  void clearRightMessage();
  void showRightMessage(const QString & text);
  void showMessage(const QString & text, int ms = 2000);
  void setIcons();
  bool confirmAbortProcessingOnCloseRequest();
  void enableWidgetList(bool on);
  bool askUserForGTKFavesImport();
  void buildFiltersTree();
  void retrieveFilterAndParametersFromPluginParameters(QString & hash, QList<QString> & parameters);
  static QString screenGeometries();
  void updateFilters(bool internet);

  enum class ProcessingAction
  {
    NoAction,
    Ok,
    Close,
    Apply
  };

  Ui::Bqm_Widget * ui;
  ProcessingAction _pendingActionAfterCurrentProcessing;
  bool _showEventReceived = false;
  bool _okButtonShouldApply = false;
  QIcon _expandIcon;
  QIcon _collapseIcon;
  QIcon * _expandCollapseIcon;
  int _messageTimerID;
  bool _lastExecutionOK;
  bool _newSession;
  bool _gtkFavesShouldBeImported;
  QVector<QWidget *> _filterUpdateWidgets;
  FiltersPresenter * _filtersPresenter;
  GmicProcessor _processor;
  ulong _lastPreviewKeypointBurstUpdateTime;
  static bool _isAccepted;
  RunParameters _pluginParameters;
  VisibleTagSelector * _visibleTagSelector;
};

} // namespace DigikamBqmGmicQtPlugin

#endif // GMIC_QT_BQM_WIDGET_H
