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

#include "mapinfobox.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <cmath>

#include "resource.h"
#include "generalconfig.h"

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
  QFrame( parent )
{
  basics( borderColor );
  QHBoxLayout* topLayout = (QHBoxLayout*) this->layout();
  _preMinus = minusInPretext;
  _maxFontDotsize = fontDotsize;

  QVBoxLayout* preLayout = new QVBoxLayout;
  preLayout->setMargin(0);
  preLayout->setSpacing(3);
  preLayout->setAlignment(Qt::AlignTop);

  _ptext = new QLabel(this);

  QPalette p = _ptext->palette();
  p.setColor( QPalette::Foreground, Qt::black );
  _ptext->setPalette(p);

  QFont f;
  f.setPixelSize(16);
  _ptext->setFont(f);

  _ptext->setAlignment( Qt::AlignRight );
  _ptext->setFixedWidth(25);
  preLayout->addWidget(_ptext);

  if ( minusInPretext ) {
    _minus = new QLabel(this);
    _minus->setPixmap( GeneralConfig::instance()->loadPixmap("minus.png") );
    _minus->setFixedWidth(25);
    preLayout->addWidget( _minus );
    _minus->hide();
  }

  topLayout->addLayout(preLayout, 0);

  _text = new QLabel(this);
  _text->setIndent(0);

  topLayout->addWidget(_text,10);

  setValue("-");
  setPreText("");

  _text->installEventFilter(this);
  _ptext->installEventFilter(this);
}


MapInfoBox::MapInfoBox( QWidget *parent, const QString borderColor, const QPixmap& pixmap ) :
  QFrame( parent )
{
  basics( borderColor );
  QHBoxLayout* topLayout = (QHBoxLayout*) this->layout();
  _text = new QLabel(this);
  _text->setPixmap(pixmap);
  topLayout->addWidget( _text );
}


void MapInfoBox::basics( const QString borderColor )
{
  QHBoxLayout* topLayout = new QHBoxLayout(this);
  topLayout->setMargin(0);
  topLayout->setSpacing(0);

  QPalette p = palette();
  p.setColor( QPalette::Foreground, QColor(borderColor) );
  setPalette(p);
  setFrameStyle( QFrame::Box | QFrame::Plain );
  setLineWidth(3);
  setMaximumHeight(60);

  _PreText = QString("");
  _value =  QString("");
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
void MapInfoBox::setPreText( const QString newVal)
{
  // Are we text or pixmap? For pixmap, ptext is unused
  if ( _ptext == 0 )
    return;  
  _PreText = newVal;
  _ptext->setText(_PreText);
}

/** Set new pixmap as the main content */
void MapInfoBox::setPixmap( const QPixmap& newPixmap )
{
  // Are we text or pixmap? For pixmap, ptext is unused
  if ( _ptext != 0 )
    return;  
  _text->setPixmap(newPixmap);
}


/** Read property of QString _value. */
const QString& MapInfoBox::getValue()
{
  return _value;
}


/** Write property of QString _value. */
void MapInfoBox::setValue( const QString newVal)
{
  int fontDotsize = _maxFontDotsize;
  int diff;

  _value = newVal;

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
  if ( _preMinus )
    if ( _value.startsWith('-') && _value.size() > 1) {
	  _value = _value.remove( 0, 1 );
      _minus->show();
    } else
      _minus->hide();
    
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
    diff = minimumSizeHint().width() - width();
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
