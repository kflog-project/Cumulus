/***************************************************************************
                          mapinfobox.cpp  -  description
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002 by Andre Somers, 2008 Axel Pauli
    email                : andre@kflog.org
    
    $Id$
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QBoxLayout>

#include "mapinfobox.h"
#include "resource.h"

CuLabel::CuLabel ( QWidget * parent, Qt::WFlags f ):
  QLabel(parent, f)
{}

CuLabel::CuLabel ( const QString & text, QWidget * parent, Qt::WFlags f ):
  QLabel(text, parent, f)
{}

void CuLabel::mousePressEvent ( QMouseEvent * /*e*/ )
{
  emit mousePress();
}


MapInfoBox::MapInfoBox(QWidget * parent, int FontDotsize ):QWidget(parent,"")
{
  QFont tfnt( "Helvetica", FontDotsize, QFont::Bold  );
  QFont pfnt( "Helvetica", 4, QFont::Normal  );
  QBoxLayout * layout = new QHBoxLayout(this,0);

  _ptext = new QLabel(this, "" );
  _ptext->setFont(pfnt);
  _ptext->setFixedHeight( 10 );
  _ptext->setIndent(0);

  _text = new QLabel(this, "" );
  _text->setFont(tfnt);

  // avoid clipping of 'g,q,j' for small fonts
  /*if( FontDotsize <= 10 )
    {
    this->setFixedHeight(tfnt.pointSize()+6); 
    _text->setFixedHeight( tfnt.pointSize()+6 );
    }
    else
    {
    this->setFixedHeight(tfnt.pointSize());
    _text->setFixedHeight( tfnt.pointSize());
    }       */
  _text->setFixedHeight(MAX(16, FontDotsize));
  setFixedHeight(MAX(16, FontDotsize));
  layout->addWidget(_ptext,0,Qt::AlignTop);
  layout->addWidget(_text,-4,Qt::AlignBottom);
  layout->addStretch(10);
  setValue("-");
  setPreText("");

  _text->installEventFilter(this);
  _ptext->installEventFilter(this);
}


MapInfoBox::~MapInfoBox()
{}


/** Read property of QString _PreText. */
const QString& MapInfoBox::getPreText()
{
  return _PreText;
}


/** Write property of QString _PreText. */
void MapInfoBox::setPreText( const QString _newVal)
{
  _PreText = _newVal;
  _ptext->setText(_PreText);
}


/** Read property of QString _value. */
const QString& MapInfoBox::getValue()
{
  return _value;
}


/** Write property of QString _value. */
void MapInfoBox::setValue( const QString _newVal)
{
  _value = _newVal;
  _text->setText(_value);
}


bool MapInfoBox::event(QEvent * e)
{
  if (e->type() == QEvent::MouseButtonPress) {
    emit mousePress() ;
    return true;
  }

  return QWidget::event( e );
}


bool MapInfoBox::eventFilter(QObject * o, QEvent * e)
{
  if (e->type() == QEvent::MouseButtonPress) {
    emit mousePress();
    return true;
  }

  return QWidget::eventFilter( o, e );
}
