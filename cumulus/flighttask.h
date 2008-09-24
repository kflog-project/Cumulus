/***********************************************************************
**
**   flighttask.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**                   2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
**   This class provides calculation, drawing and other support
**   for flight tasks and contains the data of a task.
**
***********************************************************************/

#ifndef FLIGHT_TASK_H
#define FLIGHT_TASK_H

#include <QObject>
#include <QString>
#include <QList>
#include <QPoint>
#include <QPolygonF>
#include <QPainter>
#include <QPainterPath>

#include "waypoint.h"
#include "basemapelement.h"
#include "distance.h"
#include "altitude.h"
#include "speed.h"

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

 public: //methods

  /**
   * Creates a task with the given points.
   *
   * @param wpList the list of waypoints. Object ownership will be
   *               overtaken by this class
   *
   * @param  fai if true, drawing according of FAI rules, otherwise not
   *
   * @param  speed the planned cruising speed
   *
   */
  FlightTask( QList<wayPoint*> *wpList=0, bool fai=true,
              QString taskName=QObject::tr("unknown"), int speed=0 );

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
   * A triangle is a valig FAI-triangle, if no side is less than 28%
   * of the total length (total length less than 500 km), or no side
   * is less than 25% or larger than 45% of the total length
   * (totallength >= 500km).
   *
   * @param  d_wp  totallength
   * @param  d1  first side
   * @param  d2  second side
   * @param  d3  third side
   */
  static bool isFAI(double d_wp, double d1, double d2, double d3);

  /**
   * Returns the waypoint list by reference
   */
  QList<wayPoint*>& getWPList()
  {
    return *wpList;
  };

  /**
   * Returns a deep copy of the waypoint list. The ownership of the
   * list is overtaken by the caller.
   */
  QList<wayPoint*> *getCopiedWPList();

  /**
   * Returns a deep copy of the passed waypoint list. The ownership of
   * the list is overtaken by the caller. For convenvience provided as
   * static method.
   */
  static QList<wayPoint*> *copyWpList(QList<wayPoint*> *wpList);

  /**
   * Returns the type of the task.
   * @see #TaskType
   */
  const int getTaskType() const
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
   * are greather than 30.
   */
  QString getTotalDistanceTimeString();

  /**
   * @returns a string representing the distance time as hh:mm to use
   * for user-display. The time is rounded up if seconds are greather
   * than 30. Input has to be passed as seconds. For convenvience
   * provided as static method.
   */
  static QString getDistanceTimeString(const int timeInSec);

  /**
   * Draws the flight and the task into the given painter. Reimplemented
   * from BaseMapElement.
   * @param  targetP  The painter to draw the element into.
   */
  void drawMapElement(QPainter* painter);

  /** */
  QString getTotalDistanceString() const;

  /** */
  QString getTaskDistanceString() const;

  /** */
  QString getTaskName() const
  {
    return _taskName;
  };

  /** */
  void setTaskName( QString& newName )
  {
    _taskName = newName;
  };

  /** */
  QString getPointsString() const;

  /** */
  void setWaypointList(QList<wayPoint*> *newWpList);

  /** */
  void addWaypoint( wayPoint *newWP );

  /** */
  void setPlanningType( const int type );

  /** */
  const int getPlanningType() const
  {
    return __planningType;
  };

  /** returns the set cruising speed */
  int getSpeed() const { return cruisingSpeed; };

  /** sets the cruising speed */
  void setSpeed( const int newSpeed )
  {
    cruisingSpeed = newSpeed;
    updateTask();
  };

  /** sets FAI rule flag */
  void setFaiRules( const bool newValue )
  {
    faiRules = newValue;
    __determineTaskType();
  };

  /* returns FAI rule flag */
  const bool getFaiRules() const
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
  int calculateFinalGlidePath( const int taskPointIndex,
                               Altitude &arrivalAlt,
                               Speed &bestSpeed );

  virtual bool isVisible() const
    {
      return true;
    };

  /////////////////////////////////////////////////////////////////////////

 private:

  /**
   * Does all map drawing actions for task points using cylinder scheme
   *
   */
  void circleSchemeDrawing( QPainter* painter );

  /**
   * Draws a circle around the given position.
   *
   * Paiter as Cumulus special painter device
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
   * bisector angle in debrees
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
   * Calculates the task point sector angles in radian. The sector angle
   * between two task points is the bisecting line of the angle.
   */
  double __calculateSectorAngles( int loop );

  /**
   * Sets the status of the waypoints, the durations in seconds and
   * the distances in km.
   */
  void __setTaskPointTypes();

  /** */
  QList<wayPoint*> *wpList;

  /**
   * if true, FAI rules will be taken into account
   */
  bool faiRules;

  /** planned cruising speed */
  int cruisingSpeed;

  /** */
  uint task_end;

  /** */
  uint task_begin;

  /** */
  double olcPoints;

  /** */
  double taskPoints;

  /** */
  unsigned int flightType;

  /** Total length */
  double distance_total;

  /** WertungsDistanz f�r DMST*/
  double distance_wert;

  /** Task length */
  double distance_task;

  /** Total duration of task in seconds */
  int duration_total;

  /** planned task type */
  int __planningType;

  /** task name */
  QString _taskName;
};

#endif
