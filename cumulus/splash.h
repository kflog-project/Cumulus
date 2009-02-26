/***********************************************************************
**
**   splash.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/** This widget loads a pixmap as background picture and
 *  is used as splash screen during startup of cumulus.
 */

#ifndef Splash_h
#define Splash_h

#include <QWidget>
#include <QPixmap>

class Splash : public QWidget
{
  Q_OBJECT

 public:

  Splash( QWidget *parent = 0);
  virtual ~Splash();

 protected:

  /** Handles the paint events of the widget */
  void paintEvent(QPaintEvent *event);

 private:

   /** Pixmap which contains the background picture */
   QPixmap pixmap;
};

#endif
