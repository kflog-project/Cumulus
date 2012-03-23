/***********************************************************************
**
**   messagewidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

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
      resize( QSize(800, 400) );
    }

  setVisible( true );
}

MessageWidget::~MessageWidget()
{
}

void MessageWidget::showEvent( QShowEvent* )
{
  QSize ws = size();

  QTextDocument *doc = m_text->document();

  QSize ds = doc->size().toSize();

  while( true )
    {
      ds = doc->size().toSize();

      // qDebug() << "ds=" << ds << "ws=" << ws
      //          << "FPS=" << m_text->currentFont().pointSize()
      //          << "Diff=" << (ws - ds - QSize(20, 50 ));

      if( (ws - ds - QSize(20, 50 )).isValid() == false )
        {
          m_text->zoomOut();
          continue;
        }

      break;
    }

  if( m_text->currentFont().pointSize() != -1 && font().pointSize() != -1 &&
      m_text->currentFont().pointSize() < font().pointSize() )
    {
      // Make the global font smaller, if the document font was narrowed.
      QFont cf = font();
      cf.setPointSize(m_text->currentFont().pointSize());
      QApplication::setFont( cf );
    }
}
