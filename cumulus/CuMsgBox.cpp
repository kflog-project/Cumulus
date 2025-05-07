/***********************************************************************
**
**   CuMsgBox.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2025 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "CuMsgBox.h"
#include "layout.h"

CuMsgBox::CuMsgBox( QWidget* parent ) :
  QMessageBox( parent )
{
}

void CuMsgBox::paintEvent( QPaintEvent *event )
{
  // first draw parent
  QMessageBox::paintEvent( event );

  if( event->rect() != rect() )
    {
      // No the whole window is given
      return;
    }

  // The whole window was drawn. We draw a rectangle now the borders.
  QPainter painter( this );

  QPen pen( Qt::black );
  int w = 3 * Layout::getIntScaledDensity();
  pen.setWidth( w );
  painter.setPen( pen );
  painter.setBrush( Qt::NoBrush );

  QRect rt( w/2, w/2, width() - w, height() - w );
  painter.drawRect( rt );
}
