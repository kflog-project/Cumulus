/***********************************************************************
**
**   splash.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009-2012 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "generalconfig.h"
#include "splash.h"

/** This widget loads a pixmap as background picture and
 *  is used as splash screen during startup of Cumulus.
 */
Splash::Splash( QWidget *parent) : QWidget( parent )
{
  setObjectName( "Slash" );
  setAttribute( Qt::WA_DeleteOnClose );

  if( parent )
    {
      resize( parent->size() );
    }
  else
    {
      resize( 800, 480 );
    }

  // load background picture
  pixmap = GeneralConfig::instance()->loadPixmap( "splash.png" );
}

Splash::~Splash()
{
  // remove splash pixmap from global cache
  GeneralConfig::instance()->removePixmap( "splash.png" );
}

/** Handles the paint events of the widget */
void Splash::paintEvent(QPaintEvent * /* event */ )
{
  QPainter painter(this);

  // draws the background picture
  painter.drawPixmap( rect(), pixmap, pixmap.rect() );
}
