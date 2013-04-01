/***********************************************************************
**
**   aboutwidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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

#include "aboutwidget.h"

AboutWidget::AboutWidget( QWidget *parent ) :
  QWidget( parent, Qt::Tool )
{
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  QVBoxLayout *vbox = new QVBoxLayout( this );

  headerIcon = new QLabel( this );
  headerText = new QLabel( this );

  about = new QTextBrowser( this );
  about->setOpenLinks( true );
  about->setOpenExternalLinks( true );

  team = new QTextBrowser( this );
  team->setOpenLinks( true );
  team->setOpenExternalLinks( true );

  disclaimer = new QTextBrowser( this );
  disclaimer->setOpenLinks( true );
  disclaimer->setOpenExternalLinks( true );

  QTabWidget *tabWidget = new QTabWidget( this );
  tabWidget->addTab( about, tr("About") );
  tabWidget->addTab( team, tr("Team") );
  tabWidget->addTab( disclaimer, tr("Disclaimer") );

#ifdef QSCROLLER
  QScroller::grabGesture(about->viewport(), QScroller::LeftMouseButtonGesture);
  QScroller::grabGesture(team->viewport(), QScroller::LeftMouseButtonGesture);
  QScroller::grabGesture(disclaimer->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(about->viewport(), QtScroller::LeftMouseButtonGesture);
  QtScroller::grabGesture(team->viewport(), QtScroller::LeftMouseButtonGesture);
  QtScroller::grabGesture(disclaimer->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  connect( about, SIGNAL(cursorPositionChanged()), SLOT(slotAboutCursorChanged()));
  connect( team, SIGNAL(cursorPositionChanged()), SLOT(slotTeamCursorChanged()));
  connect( disclaimer, SIGNAL(cursorPositionChanged()), SLOT(slotDisclaimerCursorChanged()));

  QPushButton *close = new QPushButton( tr("Ok"), this );

  QHBoxLayout *hbox = new QHBoxLayout;

  hbox->addWidget( headerIcon );
  hbox->addSpacing( 10 );
  hbox->addWidget( headerText );
  hbox->addStretch( 10 );

  vbox->addLayout( hbox );
  vbox->addWidget( tabWidget );
  vbox->addWidget( close );

  connect( close, SIGNAL(clicked()),  this, SLOT( close()) );
}

void AboutWidget::slotAboutCursorChanged()
{
  // Clear cursor's text selection.
  QTextCursor textCursor = about->textCursor();
  textCursor.clearSelection();
  about->setTextCursor( textCursor );
}

void AboutWidget::slotTeamCursorChanged()
{
  // Clear cursor's text selection.
  QTextCursor textCursor = team->textCursor();
  textCursor.clearSelection();
  team->setTextCursor( textCursor );
}

void AboutWidget::slotDisclaimerCursorChanged()
{
  // Clear cursor's text selection.
  QTextCursor textCursor = disclaimer->textCursor();
  textCursor.clearSelection();
  disclaimer->setTextCursor( textCursor );
}
