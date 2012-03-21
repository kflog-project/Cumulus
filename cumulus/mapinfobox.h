/***************************************************************************
                          mapinfobox.h  -  description
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002      by Andre Somers
                               2008      by Josua Dietze
                               2008-2011 by Axel Pauli

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

#ifndef MAP_INFO_BOX_H
#define MAP_INFO_BOX_H

#include <QWidget>
#include <QLabel>
#include <QFont>
#include <QString>
#include <QEvent>

/**
 * \class CuLabel
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Slight modification of a QLabel.
 *
 * This is a slight modification of a QLabel. It adds a mousePress event.
 *
 * \date 2002-2011
 *
 */
class CuLabel : public QLabel
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( CuLabel )

public:

  CuLabel ( QWidget *parent, Qt::WFlags flags=0 );

  CuLabel ( const QString &text, QWidget *parent, Qt::WFlags flags=0 );

signals:

     /** Emitted when the mouse is pressed over the label */
    void mousePress();

protected:

    void mousePressEvent ( QMouseEvent *event );
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
 * \date 2002-2012
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
   * Write property of QString _preText.
   */
  void setPreText( const QString& _newVal);

  /**
   * Show resp. hide the pretext label widgets.
   */
  void setPreTextVisible( bool visible )
  {
    _ptext->setVisible( visible );

    if( _punit ) _punit->setVisible( visible );
    if( _pminus ) _pminus->setVisible( visible );
  };

  /**
   * Show resp. hide the whole pretext widget.
   */
  void setPreTextWidgetVisible( bool visible )
  {
    _preWidget->setVisible( visible );
  };

  /**
   * Write property of QString _preUnit.
   */
  void setPreUnit( const QString& _newVal);

  /**
   * Read property of QString _preText.
   */
  const QString& getPreText()
  {
    return _preText;
  };

  /**
   * Write property of QString _value.
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
   * Read property of QString _value.
   */
  const QString& getValue()
  {
    return _value;
  };

  /**
   * Returns the pretext label widget.
   */
  QLabel* getPreTextLabelWidget()
  {
    return _ptext;
  };

  /**
   * Returns the text label widget.
   */
  QLabel* getTextLabelWidget()
  {
    return _text;
  };

  void setPixmap( const QPixmap& newPixmap );

signals:

  /**
   * The mouse is pressed over the widget
   */
  void mousePress();

protected:

  /**
   * Reimplemented from QWidget
   */
  void mousePressEvent( QMouseEvent * event );

  /**
   * Reimplemented from QWidget
   */
  void showEvent(QShowEvent *event);

private:

  /**
   * Initializes the basics of this widget.
   *
   * @param borderColor Color to be used as border.
   */
  void basics( const QString& borderColor );

  /** The upper text displayed before the value. */
  QString _preText;
  /** The lower text displayed before the value. */
  QString _preUnit;
  /** The value of the box */
  QString _value;
  /** The text window label background color. */
  QString _textBGColor;
  /** Pointer to the internal text label */
  QLabel  *_text;
  /** Pointer to the internal pre-text label */
  QLabel  *_ptext;
  /** Pointer to the internal pre-unit label */
  QLabel  *_punit;
  /** Pointer to the pre-text minus sign */
  QLabel  *_pminus;
  /** Widget containing pre labels. */
  QWidget* _preWidget;
  /** The maximum font size */
  int _maxFontDotsize;
  /** The font unit to be used in style sheet definition. */
  QString _fontUnit;
};

#endif
