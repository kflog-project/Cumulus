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

CuMsgBox::CuMsgBox( QWidget* parent ) :
  QMessageBox( parent )
{
}

void CuMsgBox::paintEvent( QPaintEvent *event )
{
  // first draw parent
  QMessageBox::paintEvent( event );

  QPainter painter( this );

  QPen pen( Qt::red );
  pen.setWidth( 5 );
  painter.setPen( pen );
  painter.setBrush( Qt::NoBrush );
  painter.drawRect( rect() );

  qDebug() << "Rect" << rect() << "frameGeometry" << frameGeometry();
}
