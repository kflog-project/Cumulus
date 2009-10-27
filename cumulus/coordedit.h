/***************************************************************************
                          coordedit.h  -  description
                             -------------------
    begin                : Mon Dec 3 2001
    copyright            : (C) 2001 by Harald Maier, 2008 Axel Pauli
    email                : harry@kflog.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COORDEDIT_H
#define COORDEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QString>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QShowEvent>

/**
 * @author Harald Maier
 */
class CoordEdit : public QLineEdit
{
    Q_OBJECT

public:

    CoordEdit(QWidget *parent=0);
    virtual ~CoordEdit(){};

    void keyPressEvent (QKeyEvent *e);
    void focusInEvent (QFocusEvent *e);

    /**
     * No descriptions
     */
    void showEvent(QShowEvent *);

    /**
     * Sets the editbox to reflect the given value.
     */
    void setKFLogDegree(int value, bool isLat);

    /**
     * Returns the value of the editbox in the KFLog
     * internal format for degrees
     */
    int KFLogDegree();

    /**
     * Set cursor in dependency of position and input
     * mask to next number field
     */
    void setCursor2NextNo( int pos );

    /**
     * @Returns true, if initial input text has been changed
     */
    bool isInputChanged();


public slots: // Public slots
    /** No descriptions */
    void clear();


protected:
    /** No descriptions */
    QString mask;
    QString validDirection;
    QString initText;
    bool firstSet;
    int format;
};

class LatEdit : public CoordEdit
{
    Q_OBJECT

public:

    LatEdit(QWidget *parent=0, const int base=1);
    virtual ~LatEdit() {};

    /**
     * overloaded function
     */
    void setKFLogDegree(int value);
};


class LongEdit : public CoordEdit
{
    Q_OBJECT

public:

    LongEdit(QWidget *parent=0, const int base=1);
    virtual ~LongEdit() {};

    /**
     * Overloaded function
     */
    void setKFLogDegree(int value);
};

#endif
