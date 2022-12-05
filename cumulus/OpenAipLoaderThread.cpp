/***********************************************************************
**
**   OpenAipLoaderThread.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014-2022 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <csignal>

#include <QtCore>

#include "OpenAipPoiLoader.h"
#include "OpenAipLoaderThread.h"

OpenAipLoaderThread::OpenAipLoaderThread( QObject *parent,
                                          enum Poi poiSource,
                                          bool readSource ) :
  QThread( parent ),
  m_poiSource(poiSource),
  m_readSource(readSource)
{
  setObjectName( "OpenAipLoaderThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

OpenAipLoaderThread::~OpenAipLoaderThread()
{
}

void OpenAipLoaderThread::run()
{
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  if( m_poiSource == Airfields )
    {
      // Check if signal is connected to a slot.
      if( receivers( SIGNAL( loadedAfList( int, QList<Airfield>*, QList<Airfield>* )) ) == 0 )
        {
          qWarning() << "OpenAipLoaderThread: No Slot connection to Signal loadedAfList!";
          return;
        }
    }
  else if( m_poiSource == Hotspots )
    {
      // Check if signal is connected to a slot.
      if( receivers( SIGNAL( loadedHotspotList( int, QList<ThermalPoint>* )) ) == 0 )
        {
          qWarning() << "OpenAipLoaderThread: No Slot connection to Signal loadedHotspotList!";
          return;
        }
    }
  else if( m_poiSource == NavAids )
    {
      // Check if signal is connected to a slot.
      if( receivers( SIGNAL( loadedNavAidList( int, QList<RadioPoint>* )) ) == 0 )
        {
          qWarning() << "OpenAipLoaderThread: No Slot connection to Signal loadedNavAidList!";
          return;
        }
    }
  else if( m_poiSource == Reportings )
    {
      // Check if signal is connected to a slot.
      if( receivers( SIGNAL( loadedReportingPointList( int, QList<SinglePoint>* )) ) == 0 )
        {
          qWarning() << "OpenAipLoaderThread: No Slot connection to Signal loadedReportingPointList!";
          return;
        }
    }

  OpenAipPoiLoader oaipl;
  int ok = false;

  if( m_poiSource == Airfields )
    {
      QList<Airfield>* afList = new QList<Airfield>;

      ok = oaipl.load( *afList, m_readSource );

      /* It is expected that a receiver slot is connected to this signal. The
       * receiver is responsible to delete the passed lists. Otherwise a big
       * memory leak will occur.
       */
      emit loadedAfList( ok, afList );
    }
  else if( m_poiSource == Hotspots )
    {
      QList<ThermalPoint>* poiList = new QList<ThermalPoint>;

      ok = oaipl.load( *poiList, m_readSource );

      /* It is expected that a receiver slot is connected to this signal. The
       * receiver is responsible to delete the passed list. Otherwise a big
       * memory leak will occur.
       */
      emit loadedHotspotList( ok, poiList );
    }
  else if( m_poiSource == NavAids )
    {
      QList<RadioPoint>* poiList = new QList<RadioPoint>;

      ok = oaipl.load( *poiList, m_readSource );

      /* It is expected that a receiver slot is connected to this signal. The
       * receiver is responsible to delete the passed list. Otherwise a big
       * memory leak will occur.
       */
      emit loadedNavAidList( ok, poiList );
    }
  else if( m_poiSource == Reportings )
    {
      QList<SinglePoint>* poiList = new QList<SinglePoint>;

      ok = oaipl.load( "*_rpp.json",
                       BaseMapElement::CompPoint,
                       *poiList,
                       m_readSource );

      /* It is expected that a receiver slot is connected to this signal. The
       * receiver is responsible to delete the passed list. Otherwise a big
       * memory leak will occur.
       */
      emit loadedReportingPointList( ok, poiList );
    }
}
