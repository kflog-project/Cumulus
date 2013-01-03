/***********************************************************************
**
**   CoordEditNumPad.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2001      by Harald Maier
**                   2009      by Axel Pauli complete redesign done
**                   2009-2013 by Axel Pauli
**
**   Email: axel@kflog.org
**
************************************************************************
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef COORD_EDIT_NUM_PAD_H
#define COORD_EDIT_NUM_PAD_H

#include <QWidget>

class QString;
class QPushButton;
class NumberEditor;

/**
 * \class CoordEditNumPad
 *
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
 *
 * \date 2001-2013
 *
 * \version $Id$
 */

class CoordEditNumPad : public QWidget
{
    Q_OBJECT

  private:

    Q_DISABLE_COPY ( CoordEditNumPad )

  public:

    CoordEditNumPad( QWidget *parent=0 );

    virtual ~CoordEditNumPad();

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
    void slot_numberEdited( const QString& number );

    /**
     * Used to change the sky direction label in the push button.
     */
    void slot_changeSkyDirection();

  protected:

    /**
     * Catch show events in this class to set the widths of some widgets.
     */
    void showEvent(QShowEvent *);

    /** Input fields for coordinate */
    NumberEditor *degreeBox;
    NumberEditor *minuteBox;
    NumberEditor *secondBox;

    /** Sky directions */
    QPushButton *skyDirection;

    /** Initial values saved here for change control. */
    int     iniKflogDegree;
    QString iniDegree;
    QString iniMinute;
    QString iniSecond;
    QString iniDirection;
};

/**
 * \class LatEdit
 *
 * \author Harald Maier, Axel Pauli
 *
 * \brief Editor widget for WGS84 latitude coordinates.
 *
 * This class is used to edit WGS84 latitude coordinates. It is derived
 * from \ref CoordEditNumPad. Three different coordinate formats are supported.
 *
 * -degrees, minutes, seconds
 * -degrees and decimal minutes
 * -decimal degrees
 *
 * \date 2001-2013
 */
class LatEditNumPad : public CoordEditNumPad
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( LatEditNumPad )

public:

  LatEditNumPad( QWidget *parent=0, const int base=1 );

  virtual ~LatEditNumPad() {};

  /**
   * overloaded function
   */
  void setKFLogDegree(const int coord);
};

/**
 * \class LongEdit
 *
 * \author Harald Maier, Axel Pauli
 *
 * \brief Editor widget for WGS84 longitude coordinates.
 *
 * This class is used to edit WGS84 longitude coordinates. It is derived
 * from \ref CoordEditNumPad. Three different coordinate formats are supported.
 *
 * -degrees, minutes, seconds
 * -degrees and decimal minutes
 * -decimal degrees
 *
 * \date 2001-2013
 */
class LongEditNumPad : public CoordEditNumPad
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( LongEditNumPad )

public:

  LongEditNumPad( QWidget *parent=0, const int base=1 );

  virtual ~LongEditNumPad() {};

  /**
   * Overloaded function
   */
  void setKFLogDegree(const int coord);
};

#endif // COORD_EDIT_NUM_PAD_H
