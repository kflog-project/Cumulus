/***********************************************************************
**
**   whatsthat.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers / TrollTech
**                   2008 Axel Pauli                
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <math.h>

#include <QFont>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QToolTip>
#include <QSizeF>

#include "whatsthat.h"
#include "map.h"

uint WhatsThat::instance = 0;

WhatsThat::WhatsThat( QWidget* parent, QString& txt, int timeout ) :
  QWidget( parent, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint )
{
  setObjectName("WhatsThat");

  blockHide=false;
  waitingForRedraw=false;

  // qDebug("WhatsThat::WhatsThat()");
  instance++;

  autohideTimer = new QTimer(this);
  setBackgroundRole( QPalette::Window );
  setPalette( Qt::red );
  setAutoFillBackground(true);

  QHBoxLayout *box = new QHBoxLayout( this );
  box->setMargin( 5 );

  QTextEdit *display = new QTextEdit( this );
  box->addWidget( display );

  display->setLineWrapMode( QTextEdit::NoWrap );
  display->setReadOnly( true );
  display->setText( txt );
  display->setAlignment( Qt::AlignCenter );
  display->document()->setDefaultFont( QFont ("Helvetica", 16 ) );
  display->document()->adjustSize();

  QSizeF docSize = display->document()->size();

  int w = (int) rint( docSize.rwidth() );
  int h = (int) rint( docSize.rheight() );

  resize( w+25, h+30 );
  
  // @AP: Widget will be destroyed, if timer expired. If timeout is
  // zero, manual quit is expected by the user.

  if( timeout > 0 ) {
    connect(autohideTimer, SIGNAL(timeout()),
            this, SLOT(hide()));
    autohideTimer->start(timeout,true);
  }

  // @AP: We will use a trick here to close the window because
  // QTextEdit is catching the mouse events.
  connect( display, SIGNAL(cursorPositionChanged()), this, SLOT(hide()) );

  //@AS: Quick 'n dirty connection to map...
  connect(Map::getInstance(), SIGNAL(isRedrawing(bool)),
          this, SLOT(mapIsRedrawing(bool)));
}


WhatsThat::~WhatsThat()
{
  instance--;
}


void WhatsThat::hide()
{
  // @AP: stop timer to avoid callbacks into nirvana
  autohideTimer->stop();

  if (blockHide) {
    waitingForRedraw=true;
    qDebug("Mapredraw in progress, waiting to finish...");
  } else {
    QWidget::close(true);
  }
}


void WhatsThat::mousePressEvent( QMouseEvent* )
{
  qDebug("WhatsThat::mousePressEvent()");
  hide();
}


void WhatsThat::keyPressEvent( QKeyEvent* )
{
  hide();
}


void WhatsThat::mapIsRedrawing(bool redraw)
{
  blockHide=redraw;
  
  if ((!redraw) && waitingForRedraw)
    {
      hide();
    }
}

