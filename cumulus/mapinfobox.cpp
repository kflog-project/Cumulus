/***************************************************************************
                          mapinfobox.cpp  -  description
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002      by Andre Somers
                               2008      by Josua Dietze
                               2008-2013 by Axel Pauli

    email                : kflog.cumulus@gmail.com

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

#include <cmath>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "generalconfig.h"
#include "layout.h"
#include "mapinfobox.h"

CuLabel::CuLabel( QWidget * parent, Qt::WindowFlags flags ) :
  QLabel( parent, flags )
{}

CuLabel::CuLabel( const QString& text, QWidget* parent, Qt::WindowFlags flags ) :
  QLabel( text, parent, flags )
{}

void CuLabel::mousePressEvent ( QMouseEvent* event )
{
  Q_UNUSED(event)

  emit mousePress();
}

//------------------------------------------------------------------------------

// The maximum default height of the infobox in pixels
#define MAXIMUM_INFO_BOX_HEIGHT 60

MapInfoBox::MapInfoBox( QWidget *parent,
                        const QString& borderColor,
                        bool unitInPretext,
                        bool minusInPretext,
                        int fontDotsize) :
  QFrame( parent ),
  m_textBGColor( "white" ),
  m_maxFontDotsize( fontDotsize ),
  m_maxTextLabelFontHeight( -1 ),
  m_minUpdateInterval( 0 ),
  m_lastUpdateTime(0, 0, 0)
{
  initMousePressTimer();

  // Maximum pretext width in pixels. That value is a hard coded limit now!
  const int ptw = 35;

  // start font size
  int start = 14;

  QFont f = font();

  if( f.pointSize() != -1 )
    {
      m_fontUnit = "pt";
      f.setPointSize( start );
    }
  else
    {
      m_fontUnit = "px";
      f.setPixelSize( start );
    }

  // Adapt the font to the predefined limit width. The minimum font size is 6.
  while( start >= 6 )
    {
      QFontMetrics qfm(f);

      if( (qfm.boundingRect("Brg").width() + 5) >= ptw )
        {
          start--;

          if( f.pointSize() != -1 )
            {
              f.setPointSize( start );
            }
          else
            {
              f.setPixelSize( start );
            }
        }
      else
        {
          break;
        }
    }

  // Set the calculated font.
  setFont(f);

  basics( borderColor );
  QHBoxLayout* topLayout = (QHBoxLayout*) this->layout();

  m_preWidget = new QWidget( this );

  QVBoxLayout* preLayout = new QVBoxLayout( m_preWidget );
  preLayout->setContentsMargins(3,0,3,0);
  preLayout->setSpacing(3);

  m_ptext = new QLabel( this );
  m_ptext->setFont(f);
  m_ptext->setMargin(0);
  m_ptext->setIndent(0);
  m_ptext->setTextFormat(Qt::PlainText);
  m_ptext->setScaledContents(true);

  QPalette p = m_ptext->palette();
  p.setColor( QPalette::WindowText, Qt::black );
  m_ptext->setPalette(p);
  m_ptext->setFixedWidth( QFontMetrics(f).boundingRect("MM").width() );

  preLayout->addWidget(m_ptext );

  if( minusInPretext )
    {
      m_pminus = new QLabel( this );
      m_pminus->setFont(f);
      m_pminus->setMargin(0);
      m_pminus->setPixmap( GeneralConfig::instance()->loadPixmap( "minus.png" ) );
      m_pminus->setFixedWidth( 25 );
      m_pminus->setScaledContents(true);
      preLayout->addStretch( 1 );
      preLayout->addWidget( m_pminus );
     m_pminus->setVisible(false);
    }

  preLayout->addStretch( 10 );

  if( unitInPretext )
    {
      // A unit shall be displayed in the pre-text box.
      m_punit = new QLabel( this );
      m_punit->setFont(f);
      m_punit->setMargin(0);
      m_punit->setIndent(0);
      m_punit->setTextFormat(Qt::PlainText);
      m_punit->setScaledContents(true);

      p = m_punit->palette();
      p.setColor( QPalette::WindowText, Qt::black );

      m_punit->setPalette(p);

      //m_punit->setFixedWidth( fm.boundingRect("MM").width() );
      preLayout->addWidget(m_punit, 0, Qt::AlignRight);
    }

  topLayout->addWidget( m_preWidget );

  m_text = new QLabel( this );

  topLayout->addWidget(m_text, 10);

  m_text->setStyleSheet( QString( "border-style: none;"
                                  "border-width: 0px;"
                                  "background-color: %1;"
                                  "padding-left: 1px;"
                                  "padding-right: 1px;"
                                  "margin: 0px;"
                                  "text-align: left;" )
                                  .arg(m_textBGColor) );

  m_text->setIndent( 5 );

  setValue("-");
  setPreText("");
  setPreUnit("");
}

MapInfoBox::MapInfoBox( QWidget *parent, const QString& borderColor, const QPixmap& pixmap ) :
  QFrame( parent )
{
  initMousePressTimer();
  basics( borderColor );
  QHBoxLayout* topLayout = (QHBoxLayout*) this->layout();
  m_text = new QLabel(this);
  m_text->setPixmap(pixmap);
  m_text->setScaledContents(true);
  topLayout->addWidget( m_text );
}

void MapInfoBox::initMousePressTimer()
{
  m_mousePressTimer = new QTimer(this);
  m_mousePressTimer->setSingleShot(true);
  m_mousePressTimer->setInterval(250);
  connect( m_mousePressTimer, SIGNAL(timeout()), SIGNAL(mouseLongPress()));
}

void MapInfoBox::basics( const QString& borderColor )
{
  QHBoxLayout* topLayout = new QHBoxLayout(this);
  topLayout->setContentsMargins(0, 0, 0, 0);
  topLayout->setSpacing(0);

  QPalette p = palette();
  p.setColor( QPalette::WindowText, QColor(borderColor) );
  setPalette(p);
  setFrameStyle( QFrame::Box | QFrame::Plain );
  setLineWidth(3);
  setMaximumHeight( MAXIMUM_INFO_BOX_HEIGHT );

  m_preText = QString("");
  m_preUnit = QString("");
  m_value =  QString("");
  m_ptext = 0; // can be unused
  m_pminus = 0; // can be unused
  m_punit = 0; // can be unused
  m_preWidget = 0;  // can be unused
}

/** Write property of QString m_preText. */
void MapInfoBox::setPreText( const QString& newVal )
{
  // Are we text or pixmap? For pixmap, ptext is unused
  if ( m_ptext == 0 )
    {
      return;
    }

  m_preText = newVal;
  m_ptext->setText(m_preText);
}

/** Write property of QString m_preUnit. */
void MapInfoBox::setPreUnit( const QString& newVal )
{
  // pre-unit can be unused
  if ( m_punit == 0 )
    {
      return;
    }

  m_preUnit = newVal;
  m_punit->setText(m_preUnit);
}


/** Set new pixmap as the main content */
void MapInfoBox::setPixmap( const QPixmap& newPixmap )
{
  // Are we text or pixmap? For pixmap, ptext is unused
  if ( m_ptext != 0 )
    {
      return;
    }

  m_text->setPixmap(newPixmap);
}

/** Write property of QString m_value. */
void MapInfoBox::setValue( const QString& newVal, bool showEvent )
{
  int lastValueSize = m_value.size();

  m_value = newVal;

  if( m_minUpdateInterval > 0 && showEvent == false )
    {
      // A time update interval is set.
      if( m_lastUpdateTime.elapsed() < m_minUpdateInterval )
        {
          // We return, if the next update time is not yet reached.
          return;
        }

      m_lastUpdateTime.start();
    }

  // Show new value in box.
  // Note a minus sign is also used, if no value is available!
  if( m_value.startsWith( QChar('-') ) && m_value.size() > 1 )
    {
      m_text->setText( m_value.mid(1) );

      if( m_pminus )
        {
          m_pminus->setVisible( true );
        }
    }
  else
    {
      m_text->setText( m_value );

      if( m_pminus )
        {
          m_pminus->setVisible( false );
        }
    }

  if( m_value.isEmpty() )
    {
      // We return on empty text.
      return;
    }

  if( m_maxTextLabelFontHeight == -1 )
    {
      // Determine the maximum font height for the text label, if not set.
      determineMaxFontHeight();

      // force update of text label box adaption
      showEvent = true;
    }

  // Check, if the label text fits in the label widget.
  QFontMetrics qfm( m_text->font() );

  if( (qfm.boundingRect( m_text->text() ).width() ) < m_text->width() &&
      lastValueSize == newVal.size() &&
      showEvent == false )
    {
      // The displayed value fits the label widget.
      // The text size has not changed.
      // No show event has occurred
      return;
    }

  adaptText2LabelBox();
}

void MapInfoBox::setTextLabelBGColor( const QString& newValue )
{
  m_textBGColor = newValue;

  m_text->setStyleSheet( QString( "border-style: none;"
                                 "border-width: 0px;"
                                 "background-color: %1;"
                                 "padding-left: 1px;"
                                 "padding-right: 1px;"
                                 "margin: 0px;"
                                 "text-align: left;" )
                                 .arg(m_textBGColor) );
  setValue( m_value );
};

void MapInfoBox::setPreWidgetsBGColor( const QColor& newValue )
{
  if( m_preWidget )
    {
      m_preWidget->setAutoFillBackground( true );
      m_preWidget->setBackgroundRole( QPalette::Window );
      m_preWidget->setPalette( QPalette( newValue ) );
    }
}

void MapInfoBox::showEvent(QShowEvent *event)
{
  QFrame::showEvent( event );

  // reset this variable to force a recalculation
  m_maxTextLabelFontHeight = -1;

  if( m_text->pixmap() == 0 )
    {
      // Update text box only, if it is a text box. Calling setValue shall ensure
      // that the font is adapted to the layout size.
      setValue( m_value, true );
    }
}

void MapInfoBox::mousePressEvent( QMouseEvent* event )
{
  m_mousePressTimer->start();
  event->accept();
}

void MapInfoBox::mouseReleaseEvent( QMouseEvent* event )
{
  if( m_mousePressTimer->isActive() == true )
    {
      m_mousePressTimer->stop();
      emit mouseShortPress();
    }

  event->accept();
}

void MapInfoBox::resizeEvent( QResizeEvent* event )
{
  // Call base class
  QFrame::resizeEvent( event );

  // reset this variable to force a recalculation
  m_maxTextLabelFontHeight = -1;

  if( m_text->pixmap() == 0 )
    {
      // We are a text label.
      if( isVisible() )
        {
          setValue( m_value, true );
        }
    }
}

void MapInfoBox::determineMaxFontHeight()
{
  if( m_text->pixmap() != 0 )
    {
      // MapInfoBox is not a text display box.
      return;
    }

  QFont fontRef = m_text->font();

  Layout::adaptFont( fontRef,
                     m_text->height() + 2 * m_text->margin(),
                     m_maxFontDotsize,
                     7 );

  // Set calculated font point size
  if( fontRef.pointSize() != -1 )
    {
      m_maxTextLabelFontHeight = fontRef.pointSize();
    }
  else
    {
      m_maxTextLabelFontHeight = fontRef.pixelSize();
    }
}

void MapInfoBox::adaptText2LabelBox()
{
  if( m_text->pixmap() != 0 || m_text->text().isEmpty() )
    {
      // MapInfoBox is not a text display box or text is empty.
      return;
    }

  QFont fontRef = m_text->font();

  // Check, if the font height maximum is set. If not calculate it.
  if( m_maxTextLabelFontHeight == -1 )
    {
      determineMaxFontHeight();
    }

  if( fontRef.pointSize() != -1 )
    {
      fontRef.setPointSize( m_maxTextLabelFontHeight );
    }
  else
    {
      fontRef.setPixelSize( m_maxTextLabelFontHeight );
    }

  int value = m_maxTextLabelFontHeight;

  // We have to consider the set style padding and the frame line width
  int margin = m_text->margin() + 2 + m_text->lineWidth() * 2;

  // We have to consider twice the set indent.
  margin += ( m_text->indent() != -1 ) ? m_text->indent() * 2 : 0;

  // Adapt the font point size to the given text box width.
  while( value > 7 )
    {
      QFontMetrics qfm(fontRef);

      if( (qfm.boundingRect( m_text->text() ).width() + margin) > m_text->width() )
        {
          value--;

          // Determine, what kind of font is used
          if( fontRef.pointSize() != -1 )
            {
              fontRef.setPointSize( value );
            }
          else
            {
              fontRef.setPixelSize( value );
            }
        }
      else
        {
          break;
        }
    }

  // Set the new font size for the text label.
  m_text->setFont( fontRef );
}
