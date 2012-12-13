/***********************************************************************
**
**   numberInputPad.cpp
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

#include "numberInputPad.h"

/**
 * Constructor
 */
NumberInputPad::NumberInputPad( QString text, QWidget *parent ) :
  QWidget( parent ),
  m_autoSip(true)
{
  setObjectName("NumberInputPad");
  setWindowFlags(Qt::Tool);
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  // Save the current state of the software input panel
  m_autoSip = qApp->autoSipEnabled();

  int row = 0;
  QGridLayout* gl = new QGridLayout(this);
  gl->setMargin(5);

  m_editor = new QLineEdit;
  m_editor->setText( text );
  gl->addWidget( m_editor, row, 0, 1, 5 );

  m_ok = new QPushButton( "Ok" );
  gl->addWidget( m_ok, row, 6 );
  row++;

  gl->setRowMinimumHeight( row, 5 );
  gl->setColumnMinimumWidth( 5, 5 );
  row++;

  m_num1 = new QPushButton( "1" );
  gl->addWidget( m_num1, row, 0 );

  m_num2 = new QPushButton( "2" );
  gl->addWidget( m_num2, row, 1 );

  m_num3 = new QPushButton( "3" );
  gl->addWidget( m_num3, row, 2 );

  m_num4 = new QPushButton( "4" );
  gl->addWidget( m_num4, row, 3 );

  m_num5 = new QPushButton( "5" );
  gl->addWidget( m_num5, row, 4 );
  row++;

  m_num6 = new QPushButton( "6" );
  gl->addWidget( m_num6, row, 0 );

  m_num7 = new QPushButton( "7" );
  gl->addWidget( m_num7, row, 1 );

  m_num8 = new QPushButton( "8" );
  gl->addWidget( m_num8, row, 2 );

  m_num9 = new QPushButton( "9" );
  gl->addWidget( m_num9, row, 3 );

  m_num0 = new QPushButton( "0" );
  gl->addWidget( m_num0, row, 4 );
  row++;

  m_decimal = new QPushButton( "." );
  gl->addWidget( m_decimal, row, 0 );

  m_left = new QPushButton( "<-" );
  gl->addWidget( m_left, row, 1 );

  m_right = new QPushButton( "->" );
  gl->addWidget( m_right, row, 2 );

  m_delLeft = new QPushButton( "<x" );
  gl->addWidget( m_delLeft, row, 3 );

  m_delRight = new QPushButton( "x>" );
  gl->addWidget( m_delRight, row, 4 );

  m_cancel = new QPushButton( "X" );
  gl->addWidget( m_cancel, row, 6 );

  m_signalMapper = new QSignalMapper(this);

  connect(m_num1, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_num2, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_num3, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_num4, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_num5, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_num6, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_num7, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_num8, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_num9, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_num0, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_decimal, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_left, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_right, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_delLeft, SIGNAL(pressed()), m_signalMapper, SLOT(map()));
  connect(m_delRight, SIGNAL(pressed()), m_signalMapper, SLOT(map()));

  m_signalMapper->setMapping(m_num1, "1");
  m_signalMapper->setMapping(m_num2, "2");
  m_signalMapper->setMapping(m_num3, "3");
  m_signalMapper->setMapping(m_num4, "4");
  m_signalMapper->setMapping(m_num5, "5");
  m_signalMapper->setMapping(m_num6, "6");
  m_signalMapper->setMapping(m_num7, "7");
  m_signalMapper->setMapping(m_num8, "8");
  m_signalMapper->setMapping(m_num9, "9");
  m_signalMapper->setMapping(m_num0, "0");
  m_signalMapper->setMapping(m_decimal, ".");
  m_signalMapper->setMapping(m_left, "<");
  m_signalMapper->setMapping(m_right, ">");
  m_signalMapper->setMapping(m_delLeft, "xl");
  m_signalMapper->setMapping(m_delRight, "xr");

  connect( m_signalMapper, SIGNAL(mapped(const QString &)),
           this, SLOT(buttonPressed(const QString &)));

  connect( m_ok, SIGNAL(pressed() ), this, SLOT(slot_Ok()) );
  connect( m_cancel, SIGNAL(pressed() ), this, SLOT(slot_Close()) );
}

NumberInputPad::~NumberInputPad()
{
  // restore sip state
  qApp->setAutoSipEnabled( m_autoSip );
}

void NumberInputPad::buttonPressed( const QString& text )
{
  QRegExp rxNumber("[0-9]");

  if( rxNumber.exactMatch(text) )
    {
      // 0...9 was pressed
      m_editor->insert( text );
    }
  else if( text == "." && m_editor->text().contains(".") == false )
    {
      m_editor->insert( text );
    }
  else if( text == "<" )
    {
      m_editor->cursorBackward( false );
    }
  else if( text == ">" )
    {
      m_editor->cursorForward( false );
    }
  else if( text == "xl" )
    {
      m_editor->backspace();
    }
  else if( text == "xr" )
    {
      m_editor->del();
    }

  m_editor->setFocus();
}

void NumberInputPad::slot_Ok()
{
  qDebug() << "NumberInputPad::slot_Ok():" << m_editor->text();
  emit number( m_editor->text() );
  slot_Close();
}

void NumberInputPad::slot_Close()
{
  setVisible( false );
  QWidget::close();
}
