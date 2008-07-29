/***********************************************************************
**
**   colorlistviewitem.h 
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by Eckhard Völlm, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   Purpose: Reimplements paintCell to allow colored style for
**            non reachable items
**
**   $Id$
**
***********************************************************************/

#include <QTreeWidget>
#include <QColor>
#include <QPainter>

/**
 * @short Colored list view items
 * @author Eckhard Völlm
 *
 * Reimplements paintCell to allow colored style for
 * non reachable items.
 */
class ColorListViewItem : public QTreeWidgetItem
{
public:
    ColorListViewItem( QTreeWidget* parent, const QStringList& strings, int type = 0 );

    virtual ~ColorListViewItem();

    /**
     * Sets the color the item is painted in.
     */
    void setColor(QColor c);

    /**
     * Resets the color to use the default color again.
     */
    void resetColor();

    /**
     * Generate a sorting key that works for each numerical entry
     */
//    virtual QString key(int column, bool ascending) const;
    
    /**
     * reimplemented from QTreeWidgetItem
     */
//    virtual void paintCell(QPainter * p, const QColorGroup & cg,
//                           int column, int width, int align);

private:
    //the color used to paint the item if _colorSet is true
    QColor _color;
    // if set, the color set in _color is used, if not, the default color
    bool _colorSet;
};
