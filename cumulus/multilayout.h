/***************************************************************************
                          multilayout.h  -  layout manager class
                             -------------------
    begin                : Sat Jan 24 2004
    copyright            : (C) 2004 by Andre Somers
    email                : andre@kflog.org
 
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qlayout.h>
#include <qlist.h>
#include <qvector.h>
#include <qbitarray.h>

/**
 * @short Layout manager class
 * @author André Somers
 *
 * This class provides an advanced layout management system for dynamic
 * layouts. It can be used to create multiple layouts for the same (set
 * of) widgets, that can be easily interchanged.
 *
 * Usage is simple. Just create the widgets you need, and use the standard
 * layouts manage their sizes and positions. Now, add this widget or layout
 * to an instance of MultiLayout. Repeat this process for any layout you
 * wish. Different layouts don't need to use all widgets. Widgets that are
 * not used in a particular layout are automatically hidden and shown again
 * when needed.
 */
class MultiLayout:public QLayout
{
    //Q_OBJECT

public:
    /**
     * Constructor. Reimplemented from QLayout
     */
    MultiLayout(QWidget *parent)
            :QLayout(parent)
    {
        _current=0;
        _i=-1;
    }

    /**
     * Constructor. Reimplemented from QLayout
     */
    MultiLayout(QLayout *parent)
            :QLayout(parent)
    {
        _current=0;
        _i=-1;
    }

    /**
     * Constructor. Reimplemented from QLayout
     */
    ~MultiLayout();

    /**
     * Add a QLayoutItem (either a QWidget or a QLayout) to the list of possible layouts.
     */
    void addItem(QLayoutItem *item);

    /**
     * Add a QLayoutItem (either a QWidget or a QLayout) to the list of possible layouts.
     * This function is an extended version of the standard AddItem, because it has a return
     * value.
     * @returns int The index of the new LayoutItem you just added.
     */
    int addItemExt(QLayoutItem *item);

    /**
     * Reimplemented from QLayout
     */
    QSize sizeHint() const;

    /**
     * Reimplemented from QLayout
     */
    QSize minimumSize() const;

    /**
     * Reimplemented from QLayout
     */
    QLayoutIterator iterator();

    /**
     * Reimplemented from QLayout
     */
    void setGeometry(const QRect &rect);

    /**
     * Activates the indicated layout.
     */
    void activate(uint);

    /**
     * @returns the index of the currently active layout. May be -1 if no layout is active.
     */
    int current();

private:
    //holds the QLayoutItems that have been added to the MultiLayout
    QList<QLayoutItem> list;
    //reference to the currently active QLayoutItem
    QLayoutItem* _current;
    //index of the currently active QLayoutItem
    int _i;

    //Holds references to all the widgets that are managed by this class
    QList<QWidget> widgetList;

    /**
     * Holds a list of QBitArray objects. Each QBitArray belongs to a QLayoutItem that has
     * been added to the object. Each bit position refers to an index in the @ref widgetList.
     * A set bit means that the widget with the same index as this bit is visible in this
     * particular layout. This enables the class to show and hide the correct widgets for
     * each activated layout.
     */
    QList<QBitArray> visibleWidgetList;

    /**
     * Recursively scans the referenced QLayoutItem for QWidgets. If a widget
     * is not yet in the list of managed widgets, it is added. For each widget
     * in the list, the corresponding bit is set in the QBitArray if that widget
     * is in the QLayoutItem that is being scanned.
     */
    void scanItem(QLayoutItem *, QBitArray *);
};
