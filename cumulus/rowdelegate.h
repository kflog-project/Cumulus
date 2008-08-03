/***********************************************************************
**
**   rowdelegate.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008 by Josua Dietze
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef ROWDELEGATE_H
#define ROWDELEGATE_H

#include <QItemDelegate>

class RowDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    /**
     *  Overwritten to add verticalRowMargin above and below the row
     */
    RowDelegate(QWidget *parent = 0, int verticalRowMargin = 0 ) : QItemDelegate(parent) {
      vMargin = verticalRowMargin;
    };

    /**
     *  Set a new vertical margin value
     */
    void setVerticalMargin(int newValue) { vMargin = newValue; };   

    /**
     *  Overwrite QItemDelegate::drawDisplay to add a vertical margin for
     *  text drawing
     */
/*    void drawDisplay(QPainter* painter, const QStyleOptionViewItem &option,
                const QRect &rect, const QString &text ) const;
*/
    /**
     *  Overwrite QItemDelegate::drawDecoration to test for an icon in the
     *  item column (which affects alignment)
     */
/*    void drawDecoration( QPainter* painter, const QStyleOptionViewItem &option,
                          const QRect &rect, const QPixmap &pixmap ) const;
*/
    /**
     *  Overwrite QItemDelegate::sizeHint to make row height variable
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    /** This stores the given height margin */
    int vMargin;
    /** This stores the icon status of the item column */
//    mutable bool iconFlag;
};

#endif
