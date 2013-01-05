/***********************************************************************
**
**   doubleNumberEditor.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: numberEditor.h 5648 2013-01-03 21:35:48Z axel $
**
***********************************************************************/

#include <QtGui>

#include "doubleNumberEditor.h"

DoubleNumberEditor::DoubleNumberEditor( QWidget *parent ) :
  NumberEditor( parent ),
  m_precision(2)
{
  setObjectName("DoubleNumberEditor");
  setDecimalVisible( true );
}

DoubleNumberEditor::~DoubleNumberEditor()
{
}

void DoubleNumberEditor::slot_NumberEdited( const QString& number )
{
  m_nip = 0;

  // Sets the edited value in the right format in the display label
  setValue( number.toDouble() );

  // Tell all that the displayed number was edited.
  emit numberEdited( m_number );
}
