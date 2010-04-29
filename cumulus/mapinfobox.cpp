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

MapInfoBox::MapInfoBox( QWidget *parent, const QString& borderColor,
                        int fontDotsize, bool minusInPretext ) :
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
  _ptext->setFixedWidth( 27 );
  preLayout->addWidget(_ptext);

  if( minusInPretext )
    {
      _minus = new QLabel( this );
      _minus->setPixmap( GeneralConfig::instance()->loadPixmap( "minus.png" ) );
      _minus->setFixedWidth( 25 );
      preLayout->addWidget( _minus );
      _minus->setVisible(false);
    }

  topLayout->addLayout(preLayout, 0);

  _text = new QLabel(this);
  _text->setIndent(0);
  f.setPixelSize(fontDotsize);
  _text->setFont(f);

  topLayout->addWidget(_text,10);

  _text->setStyleSheet( QString( "border-style: none;"
                                 "border-width: 0px;"
                                 "background-color: white;"
                                 "padding-left: 1px;"
                                 "padding-right: 1px;"
                                 "margin: 0px;"
                                 "font-size: %1px;"
                                 "text-align: left;" ).arg(fontDotsize) );

  setValue("-");
  setPreText("");

  _text->installEventFilter(this);
  _ptext->installEventFilter(this);
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
  p.setColor( QPalette::Foreground, QColor(borderColor) );
  setPalette(p);
  setFrameStyle( QFrame::Box | QFrame::Plain );
  setLineWidth(3);
  setMaximumHeight(60);

  _preText = QString("");
  _value =  QString("");
  _ptext = 0;
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

  if(  _minus && _preMinus && showEvent == false )
    {
      if( _value.startsWith( '-' ) && _value.size() > 1 )
        {
          _value = _value.remove( 0, 1 );
          _minus->setVisible(true);
        }
      else
        {
          _minus->setVisible(false);
        }
    }

  _text->setText(_value);

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
                                     "background-color: white;"
                                     "padding-left: 1px;"
                                     "padding-right: 1px;"
                                     "margin: 0px;"
                                     "font-size: %1px;"
                                     "text-align: left;" ).arg(fontDotsize) );
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
                                     "background-color: white;"
                                     "padding-left: 1px;"
                                     "padding-right: 1px;"
                                     "margin: 0px;"
                                     "font-size: %1px;"
                                     "text-align: left;" ).arg( fontDotsize ) );

      diff = minimumSizeHint().width() - width();

#ifdef DEBUG
      qDebug() << "Loop MSH=" <<  minimumSizeHint().width()
               << "W=" << width()
               << "Diff=" << diff
               << "FoSize=" << fontDotsize;
#endif

    }
}

void MapInfoBox::showEvent(QShowEvent *event)
{
  Q_UNUSED(event)

  if( _ptext != 0 )
    {
      // Update text box only, if it is a text box. Calling setValue shall ensure
      // that the font is adapted to the layout size.
      setValue( _value, true );
    }
}

bool MapInfoBox::event( QEvent* event )
{
  if( event->type() == QEvent::MouseButtonPress )
    {
      emit mousePress();
      return true;
    }

  return QWidget::event( event );
}

bool MapInfoBox::eventFilter(QObject* o, QEvent* e)
{
  if( e->type() == QEvent::MouseButtonPress )
    {
      emit mousePress();
      return true;
    }

  return QWidget::eventFilter( o, e );
}
