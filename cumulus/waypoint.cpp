/***********************************************************************
**
**   waypoint.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004-2022 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

// Magic of waypoint file with version number in lsb
#define WP_FILE_MAGIC 0x57504601

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
  frequencyList  = inst.frequencyList;
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

bool Waypoint::write( Waypoint* wp, const QString& fileName )
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

  out << quint32( WP_FILE_MAGIC );
  out << wp->name;
  out << (qint16) wp->type;
  out << (qint32) wp->wgsPoint.lat();
  out << (qint32) wp->wgsPoint.lon();
  out << wp->description;
  out << wp->icao;
  out << wp->comment;
  out << wp->elevation;
  out << (quint8) wp->priority;
  out << (qint16) wp->taskPointIndex;
  out << wp->country;
  out << wp->wpListMember;

  // The frequency list is saved
  Frequency::saveFrequencies( out, wp->getFrequencyList() );

  // The runway list is saved
  Runway::saveRunways(out, wp->getRunwayList() );

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

  if (fileMagic != WP_FILE_MAGIC)
    {
      file.close();
      return false;
    }

  wp->frequencyList.clear();
  wp->rwyList.clear();

  qint16 qint16v;
  qint32 qint32v;
  quint8 quint8v;
  QString qs;

  in >> wp->name;
  in >> qint16v;  wp->type = qint16v;
  in >> qint32v;  wp->wgsPoint.setLat(qint32v);
  in >> qint32v;  wp->wgsPoint.setLon(qint32v);
  in >> wp->description;
  in >> wp->icao;
  in >> wp->comment;
  in >> wp->elevation;
  in >> quint8v; wp->priority = (enum Priority) quint8v;
  in >> qint16v; wp->taskPointIndex = qint16v;
  in >> wp->country;
  in >> wp->wpListMember;

  // The frequency list is read
  Frequency::loadFrequencies( in, wp->getFrequencyList() );

  // The runway list is read
  Runway::loadRunways(in, wp->getRunwayList() );

  file.close();

  // Map the WGS84 coordinates to the current projection.
  wp->projPoint = _globalMapMatrix->wgsToMap(wp->wgsPoint);
  return true;
}
