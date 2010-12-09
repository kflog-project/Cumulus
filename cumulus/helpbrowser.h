/***********************************************************************
**
**   helpbrowser.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2008-2010 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
 * \class HelpBrowser
 *
 * \author Axel Pauli
 *
 * \brief This class provides a widget usable as help browser.
 *
 * Creates a help browser widget as single window and loads
 * the Cumulus help file into it according to the selected
 * language. The user can navigate through the text, zoom in and out,
 * maximize/normalize the window display size.
 *
 * \date 2008-2010
 */

class HelpBrowser : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( HelpBrowser )

 public:

  HelpBrowser( QWidget *parent=0 );
  virtual ~HelpBrowser();
  
  /** catch show events */
  void showEvent( QShowEvent *event );

  /** catch key events */
  void keyPressEvent( QKeyEvent *event );

 private slots:

  /** User request, to zoom into the document. */
  void slotZoomIn();

  /** User request, to zoom out the document. */
  void slotZoomOut();

 private:

  bool firstCall;
  QTextBrowser *browser;
};

#endif
