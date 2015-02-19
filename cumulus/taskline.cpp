/***********************************************************************
**
**   taskline.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013-2015 Axel Pauli
**
**   Created on: 28.01.2013
**
**   Author: Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>
#include <QPainter>
#include <QPolygon>

#include "generalconfig.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "taskline.h"

// Activate this define to get debug messages displayed
// #define TL_DEBUG

extern MapMatrix* _globalMapMatrix;

TaskLine::TaskLine() :
 m_lineLength(0.0),
 m_direction(-1),
 m_inboundCounter(0),
 m_outboundCounter(0)
{
}

TaskLine::TaskLine( QPoint& center, const double length, const int direction ) :
  m_lineCenter(center),
  m_lineLength(length),
  m_direction(direction),
  m_inboundCounter(0),
  m_outboundCounter(0)
{
  calculateElements();
}

TaskLine::~TaskLine()
{
}

void TaskLine::calculateElements()
{
  if( m_lineCenter.isNull() || m_direction == -1 || m_lineLength == 0 )
    {
      // It seems that the line data are not initialized.
      qWarning() << __PRETTY_FUNCTION__ << "Line data are not initialized!";
      return;
    }

  double llh = m_lineLength / 2.;

  // Calculate middle line begin right hand from center position
  m_lineBegin = MapCalc::getPosition( m_lineCenter, llh , (m_direction + 90) );

  // Calculate middle line end left hand from center position
  m_lineEnd = MapCalc::getPosition( m_lineCenter, llh, (m_direction - 90) );

  // Calculate outbound line points. They lay 500m in outbound direction from the
  // middle line.
  m_outboundLineBegin = MapCalc::getPosition( m_lineBegin, 500., (m_direction) );
  m_outboundLineEnd   = MapCalc::getPosition( m_lineEnd, 500., (m_direction) );

  // Calculate line1 inbound points. They are cover the middle line and are 250m
  // longer at every side of the middle line.
  m_inboundLine1Begin = MapCalc::getPosition( m_lineCenter, llh + 250., (m_direction + 90) );
  m_inboundLine1End   = MapCalc::getPosition( m_lineCenter, llh + 250., (m_direction - 90) );

  // Calculate line2 inbound points. They lay 500m before line 1 inbound.
  m_inboundLine2Begin = MapCalc::getPosition( m_inboundLine1Begin, 500., (m_direction + 180) );
  m_inboundLine2End   = MapCalc::getPosition( m_inboundLine1End,   500., (m_direction + 180) );

  // Setup inbound and outbound regions. The regions are squares with the edge
  // length of the line.
  QPolygon outbp;
  outbp << m_lineBegin << m_outboundLineBegin << m_outboundLineEnd << m_lineEnd;

  QPolygon inbp;
  inbp << m_inboundLine1Begin << m_inboundLine1End << m_inboundLine2End << m_inboundLine2Begin;

  m_outboundRegion = QPainterPath();
  m_outboundRegion.addPolygon( outbp );
  m_outboundRegion.closeSubpath();

  m_inboundRegion = QPainterPath();;
  m_inboundRegion.addPolygon( inbp );
  m_inboundRegion.closeSubpath();

  resetCounters();
}

bool TaskLine::checkCrossing( const QPoint& position )
{
#ifdef TL_DEBUG
  qDebug() << "TL::checkCrossing" << "heading=" << m_direction
           << "Length=" << m_lineLength
           << "IBR=" << (! m_inboundRegion.isEmpty() )
           << "OBR=" << (! m_outboundRegion.isEmpty() );
#endif

  if( m_direction == -1 || m_lineLength == 0 ||
      m_inboundRegion.isEmpty() || m_outboundRegion.isEmpty() )
    {
      // It seems that the line data are not initialized.
#ifdef TL_DEBUG
      qDebug() << "TaskLine::checkCrossing: Input check -> Ret=false";
#endif

      return false;
    }

  // As first check, if we have the inbound region arrived
  if( m_inboundRegion.contains( position ) )
    {
#ifdef TL_DEBUG
      qDebug() << "TL::checkCrossing: Inbound region true -> Ret=false";
#endif

      m_inboundCounter++;
      m_outboundCounter = 0;
      return false;
    }

  // As next check, if we have the outbound region arrived
  if( m_outboundRegion.contains( position ) )
    {
      if( m_inboundCounter > 0 )
        {
#ifdef TL_DEBUG
          qDebug() << "TL::checkCrossing: Outbound region true -> Ret=true";
#endif
          // It seems we came from inbound. So we decide, that the line
          // has been crossed.
          m_inboundCounter = 0;
          m_outboundCounter++;
          return true;
        }
      else
        {
#ifdef TL_DEBUG
          qDebug() << "TL::checkCrossing: Outbound region true -> Ret=false";
#endif
          m_outboundCounter++;
          return false;
        }
    }

#ifdef TL_DEBUG
  qDebug() << "TL::checkCrossing: No regions -> Ret=false";
#endif

  // Outside of regions, we reset the counters.
  resetCounters();
  return false;
}

void TaskLine::drawLine( QPainter* painter )
{
  // fetch current scale, scale uses unit meter/pixel
  const double cs = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

  if( cs > 350 || m_direction == -1 || m_lineLength == 0 )
    {
      // No data are set
      return;
    }

  // Map WGS84 points to map projection
  QPoint pBegin = _globalMapMatrix->wgsToMap(m_lineBegin);
  QPoint pEnd   = _globalMapMatrix->wgsToMap(m_lineEnd);

  // Map projected points to map display
  QPoint mBegin = _globalMapMatrix->map(pBegin);
  QPoint mEnd   = _globalMapMatrix->map(pEnd);

  QRect viewport = painter->viewport();

  if( viewport.contains(mBegin) || viewport.contains(mEnd) )
    {
      // If the points are visible we draw them
      qreal lineWidth  = GeneralConfig::instance()->getTaskFiguresLineWidth();
      QColor& color    = GeneralConfig::instance()->getTaskFiguresColor();

      painter->save();
      painter->setPen(QPen(color, lineWidth));
      painter->drawLine( mBegin, mEnd );
      painter->restore();
    }
}

void TaskLine::drawRegionBoxes( QPainter* painter )
{
  QPolygon outbp;
  outbp << m_lineBegin << m_outboundLineBegin << m_outboundLineEnd << m_lineEnd;

  QPolygon inbp;
  inbp << m_inboundLine1Begin << m_inboundLine1End << m_inboundLine2End << m_inboundLine2Begin;

  // Map WGS84 points to map projection
  for(int i = 0; i < outbp.count(); i++ )
    {
      outbp.setPoint( i, _globalMapMatrix->wgsToMap(outbp.at(i)) );
    }

  // Map WGS84 points to map projection
  for( int i = 0; i < inbp.count(); i++ )
    {
      inbp.setPoint( i, _globalMapMatrix->wgsToMap(inbp.at(i)) );
    }

  // Map projection to map display
  QPolygon mOutP = _globalMapMatrix->map(outbp);
  QPolygon mInP  = _globalMapMatrix->map(inbp);

  painter->save();
  painter->setPen(QPen(Qt::black, 3));
  painter->setBrush( Qt::NoBrush );
  painter->drawPolygon( mInP );
  painter->drawPolygon( mOutP );
  painter->restore();
}
