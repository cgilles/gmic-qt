/*
 *  This file is part of G'MIC-Qt, a generic plug-in for raster graphics
 *  editors, offering hundreds of filters thanks to the underlying G'MIC
 *  image processing framework.
 *
 *  Copyright (C) 2019-2024 Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 *  Description: digiKam Batch Queue Manager plugin for Gmic-Qt.
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

#ifndef DIGIKAM_GMIC_FILTER_CHAIN_H
#define DIGIKAM_GMIC_FILTER_CHAIN_H

// Qt includes

#include <QBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QTreeWidget>
#include <QWidget>
#include <QIcon>
#include <QMap>

namespace DigikamBqmGmicQtPlugin
{

class GmicFilterChain;
class GmicFilterChainView;

/**
 * Type of static fonction used to customize sort items in list.
 * Sort items call this method in GmicFilterChainViewItem::operator<.
 * To setup this method, uses GmicFilterChain::setIsLessThanHandler().
 */
typedef bool (*GmicFilterChainIsLessThanHandler)(const QTreeWidgetItem* current, const QTreeWidgetItem& other);

class GmicFilterChainViewItem : public QTreeWidgetItem
{

public:

    explicit GmicFilterChainViewItem(GmicFilterChainView* const view,
                                     const QString& title,
                                     const QString& command);
    ~GmicFilterChainViewItem()            override;

    void setCommand(const QString& command);
    QString command()               const;

    void setTitle(const QString& title);
    QString title()                 const;

    void setIsLessThanHandler(GmicFilterChainIsLessThanHandler fncptr);

protected:

    GmicFilterChainView* view()     const;

private:

    bool operator<(const QTreeWidgetItem& other) const override;

private:

    class Private;
    Private* const d = nullptr;

private:

    Q_DISABLE_COPY(GmicFilterChainViewItem)
};

// -------------------------------------------------------------------------

class GmicFilterChainView : public QTreeWidget
{
    Q_OBJECT

public:

    enum ColumnType
    {
        Title = 0,
        Command
    };

public:

    explicit GmicFilterChainView(GmicFilterChain* const parent);
    ~GmicFilterChainView()                                     override = default;

    GmicFilterChainViewItem* findItem(const QString& title);
    QModelIndex indexFromItem(GmicFilterChainViewItem* item,
                              int column = 0)            const;

    GmicFilterChainViewItem* currentFilterItem()         const;

    GmicFilterChainIsLessThanHandler isLessThanHandler() const;

Q_SIGNALS:

    void signalItemClicked(QTreeWidgetItem*);

private Q_SLOTS:

    void slotItemClicked(QTreeWidgetItem* item, int column);
};

// -------------------------------------------------------------------------

class GmicFilterChain : public QWidget
{
    Q_OBJECT

public:

    enum ControlButtonPlacement
    {
        NoControlButtons = 0,
        ControlButtonsLeft,
        ControlButtonsRight,
        ControlButtonsAbove,
        ControlButtonsBelow
    };

    enum ControlButton
    {
       Add       = 0x1,
       Remove    = 0x2,
       MoveUp    = 0x4,
       MoveDown  = 0x8,
       Clear     = 0x10
    };
    Q_DECLARE_FLAGS(ControlButtons, ControlButton)

public:

    explicit GmicFilterChain(QWidget* const parent);
    ~GmicFilterChain()                                                     override;

    GmicFilterChainView* listView()                                  const;

    void                setControlButtons(ControlButtons buttonMask);

    /**
     * Plug the control buttons near to the list, following 'placement' position.
     * Return the instance of the layout supporting the control buttons, if any.
     * This method must be calls after to use appendControlButtonsWidget().
     */
    QBoxLayout*         setControlButtonsPlacement(ControlButtonPlacement placement);

   /**
    * Append a extra widget to the end of Control Button layout (as a progress bar for exemple).
    * This method must be call before setControlButtonsPlacement().
    * Ownership of the widget is not transferred to the DItemList.
    */
    void                appendControlButtonsWidget(QWidget* const widget);

    void setChainedFilters(const QMap<QString, QVariant>& filters);
    QMap<QString, QVariant> chainedFilters()                         const;

    QStringList         chainedCommands()                           const;
    QString             currentTitle()                              const;
    QString             currentCommand()                            const;

    void                updateCurrentFilter(const QString& title,
                                            const QString& command);

    void                removeItemByTitle(const QString& title);

    ///@{
    /**
     * Methods to handle function pointer used to customize sort items in list.
     * See GmicFilterChainIsLessThanHandler type for details.
     */
    void setIsLessThanHandler(GmicFilterChainIsLessThanHandler fncptr);
    GmicFilterChainIsLessThanHandler isLessThanHandler() const;
    ///@}

Q_SIGNALS:

    void signalAddItem(const QString& title, const QString& command);
    void signalMoveUpItem();
    void signalMoveDownItem();
    void signalRemovedItems(const QList<int>&);
    void signalItemListChanged();
    void signalItemClicked(QTreeWidgetItem*);

public Q_SLOTS:

    void slotAddItem();
    void slotRemoveItems();

private Q_SLOTS:

    void slotMoveUpItems();
    void slotMoveDownItems();
    void slotClearItems();
    void slotItemListChanged();

private:

    // Disable
    GmicFilterChain() = delete;

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin

Q_DECLARE_OPERATORS_FOR_FLAGS(DigikamBqmGmicQtPlugin::GmicFilterChain::ControlButtons)

#endif // DIGIKAM_GMIC_FILTER_CHAIN_H
