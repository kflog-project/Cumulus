/***********************************************************************
 **
 **   taskpoint.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2010-2016 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#ifndef TASK_POINT_H
#define TASK_POINT_H

#include "distance.h"
#include "generalconfig.h"
#include "singlepoint.h"
#include "taskline.h"
#include "taskpointtypes.h"
#include "waypoint.h"

/**
 * \class TaskPoint
 *
 * \author Axel Pauli
 *
 * \brief Contains all data attributes of a task point.
 *
 * This class is an extension of the single point class. It handles all data
 * items concerning a flight task.
 *
 * \date 2010-2016
 *
 * \version 1.1
 */
class TaskPoint : public SinglePoint
{
 public:

  /**
   * This enumeration is used to describe the task point passage state.
   */
  enum PassageState{ Outside, Near, Touched, Passed };

  /**
   * Default constructor.
   */
  TaskPoint( enum TaskPointTypes::TaskPointType type=TaskPointTypes::Unknown );

  /**
   * Constructs a taskpoint object from a waypoint object.
   */
  TaskPoint( const Waypoint& wp,
             enum TaskPointTypes::TaskPointType type=TaskPointTypes::Unknown );

  virtual ~TaskPoint();

  /**
   * Sets the type of the task point.
   *
   * \param value The type of the task point.
   */
  void setTaskPointType( enum TaskPointTypes::TaskPointType value )
    {
      m_taskPointType = value;
    };

  /**
   * Gets the type of the task point.
   *
   * \return The type of the task point.
   */
  enum TaskPointTypes::TaskPointType getTaskPointType() const
    {
      return m_taskPointType;
    };

  /**
   * Returns a waypoint object containing the taskpoint basic data.
   *
   * \return A waypoint object initialized with the taskpoint basic data.
   */
  Waypoint* getWaypointObject();

  /**
   * Checks the task point passage according to the assigned schema.
   *
   * @param dist2Tp Distance to taskpoint
   *
   * @param position Current position as KFLOG WGS84 datum
   *
   * @return State of passage.
   */
  enum PassageState checkPassage( const Distance& dist2Tp, const QPoint& position );

  /**
   * Determines the task point passage for a line figure.
   *
   * @param dist2Tp Distance to taskpoint in meters
   *
   * @param position current WGS position as KFLOG WGS84 datum
   *
   * @return State of passage.
   */
  enum PassageState determineLinePassageState( const Distance& dist2Tp,
                                               const QPoint& position );
  /**
   * Determines the task point passage for a circle figure.
   *
   * @param dist2Tp Distance to taskpoint in meters
   *
   * @param insideRadius Inside radius to taskpoint in meters
   *
   * @return State of passage.
   */
  enum PassageState determineCirclePassageState( const double dist2Tp,
                                                 const double insideRadius );

  /**
   * Determines the task point passage for a sector figure.
   *
   * @param dist2Tp Distance to taskpoint in meters
   *
   * @param position current WGS position as KFLOG WGS84 datum
   *
   * @return State of passage.
   */
  enum PassageState determineSectorPassageState( const double dist2Tp,
                                                 const QPoint& position );

  /**
   * Gets the type of a task point in a string format.
   *
   * \param detailed If true a long format is used.
   *
   * \return The type of a task point in a string format.
   */
  QString getTaskPointTypeString( bool detailed=false ) const;

  /**
   * Gets the figure of a task point as string format. Figure can be
   * a cylinder, a sector or a line description.
   *
   * \return The figure of a task point in a string format.
   */
  QString getTaskPointFigureString() const;

  /**
   *  Gets a combined and shorten task point type and figure schema string.
   *
   * \return A combined and shorten task point type and figure schema string.
   */
  QString getTaskPointTypeFigureString() const;

  /** The bisecting line of the sector in radian */
  double angle;
  /** The minimum angle of the sector in radian */
  double minAngle;
  /** The maximum angle of the sector in radian */
  double maxAngle;
  /** The bearing from the previous task point in radian */
  double bearing;
  /** The distance to the previous task point in km */
  double distance;
  /** The time distance to the previous task point in seconds */
  int distTime;
  /** wind correction angle from the previous task point in degree. */
  double wca;
  /** The true heading ( wind was considered) from the previous task point in degree. */
  double trueHeading;
  /**
   * The ground speed ( wind was considered) from the previous task point
   * to this point.
   */
  double groundSpeed;
  /** Result of wind triangle calculation. */
  bool wtResult;

  //------------------------------------------------------------
 protected:

  /** The type of the task point. */
  enum TaskPointTypes::TaskPointType m_taskPointType;

  enum GeneralConfig::ActiveTaskFigureScheme m_taskActiveTaskPointFigureScheme;

  /**
   * The last calculated passage state of this task point.
   */
  enum PassageState m_lastPassageState;

  /**
   * The last calculated distance to the taskpoint center in meters. If the
   * distance is undefined, a negative value is assigned.
   */
  double m_lastDistance;

  Distance m_taskCircleRadius;
  Distance m_taskSectorInnerRadius;
  Distance m_taskSectorOuterRadius;
  int      m_taskSectorAngle;

  /** Task line object */
  TaskLine m_taskLine;

  /**
   * Auto zoom flag. If enabled, it is zoomed in the map in the near of the
   * task point.
   */
  bool m_autoZoom;

  /**
   * Flag to indicate that the task point has been edited by the user.
   */
  bool m_userEdited;

  /** Index of taskpoint in the flight task list */
  short m_flightTaskListIndex;

  /** A waypoint object, filled with the taskpoint basic data.*/
  Waypoint m_wpObject;

 public:

  /**
   * Gets the flight task list index of this task point.
   *
   * \param value The new flight task list index of this task point.
   */
  void setFlightTaskListIndex( const short value )
    {
      m_flightTaskListIndex = value;
    };

  /**
   * Sets the flight task list index of this task point
   *
   * \return The flight task list index of this task point.
   */
  short getFlightTaskListIndex() const
    {
      return m_flightTaskListIndex;
    };

  /** Gets the active task point figure scheme. That can be cylinder, sector or line. */
  enum GeneralConfig::ActiveTaskFigureScheme getActiveTaskPointFigureScheme() const
  {
    return m_taskActiveTaskPointFigureScheme;
  };

  /** Sets the active task point figure scheme. That can be cylinder, sector or line. */
  void setActiveTaskPointFigureScheme( const enum GeneralConfig::ActiveTaskFigureScheme newValue )
  {
    m_taskActiveTaskPointFigureScheme = newValue;
  };

  /**
   * @return A reference to the task line object.
   */
  TaskLine& getTaskLine()
  {
    return m_taskLine;
  }

  /**
   * Gets the line length of a start or finish line.
   *
   * @return Task line length distance object.
   */
  Distance getTaskLineLength() const
  {
    return Distance( m_taskLine.getLineLength() );
  };

  /**
   * Sets the line length of a start or finish line.
   *
   * @param taskLineLength Task line length distance object.
   */
  void setTaskLineLength( const Distance& newValue)
  {
    m_taskLine.setLineLength( newValue.getMeters() );
  };

  /** Gets the task circle radius. */
  Distance getTaskCircleRadius() const
  {
    return m_taskCircleRadius;
  };

  /** Sets the task circle radius. */
  void setTaskCircleRadius( const Distance &newValue )
  {
    m_taskCircleRadius = newValue;
  };

  /** Gets task sector inner radius. */
  Distance getTaskSectorInnerRadius() const
  {
    return m_taskSectorInnerRadius;
  };

  /** Sets task sector inner radius. */
  void setTaskSectorInnerRadius( const Distance &newValue )
  {
    m_taskSectorInnerRadius = newValue;
  };

  /** Gets task sector outer radius. */
  Distance getTaskSectorOuterRadius() const
  {
    return m_taskSectorOuterRadius;
  };

  /** Sets task sector outer radius. */
  void setTaskSectorOuterRadius( const Distance &newValue )
  {
    m_taskSectorOuterRadius = newValue;
  };

  /** Gets task sector angle 1-360 degrees.  */
  int getTaskSectorAngle() const
  {
    return m_taskSectorAngle;
  };

  /** Sets task sector angle 1-360 degrees.  */
  void setTaskSectorAngle( const int newValue )
  {
    m_taskSectorAngle = newValue;
  };

  /**
   * @return The state of the auto zoom flag.
   */
  bool getAutoZoom() const
  {
    return m_autoZoom;
  };

  /**
   * @param autoZoom The new state of the auto zoom flag.
   */
  void setAutoZoom( bool autoZoom )
  {
    m_autoZoom = autoZoom;
  };

  /**
   * @return The state of the user edit flag
   */
  bool getUserEditFlag() const
  {
    return m_userEdited;
  };

  /**
   * @param newValue The new state of the user edit flag
   */
  void setUserEditFlag(bool newValue )
  {
    m_userEdited = newValue;
  };

  /**
   * Called to reset alliconSize task point schema items to defaults.
   */
  void setConfigurationDefaults();

  /**
   * Called to get the task point icon.
   *
   * \param iconSize The size of the desired icon.
   *
   * \return The task point icon.
   *
   */
  QPixmap& getIcon( const int iconSize );

  /**
   * Create a circle icon.
   */
  QPixmap& createCircleIcon( const int iconSize );

  /**
   * Create a sector icon.
   */
  QPixmap& createSectorIcon( const int iconSize );

  /**
   * Create a line icon.
   */
  QPixmap& createLineIcon( const int iconSize );
};

#endif /* TASK_POINT_H */
