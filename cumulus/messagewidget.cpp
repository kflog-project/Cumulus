/***********************************************************************
**
**   messagewidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012-2021 Axel Pauli
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

#include "messagewidget.h"

MessageWidget::MessageWidget( QString text, QWidget *parent ) :
  QWidget( parent )
{
  QVBoxLayout *vbox = new QVBoxLayout(this);

  m_text = new QTextEdit(this);
  m_text->setReadOnly( true );

  if( text.contains("<html>", Qt::CaseInsensitive ) )
    {
      m_text->setHtml( text );
    }
  else
    {
      m_text->setPlainText( text );
    }

  vbox->addWidget( m_text );

  QPushButton *yes = new QPushButton( tr("Yes") );
  QPushButton *no  = new QPushButton( tr("No") );

  QHBoxLayout *buttonBox = new QHBoxLayout;
  buttonBox->addStretch( 10 );
  buttonBox->addWidget( yes );
  buttonBox->addSpacing( 30 );
  buttonBox->addWidget( no );
  buttonBox->addStretch( 10 );

  vbox->addLayout( buttonBox );

  connect( yes, SIGNAL(clicked()),  this, SIGNAL( yesClicked()) );
  connect( no, SIGNAL(clicked()),  this, SIGNAL( noClicked()) );

  if( parent )
    {
      resize( parent->size() );
    }
  else
    {
      resize( QSize(800, 480) );
    }

  setVisible( true );
}

MessageWidget::~MessageWidget()
{
}

void MessageWidget::showEvent( QShowEvent* )
{
  qDebug() << "MessageWidget::showEvent: font=" << font().toString();

  QSize ws = size();

  QTextDocument *doc = m_text->document();
  QSize ds = doc->size().toSize();
  QSize extraSpace( 0, 20);

  // Try to get displayed the whole message in the Window without scrolling.
  while( m_text->currentFont().pointSize() >= 10 )
    {
      ds = doc->size().toSize();

      qDebug() << "MessageWidget:"
               << "ds=" << ds << "ws=" << ws
               << "FPS=" << m_text->currentFont().pointSize()
               << "Diff=" << (ws - ds - extraSpace);

      if( (ws - ds - extraSpace).isValid() == false )
        {
          // The displayed text is too big for the window, try to narrow the font.
          m_text->zoomOut();
          continue;
        }

      break;
    }

  if( m_text->currentFont().pointSize() != -1 && font().pointSize() != -1 &&
      m_text->currentFont().pointSize() < font().pointSize() )
    {
      int mwPs  = m_text->currentFont().pointSize();
      int guiPs = font().pointSize();

      // Make the global font smaller, if the document font was narrowed.
      QFont cf = font();
      cf.setPointSize(m_text->currentFont().pointSize());
      QApplication::setFont( cf );
      qDebug() << "MessageWidget: GUI font has narrowed from"
               << guiPs << "to" << mwPs;
    }
}
