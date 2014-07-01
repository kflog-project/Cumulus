/***********************************************************************
**
**   colordialog.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2014 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "colordialog.h"

QColor ColorDialog::getColor( const QColor& initial,
                              QWidget* parent,
                              const QString& title )
{
  QColorDialog cd( initial, parent );
  cd.setWindowTitle( title );

  // Show color dialog
  cd.show();

  // That is the trick to prevent dialog closing, if the return key is pressed.
  // But note, it has only effect, if the dialog is shown!
  QList<QPushButton *> buttonList = cd.findChildren<QPushButton *>();

  foreach( QPushButton *pb, buttonList )
    {
      pb->setDefault( false );
      pb->setAutoDefault( false );
    }

  // Adapt the size of the dialog to the parent window, if necessary.
  if( parent != 0 &&
     (cd.minimumSizeHint().height() > (parent->height() - 15) ||
      cd.minimumSizeHint().width() > (parent->width() -15)) )
    {
      QFont cdFont = cd.font();
      int fs = cd.font().pointSize();

      while( (cd.minimumSizeHint().height() > (parent->size().height() - 15) ||
	      cd.minimumSizeHint().width() > (parent->width() -15)) && fs > 7 )
	{
	  fs--;
	  cdFont.setPointSize( fs );
	  cd.setFont( cdFont );
	  cd.adjustSize();
	}
    }

#ifdef ANDROID

  if( parent )
    {
      QSize ps = parent->size();
      QSize cs = cd.size();

      // Move window into center of parent.
      QPoint pos = parent->mapToGlobal( QPoint( (ps.width() - cs.width()) / 2,
                                                (ps.height() - cs.height()) / 2 ) );
      cd.move( pos );
    }

#endif

  if( cd.exec() == QDialog::Accepted )
    {
      return cd.selectedColor();
    }

  return QColor();
}
