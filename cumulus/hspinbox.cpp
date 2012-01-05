/***********************************************************************
**
**   hspinbox.cpp
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

#include "hspinbox.h"

HSpinBox::HSpinBox( QAbstractSpinBox* spinBox, QWidget* parent ) :
  QWidget( parent ),
  _spinBox( spinBox )
{
  setObjectName("HSpinBox");

  if( spinBox == 0 )
    {
      qWarning() << "HSpinBox::HSpinBox(): no spinbox instance passed!";
      return;
    }

  _spinBox->setButtonSymbols(QSpinBox::NoButtons);

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setSpacing(0);
  hbox->setContentsMargins( 0, 0, 0, 0 );

  // take a bold font for the plus and minus sign
  QFont bFont = font();
  bFont.setBold(true);

  plus  = new QPushButton("+");
  plus->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Fixed);
  plus->setFont(bFont);
  connect(plus, SIGNAL(pressed()), this, SLOT(slotPlusPressed()));

  minus = new QPushButton("-");
  minus->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Fixed);
  minus->setFont(bFont);
  connect(minus, SIGNAL(pressed()), this, SLOT(slotMinusPressed()));

  hbox->addWidget(plus);
  hbox->addWidget(_spinBox);
  hbox->addWidget(minus);
  hbox->addStretch(10);

  setLayout( hbox );
}

HSpinBox::~HSpinBox()
{
}

void HSpinBox::showEvent( QShowEvent *event )
{
  Q_UNUSED(event)

  int height = _spinBox->height();

  // Take the current height of the spinbox to make the buttons symmetrically.
  plus->setMaximumSize( height, height );
  plus->setMinimumSize( height, height );

  minus->setMaximumSize( height, height );
  minus->setMinimumSize( height, height );
}

void HSpinBox::slotPlusPressed()
{
  if( plus->isDown() )
    {
      _spinBox->stepUp();

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotPlusPressed()));
    }
}

void HSpinBox::slotMinusPressed()
{
  if( minus->isDown() )
    {
      _spinBox->stepDown();

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotMinusPressed()));
    }
}
