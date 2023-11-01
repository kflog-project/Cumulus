/***********************************************************************
**
**   whatsthat.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#pragma once

#include <QWidget>
#include <QTimer>
#include <QString>
#include <QTextDocument>

/**
 * \class WhatsThat
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Popup window to display user information.
 *
 * This class is used to display user information e.g. about airspaces.
 * It is realized as a frame less popup window and is closed by tipping on
 * it or automatically after a certain time. It can display plain or html text.
 *
 * \date 2002-2023
 *
 * \version 1.3
 *
 */
class WhatsThat : public QWidget
{
    Q_OBJECT

  private:
    /**
     * That macro forbids the copy constructor and the assignment operator.
     */
    Q_DISABLE_COPY( WhatsThat )

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
  void mouseReleaseEvent( QMouseEvent* );
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
