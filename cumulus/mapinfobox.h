/***************************************************************************
                          mapinfobox.h  -  description
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

#ifndef MAPINFOBOX_H
#define MAPINFOBOX_H

#include <QWidget>
#include <QLabel>
#include <QFont>
#include <QString>
#include <QEvent>

/**
 * This is a slight modification of a QLabel. It adds a mousePress event.
 * @author Andre Somers
 */
class CuLabel : public QLabel
{
    Q_OBJECT
public:
  CuLabel ( QWidget *parent, Qt::WFlags f = 0 );

  CuLabel ( const QString &text, QWidget *parent, Qt::WFlags f = 0 );

signals:

     // Emitted when the mouse is pressed over the label
    void mousePress();

private:
    void mousePressEvent ( QMouseEvent * e );
};


/**
 * This is a specialized widget that can display a label with a
 * pre-text, used on the MapView.
 * @author Andre Somers
 */
class MapInfoBox : public QFrame
{
    Q_OBJECT

public:
    MapInfoBox(QWidget * parent, const QString borderColor, int fontDotsize = 38, bool minusInPretext = false );
    virtual ~MapInfoBox();

    /**
     * Write property of QString _PreText.
     */
    virtual void setPreText( const QString _newVal);

    /**
     * Read property of QString _PreText.
     */
    virtual const QString& getPreText();

    /**
     * Write property of QString _value.
     */
    virtual void setValue( const QString _newVal);

    /**
     * Read property of QString _value.
     */
    virtual const QString& getValue();

signals:
    /**
     * The mouse is pressed over the widget
     */
    void mousePress();

protected:
    /**
     * Reimplemented from QWidget
     */
    bool event(QEvent *);

    /**
     * Reimplemented from QObject
     */
    bool eventFilter(QObject *, QEvent *);

private:
    /** The text displayed before the value. */
    QString _PreText;
    /** The value of the box */
    QString _value;
    /** Pointer to the internal text label */
    QLabel  *_text;
    /** Pointer to the internal pre-text label */
    QLabel  *_ptext;
    /** The maximum font size */
    int _maxFontDotsize;
    /** If the minus sign is shown in pre-text or text label */ 
    bool _preMinus;
};

#endif
