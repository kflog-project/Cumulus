/***********************************************************************
**
**   numberInputPad.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012-2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <climits>
#include <QtGui>

#include "generalconfig.h"
#include "numberInputPad.h"

/**
 * Constructor
 */
NumberInputPad::NumberInputPad( QString number, QWidget *parent ) :
  QFrame( parent ),
  m_autoSip(true),
  m_setNumber(number),
  m_intMaximum(false, INT_MAX),
  m_intMinimum(false, INT_MIN),
  m_doubleMaximum(false, 0.0),
  m_doubleMinimum(false, 0.0),
  m_pressedButton( 0 )
{
  setFrameStyle( QFrame::StyledPanel | QFrame::Plain );
  setLineWidth ( 3 );

  setObjectName("NumberInputPad");
  setWindowFlags(Qt::Tool);
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  // Save the current state of the software input panel
  m_autoSip = qApp->autoSipEnabled();

  // Disable software input panel
  qApp->setAutoSipEnabled( false );

  int row = 0;
  QGridLayout* gl = new QGridLayout(this);
  gl->setMargin(5);

  m_tipLabel = new QLabel;
  m_tipLabel->setAlignment(Qt::AlignCenter);
  gl->addWidget( m_tipLabel, row, 0, 1, 5 );
  row++;

  m_editor = new QLineEdit;
  connect( m_editor, SIGNAL(textChanged(const QString&)),
           this, SLOT(slot_TextChanged(const QString&)) );

  setNumber( number );
  gl->addWidget( m_editor, row, 0, 1, 5 );

  m_ok = new QPushButton( " " );
  m_ok->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("ok.png")) );
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

  m_home = new QPushButton( "H" );
  gl->addWidget( m_home, row, 6 );
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

  m_pm = new QPushButton( "+-" );
  gl->addWidget( m_pm, row, 6 );

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

  m_cancel = new QPushButton( " " );
  m_cancel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")) );
  gl->addWidget( m_cancel, row, 6 );

  m_ok->setFocusPolicy( Qt::NoFocus );
  m_cancel->setFocusPolicy( Qt::NoFocus );
  m_delLeft->setFocusPolicy( Qt::NoFocus );
  m_delRight->setFocusPolicy( Qt::NoFocus );
  m_left->setFocusPolicy( Qt::NoFocus );
  m_right->setFocusPolicy( Qt::NoFocus );
  m_decimal->setFocusPolicy( Qt::NoFocus );
  m_pm->setFocusPolicy( Qt::NoFocus );
  m_home->setFocusPolicy( Qt::NoFocus );
  m_num0->setFocusPolicy( Qt::NoFocus );
  m_num1->setFocusPolicy( Qt::NoFocus );
  m_num2->setFocusPolicy( Qt::NoFocus );
  m_num3->setFocusPolicy( Qt::NoFocus );
  m_num4->setFocusPolicy( Qt::NoFocus );
  m_num5->setFocusPolicy( Qt::NoFocus );
  m_num6->setFocusPolicy( Qt::NoFocus );
  m_num7->setFocusPolicy( Qt::NoFocus );
  m_num8->setFocusPolicy( Qt::NoFocus );
  m_num9->setFocusPolicy( Qt::NoFocus );

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
  connect(m_home, SIGNAL(pressed()), m_signalMapper, SLOT(map()));

  m_signalMapper->setMapping(m_num1, m_num1);
  m_signalMapper->setMapping(m_num2, m_num2);
  m_signalMapper->setMapping(m_num3, m_num3);
  m_signalMapper->setMapping(m_num4, m_num4);
  m_signalMapper->setMapping(m_num5, m_num5);
  m_signalMapper->setMapping(m_num6, m_num6);
  m_signalMapper->setMapping(m_num7, m_num7);
  m_signalMapper->setMapping(m_num8, m_num8);
  m_signalMapper->setMapping(m_num9, m_num9);
  m_signalMapper->setMapping(m_num0, m_num0);
  m_signalMapper->setMapping(m_decimal, m_decimal);
  m_signalMapper->setMapping(m_left, m_left);
  m_signalMapper->setMapping(m_right, m_right);
  m_signalMapper->setMapping(m_delLeft, m_delLeft);
  m_signalMapper->setMapping(m_delRight, m_delRight);
  m_signalMapper->setMapping(m_home, m_home);

  connect( m_signalMapper, SIGNAL(mapped(QWidget *)),
           this, SLOT(slot_ButtonPressed(QWidget *)));

  connect( m_pm, SIGNAL(pressed() ), this, SLOT(slot_Pm()) );
  connect( m_ok, SIGNAL(pressed() ), this, SLOT(slot_Ok()) );
  connect( m_cancel, SIGNAL(pressed() ), this, SLOT(slot_Close()) );

  m_timer = new QTimer( this );
  m_timer->setSingleShot( true );

  connect( m_timer, SIGNAL(timeout()), this, SLOT(slot_Repeat()));
}

NumberInputPad::~NumberInputPad()
{
  // restore sip state
  qApp->setAutoSipEnabled( m_autoSip );
}

void NumberInputPad::showEvent( QShowEvent* /* event */ )
{
  m_editor->setFocus();
  m_editor->home( false );
  m_tipLabel->text().isEmpty() ? m_tipLabel->hide() : m_tipLabel->show();
}

void NumberInputPad::setTip( QString tip )
{
  m_tipLabel->setText( tip );

  if( tip.isEmpty() )
    {
      m_tipLabel->hide();
      return;
    }

  if( isVisible() )
    {
      m_tipLabel->show();
    }
}

void NumberInputPad::slot_ButtonPressed( QWidget* widget )
{
  QPushButton* button = dynamic_cast<QPushButton*> (widget);

  if( button == 0 )
    {
      // Cast failed!
      return;
    }

  // Find out, which button was pressed.
  QString text = button->text();

  QRegExp rxNumber("[0-9]");

  if( rxNumber.exactMatch(text) )
    {
      // 0...9 was pressed
      m_editor->setSelection(m_editor->cursorPosition(), 1);
      m_editor->insert( text );
      m_pressedButton = button;
    }
  else if( text == "." )
    {
      if( m_editor->text().contains(".") == false )
        {
          // First input of decimal point
          m_editor->setSelection(m_editor->cursorPosition(), 1);
          m_editor->insert( text );
          m_pressedButton = 0;
        }
      else
        {
          // Check, if decimal point is at the old place. Under this condition
          // move cursor one position to the right and accept input.
          if( m_editor->text().indexOf(".") == m_editor->cursorPosition() )
            {
              m_editor->setSelection(m_editor->cursorPosition(), 1);
              m_pressedButton = 0;
            }
        }
    }
  else if( text == "<-" )
    {
      m_editor->cursorBackward( false );
      m_pressedButton = m_left;
    }
  else if( text == "->" )
    {
      m_editor->cursorForward( false );
      m_pressedButton = m_right;
    }
  else if( text == "<x" )
    {
      m_editor->backspace();
      m_pressedButton = m_delLeft;
    }
  else if( text == "x>" )
    {
      m_editor->del();
      m_pressedButton = m_delRight;
    }
  else if( text == "H" )
    {
      m_editor->home( false );
      m_pressedButton = 0;
    }

  if( m_pressedButton )
    {
      // Setup a timer to handle a longer button press as repeat.
      m_timer->start(300);
    }
  else
    {
      m_timer->stop();
    }
}

void NumberInputPad::slot_TextChanged( const QString& text )
{
  // Check input as integer value against the allowed maximum/minimum.
  bool ok;
  int iValue = text.toInt( &ok );

  if( ok )
    {
      if( m_intMaximum.first && iValue > m_intMaximum.second )
        {
          // Reset input to allowed maximum
          m_editor->setText( QString::number(m_intMaximum.second) );
        }
      else if( m_intMinimum.first && iValue < m_intMinimum.second )
        {
          // Reset input to allowed minimum
          m_editor->setText( QString::number(m_intMinimum.second) );
        }

      emit valueChanged( m_editor->text().toInt() );
    }

  // Check value as double against the allowed maximum/minimum.
  double dValue = text.toDouble( &ok );

  if( ok )
    {
      if( m_doubleMaximum.first && dValue > m_doubleMaximum.second )
        {
          // Reset input to allowed maximum
          m_editor->setText( QString::number(m_doubleMaximum.second) );
        }
      else if( m_doubleMinimum.first && dValue < m_doubleMinimum.second )
        {
          // Reset input to allowed minimum
          m_editor->setText( QString::number(m_doubleMinimum.second) );
        }

      emit valueChanged( m_editor->text().toDouble() );
    }
}

void NumberInputPad::slot_Repeat()
{
  if( m_pressedButton && m_pressedButton->isDown() )
    {
      slot_ButtonPressed( m_pressedButton );
    }
  else
    {
      m_pressedButton = 0;
    }
}

void NumberInputPad::slot_Pm()
{
  if( m_editor->text().startsWith("-") )
    {
      m_editor->setText( m_editor->text().remove(0, 1) );
    }
  else
    {
      m_editor->setCursorPosition( 0 );
      m_editor->insert( "-" );
      m_editor->end( false );
    }
}

void NumberInputPad::slot_Ok()
{
  m_timer->stop();

  const QValidator* validator = m_editor->validator();

  QString value = m_editor->text();

  int pos = 0;

  if( validator != 0 )
    {
      if( validator->validate( value, pos ) == QValidator::Acceptable )
        {
          emit numberEdited( value );
        }
      else
        {
          // Validator said nok, do not return.
          return;
        }
    }
  else
    {
      // Return edited number.
      emit numberEdited( value );
    }

  setVisible( false );
  QWidget::close();
}

void NumberInputPad::slot_Close()
{
  m_timer->stop();

  // Nothing should be changed, return initial number.
  emit numberEdited( m_setNumber );
  setVisible( false );
  QWidget::close();
}
