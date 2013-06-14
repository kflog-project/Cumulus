/***********************************************************************
 **
 **   helpbrowser.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2008-2013 by Axel Pauli (axel@kflog.org)
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   This class is used for displaying the help usage of Cumulus.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "helpbrowser.h"
#include "layout.h"

HelpBrowser::HelpBrowser( QWidget *parent, QString helpFile ) :
  QWidget(parent, Qt::Tool),
  firstCall(true),
  m_helpFile(helpFile)
{
  setWindowTitle(tr("Cumulus Help"));
  setWindowIcon( GeneralConfig::instance()->loadPixmap( "cumulus.png" ) );
  setAttribute(Qt::WA_DeleteOnClose);

  m_browser = new QTextBrowser(this);

#ifdef QSCROLLER
  QScroller::grabGesture(m_browser->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_browser->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  m_browser->setOpenLinks( true );
  m_browser->setOpenExternalLinks( true );

  connect( m_browser, SIGNAL(cursorPositionChanged()), SLOT(slotCursorChanged()));

  // get the icon size to be used
  int buttonSize = Layout::getButtonSize();

#ifdef MAEMO
  buttonSize = IconSize + 5;
#endif

  int iconSize   = buttonSize - 5;

  QPushButton *home = new QPushButton();
  home->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "home_new.png") ) );
  home->setIconSize(QSize(iconSize, iconSize));
  home->setMinimumSize(buttonSize, buttonSize);
  home->setMaximumSize(buttonSize, buttonSize);
  home->setToolTip( tr("Begin") );

  QPushButton *back = new QPushButton();
  back->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "left.png") ) );
  back->setIconSize(QSize(iconSize, iconSize));
  back->setMinimumSize(buttonSize, buttonSize);
  back->setMaximumSize(buttonSize, buttonSize);
  back->setToolTip( tr("Backward") );

  QPushButton *forward = new QPushButton();
  forward->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "right.png") ) );
  forward->setIconSize(QSize(iconSize, iconSize));
  forward->setMinimumSize(buttonSize, buttonSize);
  forward->setMaximumSize(buttonSize, buttonSize);
  forward->setToolTip( tr("Forward") );

  m_zoomIn = new QPushButton();
  m_zoomIn->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "zoomin.png") ) );
  m_zoomIn->setIconSize(QSize(iconSize, iconSize));
  m_zoomIn->setMinimumSize(buttonSize, buttonSize);
  m_zoomIn->setMaximumSize(buttonSize, buttonSize);
  m_zoomIn->setToolTip( tr("Zoom in") );

  m_zoomOut = new QPushButton();
  m_zoomOut->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "zoomout.png") ) );
  m_zoomOut->setIconSize(QSize(iconSize, iconSize));
  m_zoomOut->setMinimumSize(buttonSize, buttonSize);
  m_zoomOut->setMaximumSize(buttonSize, buttonSize);
  m_zoomOut->setToolTip( tr("Zoom out") );

  QPushButton *close = new QPushButton();
  close->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "cancel.png") ) );
  close->setIconSize(QSize(iconSize, iconSize));
  close->setToolTip( tr("Close") );

  QHBoxLayout *butLayout = new QHBoxLayout;
  butLayout->setSpacing( 25 );
  butLayout->addWidget( home );
  butLayout->addWidget( back );
  butLayout->addWidget( forward );
  butLayout->addWidget( m_zoomIn );
  butLayout->addWidget( m_zoomOut );
  butLayout->addStretch();
  butLayout->addWidget( close );

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout( butLayout );
  mainLayout->addWidget( m_browser );
  setLayout( mainLayout );

  connect( home, SIGNAL(released()),    m_browser, SLOT(home()));
  connect( back, SIGNAL(released()),    m_browser, SLOT(backward()));
  connect( forward, SIGNAL(released()), m_browser, SLOT(forward()));
  connect( m_zoomIn, SIGNAL(pressed()),  this, SLOT( slotZoomIn()));
  connect( m_zoomOut, SIGNAL(pressed()), this, SLOT( slotZoomOut()));
  connect( close, SIGNAL(released()),   this, SLOT( close()));
}

HelpBrowser::~HelpBrowser()
{
}

/** Catch show events. If the first event is caught, we will load
 *  the help file into the m_browser.
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
    "/help/" + lang + "/" + m_helpFile;

  // We do check, if the help file does exists
  QFileInfo info(helpFile);

  if( ! info.isReadable() )
    {
      // fall back to English as default
      lang = "en";
      helpFile = GeneralConfig::instance()->getDataRoot() +
        "/help/" + lang + "/" + m_helpFile;
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

  m_browser->setSource( url );
}

/** User request, to zoom into the document. */
void HelpBrowser::slotZoomIn()
{
  if( m_zoomIn->isDown() )
    {
      QFont curFt = m_browser->currentFont();

      if( (curFt.pointSize() != -1 && curFt.pointSize() < 24) ||
          (curFt.pixelSize() != -1 && curFt.pixelSize() < 24) )
        {
          m_browser->zoomIn();
        }

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotZoomIn()));
    }
}

/** User request, to zoom out the document. */
void HelpBrowser::slotZoomOut()
{
  if( m_zoomOut->isDown() )
    {
      QFont curFt = m_browser->currentFont();

      if( (curFt.pointSize() != -1 && curFt.pointSize() > 8) ||
          (curFt.pixelSize() != -1 && curFt.pixelSize() > 8))
        {
          m_browser->zoomOut();
        }

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotZoomOut()));
    }
}

void HelpBrowser::slotCursorChanged()
{
  // Clear cursor's text selection.
  QTextCursor textCursor = m_browser->textCursor();
  textCursor.clearSelection();
  m_browser->setTextCursor( textCursor );
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
