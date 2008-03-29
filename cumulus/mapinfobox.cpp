/***************************************************************************
                          mapinfobox.cpp  -  description
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002 by Andre Somers, 2008 Axel Pauli, Josua Dietze
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

#include <QHBoxLayout>

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


MapInfoBox::MapInfoBox( QWidget *parent, const QString borderColor, int fontDotsize ) :
  QFrame(parent)
{
  QHBoxLayout * layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  setStyleSheet( QString(
	"margin: 0px;"
	"alignment: top;"
	"padding: 0px;"
	"font-family: helvetica;"
	"border-style: solid;"
	"border-width: 3px;"
	"border-color: %1;"
  ).arg( borderColor ) );

  _ptext = new QLabel(this, "" );
  _ptext->setStyleSheet(
	"margin-right: 0px;"
	"border-style: none;"
	"border-width: 0px;"
	"alignment: right;"
	"text-align: right;"
	"padding: 0px;"
	"font-family: helvetica;"
	"font-size: 16px;"
	"min-width: 27px;"
	"max-width: 27px;"
  );
  _ptext->setAlignment( Qt::AlignRight );
  _ptext->setIndent(0);

  _text = new QLabel(this, "" );
  _text->setIndent(0);
  _text->setStyleSheet( QString(
	"border-style: none;"
	"border-width: 0px;"
	"background-color: white;"
	"padding-left: 1px;"
	"margin: 0px;"
	"font-family: Vera,vera,helvetica;"
	"font-size: %1px;"
	"text-align: left;"
  ).arg(fontDotsize) );

  layout->addWidget(_ptext,0);
  layout->addWidget(_text,10);

  setValue("-");
  setPreText("");

  _text->installEventFilter(this);
  _ptext->installEventFilter(this);
}


MapInfoBox::~MapInfoBox()
{
}


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
