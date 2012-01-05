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

HSpinBox::HSpinBox( QWidget* parent ) : QWidget( parent )
{
  setObjectName("HSpinBox");

  _spinBox = new QSpinBox;
  _spinBox->setButtonSymbols(QSpinBox::NoButtons);

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setSpacing(0);

  // take a bold font for the plus and minus sign
  QFont bFont = font();
  bFont.setBold(true);

  plus  = new QPushButton("+");
  plus->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Fixed);
  plus->setFont(bFont);
  connect(plus, SIGNAL(clicked()), this, SLOT(slotPlus()));

  minus = new QPushButton("-");
  minus->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Fixed);
  minus->setFont(bFont);
  connect(minus, SIGNAL(clicked()), this, SLOT(slotMinus()));

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

void HSpinBox::slotPlus()
{
  _spinBox->setValue( _spinBox->value() + _spinBox->singleStep() );
}

void HSpinBox::slotMinus()
{
  _spinBox->setValue( _spinBox->value() - _spinBox->singleStep() );
}
