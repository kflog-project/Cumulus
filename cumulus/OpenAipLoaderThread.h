/***********************************************************************
**
**   OpenAipLoaderThread.h
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

#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QThread>

#include "airfield.h"
#include "radiopoint.h"
#include "singlepoint.h"
#include "ThermalPoint.h"

/**
* \class OpenAipLoaderThread
*
* \author Axel Pauli
*
* \brief Class to read OpenAIP data files in an extra thread.
*
* This class can read, parse and filter OpenAIP POI data files and store
* the content in a binary format. All work is done in an extra thread.
* The results are returned via the signal \ref loadedList.
*
* \date 2014-2022
*
* \version 1.1
*/

class OpenAipLoaderThread : public QThread
{
  Q_OBJECT

 public:

  /**
   * POI Source definitions
   */
  enum Poi { Airfields, Hotspots, NavAids, Reportings };

  OpenAipLoaderThread( QObject *parent,
                       enum Poi poiSource,
                       bool readSource=false );

  virtual ~OpenAipLoaderThread();

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();

 signals:

  /**
  * This signal emits the results of the OpenAIP load. The receiver slot is
  * responsible to delete the dynamic allocated list in every case.
  *
  * \param loadedLists  The number of loaded lists
  * \param airfieldList The list with the POI data
  *
  */
  void loadedAfList( int loadedLists,
                     QList<Airfield>* airfieldList );

  /**
  * This signal emits the results of the OpenAIP load. The receiver slot is
  * responsible to delete the dynamic allocated list in every case.
  *
  * \param loadedLists  The number of loaded lists
  * \param navAidList   The list with the POI data
  *
  */
  void loadedNavAidList( int loadedLists, QList<RadioPoint>* navAidList );

  /**
  * This signal emits the results of the OpenAIP load. The receiver slot is
  * responsible to delete the dynamic allocated list in every case.
  *
  * \param loadedLists  The number of loaded lists
  * \param hotspotList  The list with the POI data
  *
  */
  void loadedHotspotList( int loadedLists, QList<ThermalPoint>* hotspotList );

  /**
  * This signal emits the results of the OpenAIP load. The receiver slot is
  * responsible to delete the dynamic allocated list in every case.
  *
  * \param loadedLists The number of loaded lists
  * \param spList      The list with the POI data
  *
  */
  void loadedReportingPointList( int loadedLists, QList<SinglePoint>* spList );

 private:

  enum Poi m_poiSource;
  bool m_readSource;
};
