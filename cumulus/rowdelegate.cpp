/***********************************************************************
**
**   rowdelegate.cpp
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


#include "rowdelegate.h"


/*
void RowDelegate::drawDecoration( QPainter* painter, const QStyleOptionViewItem &option,
                          const QRect &rect, const QPixmap &pixmap ) const
{
  // remember if the column entry has an icon
  if ( ! pixmap.isNull() )
    iconFlag = true;
  else
    iconFlag = false;
  // call the pre-set method
  QItemDelegate::drawDecoration(painter, option, rect, pixmap);
}


void RowDelegate::drawDisplay( QPainter* painter, const QStyleOptionViewItem &option,
                          const QRect &rect, const QString &text ) const
{
  int verticalMargin = vMargin;

  // item columns with icon draw the text vertically centered, text only columns
  // need the margin - don't ask me why!
//  if ( iconFlag )
    verticalMargin = 0;

  QRect newRect = QRect( rect );
  newRect.setY( rect.y() + verticalMargin );
qDebug("delegate: margin is %d",verticalMargin);
qDebug("delegate: draw text at %d, %d",newRect.x(),newRect.y());

  // call the pre-set method with the new vertical margin
  QItemDelegate::drawDisplay(painter, option, newRect, text);
}*/


/** Adds the vertical margin to the standard sizeHint */
QSize RowDelegate::sizeHint(const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    QSize newSize = QItemDelegate::sizeHint( option, index );
    newSize.setHeight( newSize.height() + vMargin );
    return newSize;
}
