/***********************************************************************
**
**   whatsthat.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QFont>
#include <QSizeF>
#include <QPainter>

#include "whatsthat.h"

uint WhatsThat::instance = 0;

const int vMargin = 5;
const int hMargin = 5;

WhatsThat::WhatsThat( QWidget* parent, QString& txt, int timeout ) :
    QWidget( parent, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint )
{
  setObjectName("WhatsThat");
  setAttribute( Qt::WA_DeleteOnClose );

  instance++;

  autohideTimer = new QTimer(this);
  autohideTimer->setSingleShot( true );

  doc = new QTextDocument( this );

#ifndef MAEMO
  doc->setDefaultFont( QFont ("Helvetica", 16 ) );
#else
  doc->setDefaultFont( QFont ("Helvetica", 18 ) );
#endif

  // check, what kind of text has been passed
  if( txt.contains("<html>", Qt::CaseInsensitive ) ||
      txt.contains("<qt>", Qt::CaseInsensitive ) )
    {
      // qDebug("HTML=%s", txt.latin1());
      doc->setHtml( txt );
    }
  else
    {
      //qDebug("PLAIN=%s", txt.latin1());
      doc->setPlainText( txt );
    }

  // get current document size
  QSize docSize = doc->size().toSize();

  docW = docSize.width();
  docH = docSize.height();

  resize( docW + 2*hMargin, docH + 2*vMargin );

  // qDebug("DocSize: w=%d, h=%d", docW + 2*hMargin, docH + 2*vMargin);

  // @AP: Widget will be destroyed, if timer expired. If timeout is
  // zero, manual quit is expected by the user.

  if( timeout > 0 )
    {
      connect(autohideTimer, SIGNAL(timeout()),
              this, SLOT(hide()));
      autohideTimer->start(timeout);
    }

  repaint();
}


WhatsThat::~WhatsThat()
{
  instance--;
}


void WhatsThat::hide()
{
  autohideTimer->stop();
  QWidget::close();
}


void WhatsThat::mousePressEvent( QMouseEvent* )
{
  // qDebug("WhatsThat::mousePressEvent()");
  hide();
}


void WhatsThat::keyPressEvent( QKeyEvent* )
{
  hide();
}

void WhatsThat::paintEvent( QPaintEvent* )
{
  QPixmap pm = QPixmap( docW+1, docH+1 );
  pm.fill(QColor(255, 255, 224)); // LightYellow www.wackerart.de/rgbfarben.html

  QPainter docP;

  docP.begin(&pm);
  doc->drawContents( &docP );
  docP.end();

  QPainter p( this );
  p.fillRect( rect(), Qt::red );

  p.drawPixmap( hMargin, vMargin, pm );
}

