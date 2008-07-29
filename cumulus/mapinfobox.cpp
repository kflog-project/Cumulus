/***************************************************************************
                          mapinfobox.cpp  -  description
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002 by Andre Somers
                               2008 by Axel Pauli, Josua Dietze
    email                : axel@kflog.org
    
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
#include <cmath>

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


MapInfoBox::MapInfoBox( QWidget *parent, const QString borderColor, int fontDotsize, bool minusInPretext ) :
  QFrame(parent)
{
  QHBoxLayout * layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(0);
  setStyleSheet( QString(
	"margin: 0px;"
	"alignment: top;"
	"padding: 0px;"
	"border-style: solid;"
	"border-width: 3px;"
	"border-color: %1;"
  ).arg( borderColor ) );
//	"font-family: helvetica;"

  _preMinus = minusInPretext;
  _maxFontDotsize = fontDotsize;

  _ptext = new QLabel(this);
  _ptext->setStyleSheet(
	"margin-right: 0px;"
	"border-style: none;"
	"border-width: 0px;"
	"alignment: right;"
	"text-align: right;"
	"padding: 0px;"
	"font-size: 16px;"
	"min-width: 25px;"
	"max-width: 25px;"
  );
//	"font-family: helvetica;"
  _ptext->setAlignment( Qt::AlignRight );
  _ptext->setIndent(2);

  _text = new QLabel(this);
  _text->setIndent(0);

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
  int fontDotsize = _maxFontDotsize;
  int diff;

  _value = _newVal;

  _text->setStyleSheet( QString(
		"border-style: none;"
		"border-width: 0px;"
		"background-color: white;"
		"padding-left: 1px;"
		"padding-right: 1px;"
		"margin: 0px;"
		"font-size: %1px;"
		"text-align: left;"
  ).arg(fontDotsize) );
//		"font-family: Vera,vera,helvetica;"

//  qDebug("\n       field: %s",_PreText.toLatin1().data());
//  qDebug("        width: %d",width());
//  qDebug("         diff: %d",diff);
//  qDebug("        value: %s",_value.toLatin1().data());
  if ( _preMinus )
    if ( _value.startsWith('-') && _value.size() > 1) {
	  _value = _value.remove( 0, 1 );
      _ptext->setText( _PreText + "<br><img src=../icons/minus.png>" );
    } else
      _ptext->setText( _PreText );
//  qDebug("    dispvalue: %s",_value.toLatin1().data());
    
  _text->setText(_value);

  //@JD: set font size dynamically depending on size hint after
  //     displaying the new value

  diff = minimumSizeHint().width() - width();
  while ( diff > 1 ) {
    diff = diff/10;
    if (diff == 0)
      diff = 1;
    fontDotsize = fontDotsize - diff;
    _text->setStyleSheet( QString(
		"border-style: none;"
		"border-width: 0px;"
		"background-color: white;"
		"padding-left: 1px;"
		"padding-right: 1px;"
		"margin: 0px;"
		"font-size: %1px;"
		"text-align: left;"
    ).arg(fontDotsize) );
//		"font-family: Vera,vera,helvetica;"
//    qDebug("  fontDotSize: %d",fontDotsize);
//    qDebug(" minWidthHint: %d\n",minimumSizeHint().width());
    diff = minimumSizeHint().width() - width();
//    qDebug("*      newdiff: %d",diff);
  }
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
