/***********************************************************************
**
**   taskline.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013 Axel Pauli
**
**   Created on: 28.01.2013
**
**   Author: Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: taskline.h 5840 2013-02-01 21:00:50Z axel $
**
***********************************************************************/

/**
 * \class TaskLine
 *
 * \author Axel Pauli
 *
 * \brief Contains all data attributes of a task line.
 *
 * This class covers the task line handling. It calculates all required
 * values and determines the task line crossing.
 *
 * \date 2013
 *
 * \version $Id: taskline.h 5840 2013-02-01 21:00:50Z axel $
 */

#ifndef TASK_LINE_H_
#define TASK_LINE_H_

#include <QPainterPath>
#include <QPoint>

#include "distance.h"

class TaskLine
{
 public:

  /**
   * Default constructor
   */
  TaskLine();

  /**
   * Constructor
   *
   * @param center The position of the line center point in KFLog coordinate format.
   *
   * @param length The length of the line in meters.
   *
   * @param direction The course in degrees in flight direction.
   */
  TaskLine( QPoint& center, const double length, const int direction );

  virtual ~TaskLine();

  int getDirection() const
  {
    return m_direction;
  }

  void setDirection( const int direction )
  {
    m_direction = direction;
  }

  QPoint& getInboundLine1Begin()
  {
    return m_inboundLine1Begin;
  }

  QPoint& getInboundLine1End()
  {
    return m_inboundLine1End;
  }

  QPoint& getInboundLine2Begin()
  {
    return m_inboundLine2Begin;
  }

  QPoint& getInboundLine2End()
  {
    return m_inboundLine2End;
  }

  QPainterPath& getInboundRegion()
  {
    return m_inboundRegion;
  }

  QPoint& getLineBegin()
  {
    return m_lineBegin;
  }

  QPoint& getLineCenter()
  {
    return m_lineCenter;
  }

  void setLineCenter(const QPoint& lineCenter)
  {
    m_lineCenter = lineCenter;
  }

  QPoint getLineEnd() const
  {
    return m_lineEnd;
  }

  double getLineLength() const
  {
    return m_lineLength;
  }

  void setLineLength(double lineLength)
  {
    m_lineLength = lineLength;
  }

  QPoint& getOutboundLineBegin()
  {
    return m_outboundLineBegin;
  }

  QPoint& getOutboundLineEnd()
  {
    return m_outboundLineEnd;
  }

  QPainterPath& getOutboundRegion()
  {
    return m_outboundRegion;
  }

  /**
   * This method calculates all necessary line and region elements.
   */
  void calculateElements();

  /**
   * Checks, if the task line has been crossed.
   *
   * @param position Current position in KFLog coordinate format
   *
   * @return true if line crossing is detected otherwise false
   */
  bool checkCrossing( const QPoint& position );

  /**
   * This method draws a start or finish line perpendicular to the bisecting
   * line direction.
   *
   * @param painter The painter to be used for the drawing.
   */
  void drawLine( QPainter* painter );

  /**
   * This method draws the region boxes used to determine the line crossing.
   * It is only a help function for debug purposes.
   *
   * @param painter The painter to be used for the drawing.
   */
  void drawRegionBoxes( QPainter* painter );

 private:

  /**
   * Resets the inbound and outbound counter.
   */
  void resetCounters()
  {
    m_inboundCounter  = 0;
    m_outboundCounter = 0;
  }

  QPoint m_lineCenter;
  QPoint m_lineBegin;
  QPoint m_lineEnd;

  QPoint m_inboundLine1Begin;
  QPoint m_inboundLine1End;
  QPoint m_inboundLine2Begin;
  QPoint m_inboundLine2End;

  QPoint m_outboundLineBegin;
  QPoint m_outboundLineEnd;

  QPainterPath m_inboundRegion;
  QPainterPath m_outboundRegion;

  double m_lineLength;
  int m_direction;
  int m_inboundCounter;
  int m_outboundCounter;
};

#endif /* TASK_LINE_H_ */
