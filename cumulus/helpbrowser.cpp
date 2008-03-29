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

#include "helpbrowser.h"
#include "generalconfig.h"

// Creates a help browser widget as single window
HelpBrowser::HelpBrowser( QWidget *parent ) : QWidget(parent, Qt::Window),
                                              firstCall(true)
{
  setWindowTitle(tr("Cumulus Help"));
  setIcon( GeneralConfig::instance()->loadPixmap( "cumulus.png" ) );
  
  browser = new QTextBrowser(this);

  QPushButton *home = new QPushButton();
  home->setPixmap( GeneralConfig::instance()->loadPixmap( "home.png") );
  home->setFlat(true);
  home->setToolTip( tr("Goto home") );

  QPushButton *back = new QPushButton();
  back->setPixmap( GeneralConfig::instance()->loadPixmap( "back.png") );
  back->setFlat(true);
  back->setToolTip( tr("Go back") );

  QPushButton *forward = new QPushButton();
  forward->setPixmap( GeneralConfig::instance()->loadPixmap( "forward.png") );
  forward->setFlat(true);
  forward->setToolTip( tr("Go forward") );

  QPushButton *close = new QPushButton();
  close->setPixmap( GeneralConfig::instance()->loadPixmap( "standardbutton-close-16.png") );
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
      // not the first call
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
      
      QMessageBox::warning( this, "Cumulus - missing file",
                            tr("The help file was not found.\n"
                               "Maybe it is not installed?"));
      
      QWidget::close();
      return;
    }
  
  QUrl url = QUrl::fromLocalFile( helpFile );
  
  browser->setSource( url );
}

void HelpBrowser::keyPressEvent( QKeyEvent *event )
{
  if( event->key() == Qt::Key_F6 )
    {
      setWindowState(windowState() ^ Qt::WindowFullScreen);
    }
}
