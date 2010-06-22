/***************************************************************************
                          mapinfobox.h  -  description
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

#ifndef MAPINFOBOX_H
#define MAPINFOBOX_H

#include <QWidget>
#include <QLabel>
#include <QFont>
#include <QString>
#include <QEvent>

/**
 * \author André Somers
 *
 * \brief Slight modification of a QLabel.
 *
 * This is a slight modification of a QLabel. It adds a mousePress event.
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

private:

    void mousePressEvent ( QMouseEvent *event );
};

/**
 * \author Andre Somers, Josua Dietze, Axel Pauli
 *
 * \brief Specialized widget for text or icon display.
 *
 * This is a specialized widget that can display either a label with a
 * pre-text or a pixmap. Optionally it can be clicked and connected to
 * a slot. Used on the MapView.
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
              int fontDotsize=38 );

  MapInfoBox( QWidget* parent, const QString& borderColor, const QPixmap& pixmap );

  virtual ~MapInfoBox() {};

  /**
   * Initializes the basics of this widget.
   *
   * @param borderColor Color to be used as border.
   */
  void basics( const QString& borderColor );

  /**
   * Write property of QString _preText.
   */
  void setPreText( const QString& _newVal);

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
  bool event(QEvent *event);

  /**
   * Reimplemented from QWidget
   */
  void showEvent(QShowEvent *event);

  /**
   * Reimplemented from QObject
   */
  bool eventFilter(QObject *, QEvent *);

private:
  /** The upper text displayed before the value. */
  QString _preText;
  /** The lower text displayed before the value. */
  QString _preUnit;
  /** The value of the box */
  QString _value;
  /** Pointer to the internal text label */
  QLabel  *_text;
  /** Pointer to the internal pre-text label */
  QLabel  *_ptext;
  /** Pointer to the internal pre-unit label */
  QLabel  *_punit;
  /** Pointer to the pre-text minus sign */
  QLabel  *_minus;
  /** The maximum font size */
  int _maxFontDotsize;
};

#endif
