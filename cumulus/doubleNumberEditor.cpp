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
**   $Id$
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
  // Sets the edited value in the right format in the display label
  QString dText = QString("%1").arg( number.toDouble(), 0, 'f', m_precision );

  // Call base class method for further handling
  NumberEditor::slot_NumberEdited( dText );
}
