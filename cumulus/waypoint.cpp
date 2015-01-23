/***********************************************************************
**
**   waypoint.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004-2015 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef KFLOG_FILE_MAGIC
#define KFLOG_FILE_MAGIC 0x404b464c
#endif

#include <QtCore>

#include "mapmatrix.h"
#include "waypoint.h"

extern MapMatrix* _globalMapMatrix;

Waypoint::Waypoint()
{
  name           = "";
  type           = 0;
  description    = "";
  comment        = "";
  country        = "";
  icao           = "";
  elevation      = 0;
  frequency      = 0.;
  priority       = Waypoint::Low;
  taskPointIndex = -1;
  wpListMember   = false;

  wgsPoint.setPos(0,0);
  projPoint.setX(0);
  projPoint.setY(0);
}

// Copy constructor
Waypoint::Waypoint(const Waypoint& inst)
{
  //   qDebug("Waypoint::Waypoint(const Waypoint& inst) name=%s, idx=%d",
  //          inst.name.toLatin1().data(), inst.taskPointIndex );
  name           = inst.name;
  type           = inst.type;
  wgsPoint       = inst.wgsPoint;
  projPoint      = inst.projPoint;
  description    = inst.description;
  icao           = inst.icao;
  comment        = inst.comment;
  country        = inst.country;
  elevation      = inst.elevation;
  frequency      = inst.frequency;
  priority       = inst.priority;
  taskPointIndex = inst.taskPointIndex;
  wpListMember   = inst.wpListMember;
  rwyList        = inst.rwyList;
}

Waypoint::~Waypoint()
{
  // qDebug("Waypoint::~Waypoint(): name=%s, %X", name.toLatin1().data(), (uint) this);
}

bool Waypoint::equals( const Waypoint *second ) const
{
  if( second == static_cast<Waypoint *>(0) )
    {
      return false;
    }

  if( this->name == second->name &&
      this->type == second->type &&
      this->description == second->description &&
      this->wgsPoint == second->wgsPoint &&
      this->taskPointIndex == second->taskPointIndex )
    {
      return true;
    }

  return false;
}

bool Waypoint::operator==( const Waypoint& second ) const
{
  if( name == second.name &&
      type == second.type &&
      description == second.description &&
      wgsPoint == second.wgsPoint &&
      taskPointIndex == second.taskPointIndex )
    {
      return true;
    }

  return false;
}

bool Waypoint::write( const Waypoint* wp, const QString& fileName )
{
  QFile file( fileName );

  if( wp == 0 )
    {
      // If waypoint is null, we try to remove the passed filename.
      return QFile::remove(fileName);
    }

  if( file.open( QIODevice::WriteOnly ) == false )
    {
      return false;
    }

  QDataStream out( &file );
  out.setVersion( QDataStream::Qt_4_8 );

  out << quint32( KFLOG_FILE_MAGIC );
  out << wp->name;
  out << (qint16) wp->type;
  out << (qint32) wp->wgsPoint.lat();
  out << (qint32) wp->wgsPoint.lon();
  out << wp->description;
  out << wp->icao;
  out << wp->comment;
  out << wp->elevation;
  out << wp->frequency;
  out << (quint8) wp->priority;
  out << (qint16) wp->taskPointIndex;
  out << wp->country;
  out << wp->wpListMember;

  // The runway list is saved
  out << quint8( wp->rwyList.size() );

  for( int i = 0; i < wp->rwyList.size(); i++ )
    {
      Runway rwy = wp->rwyList.at(i);

      out << rwy.m_length;
      out << rwy.m_width;
      out << quint16( rwy.m_heading );
      out << quint8( rwy.m_surface );
      out << quint8( rwy.m_isOpen );
      out << quint8( rwy.m_isBidirectional );
    }

  file.close();
  return true;
}

bool Waypoint::read( Waypoint* wp, const QString& fileName )
{
  if( wp == 0 || fileName.isEmpty() )
    {
      return false;
    }

  QFile file(fileName);

  if( ! file.exists() || file.size() == 0 )
    {
      return false;
    }

  if( file.open( QIODevice::ReadOnly ) == false )
    {
      return false;
    }

  QDataStream in( &file );
  in.setVersion( QDataStream::Qt_4_8 );

  quint32 fileMagic;

  //check if the file has the correct format
  in >> fileMagic;

  if (fileMagic != KFLOG_FILE_MAGIC)
    {
      file.close();
      return -1;
    }

  wp->rwyList.clear();

  qint16 qint16v;
  qint32 qint32v;
  quint8 quint8v;
  quint16 quint16v;

  in >> wp->name;
  in >> qint16v;  wp->type = qint16v;
  in >> qint32v;  wp->wgsPoint.setLat(qint32v);
  in >> qint32v;  wp->wgsPoint.setLon(qint32v);
  in >> wp->description;
  in >> wp->icao;
  in >> wp->comment;
  in >> wp->elevation;
  in >> wp->frequency;
  in >> quint8v; wp->priority = (enum Priority) quint8v;
  in >> qint16v; wp->taskPointIndex = qint16v;
  in >> wp->country;
  in >> wp->wpListMember;

  // The runway list is saved
  in >> quint8v;

  for( int i = 0; i < quint8v; i++ )
    {
      Runway rwy;

      in >> rwy.m_length;
      in >> rwy.m_width;
      in >> quint16v; rwy.m_heading = quint16v;
      in >> quint8v;  rwy.m_surface = quint8v;
      in >> quint8v;  rwy.m_isOpen = quint8v;
      in >> quint8v;  rwy.m_isBidirectional = quint8v;
    }

  // Map the WGS84 coordinates to the current projection.
  wp->projPoint = _globalMapMatrix->wgsToMap(wp->wgsPoint);
  return true;
}
