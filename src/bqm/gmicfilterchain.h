/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-11-28
 * Description : digiKam Batch Queue Manager plugin for GmicQt.
 *
 * SPDX-FileCopyrightText: 2019-2025 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

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

class GmicFilterChainViewItem : public QTreeWidgetItem
{

public:

    explicit GmicFilterChainViewItem(GmicFilterChainView* const view,
                                     int index,
                                     const QString& title,
                                     const QString& command);
    ~GmicFilterChainViewItem()            override;

    void setCommand(const QString& command);
    QString command()               const;

    void setTitle(const QString& title);
    QString title()                 const;

    void setIndex(int index);
    int index()                     const;

private:

    bool operator<(const QTreeWidgetItem& other) const override;

private:

    Q_DISABLE_COPY(GmicFilterChainViewItem)

private:

    class Private;
    Private* const d = nullptr;
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
        Command,

        NumberOfColumns     ///< The last one to enumerate
    };

public:

    explicit GmicFilterChainView(GmicFilterChain* const parent);
    ~GmicFilterChainView()                                     override = default;

    GmicFilterChainViewItem* findItem(const QString& title);
    QModelIndex indexFromItem(GmicFilterChainViewItem* item,
                              int column = Index)        const;

    GmicFilterChainViewItem* currentFilterItem()         const;

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

    explicit GmicFilterChain(QWidget* const parent);
    ~GmicFilterChain()                                                     override;

    void setChainedFilters(const QMap<QString, QVariant>& filters);
    QMap<QString, QVariant> chainedFilters()                        const;

    QStringList chainedCommands()                                   const;
    QString     currentCommand()                                    const;

    void createNewFilter(const QString& title,
                         const QString& command);

    void updateCurrentFilter(const QString& title,
                             const QString& command);

Q_SIGNALS:

    void signalEditItem(const QString& command);
    void signalMoveUpItem();
    void signalMoveDownItem();
    void signalAddItem();
    void signalRemovedItems(const QList<int>&);
    void signalItemListChanged();

private Q_SLOTS:

    void slotEditItem();
    void slotMoveUpItems();
    void slotMoveDownItems();
    void slotRemoveItems();
    void slotClearItems();
    void slotItemListChanged();

private:

    // Disable
    GmicFilterChain() = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin
