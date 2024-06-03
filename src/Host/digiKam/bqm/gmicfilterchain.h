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
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QIcon>
#include <QUrl>

namespace DigikamBqmGmicQtPlugin
{

class GmicFilterChain;
class GmicFilterChainView;

/**
 * Type of static fonction used to customize sort items in list.
 * Sort items call this method in GmicFilterChainViewItem::operator<.
 * To setup this method, uses DItemList::setIsLessThanHandler().
 */
typedef bool (*GmicFilterChainIsLessThanHandler)(const QTreeWidgetItem* current, const QTreeWidgetItem& other);

class GmicFilterChainViewItem : public QTreeWidgetItem
{

public:

    enum State
    {
        Waiting,
        Success,
        Failed
    };

public:

    explicit GmicFilterChainViewItem(GmicFilterChainView* const view, const QUrl& url);
    ~GmicFilterChainViewItem()             override;

    bool hasValidThumbnail()    const;

    void setUrl(const QUrl& url);
    QUrl url()                  const;

    void setComments(const QString& comments);
    QString comments()          const;

    void setTags(const QStringList& tags);
    QStringList tags()          const;

    void setRating(int rating);
    int rating()                const;

    void setThumb(const QPixmap& pix, bool hasThumb=true);
    void setProgressAnimation(const QPixmap& pix);

    void setProcessedIcon(const QIcon& icon);
    void setState(State state);
    State state()               const;

    void updateInformation();

    void setIsLessThanHandler(GmicFilterChainIsLessThanHandler fncptr);

    /**
     * Implement this, if you have special item widgets, e.g. an edit line
     * they will be set automatically when adding items, changing order, etc.
     */
    virtual void updateItemWidgets() {};

protected:

    GmicFilterChainView* view()      const;

private:

    void setPixmap(const QPixmap& pix);
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
        Thumbnail = 0,
        Filename,
        User1,
        User2,
        User3,
        User4,
        User5,
        User6
    };

public:

    explicit GmicFilterChainView(GmicFilterChain* const parent);
    ~GmicFilterChainView()                                     override = default;

    void setColumnLabel(ColumnType column, const QString& label);
    void setColumnEnabled(ColumnType column, bool enable);
    void setColumn(ColumnType column, const QString& label, bool enable);

    GmicFilterChainViewItem* findItem(const QUrl& url);
    QModelIndex indexFromItem(GmicFilterChainViewItem* item,
                              int column = 0)       const;

    GmicFilterChainViewItem* getCurrentItem()            const;

    DInfoInterface* iface()                         const;
    GmicFilterChainIsLessThanHandler isLessThanHandler() const;

Q_SIGNALS:

    void signalAddedDropedItems(const QList<QUrl>&);
    void signalItemClicked(QTreeWidgetItem*);
    void signalContextMenuRequested();

private Q_SLOTS:

    void slotItemClicked(QTreeWidgetItem* item, int column);

public:

    void enableDragAndDrop(const bool enable = true);

private:

    void dragEnterEvent(QDragEnterEvent* e)               override;
    void dragMoveEvent(QDragMoveEvent* e)                 override;
    void dropEvent(QDropEvent* e)                         override;
    void contextMenuEvent(QContextMenuEvent* e)           override;

    void drawRow(QPainter* p,
                 const QStyleOptionViewItem& opt,
                 const QModelIndex& index)          const override;

private:

    QTreeWidgetItem* m_itemDraged = nullptr;
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
       Clear     = 0x10,
       Load      = 0x20,
       Save      = 0x40
    };
    Q_DECLARE_FLAGS(ControlButtons, ControlButton)

public:

    explicit GmicFilterChain(QWidget* const parent);
    ~GmicFilterChain()                                                     override;

    void                setAllowRAW(bool allow);
    void                setAllowDuplicate(bool allow);

    void                loadImagesFromCurrentSelection();

    /**
     * A function to load all the images from the album if no image has been selected by user.
     */
    void                loadImagesFromCurrentAlbum();

    /**
     * a function to check whether an image has been selected or not.
     */
    bool                checkSelection();

    void setIconSize(int size);
    int                 iconSize()                                  const;

    GmicFilterChainView*     listView()                                  const;

    void                processing(const QUrl& url);
    void                processed(const QUrl& url, bool success);
    void                cancelProcess();
    void                clearProcessedStatus();

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

    void                enableControlButtons(bool enable = true);
    void                enableDragAndDrop(const bool enable = true);

    virtual QList<QUrl> imageUrls(bool onlyUnprocessed = false)     const;
    virtual void        removeItemByUrl(const QUrl& url);

    void                setCurrentUrl(const QUrl& url);
    QUrl                getCurrentUrl()                             const;

    ///@{
    /**
     * Methods to handle function pointer used to customize sort items in list.
     * See GmicFilterChainIsLessThanHandler type for details.
     */
    void setIsLessThanHandler(GmicFilterChainIsLessThanHandler fncptr);
    GmicFilterChainIsLessThanHandler isLessThanHandler() const;
    ///@}

Q_SIGNALS:

    void signalAddItems(const QList<QUrl>&);
    void signalMoveUpItem();
    void signalMoveDownItem();
    void signalRemovedItems(const QList<int>&);
    void signalImageListChanged();
    void signalFoundRAWImages(bool);
    void signalItemClicked(QTreeWidgetItem*);
    void signalContextMenuRequested();
    void signalXMLSaveItem(QXmlStreamWriter&, int);         // clazy:exclude=signal-with-return-value
    void signalXMLLoadImageElement(QXmlStreamReader&);      // clazy:exclude=signal-with-return-value
    void signalXMLCustomElements(QXmlStreamWriter&);        // clazy:exclude=signal-with-return-value
    void signalXMLCustomElements(QXmlStreamReader&);        // clazy:exclude=signal-with-return-value

public Q_SLOTS:

    virtual void slotAddImages(const QList<QUrl>& list);
    virtual void slotRemoveItems();

protected Q_SLOTS:

    void slotProgressTimerDone();

    virtual void slotAddItems();
    virtual void slotMoveUpItems();
    virtual void slotMoveDownItems();
    virtual void slotClearItems();
    virtual void slotImageListChanged();

private:

    // Disable
    GmicFilterChain() = delete;

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamBqmGmicQtPlugin

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::GmicFilterChain::ControlButtons)

#endif // DIGIKAM_GMIC_FILTER_CHAIN_H
