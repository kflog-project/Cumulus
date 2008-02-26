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
**   $Id$
**
***********************************************************************/

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolTip>

#include "helpbrowser.h"
#include "generalconfig.h"

HelpBrowser::HelpBrowser( QWidget *parent ) : QWidget(parent)
{
  setWindowTitle(tr("Cumulus Help"));

  browser = new QTextBrowser(this);

  home = new QPushButton();
  home->setPixmap( GeneralConfig::instance()->loadPixmap( "home.png") );
  home->setFlat(true);
  home->setToolTip( tr("Goto home") );

  back = new QPushButton();
  back->setPixmap( GeneralConfig::instance()->loadPixmap( "back.png") );
  back->setFlat(true);
  back->setToolTip( tr("Go back") );

  forward = new QPushButton();
  forward->setPixmap( GeneralConfig::instance()->loadPixmap( "forward.png") );
  forward->setFlat(true);
  forward->setToolTip( tr("Go forward") );

  close = new QPushButton();
  close->setPixmap( GeneralConfig::instance()->loadPixmap( "standardbutton-close-16.png") );
  close->setFlat(true);
  close->setToolTip( tr("Close") );

  QHBoxLayout *butLayout = new QHBoxLayout;
  butLayout->addWidget( home );
  butLayout->addWidget( back );
  butLayout->addWidget( forward );
  butLayout->addWidget( close );

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout( butLayout );
  mainLayout->addWidget( browser );
  setLayout( mainLayout );

  connect( home, SIGNAL(clicked()),    browser, SLOT(home()));
  connect( back, SIGNAL(clicked()),    browser, SLOT(backward()));
  connect( forward, SIGNAL(clicked()), browser, SLOT(forward()));
  connect( close, SIGNAL(clicked()),   browser, SLOT(close()));

  QString helpFile = GeneralConfig::instance()->getInstallRoot() +
    "/help/en/cumulus.html";

  QUrl url = QUrl::fromLocalFile( helpFile );

  browser->setSource( url );
}

HelpBrowser::~HelpBrowser()
{
}
