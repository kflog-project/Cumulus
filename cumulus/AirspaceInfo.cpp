/***********************************************************************
**
**   AirspaceInfo.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2023-2025 by Axel Pauli
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
#include "KRT2Widget.h"
#include "layout.h"

#include "MainWindow.h"

const int vMargin = 5;
const int hMargin = 5;

AirspaceInfo::AirspaceInfo( QWidget* parent, QString& txt, QList<Airspace *>& asList ) :
  QFrame( parent ),
  m_timerCount(0),
  m_asList( asList )
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

  m_cmdKRT2 = new QPushButton(tr("KRT2"), this);
  m_cmdKRT2->setFont(bfont);
  buttonrow->addWidget(m_cmdKRT2);
  connect(m_cmdKRT2, SIGNAL(clicked()), SLOT(slot_openKRT2Dialog()));

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

  bool hideKrt2Button = true;

  for( int i=0; i < m_asList.size(); i++ )
    {
      if( m_asList.at(i)->getFrequencyList().size() > 0 )
        {
          hideKrt2Button = false;
          break;
        }
    }

  if( hideKrt2Button == true )
    {
      m_cmdKRT2->hide();
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

/**
 * Opens the KRT2 dialog window.
 */
void AirspaceInfo::slot_openKRT2Dialog()
{
  QString header = QString( tr("Airspace frequencies") );

  QList<Frequency> fList;

  for( int i=0; i < m_asList.size(); i++ )
    {
      if( m_asList.at(i)->getFrequencyList().size() > 0 )
        {
          QList<Frequency> fl = m_asList.at(i)->getFrequencyList();

          for( int j=0; j < fl.size(); j++ )
            {
              fList.append( fl.at(j) );
            }
        }
    }

  if( fList.size() > 0 )
    {
      slot_Stop();
      KRT2Widget* krt2 = new KRT2Widget( this, header, fList );
      krt2->show();
    }
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
