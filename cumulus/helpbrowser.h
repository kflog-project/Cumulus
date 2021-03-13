/***********************************************************************
**
**   helpbrowser.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2008-2021 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   This class is used for displaying the help usage of Cumulus.
**
***********************************************************************/

#ifndef HelpBrowser_H
#define HelpBrowser_H

#include <QWidget>
#include <QPushButton>
#include <QString>
#include <QTextBrowser>

/**
 * \class HelpBrowser
 *
 * \author Axel Pauli
 *
 * \brief This class provides a widget usable as help m_browser.
 *
 * Creates a help m_browser widget as single window and loads
 * the Cumulus help file into it according to the selected
 * language. The user can navigate through the text, zoom in and out,
 * maximize/normalize the window display size.
 *
 * \date 2008-2021
 *
 * \version 1.2
 */

class HelpBrowser : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( HelpBrowser )

 public:

  HelpBrowser( QWidget *parent=0,
               QString helpFile="cumulus.html",
               QString anker="" );

  virtual ~HelpBrowser();

  /** catch show events */
  void showEvent( QShowEvent *event );

  /** catch key press events */
  void keyPressEvent( QKeyEvent *event );

  /** catch key release events */
  void keyReleaseEvent( QKeyEvent *event );

  /** returns the text browser object. */
  QTextBrowser* getTextBrowser()
  {
    return m_browser;
  };

 private slots:

  /** User request, to zoom into the document. */
  void slotZoomIn();

  /** User request, to zoom out the document. */
  void slotZoomOut();

  /** Called, if the cursor position is changed to clear the text selection. */
  void slotCursorChanged();

 private:

  bool firstCall;
  QTextBrowser* m_browser;
  QPushButton*  m_zoomIn;
  QPushButton*  m_zoomOut;

  /** Name of help file to be displayed. */
  QString m_helpFile;

  /** Name of the anker to start display. */
  QString m_anker;
};

#endif
