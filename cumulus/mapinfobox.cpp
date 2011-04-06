/***************************************************************************
                          mapinfobox.cpp  -  description
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002      by Andre Somers
                               2008      by Josua Dietze
                               2008-2010 by Axel Pauli

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

#include <QtGui>
#include <cmath>

#include "generalconfig.h"

CuLabel::CuLabel( QWidget * parent, Qt::WFlags flags ) :
  QLabel( parent, flags )
{}

CuLabel::CuLabel( const QString& text, QWidget* parent, Qt::WFlags flags ) :
  QLabel( text, parent, flags )
{}

void CuLabel::mousePressEvent ( QMouseEvent* event )
{
  Q_UNUSED(event)

  emit mousePress();
}

//------------------------------------------------------------------------------

MapInfoBox::MapInfoBox( QWidget *parent,
                        const QString& borderColor,
                        bool unitInPretext,
                        bool minusInPretext,
                        int fontDotsize ) :
  QFrame( parent ),
  _textBGColor( "white" )
{
  basics( borderColor );
  QHBoxLayout* topLayout = (QHBoxLayout*) this->layout();

  _maxFontDotsize = fontDotsize;

  QFont f;
  f.setPixelSize(16);

  _preWidget = new QWidget( this );

  QVBoxLayout* preLayout = new QVBoxLayout( _preWidget );
  preLayout->setMargin(0);
  preLayout->setSpacing(3);

  _ptext = new QLabel(this);

  QPalette p = _ptext->palette();
  p.setColor( QPalette::WindowText, Qt::black );
  _ptext->setPalette(p);
  _ptext->setFont(f);
  _ptext->setAlignment( Qt::AlignRight );
  _ptext->setFixedWidth( 27 );
  preLayout->addWidget(_ptext, 0, Qt::AlignRight|Qt::AlignTop);

  if( minusInPretext )
    {
      _pminus = new QLabel( this );
      _pminus->setPixmap( GeneralConfig::instance()->loadPixmap( "minus.png" ) );
      _pminus->setFixedWidth( 25 );
      preLayout->addWidget( _pminus );
      preLayout->addStretch( 5 );
      _pminus->setVisible(false);
    }

  if( unitInPretext )
    {
      // A unit shall be displayed in the pre-text box.
      _punit = new QLabel(this);

      p = _punit->palette();
      p.setColor( QPalette::WindowText, Qt::black );

      _punit->setPalette(p);
      _punit->setFont(f);
      _punit->setAlignment( Qt::AlignRight );
      _punit->setFixedWidth( 27 );

      preLayout->addStretch( 5 );
      preLayout->addWidget(_punit, 0, Qt::AlignRight|Qt::AlignBottom);
    }

  topLayout->addWidget( _preWidget, 0 );

  _text = new QLabel(this);
  _text->setIndent(0);
  f.setPixelSize(fontDotsize);
  _text->setFont(f);

  topLayout->addWidget(_text,10);

  _text->setStyleSheet( QString( "border-style: none;"
                                 "border-width: 0px;"
                                 "background-color: %1;"
                                 "padding-left: 1px;"
                                 "padding-right: 1px;"
                                 "margin: 0px;"
                                 "font-size: %2px;"
                                 "text-align: left;" )
                                 .arg(_textBGColor)
                                 .arg(fontDotsize) );

  setValue("-");
  setPreText("");
  setPreUnit("");
}

MapInfoBox::MapInfoBox( QWidget *parent, const QString& borderColor, const QPixmap& pixmap ) :
  QFrame( parent )
{
  basics( borderColor );
  QHBoxLayout* topLayout = (QHBoxLayout*) this->layout();
  _text = new QLabel(this);
  _text->setPixmap(pixmap);
  topLayout->addWidget( _text );
}

void MapInfoBox::basics( const QString& borderColor )
{
  QHBoxLayout* topLayout = new QHBoxLayout(this);
  topLayout->setMargin(0);
  topLayout->setSpacing(0);

  QPalette p = palette();
  p.setColor( QPalette::WindowText, QColor(borderColor) );
  setPalette(p);
  setFrameStyle( QFrame::Box | QFrame::Plain );
  setLineWidth(3);
  setMaximumHeight(60);

  _preText = QString("");
  _preUnit = QString("");
  _value =  QString("");
  _ptext = 0; // can be unused
  _pminus = 0; // can be unused
  _punit = 0; // can be unused
  _preWidget = 0;  // can be unused
}

/** Write property of QString _preText. */
void MapInfoBox::setPreText( const QString& newVal )
{
  // Are we text or pixmap? For pixmap, ptext is unused
  if ( _ptext == 0 )
    {
      return;
    }

  _preText = newVal;
  _ptext->setText(_preText);
}

/** Write property of QString _preUnit. */
void MapInfoBox::setPreUnit( const QString& newVal )
{
  // pre-unit can be unused
  if ( _punit == 0 )
    {
      return;
    }

  _preUnit = newVal;
  _punit->setText(_preUnit);
}


/** Set new pixmap as the main content */
void MapInfoBox::setPixmap( const QPixmap& newPixmap )
{
  // Are we text or pixmap? For pixmap, ptext is unused
  if ( _ptext != 0 )
    {
      return;
    }

  _text->setPixmap(newPixmap);
}

/** Write property of QString _value. */
void MapInfoBox::setValue( const QString& newVal, bool showEvent )
{
  int fontDotsize   = _maxFontDotsize;
  int lastValueSize = _value.size();

  _value = newVal;

  if( _pminus && showEvent == false )
    {
      if( _value.startsWith( '-' ) && _value.size() > 1 )
        {
          _value = _value.remove( 0, 1 );
          _pminus->setVisible(true);
        }
      else
        {
          _pminus->setVisible(false);
        }
    }

  _text->setText( _value );

  if( _value.isEmpty() )
    {
      // We ignore empty values in further processing
      return;
    }

  if( ! isVisible() )
    {
      // We make no style actions if the widget is hidden.
      return;
    }

#ifdef DEBUG
  qDebug() << _preText << _value
           << "MSH=" <<  minimumSizeHint().width() << "W=" << width()
           << "Visible" << isVisible();
#endif

  if( minimumSizeHint().width() > width() || lastValueSize != _value.size() )
    {
      // Do only setup a new style, if it is really necessary.
      _text->setStyleSheet( QString( "border-style: none;"
                                     "border-width: 0px;"
                                     "background-color: %1;"
                                     "padding-left: 1px;"
                                     "padding-right: 1px;"
                                     "margin: 0px;"
                                     "font-size: %2px;"
                                     "text-align: left;" )
                                     .arg(_textBGColor)
                                     .arg(fontDotsize) );
    }

  //@JD: set font size dynamically depending on size hint after
  //     displaying the new value
  int diff = minimumSizeHint().width() - width();

  /** @AP: Check font size too, to avoid running under zero. Had this
   * behavior during my tests. Can cause a very long loop time.
   */
  while( diff > 0 && fontDotsize > 10 )
    {
      diff /= 5;

      if( diff < 1 )
        {
          diff = 1;
        }
      else if( diff > 5 )
        {
          diff = 5;
        }

      fontDotsize -= diff;

      _text->setStyleSheet( QString( "border-style: none;"
                                     "border-width: 0px;"
                                     "background-color: %1;"
                                     "padding-left: 1px;"
                                     "padding-right: 1px;"
                                     "margin: 0px;"
                                     "font-size: %2px;"
                                     "text-align: left;" )
                                     .arg(_textBGColor)
                                     .arg( fontDotsize ) );

      diff = minimumSizeHint().width() - width();

#ifdef DEBUG
      qDebug() << "Loop MSH=" <<  minimumSizeHint().width()
               << "W=" << width()
               << "Diff=" << diff
               << "FoSize=" << fontDotsize;
#endif

    }
}

void MapInfoBox::setTextLabelBGColor( const QString& newValue )
{
  _textBGColor = newValue;

  _text->setStyleSheet( QString( "border-style: none;"
                                 "border-width: 0px;"
                                 "background-color: %1;"
                                 "padding-left: 1px;"
                                 "padding-right: 1px;"
                                 "margin: 0px;"
                                 "font-size: %2px;"
                                 "text-align: left;" )
                                 .arg(_textBGColor)
                                 .arg( _maxFontDotsize ) );
  setValue( _value );
};

void MapInfoBox::setPreWidgetsBGColor( const QColor& newValue )
{
  if( _preWidget )
    {
      _preWidget->setAutoFillBackground( true );
      _preWidget->setBackgroundRole( QPalette::Window );
      _preWidget->setPalette( QPalette( newValue ) );
    }
}

void MapInfoBox::showEvent(QShowEvent *event)
{
  if( _ptext != 0 )
    {
      // Update text box only, if it is a text box. Calling setValue shall ensure
      // that the font is adapted to the layout size.
      setValue( _value, true );
    }

  QFrame::showEvent( event );
}

void MapInfoBox::mousePressEvent( QMouseEvent* event )
{
  Q_UNUSED( event )

  emit mousePress();
}
