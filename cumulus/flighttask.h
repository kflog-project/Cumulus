/***********************************************************************
**
**   flighttask.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2007-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
**   This class provides calculation, drawing and other support
**   for flight tasks and contains the data of a task.
**
***********************************************************************/

/**
 * \class FlightTask
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief Class to handle all things of a flight task.
 *
 * \date 2002-2010
 */

#ifndef FLIGHT_TASK_H
#define FLIGHT_TASK_H

#include <QObject>
#include <QString>
#include <QList>
#include <QPoint>
#include <QPolygonF>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>

#include "basemapelement.h"
#include "distance.h"
#include "altitude.h"
#include "speed.h"
#include "reachablepoint.h"
#include "taskpoint.h"

class FlightTask : public BaseMapElement
{
 public:
  /**
   * The flight task types.
   */
  enum TaskType {NotSet=0, ZielS = 1, ZielR = 2, FAI = 3, Dreieck = 4,
                 FAI_S = 5, Dreieck_S = 6, Abgebrochen = 7, Unknown = 8,
                 FAI_2 = 9, FAI_S2 = 10, FAI_3 = 11, FAI_S3 = 12,
                 Vieleck = 13, OLC2003 = 14};

  /**
   * The planning-types
   */
  enum PlanningType {RouteBased, AreaBased};

 public:

  /**
   * Creates a task with the given points.
   *
   * @param tpList The list of task points. Object ownership will be
   *        taken over by this class.
   *
   * @param  fai If true, drawing is done according to FAI rules, otherwise not.
   *
   * @param taskName The name of the task.
   *
   * @param  tas The planned true airspeed.
   *
   */
  FlightTask( QList<TaskPoint*> *tpList=0, bool fai=true,
              QString taskName=QObject::tr("unknown"), int tas=0 );

  /**
   * Copy constructor
   **/
  FlightTask( const FlightTask& inst );

 private:
 /**
   *  forbid assignment operator
   **/
  FlightTask& operator=( const FlightTask& inst );

 public:

  /**
   * Destructor
   */
  virtual ~FlightTask();

  /**
   * Returns true, if a triangle represented by the four length,
   * is a valid FAI-triangle.
   *
   * A triangle is a valid FAI-triangle, if no side is less than 28%
   * of the total length (total length less than 500 km), or no side
   * is less than 25% or larger than 45% of the total length
   * (total length >= 500km).
   *
   * @param  d_wp  total length
   * @param  d1  first side
   * @param  d2  second side
   * @param  d3  third side
   *
   * \return True or false depending on the check result.
   */
  static bool isFAI(double d_wp, double d1, double d2, double d3);

  /**
   * Returns the task point list by reference
   */
  QList<TaskPoint *>& getTpList()
  {
    return *tpList;
  };

  /**
   * Returns a deep copy of the task point list. The ownership of the
   * list is taken over by the caller.
   */
  QList<TaskPoint *> *getCopiedTpList();

  /**
   * Returns a deep copy of the passed task point list. The ownership of
   * the list is taken over by the caller. For convenience provided as
   * static method.
   */
  static QList<TaskPoint *> *copyTpList(QList<TaskPoint *> *tpListIn);

  /**
   * Returns the type of the task.
   * @see #TaskType
   */
  int getTaskType() const
    {
      return flightType;
    };

  /**
   * @returns a string representing the task type to use for user-display
   */
  QString getTaskTypeString() const;

  /**
   * @returns a string representing the total distance time of task as
   * hh:mm to use for user-display. The time is rounded up if seconds
   * are greater than 30.
   */
  QString getTotalDistanceTimeString();

  /**
   * @returns a string representing the distance time as hh:mm to use
   * for user-display. The time is rounded up if seconds are greater
   * than 30. Input has to be passed as seconds. For convenience
   * provided as static method.
   */
  static QString getDistanceTimeString(const int timeInSec);

  /**
   * Draws the flight and the task into the given painter.
   * @param painter  The painter to be used for the drawing.
   * @param drawnTp List of drawn task points, if taskpoint label drawing
   *        option is set.
   */
  void drawTask(QPainter* painter, QList<Waypoint *> &drawnTp );

  /**
   * function for drawing the element into the given painter.
   *
   * Not implemented in this class.
   *
   * The function must be implemented in the child-classes.
   * @param  painter  The painter to be used for the drawing.
   * @return true, if element was drawn otherwise false.
   */
  bool drawMapElement(QPainter* painter)
    {
      Q_UNUSED(painter)
      return false;
    };

  /** */
  QString getTotalDistanceString() const;

  /** */
  QString getTaskDistanceString() const;

  /** Returns wind direction and speed in string format "Degree/Speed". */
  QString getWindString() const;

  /** Returns the name of the task. */
  QString getTaskName() const
  {
    return _taskName;
  };

  /** Sets the name of the task. */
  void setTaskName( QString& newName )
  {
    _taskName = newName;
  };

  /** */
  void setTaskPointList(QList<TaskPoint *> *newTpList);

  /** */
  void addTaskPoint( TaskPoint *newTP );

  /** */
  void setPlanningType( const int type );

  /** */
  int getPlanningType() const
  {
    return __planningType;
  };

  /** returns the planned cruising speed */
  int getSpeed() const { return cruisingSpeed; };

  /** sets the planned cruising speed */
  void setSpeed( const int newSpeed )
  {
    cruisingSpeed = newSpeed;
    updateTask();
  };

  /** returns the wind speed */
  int getWindSpeed() const { return windSpeed; };

  /** sets the wind speed */
  void setWindSpeed( const int newSpeed )
  {
    windSpeed = newSpeed;
    updateTask();
  };

  /** returns the wind direction */
  int getWindDirection() const { return windDirection; };

  /** sets the wind direction */
  void setWindDirection( const int newDirection )
  {
    windDirection = newDirection;
    updateTask();
  };

  /** sets wind triangle calculation flag of this task */
  void setWtCalcFlag( const bool newValue )
  {
    wtCalculation = newValue;
  };

  /* returns wind triangle calculation flag of this task */
  bool getWtCalcFlag() const
  {
    return wtCalculation;
  };

  /** sets FAI rule flag */
  void setFaiRules( const bool newValue )
  {
    faiRules = newValue;
    __determineTaskType();
  };

  /* returns FAI rule flag */
  bool getFaiRules() const
  {
    return faiRules;
  };

  /**
   * @returns a string representing the cruising speed to use
   * for user-display.
   */
  QString getSpeedString() const;

  /** returns the total duration in seconds according to set cruising speed */
  int getDurationTotal() const { return duration_total; };

  /**
   *
   * Calculates the sector array used for the drawing of the task point
   * sector.
   *
   * p		sector result painter path
   * ocx	scaled outer radius center coordinate x
   * ocy	scaled outer radius center coordinate y
   * icx	scaled inner radius center coordinate x
   * icy	scaled inner radius center coordinate y
   * ora	scaled outer radius
   * ira	scaled inner radius
   * sba	sector biangle in degrees
   * sa	sector angle in degrees
   *
   */

  void calculateSector( QPainterPath& pp,
                        int ocx, int ocy,
                        int icx, int icy,
                        int ora, int ira,
                        int sba, int sa );


  /** check, if task sector has been arrived.
   * dist2TP: distance to task point
   * position: current position as WGS84 datum
   * taskPointIndex: index of TP in waypoint list
   * returns true, if inside of sector otherwise false
   */
  bool checkSector( const Distance& dist2TP,
                    const QPoint& position,
                    const int taskPointIndex );

  /**
   * updates the internal task data
   */
  void updateTask();

  /**
   * updates projection data of waypoint list
   */
  void updateProjection();

  /**
   * Calculates the glide path to final target of flight task from the
   * current position.
   *
   * taskPointIndex: index of next TP in waypoint list
   * arrivalAlt: returns arrival altitude
   * bestSpeed:  returns assumed speed
   * reachable:  returns info about reachability
   *
   */
  ReachablePoint::reachable
      calculateFinalGlidePath( const int taskPointIndex,
                               Altitude &arrivalAlt,
                               Speed &bestSpeed );

  virtual bool isVisible() const
    {
      return true;
    };

  /**
   * Returns the declaration date-time of the task as UTC.
   */
  QDateTime getDeclarationDateTime() const
  {
    return _declarationDateTime.toUTC();
  };

  /**
   * Sets the declaration date-time of the task as local time.
   */
  void setDeclarationDateTime()
  {
    _declarationDateTime = QDateTime::currentDateTime();
  };

  /////////////////////////////////////////////////////////////////////////

 private:

  /**
   * Does all map drawing actions for task points using cylinder scheme
   *
   */
  void circleSchemeDrawing( QPainter* painter, QList<Waypoint*> &drawnTp );

  /**
   * Reimplemented from BaseMapElement.
   * Draws a circle around the given position.
   *
   * Painter as Cumulus special painter device
   * coordinate as projected position of the point
   * scaled radius as meters
   * fillColor, do not fill, if set to invalid
   * drawShape, if set to true, draw outer circle with black color
   */

  void drawCircle( QPainter* painter,
		   QPoint& centerCoordinate, const int radius,
		   QColor& fillColor, const bool drawShape=true );

  /**
   * Draws a sector around the given position.
   *
   * Painter as Cumulus special painter device
   * coordinate as projected position of the point
   * scaled inner radius as meters
   * scaled outer radius as meters
   * bisector angle in degrees
   * spanning angle in degrees
   * fillColor, do not fill, if set to invalid
   * drawShape, if set to true, draw outer circle with black color
   */

  void drawSector( QPainter* painter,
		   QPoint& centerCoordinate,
		   const int innerRadius,
		   const int outerRadius,
		   const int biangle,
		   const int spanningAngle,
		   QColor& fillColor, const bool drawShape=true );

  /**
   * Determines the type of the task.
   */
  void __determineTaskType();

  /**
   * Calculates the task point sector angles in radians. The sector angle
   * between two task points is the bisecting line of the angle.
   */
  double __calculateSectorAngles( int loop );

  /**
   * Sets the status of the task points, the durations in seconds and
   * the distances in km.
   */
  void __setTaskPointData();

  /** Flight task with single task points. */
  QList<TaskPoint*> *tpList;

  /**
   * if true, FAI rules will be taken into account
   */
  bool faiRules;

  /** planned cruising speed */
  int cruisingSpeed;

  /** planned wind direction in degree */
  int windDirection;

  /** planned wind speed */
  int windSpeed;

  /** result of wind calculation via wind triangle */
  bool wtCalculation;

  /** Type of flight task */
  unsigned int flightType;

  /** Total length */
  double distance_total;

  /** Task length */
  double distance_task;

  /** Total duration of task in seconds */
  int duration_total;

  /** planned task type */
  int __planningType;

  /** task name */
  QString _taskName;

  /** Declaration date-time of task */
  QDateTime _declarationDateTime;
};

#endif
