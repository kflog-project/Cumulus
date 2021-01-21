/***********************************************************************
**
**   numberInputPad.cpp
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

#include <climits>
#include "generalconfig.h"
#include "layout.h"
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
  m_pressedButton( 0 ),
  m_closeOk(false),
  m_disbaleNumberCheck( false ),
  m_allowEmptyResult( false )
{
#ifdef ANDROID
  setFrameStyle( QFrame::Box );
#else
  setFrameStyle( QFrame::StyledPanel );
#endif

  setLineWidth( 3 * Layout::getIntScaledDensity() );

  QPalette palette;
  palette.setColor(QPalette::WindowText, Qt::darkBlue);
  setPalette(palette);

  setObjectName( "NumberInputPad" );
  setWindowFlags( Qt::Window );
  setWindowModality( Qt::WindowModal );
  setAttribute( Qt::WA_DeleteOnClose );

  // Save the current state of the software input panel
  m_autoSip = qApp->autoSipEnabled();

  // Disable software input panel
  qApp->setAutoSipEnabled( false );

  int row = 0;
  QGridLayout* gl = new QGridLayout (this);
  gl->setMargin( 5 * Layout::getIntScaledDensity() );
  gl->setSpacing( 10 * Layout::getIntScaledDensity() );

  m_tipLabel = new QLabel (this);
  m_tipLabel->setAlignment(Qt::AlignCenter);
  gl->addWidget( m_tipLabel, row, 0, 1, 5 );

  gl->setRowStretch(row, 5 * Layout::getIntScaledDensity() );
  row++;

  m_editor = new QLineEdit (this);

  //connect( m_editor, SIGNAL(textChanged(const QString&)),
  //         this, SLOT(slot_TextChanged(const QString&)) );

  setNumber( number );
  gl->addWidget( m_editor, row, 0, 1, 5 );

  int iconSize = Layout::iconSize( font() );
  QSize qis( iconSize, iconSize );

  int minBW = QFontMetrics(font()).width("MMM");

  m_cancel = new QPushButton( " ", this );
  m_cancel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")) );
  m_cancel->setIconSize( qis );
  m_cancel->setMinimumWidth( minBW );
  gl->addWidget( m_cancel, row, 6 );
  row++;

  gl->setRowMinimumHeight( row, 5 * Layout::getIntScaledDensity() );
  gl->setColumnMinimumWidth( 5, 5 * Layout::getIntScaledDensity());
  row++;

  m_num1 = new QPushButton( " 1 ", this );
  m_num1->setMinimumWidth( minBW );
  gl->addWidget( m_num1, row, 0 );

  m_num2 = new QPushButton( " 2 ", this );
  m_num2->setMinimumWidth( minBW );
  gl->addWidget( m_num2, row, 1 );

  m_num3 = new QPushButton( " 3 ", this );
  m_num3->setMinimumWidth( minBW );
  gl->addWidget( m_num3, row, 2 );

  m_num4 = new QPushButton( " 4 ", this );
  m_num4->setMinimumWidth( minBW );
  gl->addWidget( m_num4, row, 3 );

  m_num5 = new QPushButton( " 5 ", this );
  m_num5->setMinimumWidth( minBW );
  gl->addWidget( m_num5, row, 4 );

  m_home = new QPushButton( " ", this );
  m_home->setMinimumWidth( minBW );
  m_home->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("home_new.png")) );
  m_home->setIconSize( qis );
  gl->addWidget( m_home, row, 6 );
  row++;

  m_num6 = new QPushButton( " 6 ", this );
  m_num6->setMinimumWidth( minBW );
  gl->addWidget( m_num6, row, 0 );

  m_num7 = new QPushButton( " 7 ", this );
  m_num7->setMinimumWidth( minBW );
  gl->addWidget( m_num7, row, 1 );

  m_num8 = new QPushButton( " 8 ", this );
  m_num8->setMinimumWidth( minBW );
  gl->addWidget( m_num8, row, 2 );

  m_num9 = new QPushButton( " 9 ", this );
  m_num9->setMinimumWidth( minBW );
  gl->addWidget( m_num9, row, 3 );

  m_num0 = new QPushButton( " 0 ", this );
  m_num0->setMinimumWidth( minBW );
  gl->addWidget( m_num0, row, 4 );

  m_pm = new QPushButton( "+ -", this );
  m_pm->setMinimumWidth( minBW );

  row++;

  m_decimal = new QPushButton( " . ", this );
  m_decimal->setMinimumWidth( minBW );
  gl->addWidget( m_decimal, row, 0 );

  m_left = new QPushButton( " ", this);
  m_left->setMinimumWidth( minBW );
  //m_left->setIcon(style->standardIcon(QStyle::SP_ArrowLeft));
  m_left->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("left-32.png")) );
  m_left->setIconSize( qis );
  gl->addWidget( m_left, row, 1 );

  m_right = new QPushButton( " ", this );
  m_right->setMinimumWidth( minBW );
  //m_right->setIcon(style->standardIcon(QStyle::SP_ArrowRight));
  m_right->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("right-32.png")) );
  m_right->setIconSize( qis );
  gl->addWidget( m_right, row, 2 );

  m_delLeft = new QPushButton( " ", this );
  m_delLeft->setMinimumWidth( minBW );
  //m_delLeft->setIcon(style->standardIcon(QStyle::SP_MediaSkipBackward));
  m_delLeft->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("media-skip-backward-32.png")) );
  m_delLeft->setIconSize( qis );
  gl->addWidget( m_delLeft, row, 3 );

  m_delRight = new QPushButton( " ", this );
  m_delRight->setMinimumWidth( minBW );
  //m_delRight->setIcon (style->standardIcon(QStyle::SP_MediaSkipForward));
  m_delRight->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("media-skip-forward-32.png")) );
  m_delRight->setIconSize( qis );
  gl->addWidget( m_delRight, row, 4 );

  m_ok = new QPushButton( " ", this );
  m_ok->setMinimumWidth( minBW );
  m_ok->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("ok.png")) );
  m_ok->setIconSize( qis );
  gl->addWidget( m_ok, row, 6 );

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

  m_digitSignalMapper = new QSignalMapper(this);
  m_buttonSignalMapper = new QSignalMapper(this);

  connect(m_num1, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_num2, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_num3, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_num4, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_num5, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_num6, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_num7, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_num8, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_num9, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_num0, SIGNAL(pressed()), m_digitSignalMapper, SLOT(map()));
  connect(m_decimal, SIGNAL(pressed()), m_buttonSignalMapper, SLOT(map()));
  connect(m_left, SIGNAL(pressed()), m_buttonSignalMapper, SLOT(map()));
  connect(m_right, SIGNAL(pressed()), m_buttonSignalMapper, SLOT(map()));
  connect(m_delLeft, SIGNAL(pressed()), m_buttonSignalMapper, SLOT(map()));
  connect(m_delRight, SIGNAL(pressed()), m_buttonSignalMapper, SLOT(map()));
  connect(m_home, SIGNAL(pressed()), m_buttonSignalMapper, SLOT(map()));

  m_digitSignalMapper->setMapping(m_num1, m_num1);
  m_digitSignalMapper->setMapping(m_num2, m_num2);
  m_digitSignalMapper->setMapping(m_num3, m_num3);
  m_digitSignalMapper->setMapping(m_num4, m_num4);
  m_digitSignalMapper->setMapping(m_num5, m_num5);
  m_digitSignalMapper->setMapping(m_num6, m_num6);
  m_digitSignalMapper->setMapping(m_num7, m_num7);
  m_digitSignalMapper->setMapping(m_num8, m_num8);
  m_digitSignalMapper->setMapping(m_num9, m_num9);
  m_digitSignalMapper->setMapping(m_num0, m_num0);
  m_buttonSignalMapper->setMapping(m_decimal, m_decimal);
  m_buttonSignalMapper->setMapping(m_left, m_left);
  m_buttonSignalMapper->setMapping(m_right, m_right);
  m_buttonSignalMapper->setMapping(m_delLeft, m_delLeft);
  m_buttonSignalMapper->setMapping(m_delRight, m_delRight);
  m_buttonSignalMapper->setMapping(m_home, m_home);

  connect( m_digitSignalMapper, SIGNAL(mapped(QWidget *)),
           this, SLOT(slot_DigitPressed(QWidget *)));
  connect( m_buttonSignalMapper, SIGNAL(mapped(QWidget *)),
           this, SLOT(slot_ButtonPressed(QWidget *)));

  connect( m_editor, SIGNAL(returnPressed() ), this, SLOT(slot_Ok()) );
  connect( m_pm, SIGNAL(pressed() ), this, SLOT(slot_Pm()) );
  connect( m_ok, SIGNAL(pressed() ), this, SLOT(slot_Ok()) );
  connect( m_cancel, SIGNAL(pressed() ), this, SLOT(slot_Close()) );

  m_timerButton = new QTimer( this );
  m_timerButton->setSingleShot( true );
  m_timerDigit = new QTimer( this );
  m_timerDigit->setSingleShot( true );

  connect( m_timerButton, SIGNAL(timeout()), this, SLOT(slot_RepeatButton()));
  connect( m_timerDigit, SIGNAL(timeout()), this, SLOT(slot_RepeatDigit()));
}

NumberInputPad::~NumberInputPad()
{
  // restore sip state
  qApp->setAutoSipEnabled( m_autoSip );
}

void NumberInputPad::showEvent( QShowEvent* event )
{
  m_editor->setFocus();
  m_editor->home( false );
  m_tipLabel->text().isEmpty() ? m_tipLabel->hide() : m_tipLabel->show();

  QFrame::showEvent( event );
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

void NumberInputPad::slot_DigitPressed( QWidget* widget )
{
  QPushButton* button = dynamic_cast<QPushButton*> (widget);

  if( button == 0 )
    {
      // Cast failed!
      return;
    }

  // Remove leading and trailing spaces from the number.
  QString text = button->text().trimmed();

  // 0...9 was pressed
  m_editor->setSelection(m_editor->cursorPosition(), 1);
  m_editor->insert( text );
  m_pressedButton = button;

  // Setup a timer to handle a longer button press as repeat.
  m_timerDigit->start(300);
}

void NumberInputPad::slot_ButtonPressed( QWidget* widget )
{
  QPushButton* button = dynamic_cast<QPushButton*> (widget);

  if( button == 0 )
    {
      // Cast failed!
      return;
    }

  m_pressedButton = button;

  // make a deselect of the text to prevent unwanted behavior.
  m_editor->deselect();

  if( button == m_decimal )
    {
     if( m_disbaleNumberCheck == true ||
          m_editor->text().contains( "." ) == false )
        {
          // First input of decimal point
          m_editor->setSelection( m_editor->cursorPosition(), 1 );
          m_editor->insert( "." );
        }
      else
        {
          // Check, if decimal point is at the old place. Under this condition
          // move cursor one position to the right and accept input.
          if( m_editor->text().indexOf( "." ) == m_editor->cursorPosition() )
            {
              m_editor->cursorForward( false );
            }
        }

      m_pressedButton = 0;
    }
  else if( button == m_left )
    {
      m_editor->cursorBackward( false );
    }
  else if( button == m_right )
    {
      m_editor->cursorForward( false );
    }
  else if( button == m_delLeft )
    {
      m_editor->backspace();
    }
  else if( button == m_delRight )
    {
      m_editor->del();
    }
  else if( button == m_home )
    {
      m_editor->home( false );
      m_pressedButton = 0;
    }

  if( m_pressedButton )
    {
      // Setup a timer to handle a longer button press as repeat.
      m_timerButton->start(300);
    }
  else
    {
      m_timerButton->stop();
    }
}

void NumberInputPad::slot_TextChanged( const QString& text )
{
  // Check input as integer value against the allowed maximum/minimum.
  bool ok;
  int iValue = text.toInt( &ok );

  if( m_disbaleNumberCheck == true )
    {
      return;
    }

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

void NumberInputPad::slot_RepeatButton()
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

void NumberInputPad::slot_RepeatDigit()
{
  if( m_pressedButton && m_pressedButton->isDown() )
    {
      slot_DigitPressed( m_pressedButton );
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
      m_editor->setText( "-" + m_editor->text() );
    }
}

void NumberInputPad::slot_Ok()
{
  m_closeOk = true;
  m_timerButton->stop();
  m_timerDigit->stop();

  const QValidator* validator = m_editor->validator();

  QString value = m_editor->text().trimmed();

  if( value.isEmpty() && m_allowEmptyResult == true )
    {
      // Return of an empty value is allowed.
      emit numberEdited( value );
      // Make a delay of 200 ms before the widget is closed to prevent undesired
      // selections in an underlaying list. Problem occurred on Galaxy S3.
      QTimer::singleShot(200, this, SLOT(slot_closeWidget()));
      return;
    }

  // This checks the set minimum resp. maximum of the input value. If the limits
  // are exceeded the allowed minimum resp. maximum is taken as return value.
  slot_TextChanged( value );

  value = m_editor->text().trimmed();

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

  // Make a delay of 200 ms before the widget is closed to prevent undesired
  // selections in an underlaying list. Problem occurred on Galaxy S3.
  QTimer::singleShot(200, this, SLOT(slot_closeWidget()));
}

void NumberInputPad::slot_Close()
{
  m_closeOk = true;
  m_timerButton->stop();
  m_timerDigit->stop();

  // Nothing should be changed, return initial number.
  emit numberEdited( m_setNumber );

  // Make a delay of 200 ms before the widget is closed to prevent undesired
  // selections in an underlaying list. Problem occurred on Galaxy S3.
  QTimer::singleShot(200, this, SLOT(slot_closeWidget()));
}

void NumberInputPad::slot_closeWidget()
{
  QWidget::close();
}

/**
 * Catch the Close event, when the little x at the window frame is pressed.
 */
void NumberInputPad::closeEvent( QCloseEvent *event )
{
  if( m_closeOk == false )
    {
      // OhhhhHuuu the frame x button was pressed.
      // Nothing should be changed, return initial number.
      emit numberEdited( m_setNumber );
    }

  event->accept();
}
