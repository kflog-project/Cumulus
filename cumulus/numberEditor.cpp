/***********************************************************************
**
**   numberEditor.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012-2022 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <climits>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "mainwindow.h"
#include "numberInputPad.h"
#include "numberEditor.h"

/**
 * Constructor
 */
NumberEditor::NumberEditor( QWidget *parent,
                            QString number,
                            QString prefix,
                            QString suffix ) :
  QLabel( parent ),
  m_nip(0),
  m_prefix(prefix),
  m_number(number),
  m_suffix(suffix),
  m_title(tr("edit number")),
  m_decimalFlag(true),
  m_pmFlag(true),
  m_allowEmptyResult(false),
  m_validator(0),
  m_inputMask(""),
  m_minWidth(INT_MIN),
  m_maxLength(32767),
  m_intMax(false, INT_MAX),
  m_intMin(false, INT_MIN),
  m_doubleMax(false, double(INT_MAX)),
  m_doubleMin(false, double(INT_MIN)),
  m_specialValueText(""),
  m_fixHeight( true ),
  m_disableNumberCheck( false )
{
  setObjectName("NumberEditor");
  setBackgroundRole( QPalette::Light );
  setAutoFillBackground( true );
  setAlignment(Qt::AlignCenter);
  setMargin(2);
  setFrameStyle( QFrame::StyledPanel );

  QSizePolicy sp = sizePolicy();
  sp.setVerticalPolicy( QSizePolicy::Fixed );
  setSizePolicy( sp );

  // We set a default validator to prevent wrong input from an external
  // keyboard, if no own validator is defined by the caller.
  m_validator = new QRegExpValidator( QRegExp( "(-?[0-9]+|[0-9]+\\.[0-9]+)" ), this );

  setText();
}

NumberEditor::~NumberEditor()
{
}

void NumberEditor::showEvent( QShowEvent* event )
{
  if( m_fixHeight == true )
    {
      QSize size = minimumSizeHint();
      setMinimumHeight( size.height() );
      setMaximumHeight( size.height() );
    }

  if( m_minWidth == INT_MIN )
    {
      // Default value, if not set.
      m_minWidth = 6;
    }

  QString ms;

  // Set minimum width
  for(int i=0; i < m_minWidth; i++ )
    {
      ms += 'M';
    }

  QFontMetrics fm( font() );
  int strWidth = fm.width( QString( ms ) );
  setMinimumWidth( strWidth );

  QLabel::showEvent( event );
}

void NumberEditor::mousePressEvent( QMouseEvent* event )
{
  // Opens the number input pad after a mouse press.
  if( ! m_nip )
    {
      m_nip = new NumberInputPad( m_number, this );
      m_nip->setWindowTitle( m_title );
      m_nip->setDecimalVisible( m_decimalFlag );
      m_nip->setPmVisible( m_pmFlag );
      m_nip->setMaxLength( m_maxLength );
      m_nip->setInputMask( m_inputMask );
      m_nip->setValidator( m_validator );
      m_nip->setTip( m_tip );
      m_nip->disableNumberCheck( m_disableNumberCheck );
      m_nip->allowEmptyResult( m_allowEmptyResult );

      if( m_intMax.first ) m_nip->setIntMaximum( m_intMax.second );
      if( m_intMin.first ) m_nip->setIntMinimum( m_intMin.second );
      if( m_doubleMax.first ) m_nip->setDoubleMaximum( m_doubleMax.second );
      if( m_doubleMin.first ) m_nip->setDoubleMinimum( m_doubleMin.second );

      connect( m_nip, SIGNAL(numberEdited(const QString&) ),
               SLOT(slot_NumberEdited(const QString&)) );
      connect( m_nip, SIGNAL(valueChanged(int) ),
               this, SIGNAL(valueChanged(int)) );
      connect( m_nip, SIGNAL(valueChanged(double) ),
               this, SIGNAL(valueChanged(double)) );

      m_nip->raise();
      m_nip->show();

      QCoreApplication::sendPostedEvents();
      QCoreApplication::processEvents();

      // Set focus at the editor.
      m_nip->getEditor()->setFocus();

#ifdef ANDROID
      // Sets the window's background to another color.
      m_nip->setAutoFillBackground( true );
      m_nip->setBackgroundRole( QPalette::Window );
      m_nip->setPalette( QPalette( QColor( Qt::lightGray ) ) );
#endif

      // The number input widget to be shown is not centered over this parent.
      // Therefore this must be done by our self.
      // QSize ms = m_nip->minimumSizeHint();
      QSize ms = m_nip->size();
      ms += QSize(10, 10);

      QPoint pos = MainWindow::mainWindow()->pos();
      int mwWidth = MainWindow::mainWindow()->width();
      int mwHeight = MainWindow::mainWindow()->height();

      m_nip->setGeometry( pos.x() + mwWidth / 2 - ms.width() / 2,
                          pos.y() + mwHeight / 2 - ms.height() / 2,
                          ms.width(), ms.height() );

      emit numberPadOpened();
    }

  event->accept();
}

void NumberEditor::slot_NumberEdited( const QString& number )
{
  m_nip = 0;
  m_number = number;
  setText();
  emit numberEdited( number );
}

void NumberEditor::closeEvent( QCloseEvent *event )
{
  qDebug() << "NumberEditor::closeEvent()";

  m_nip = 0;
  event->accept();
}

void NumberEditor::setText()
{
  if( m_specialValueText.isEmpty() ||
      ( m_intMin.first == false && m_doubleMin.first == false ) )
    {
      // No special text nor minimums are set.
      QLabel::setText( m_prefix + m_number + m_suffix );
      return;
    }

  // Check for special value setting
  bool iOk;
  bool dOk;

  int    i = m_number.toInt(&iOk);
  double d = m_number.toDouble(&dOk);

  if( iOk && m_intMin.first == true && m_number.contains(".") == false )
    {
      // Check special handling for integer
      if( i == m_intMin.second )
        {
          QLabel::setText( m_specialValueText );
          return;
        }
    }
  else if( dOk && m_doubleMin.first == true && m_number.contains(".") )
    {
      // Check special handling for double
      if( d == m_doubleMin.second )
        {
          QLabel::setText( m_specialValueText );
          return;
        }
    }

  // No special handling condition was true
  QLabel::setText( m_prefix + m_number + m_suffix );
}
