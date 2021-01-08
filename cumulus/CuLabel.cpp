/***********************************************************************
**
**   CuLabel.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012-2021 Axel Pauli
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

#include "CuLabel.h"

CuLabel::CuLabel( QWidget* parent, Qt::WindowFlags flags ) :
  QLabel( parent, flags )
{
  setFrameStyle( QFrame::Box | QFrame::Plain );
  setLineWidth(3);
}

CuLabel::CuLabel( const QString& text, QWidget* parent, Qt::WindowFlags flags ) :
  QLabel( text, parent, flags )
{
  setFrameStyle( QFrame::Box | QFrame::Plain );
  setLineWidth(3);
}

void CuLabel::mousePressEvent ( QMouseEvent* event )
{
  Q_UNUSED(event)

  emit mousePress();
}
