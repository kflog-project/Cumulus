/***********************************************************************
**
**   helpbrowser.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2008 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   This class is used for displaying the help usage of Cumulus.
**
**   $Id$
**
***********************************************************************/

#ifndef HelpBrowser_H
#define HelpBrowser_H

#include <QWidget>
#include <QTextBrowser>

/**
 * @author Axel Pauli
 */

class HelpBrowser : public QWidget
{
  Q_OBJECT

 public:

  HelpBrowser( QWidget *parent=0 );
  virtual ~HelpBrowser();
  
  /** catch show events */
  void showEvent( QShowEvent *event );

  /** catch key events */
  void keyPressEvent( QKeyEvent *event );

 private:

  bool firstCall;
  QTextBrowser *browser;
};

#endif
