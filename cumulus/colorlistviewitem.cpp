/***********************************************************************
**
**   colorlistviewitem.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004 by Eckhard Völlm, 2008 Axel pauli
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

#include "colorlistviewitem.h"

ColorListViewItem::ColorListViewItem( QTreeWidget* parent, const QStringList& strings, int type ) 
                   : QTreeWidgetItem( parent, strings, type )
{
  _colorSet=false;
  setTextAlignment(1,Qt::AlignRight);
  setTextAlignment(2,Qt::AlignRight);
  setTextAlignment(4,Qt::AlignRight);
  setTextAlignment(5,Qt::AlignLeft);
}


ColorListViewItem::~ColorListViewItem()
{}


void ColorListViewItem::setColor(QColor c)
{
  _colorSet=true;
  _color=c;
  for (int i=0; i < columnCount(); i++)
    setForeground(i, QBrush(c));
}


void ColorListViewItem::resetColor()
{
  _colorSet=false;
  for (int i=0; i < columnCount(); i++)
    setForeground(i, QBrush(Qt::black));
}


// Generate a sorting key that works for each numerical entry
/*
QString ColorListViewItem::key(int column, bool ascending) const
{
  QString s;
  if ( column != 0 && column != 5) {
    s.sprintf( "%05d ", (int)(10*text( column ).toFloat()+10000.0) );
    // qDebug("sprintf: %s",(const char *) s );
  } else
    s=text( column );
  return s;
}
*/

// override paint method to paint in different color
/*void ColorListViewItem::paintCell(QPainter * p, const QColorGroup & cg, int
                                  column, int width, int align)
{
    QColorGroup new_color;

    if (!_colorSet)
        QTreeWidgetItem::paintCell(p, cg, column, width, align);
    else {
        new_color = QColorGroup(cg.foreground(), cg.background(),
                                cg.light(), cg.dark(), cg.mid(), _color, cg.base());
        QTreeWidgetItem::paintCell(p, new_color, column, width, align);
    }
}
*/
