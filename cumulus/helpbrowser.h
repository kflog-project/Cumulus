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
**   $Id$
**
***********************************************************************/

#ifndef HelpBrowser_H
#define HelpBrowser_H

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>

/**
 * @author Axel Pauli
 */

class HelpBrowser : public QWidget
{
  Q_OBJECT

 public:

  HelpBrowser( QWidget *parent=0 );
  virtual ~HelpBrowser();

 private:

  QTextBrowser *browser;
  QPushButton  *home;
  QPushButton  *back;
  QPushButton  *forward;
  QPushButton  *close;

};

#endif
