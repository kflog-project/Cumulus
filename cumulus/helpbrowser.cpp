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

HelpBrowser::HelpBrowser( QWidget *parent ) :
  QWidget(parent, Qt::Window),
  firstCall(true)
{
  setWindowTitle(tr("Cumulus Help"));
  setWindowIcon( GeneralConfig::instance()->loadPixmap( "cumulus.png" ) );
  setAttribute(Qt::WA_DeleteOnClose);

  browser = new QTextBrowser(this);

#ifdef QSCROLLER
  QScroller::grabGesture(browser, QScroller::LeftMouseButtonGesture);
#endif

  browser->setOpenLinks( true );
  browser->setOpenExternalLinks( true );

  QPushButton *home = new QPushButton();
  home->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "home_new.png") ) );
  home->setIconSize(QSize(IconSize, IconSize));
  home->setToolTip( tr("Begin") );

  QPushButton *back = new QPushButton();
  back->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "left.png") ) );
  back->setIconSize(QSize(IconSize, IconSize));
  back->setToolTip( tr("Backward") );

  QPushButton *forward = new QPushButton();
  forward->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "right.png") ) );
  forward->setIconSize(QSize(IconSize, IconSize));
  forward->setToolTip( tr("Forward") );

  zoomIn = new QPushButton();
  zoomIn->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "zoomin.png") ) );
  zoomIn->setIconSize(QSize(IconSize, IconSize));
  zoomIn->setToolTip( tr("Zoom in") );

  zoomOut = new QPushButton();
  zoomOut->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "zoomout.png") ) );
  zoomOut->setIconSize(QSize(IconSize, IconSize));
  zoomOut->setToolTip( tr("Zoom out") );

  QPushButton *close = new QPushButton();
  close->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "cancel.png") ) );
  close->setIconSize(QSize(IconSize, IconSize));
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

  connect( home, SIGNAL(released()),    browser, SLOT(home()));
  connect( back, SIGNAL(released()),    browser, SLOT(backward()));
  connect( forward, SIGNAL(released()), browser, SLOT(forward()));
  connect( zoomIn, SIGNAL(pressed()),  this, SLOT( slotZoomIn()));
  connect( zoomOut, SIGNAL(pressed()), this, SLOT( slotZoomOut()));
  connect( close, SIGNAL(released()),   this, SLOT( close()));
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

      QMessageBox mb( QMessageBox::Warning,
                      tr( "Missing help file" ),
                      tr("<html><b>The help file was not found.<br>"
                         "Maybe it is not installed?</b></html>"),
                      QMessageBox::Ok,
                      this );
#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      mb.exec();
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
      QFont curFt = browser->currentFont();

      if( (curFt.pointSize() != -1 && curFt.pointSize() < 24) ||
          (curFt.pixelSize() != -1 && curFt.pixelSize() < 24) )
        {
          browser->zoomIn();
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
      QFont curFt = browser->currentFont();

      if( (curFt.pointSize() != -1 && curFt.pointSize() > 8) ||
          (curFt.pixelSize() != -1 && curFt.pixelSize() > 8))
        {
          browser->zoomOut();
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

  QWidget::keyPressEvent( event );
}

/** catch key release events */
void HelpBrowser::keyReleaseEvent( QKeyEvent *event )
{
  // Check for exit keys.
  if( event->key() == Qt::Key_Close || event->key() == Qt::Key_Escape )
    {
      close();
      return;
    }

  QWidget::keyReleaseEvent( event );
}
