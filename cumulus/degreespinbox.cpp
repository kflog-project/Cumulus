/***********************************************************************
**
**   degreespinbox.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers,
**                   2008-2009 by Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "degreespinbox.h"

DegreeSpinBox::DegreeSpinBox(QWidget *parent) : QSpinBox(parent)
{
  this->setMinimum(0);
  this->setMaximum(36);
  this->setWrapping(true);
  this->setSingleStep(1);
  this->setValue(0); //default=Unknown
}


DegreeSpinBox::~DegreeSpinBox()
{}


QString DegreeSpinBox::textFromValue(int value) const
{
  if( value == 0 )
    {
      return QString(tr("Unknown"));
    }

  return QString("%1").arg(value, 2, 10, QLatin1Char('0'));
}


int DegreeSpinBox::valueFromText( const QString &text ) const
{
  if( text == QString(tr("Unknown")) )
    {
      return 0;
    }

  return int(text.toInt());
}

