/***********************************************************************
**
**   coordedit.h - Editor for WGS84 coordinates, supports three formats.
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**                   2009 by Axel Pauli complete redesign done
**
**   Email:           axel@kflog.org
**
************************************************************************
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
  * \author Harald Maier, Axel Pauli
  *
  * \brief Editor widget for WGS84 coordinates.
  *
  * This class is used to edit WGS84 coordinates. It is subclassed by
  * two extensions to handle latitude and longitude coordinates. Three
  * different coordinate formats are supported.
  *
  * -degrees, minutes, seconds
  * -degrees and decimal minutes
  * -decimal degrees
  */

#ifndef COOR_DEDIT_H
#define COOR_DEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QString>

/**
 * @author Harald Maier, Axel Pauli
 */
class CoordEdit : public QWidget
{
    Q_OBJECT

public:

    CoordEdit( QWidget *parent=0 );

    virtual ~CoordEdit();

    /**
     * Sets all edit fields according to the passed coordinate value.
     * The coordinate value is encoded in the KFLog internal format for degrees.
     */
    void setKFLogDegree(const int coord, const bool isLat);

    /** Calculates a degree value in the KFLog internal format for degrees from
     *  the input data fields.
     */
    int KFLogDegree();

    /**
     * Returns true, if initial input values have been modified.
     */
    bool isInputChanged();

    private slots:

    /**
     * Used to check the user input in the editor fields.
     */
    void slot_textEdited( const QString& text );

protected:

    /**
     * Catch show events in this class to set the widths of some widgets.
     */
    void showEvent(QShowEvent *);

    /** Input fields for coordinate */
    QLineEdit *degreeBox;
    QLineEdit *minuteBox;
    QLineEdit *secondBox;

    /** Sky directions */
    QComboBox *directionBox;

    /** Initial values saved here for change control. */
    int     iniKflogDegree;
    QString iniDegree;
    QString iniMinute;
    QString iniSecond;
    QString iniDirection;
};

/** Class extension for latitude coordinates. */
class LatEdit : public CoordEdit
{
    Q_OBJECT

public:

    LatEdit( QWidget *parent=0, const int base=1 );

    virtual ~LatEdit() {};

    /**
     * overloaded function
     */
    void setKFLogDegree(const int coord);
};

/** Class extension for longitude coordinates. */
class LongEdit : public CoordEdit
{
    Q_OBJECT

public:

    LongEdit( QWidget *parent=0, const int base=1 );

    virtual ~LongEdit() {};

    /**
     * Overloaded function
     */
    void setKFLogDegree(const int coord);
};

#endif
