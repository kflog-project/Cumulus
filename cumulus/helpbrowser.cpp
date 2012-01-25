/***********************************************************************
 **
 **   helpbrowser.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2008-2012 by Axel Pauli (axel@kflog.org)
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   This class is used for displaying the help usage of Cumulus.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "helpbrowser.h"
#include "generalconfig.h"
#include "layout.h"

HelpBrowser::HelpBrowser( QWidget *parent ) : QWidget(parent, Qt::Window),
                                              firstCall(true)
{
  setWindowTitle(tr("Cumulus Help"));
  setWindowIcon( GeneralConfig::instance()->loadPixmap( "cumulus.png" ) );
  setAttribute(Qt::WA_DeleteOnClose);

  browser = new QTextBrowser(this);

#ifdef ANDROID
  QScroller::grabGesture(browser, QScroller::LeftMouseButtonGesture);
#endif

  browser->setOpenLinks( true );
  browser->setOpenExternalLinks( true );

  QPushButton *home = new QPushButton();
  home->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "home_new.png") ) );
  home->setIconSize(QSize(IconSize, IconSize));
  // home->setFlat(true);
  home->setToolTip( tr("Begin") );

  QPushButton *back = new QPushButton();
  back->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "left.png") ) );
  back->setIconSize(QSize(IconSize, IconSize));
  //back->setFlat(true);
  back->setToolTip( tr("Backward") );

  QPushButton *forward = new QPushButton();
  forward->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "right.png") ) );
  forward->setIconSize(QSize(IconSize, IconSize));
  // forward->setFlat(true);
  forward->setToolTip( tr("Forward") );

  zoomIn = new QPushButton();
  zoomIn->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "zoomin.png") ) );
  zoomIn->setIconSize(QSize(IconSize, IconSize));
  // forward->setFlat(true);
  zoomIn->setToolTip( tr("Zoom in") );

  zoomOut = new QPushButton();
  zoomOut->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "zoomout.png") ) );
  zoomOut->setIconSize(QSize(IconSize, IconSize));
  // forward->setFlat(true);
  zoomOut->setToolTip( tr("Zoom out") );

  QPushButton *close = new QPushButton();
  close->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "cancel.png") ) );
  close->setIconSize(QSize(IconSize, IconSize));
  // close->setFlat(true);
  close->setToolTip( tr("Close") );

  QHBoxLayout *butLayout = new QHBoxLayout;
  butLayout->addWidget( home );
  butLayout->addWidget( back );
  butLayout->addWidget( forward );
  butLayout->addWidget( zoomIn );
  butLayout->addWidget( zoomOut );
  butLayout->addStretch();
  butLayout->addWidget( close );

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout( butLayout );
  mainLayout->addWidget( browser );
  setLayout( mainLayout );

  connect( home, SIGNAL(clicked()),    browser, SLOT(home()));
  connect( back, SIGNAL(clicked()),    browser, SLOT(backward()));
  connect( forward, SIGNAL(clicked()), browser, SLOT(forward()));
  connect( zoomIn, SIGNAL(pressed()),  this, SLOT( slotZoomIn()));
  connect( zoomOut, SIGNAL(pressed()), this, SLOT( slotZoomOut()));
  connect( close, SIGNAL(clicked()),   this, SLOT( close()));
}

HelpBrowser::~HelpBrowser()
{
  // qDebug("HelpBrowser::~HelpBrowser()");
}

/** Catch show events. If the first event is caught, we will load
 *  the help file into the browser.
 */
void HelpBrowser::showEvent( QShowEvent * )
{
  if( ! firstCall )
    {
      // after the first call, ignore this event
      return;
    }

  // first call, we try to load the Cumulus HTML help file
  firstCall = false;

  QString lang = GeneralConfig::instance()->getLanguage();

  QString helpFile = GeneralConfig::instance()->getDataRoot() +
    "/help/" + lang + "/cumulus.html";

  // We do check, if the help file does exists
  QFileInfo info(helpFile);

  if( ! info.isReadable() )
    {
      // fall back to English as default
      lang = "en";
      helpFile = GeneralConfig::instance()->getDataRoot() +
        "/help/" + lang + "/cumulus.html";
      info.setFile(helpFile);
    }

  if( ! info.exists() || ! info.isReadable() )
    {
      hide();

      QMessageBox::warning( this, "Missing help file",
                            tr("<html><b>The help file was not found.<br>"
                               "Maybe it is not installed?</b></html>"));
      QWidget::close();
      return;
    }

  QUrl url = QUrl::fromLocalFile( helpFile );

  browser->setSource( url );
}

/** User request, to zoom into the document. */
void HelpBrowser::slotZoomIn()
{
  if( zoomIn->isDown() )
    {
      QFont curFt = font();

      if( curFt.pointSize() < 24 )
        {
          curFt.setPointSize( curFt.pointSize() + 1 );
          setFont( curFt );
          update();
        }

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotZoomIn()));
    }
}

/** User request, to zoom out the document. */
void HelpBrowser::slotZoomOut()
{
  if( zoomOut->isDown() )
    {
      QFont curFt = font();

      if( curFt.pointSize() > 10 )
        {
          curFt.setPointSize( curFt.pointSize() - 1 );
          setFont( curFt );
          update();
        }

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotZoomOut()));
    }
}

/** catch certain key events for special handling */
void HelpBrowser::keyPressEvent( QKeyEvent *event )
{
  // Toggle display between full and normal screen. That is a predefined
  // Maemo hardware key.
  if( event->key() == Qt::Key_F6 || event->key() == Qt::Key_Space )
    {
      setWindowState(windowState() ^ Qt::WindowFullScreen);
      return;
    }

  // Zoom in with key F7. That is a predefined Maemo hardware key for zooming.
  if( event->key() == Qt::Key_F7 || event->key() == QKeySequence::ZoomIn )
    {
      slotZoomIn();
      return;
    }

  // Zoom out with key F8. That is a predefined Maemo hardware key for zooming.
  if( event->key() == Qt::Key_F8 || event->key() == QKeySequence::ZoomOut )
    {
      slotZoomOut();
      return;
    }

  // Check for exit keys.
  if( event->key() == Qt::Key_Close || event->key() == Qt::Key_Escape )
    {
      close();
      return;
    }
}
