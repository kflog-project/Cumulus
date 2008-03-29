/***********************************************************************
**
**   whatsthat.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WHATSTHAT_H
#define WHATSTHAT_H

#include <QWidget>
#include <QTimer>
#include <QString>
#include <QTextDocument>

/**
 * Redesign by Axel Pauli at 2008. Now we use the Qt4 official classes
 * for realization.
 *
 * @author André Somers
 */
class WhatsThat : public QWidget
{
    Q_OBJECT

  public:

  WhatsThat( QWidget* parent, QString& txt, int timeout=5000 );

  virtual ~WhatsThat();

  static uint getInstance()
    {
      return instance;
    };

  public slots:

  void hide();

 protected:

  void mousePressEvent( QMouseEvent* );
  void keyPressEvent( QKeyEvent* );
  void paintEvent( QPaintEvent* );

 private:

  /** Timer for automatic window hide */
  QTimer* autohideTimer;

  // text to be displayed
  QTextDocument *doc;
  // width of document
  int docW;
  // height of document
  int docH;

  // instance counter
  static uint instance;

};


#endif
