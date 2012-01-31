/***********************************************************************
**
**   varspinbox.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "varspinbox.h"

VarSpinBox::VarSpinBox( QAbstractSpinBox* spinBox, QWidget* parent, enum ButtonOrder buttonOrder ) :
  QWidget( parent ),
  m_spinBox( spinBox ),
  m_buttonOrder( buttonOrder )
{
  setObjectName("VarSpinBox");

  if( spinBox == 0 )
    {
      qWarning() << "VarSpinBox::VarSpinBox(): No spinbox instance passed!";
      return;
    }

  m_spinBox->setButtonSymbols(QSpinBox::NoButtons);
  m_spinBox->setAlignment( Qt::AlignHCenter );

  QBoxLayout* lbox = 0;

  if( buttonOrder == Vertical )
    {
      // Arrange the buttons vertically.
      lbox = new QVBoxLayout;
    }
  else
    {
      // Horizontal order is the default
      lbox = new QHBoxLayout;
    }

  // Reset all spacing used by the layout
  lbox->setSpacing(0);
  lbox->setContentsMargins( 0, 0, 0, 0 );

  // take a bold font for the m_plus and m_minus sign
  QFont bFont = font();
  bFont.setBold(true);

  m_plus  = new QPushButton("+");
  m_plus->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Fixed);
  m_plus->setFont(bFont);
  connect(m_plus, SIGNAL(pressed()), this, SLOT(slotPlusPressed()));

  m_minus = new QPushButton("-");
  m_minus->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Fixed);
  m_minus->setFont(bFont);
  connect(m_minus, SIGNAL(pressed()), this, SLOT(slotMinusPressed()));

  if( buttonOrder == Vertical )
    {
      lbox->addStretch(10);
    }

  lbox->addWidget(m_plus);
  lbox->addWidget(m_spinBox);
  lbox->addWidget(m_minus);

  if( buttonOrder == Vertical )
    {
      lbox->addStretch(10);
    }

  setLayout( lbox );
}

VarSpinBox::~VarSpinBox()
{
}

void VarSpinBox::showEvent( QShowEvent *event )
{
  Q_UNUSED(event)

  int height = m_spinBox->height();

  if( m_buttonOrder == Vertical )
    {
      // The buttons should have the same size as the spinbox.
      m_plus->setMaximumSize( m_spinBox->size() );
      m_plus->setMaximumSize( m_spinBox->size() );

      m_minus->setMaximumSize( m_spinBox->size() );
      m_minus->setMaximumSize( m_spinBox->size() );
    }
  else
    {
      // Take the current height of the spinbox to make the buttons symmetrically.
      m_plus->setMaximumSize( height, height );
      m_plus->setMinimumSize( height, height );

      m_minus->setMaximumSize( height, height );
      m_minus->setMinimumSize( height, height );
    }
}

void VarSpinBox::slotPlusPressed()
{
  if( m_plus->isDown() )
    {
      m_spinBox->stepUp();

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotPlusPressed()));
    }
}

void VarSpinBox::slotMinusPressed()
{
  if( m_minus->isDown() )
    {
      m_spinBox->stepDown();

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotMinusPressed()));
    }
}
