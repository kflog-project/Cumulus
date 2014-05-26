/***********************************************************************
**
**   splash.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009 Axel Pauli, kflog.cumulus@gmail.com
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class Splash
 *
 * \author Axel Pauli
 *
 * \brief A splash screen for Cumulus.
 *
 * This widget loads a pixmap as background picture and
 * is used as splash screen during startup of Cumulus.
 *
 * \date 2009
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
