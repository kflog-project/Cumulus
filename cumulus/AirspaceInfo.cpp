/***********************************************************************
**
**   AirspaceInfo.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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

#include "layout.h"
#include "MainWindow.h"
#include "AirspaceInfo.h"
#include "generalconfig.h"

const int vMargin = 5;
const int hMargin = 5;

AirspaceInfo::AirspaceInfo( QWidget* parent, QString& txt ) :
  QFrame( parent ),
  m_timerCount(0)
{
  setObjectName("AirspaceInfo");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute( Qt::WA_DeleteOnClose );
  setStyleSheet("#AirspaceInfo { border: 5px solid red; }");
  setFrameStyle( QFrame::Box );

  if( _globalMainWindow != 0 )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  QFont bfont = font();
  bfont.setBold(true);

  QBoxLayout *topLayout = new QVBoxLayout( this );

  m_display = new QTextEdit( this );
  m_display->setReadOnly( true );
  // LightYellow www.wackerart.de/rgbfarben.html
  m_display->setStyleSheet( QString( "QTextEdit { background-color: rgb(255, 255, 224); }" ) );

#ifdef QSCROLLER
  QScroller::grabGesture( m_display->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( m_display->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  topLayout->addWidget( m_display, 10 );

  QHBoxLayout *buttonrow = new QHBoxLayout;
  topLayout->addLayout(buttonrow);

  m_cmdClose = new QPushButton(tr("Close"), this);
  m_cmdClose->setFont(bfont);
  buttonrow->addWidget(m_cmdClose);
  connect(m_cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()));

  m_cmdStop = new QPushButton(tr("Stop"), this);
  m_cmdStop->setFont(bfont);
  buttonrow->addWidget(m_cmdStop);
  connect(m_cmdStop, SIGNAL(clicked()), this, SLOT(slot_Stop()));

  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_Timeout()));

  doc = new QTextDocument( this );
  QFont font = doc->defaultFont();
  font.setPointSize( WhatsThatFontPointSize );
  doc->setDefaultFont( font );

  // check, what kind of text has been passed
  if( txt.contains("<html>", Qt::CaseInsensitive ) )
    {
      // qDebug("HTML=%s", txt.latin1());
      doc->setHtml( txt );
    }
  else
    {
      //qDebug("PLAIN=%s", txt.latin1());
      doc->setPlainText( txt );
    }

  // activate keyboard shortcuts for closing of widget
  connect( new QShortcut( Qt::Key_Escape, this ), SIGNAL(activated()),
           this, SLOT( slot_Close() ));

  connect( new QShortcut( Qt::Key_Close, this ), SIGNAL(activated()),
           this, SLOT( slot_Close() ));

  repaint();

  // @AP: Widget will be destroyed, if timer expired. If timeout is
  // zero, manual quit is expected by the user.
  m_timerCount = GeneralConfig::instance()->getAirspaceDisplayTime();

  if( m_timerCount > 0 )
    {
      m_timer->start( 1000 );
    }
}

AirspaceInfo::~AirspaceInfo()
{
}

void AirspaceInfo::slot_Close()
{
  m_timer->stop();
  hide();
  QWidget::close();
}

void AirspaceInfo::mousePressEvent( QMouseEvent* )
{
}

void AirspaceInfo::mouseReleaseEvent( QMouseEvent* )
{
}

void AirspaceInfo::keyPressEvent( QKeyEvent* )
{
  // slot_Close();
}

void AirspaceInfo::paintEvent( QPaintEvent* qpe )
{
  m_display->setDocument( doc );
  QFrame::paintEvent( qpe );
}

/** This slot get called on the timer timeout. If timer expires the
    widget will be closed automatically. */
void AirspaceInfo::slot_Timeout()
{
  if( --m_timerCount <= 0 )
    {
      m_timer->stop();
      slot_Close();
    }
  else
    {
      QString txt = tr("Close in %1 s").arg(m_timerCount);
      m_cmdClose->setText(txt);
    }
}

/** This slot is called by pressing the Stop button to keep the dialog open. :-) */
void AirspaceInfo::slot_Stop()
{
  m_timer->stop();
  m_cmdClose->setText(tr("Close"));
  m_cmdStop->hide();
}
