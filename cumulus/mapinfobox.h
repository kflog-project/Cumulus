/***************************************************************************
                          mapinfobox.h  -  description
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002      by Andre Somers
                               2008      by Josua Dietze
                               2008-2014 by Axel Pauli

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

#ifndef MAP_INFO_BOX_H
#define MAP_INFO_BOX_H

#include <QEvent>
#include <QFont>
#include <QLabel>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QWidget>

/**
 * \class CuLabel
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Slight modification of a QLabel.
 *
 * This is a slight modification of a QLabel. It adds a mousePress event.
 *
 * \date 2002-2014
 *
 */
class CuLabel : public QLabel
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( CuLabel )

public:

  CuLabel ( QWidget *parent, Qt::WindowFlags flags=0 );

  CuLabel ( const QString &text, QWidget *parent, Qt::WindowFlags flags=0 );

signals:

   /** Emitted when the mouse is pressed over the label */
  void mousePress();

protected:

  void mousePressEvent( QMouseEvent *event );
};

/**
 * \class MapInfoBox
 *
 * \author Andre Somers, Josua Dietze, Axel Pauli
 *
 * \brief Specialized widget for text or icon display.
 *
 * \see MapView
 *
 * This is a specialized widget that can display either a label with a
 * pre-text or a pixmap. Optionally it can be clicked and connected to
 * a slot. Used on the \ref MapView.
 *
 * \date 2002-2014
 *
 * \version $Id$
 *
 */

class MapInfoBox : public QFrame
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( MapInfoBox )

public:

  MapInfoBox( QWidget* parent,
              const QString& borderColor,
              bool unitInPretext=false,
              bool minusInPretext=false,
              int fontDotsize=36 );

  MapInfoBox( QWidget* parent, const QString& borderColor, const QPixmap& pixmap );

  virtual ~MapInfoBox() {};

  /**
   * Write property of QString m_preText.
   */
  void setPreText( const QString& _newVal);

  /**
   * Show resp. hide the pretext label widgets.
   */
  void setPreTextVisible( bool visible )
  {
    m_ptext->setVisible( visible );

    if( m_punit ) m_punit->setVisible( visible );
    if( m_pminus ) m_pminus->setVisible( visible );
  };

  /**
   * Show resp. hide the whole pretext widget.
   */
  void setPreTextWidgetVisible( bool visible )
  {
    m_preWidget->setVisible( visible );
  };

  /**
   * Write property of QString m_preUnit.
   */
  void setPreUnit( const QString& _newVal);

  /**
   * Read property of QString m_preText.
   */
  const QString& getPreText()
  {
    return m_preText;
  };

  /**
   * Write property of QString m_value.
   */
  void setValue( const QString& _newVal, bool showEvent=false );

  /**
   * Sets the text label background color.
   */
  void setTextLabelBGColor( const QString& newValue );

  /**
   * Sets the background color of all pre-widgets.
   */
  void setPreWidgetsBGColor( const QColor& newValue );

  /**
   * Read property of QString m_value.
   */
  const QString& getValue() const
  {
    return m_value;
  };

  /**
   * Returns the pretext label widget.
   */
  QLabel* getPreTextLabelWidget() const
  {
    return m_ptext;
  };

  QWidget* getPreWidget() const
  {
    return m_preWidget;
  };

  /**
   * Returns the text label widget.
   */
  QLabel* getTextLabelWidget() const
  {
    return m_text;
  };

  void setPixmap( const QPixmap& newPixmap );

  /** Gets the maximum height of the text label box. */
  int getMapInfoBoxMaxlHeight() const
  {
    return maximumHeight();
  }

  /** Sets the maximum height of the text label box. */
  void setMapInfoBoxMaxHeight( int value )
  {
    setMaximumHeight( value );
  }

  /** Sets the minimum update time interval of the text label box. */
  void setUpdateInterval( int value )
  {
    m_minUpdateInterval = value;
    m_lastUpdateTime.setHMS( 0, 0, 0);
  };

  /** Gets the minimum update time interval of the text label box. */
  int getUpdateInterval()
  {
    return m_minUpdateInterval;
  };

 signals:

  /**
   * The mouse is short time pressed over the widget.
   */
  void mouseShortPress();

  /**
   * TThe mouse is long time pressed over the widget.
   */
  void mouseLongPress();

 protected:

  /**
   * Reimplemented from QWidget
   */
  void mousePressEvent( QMouseEvent* event );

  /**
   * Reimplemented from QWidget
   */
  void mouseReleaseEvent( QMouseEvent * event );

  /**
   * Reimplemented from QWidget
   */
  void showEvent(QShowEvent* event);

  /**
   * Reimplemented from QWidget
   */
  void resizeEvent( QResizeEvent* event );

private:

  /**
   * Initializes the mouse press timer.
   */
  void initMousePressTimer();

  /**
   * Determines the maximum usable font height in the text label box.
   */
  void determineMaxFontHeight();

  /**
   * Adapt text to text label box.
   */
  void adaptText2LabelBox();

  /**
   * Initializes the basics of this widget.
   *
   * @param borderColor Color to be used as border.
   */
  void basics( const QString& borderColor );

  /** The upper text displayed before the value. */
  QString m_preText;
  /** The lower text displayed before the value. */
  QString m_preUnit;
  /** The value of the box */
  QString m_value;
  /** The text window label background color. */
  QString m_textBGColor;
  /** Pointer to the internal text label */
  QLabel  *m_text;
  /** Pointer to the internal pre-text label */
  QLabel  *m_ptext;
  /** Pointer to the internal pre-unit label */
  QLabel  *m_punit;
  /** Pointer to the pre-text minus sign */
  QLabel  *m_pminus;
  /** Widget containing pre labels. */
  QWidget* m_preWidget;
  /** The maximum font dot size passed in the constructor. */
  int m_maxFontDotsize;
  /** The font unit to be used in style sheet definition. */
  QString m_fontUnit;
  /** The maximum text label font height. */
  int m_maxTextLabelFontHeight;

  /**
   * Minimum update interval of display label in milli seconds. The variable
   * is undefined, if set to zero.
   */
  int m_minUpdateInterval;

  /** Last time of label update. */
  QTime m_lastUpdateTime;

  /** Timer to generate long mouse press signals. */
  QTimer* m_mousePressTimer;
};

#endif
