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
#include <QGroupBox>
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

    void setIndex(int index);

    void setIsLessThanHandler(GmicFilterChainIsLessThanHandler fncptr);

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
        Index = 0,
        Title,
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

    void refreshIndex();

Q_SIGNALS:

    void signalEditItem(const QString& command);

private Q_SLOTS:

    void slotItemDoubleClicked(QTreeWidgetItem* item, int column);
};

// -------------------------------------------------------------------------

class GmicFilterChain : public QGroupBox
{
    Q_OBJECT

public:

    enum ControlButtonPlacement
    {
        ControlButtonsLeft = 0,
        ControlButtonsRight,
        ControlButtonsAbove,
        ControlButtonsBelow
    };

public:

    explicit GmicFilterChain(QWidget* const parent);
    ~GmicFilterChain()                                                     override;

    /**
     * Plug the control buttons near to the list, following 'placement' position.
     * Return the instance of the layout supporting the control buttons, if any.
     * This method must be calls after to use appendControlButtonsWidget().
     */
    QBoxLayout*         setControlButtonsPlacement(ControlButtonPlacement placement);

    void setChainedFilters(const QMap<QString, QVariant>& filters);
    QMap<QString, QVariant> chainedFilters()                        const;

    QStringList         chainedCommands()                           const;
    QString             currentCommand()                            const;

    void                createNewFilter(const QString& title,
                                        const QString& command);

    void                updateCurrentFilter(const QString& title,
                                            const QString& command);

    ///@{
    /**
     * Methods to handle function pointer used to customize sort items in list.
     * See GmicFilterChainIsLessThanHandler type for details.
     */
    void setIsLessThanHandler(GmicFilterChainIsLessThanHandler fncptr);
    GmicFilterChainIsLessThanHandler isLessThanHandler() const;
    ///@}

Q_SIGNALS:

    void signalAddItem();
    void signalEditItem(const QString& command);
    void signalMoveUpItem();
    void signalMoveDownItem();
    void signalRemovedItems(const QList<int>&);
    void signalItemListChanged();

private Q_SLOTS:

    void slotMoveUpItems();
    void slotMoveDownItems();
    void slotRemoveItems();
    void slotClearItems();
    void slotItemListChanged();

private:

    // Disable
    GmicFilterChain() = delete;

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin

#endif // DIGIKAM_GMIC_FILTER_CHAIN_H
