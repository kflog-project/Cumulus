/***********************************************************************
 **
 **   helpbrowser.cpp
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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QToolTip>
#include <QFileInfo>
#include <QFont>
#include <QKeyEvent>

#include "helpbrowser.h"
#include "generalconfig.h"

/** Creates a help browser widget as single window and loads
 *  the cumulus help file into it according to the selected
 *  language. The user can navigate through the text, zoom in and out,
 *  maximize/normalize the window display size.
 */
HelpBrowser::HelpBrowser( QWidget *parent ) : QWidget(parent, Qt::Window),
                                              firstCall(true)
{
  setWindowTitle(tr("Cumulus Help"));
  setWindowIcon( GeneralConfig::instance()->loadPixmap( "cumulus.png" ) );
  
  browser = new QTextBrowser(this);

  QPushButton *home = new QPushButton();
  home->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "home.png") ) );
  home->setFlat(true);
  home->setToolTip( tr("Goto home") );

  QPushButton *back = new QPushButton();
  back->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "back.png") ) );
  back->setFlat(true);
  back->setToolTip( tr("Go back") );

  QPushButton *forward = new QPushButton();
  forward->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "forward.png") ) );
  forward->setFlat(true);
  forward->setToolTip( tr("Go forward") );

  QPushButton *close = new QPushButton();
  close->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "standardbutton-close-16.png") ) );
  close->setFlat(true);
  close->setToolTip( tr("Close") );

  QHBoxLayout *butLayout = new QHBoxLayout;
  butLayout->addWidget( home );
  butLayout->addWidget( back );
  butLayout->addWidget( forward );
  butLayout->addWidget( close );
  butLayout->addStretch();

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout( butLayout );
  mainLayout->addWidget( browser );
  setLayout( mainLayout );

  connect( home, SIGNAL(clicked()),    browser, SLOT(home()));
  connect( back, SIGNAL(clicked()),    browser, SLOT(backward()));
  connect( forward, SIGNAL(clicked()), browser, SLOT(forward()));
  connect( close, SIGNAL(clicked()),   this, SLOT( close()));
}

HelpBrowser::~HelpBrowser()
{
  // qDebug("HelpBrowser::~HelpBrowser()");
}

/** Catch show events. If the first event is catched, we will load
 *  the help file into the browser.
 */
void HelpBrowser::showEvent( QShowEvent * )
{  
  if( ! firstCall )
    {
      // ot the first call, ignore this event
      return;
    }
  
  // first call, we try to load the cumulus html help file
  firstCall = false;
  
  QString lang = GeneralConfig::instance()->getLanguage();

  QString helpFile = GeneralConfig::instance()->getInstallRoot() +
    "/help/" + lang + "/cumulus.html";
  
  // We do check, if the help file does exists
  QFileInfo info(helpFile);

  if( ! info.isReadable() )
    {
      // fall back to english as default
      lang = "en";
      helpFile = GeneralConfig::instance()->getInstallRoot() +
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

/** catch certain key events for special handling */
void HelpBrowser::keyPressEvent( QKeyEvent *event )
{
  // Toggle display between full and normal screen. That is a predefined
  // Maemo hardware key.
  if( event->key() == Qt::Key_F6 )
    {
      setWindowState(windowState() ^ Qt::WindowFullScreen);
      return;
    }
    
  // Zoom in with key F7. That is a predefined Maemo hardware key for zooming.
  QFont curFt = font();
  
  if( event->key() == Qt::Key_F7 )
    {
      if( curFt.pointSize() < 18 )
        {
          curFt.setPointSize( curFt.pointSize() + 1 );
          setFont( curFt );
          update();
        }
        
      return;
    }

  // Zoom out with key F8. That is a predefined Maemo hardware key for zooming.
  if( event->key() == Qt::Key_F8 )
    {
      if( curFt.pointSize() > 10 )
        {
          curFt.setPointSize( curFt.pointSize() - 1 );
          setFont( curFt );
          update();
        }

      return;
    }
}
