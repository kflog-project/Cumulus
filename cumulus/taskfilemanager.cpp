/***********************************************************************
**
**   taskfilemanager.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013 Axel Pauli
**
**   Created on: 16.01.2013
**
**   Author: Axel Pauli <kflog.cumulus@gmail.com>
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: taskfilemanager.cpp 6055 2013-03-25 21:11:12Z axel $
**
***********************************************************************/

#include <QtCore>

#include "generalconfig.h"
#include "flighttask.h"
#include "mapmatrix.h"
#include "taskeditor.h"
#include "taskfilemanager.h"

extern MapMatrix *_globalMapMatrix;

#warning "Task list file 'tasks.tsk' is stored at User Data Directory"

TaskFileManager::TaskFileManager() :
 m_taskFileName( "" ),
 m_tas( 0 )
{
  // Sets the default task file name
  m_taskFileName = GeneralConfig::instance()->getUserDataDirectory() + "/tasks.tsk";
}

TaskFileManager::TaskFileManager( QString fileName, const int tas ) :
  m_taskFileName( fileName ),
  m_tas( tas )
{
}

/**

 Task file syntax:

# hashmark starts a comment line

A task consists of the TS key, several TW keys and a TE key.

Example:

TS,<TaskName>,<No of task points>
TW,<Latitude>,<Longitude>,<Elevation>,<Name>,<ICAO>,<Description>,<Frequency>,
   <Comment>,<landable><Runway-directions><Runway-length><Surface>
   <Waypoint-type>,<ActiveTaskPointFigureScheme>,<TaskLineLength>,
   <TaskCircleRadius>,<TaskSectorInnerRadius>,<TaskSectorOuterRadius>,
   <TaskSectorAngle>
...
TE

--------------------------------------------------------------------------------
# KFLog/Cumulus-Task-File created at 2013-01-17 11:14:36 by Cumulus 2.20.0

TS,Dreieck 1,6
TW,31488167,8450167,67,Eggersdo,EDCE,Eggersdorf Muenc,123,,1,1560,2240,1,5
TW,31488167,8450167,67,Eggersdo,EDCE,Eggersdorf Muenc,123,,1,1560,2240,1,5
TW,31567833,8545667,12,Neuharde,EDON,Neuhardenberg,119.125,,1,2074,2390,3,5
TW,31548000,8349333,78,Strausbe,EDAY,Strausberg,123.05,,1,1303,1200,3,5
TW,31488167,8450167,67,Eggersdo,EDCE,Eggersdorf Muenc,123,,1,1560,2240,1,5
TW,31488167,8450167,67,Eggersdo,EDCE,Eggersdorf Muenc,123,,1,1560,2240,1,5
TE
TS,EDCE-Dessau,5
TW,31488167,8450167,67,Eggersdo,EDCE,Eggersdorf Muenc,123,,1,1560,2240,1,5
TW,31488167,8450167,67,Eggersdo,EDCE,Eggersdorf Muenc,123,,1,1560,2240,1,5
TW,31369167,8281333,33,Frieders,EDCF,Friedersdorf,122.2,,1,2845,950,1,5
TW,31099000,7310667,56,Dessau,EDAD,Dessau,118.175,,1,2331,980,2,5
TW,31099000,7310667,56,Dessau,EDAD,Dessau,118.175,,1,2331,980,2,5
TE
--------------------------------------------------------------------------------

 */

bool TaskFileManager::loadTaskList( QList<FlightTask*>& flightTaskList,
                                    QString fileName )
{
  while( ! flightTaskList.isEmpty() )
    {
      // Clears the list as first.
      delete flightTaskList.takeFirst();
    }

  QString fn;

  if( fileName.isEmpty() )
    {
      // Use task default file name
      fn = m_taskFileName;
    }

  QFile f( fn );

  if ( ! f.open( QIODevice::ReadOnly ) )
    {
      qWarning() << __FUNCTION__ << "Could not open task-file:" << fn;
      return false;
    }

  QTextStream stream( &f );

  bool isTask    = false;
  bool isOldTask = false;
  QString taskName;
  QStringList tmpList;
  QList<TaskPoint *> *tpList = 0;

  while ( !stream.atEnd() )
    {
      QString line = stream.readLine().trimmed();

      if ( line.isEmpty() || line.mid( 0, 1 ) == "#" )
        {
          // Ignore empty and comment lines
          continue;
        }

      if ( line.mid( 0, 2 ) == "TS" )
        {
          // new task ...
          isTask    = true;
          isOldTask = false;

          if ( tpList != 0 )
            {
              // remove all elements from previous incomplete step
              qDeleteAll(*tpList);
              tpList->clear();
            }
          else
            {
              tpList = new QList<TaskPoint *>;
            }

          tmpList = line.split( ",", QString::KeepEmptyParts );
          taskName = tmpList.at(1);
        }
      else
        {
          if ( line.mid( 0, 2 ) == "TW" && isTask )
            {
              // new task point
              TaskPoint* tp = new TaskPoint;
              tpList->append( tp );

              tmpList = line.split( ",", QString::KeepEmptyParts );

              tp->wgsPoint.setLat( tmpList.at( 1 ).toInt() );
              tp->wgsPoint.setLon( tmpList.at( 2 ) .toInt() );
              tp->projPoint = _globalMapMatrix->wgsToMap( tp->wgsPoint );
              tp->elevation = tmpList.at( 3 ).toInt();
              tp->name = tmpList.at( 4 );
              tp->icao = tmpList.at( 5 );
              tp->description = tmpList.at( 6 );
              tp->frequency = tmpList.at( 7 ).toDouble();
              tp->comment = tmpList.at( 8 );
              tp->isLandable = tmpList.at( 9 ).toInt();
              tp->runway = tmpList.at( 10 ).toInt();
              tp->length = tmpList.at( 11 ).toInt();
              tp->surface = tmpList.at( 12 ).toInt();
              tp->type = tmpList.at( 13 ).toInt();

              if( tmpList.size() == 22 )
                {
                  // New task file version has been read
                  tp->setActiveTaskPointFigureScheme( static_cast<enum GeneralConfig::ActiveTaskFigureScheme> (tmpList.at( 14 ).toInt()) );
                  tp->setTaskLineLength( tmpList.at( 15 ).toDouble() );
                  tp->setTaskCircleRadius( tmpList.at( 16 ).toDouble() );
                  tp->setTaskSectorInnerRadius( tmpList.at( 17 ).toDouble() );
                  tp->setTaskSectorOuterRadius( tmpList.at( 18 ).toDouble() );
                  tp->setTaskSectorAngle( tmpList.at( 19 ).toInt() );
                  tp->setAutoZoom( tmpList.at( 20 ).toInt() > 0 ? true : false );
                  tp->setUserEditFlag( tmpList.at( 21 ).toInt() > 0 ? true : false );
                }
              else
                {
                  // Port old task file version. Missing values are set to
                  // configuration defaults.
                  tp->setConfigurationDefaults();
                  isOldTask = true;
                }
            }
          else
            {
              if ( line.mid( 0, 2 ) == "TE" && isTask )
                {
                  // task complete
                  isTask = false;

                  if( isOldTask )
                    {
                      // An old task format has been read. Convert it to the
                      // new format.
                      TaskEditor::setTaskPointFigureSchemas( *tpList );
                    }

                  FlightTask* task = new FlightTask( tpList, true, taskName, m_tas );
                  flightTaskList.append( task );

                  // ownership about the list was taken over by FlighTask
                  tpList = 0;
                }
            }
        }
    }

  if ( tpList != 0 )
    {
      // remove all elements from a previous incomplete step
      qDeleteAll(*tpList);
      delete tpList;
    }

  f.close();

  qDebug() << "TFM:" << flightTaskList.size()
           << "task objects read from file"
           << fn;

  return true;
}

bool TaskFileManager::saveTaskList( QList<FlightTask*>& flightTaskList,
                                    QString fileName )
{
  QString fn;

  if( fileName.isEmpty() )
    {
      // Use task default file name
      fn = m_taskFileName;
    }

  QFile f( fn );

  if ( ! f.open( QIODevice::WriteOnly ) )
    {
      qWarning() << __PRETTY_FUNCTION__ << "Could not write to task-file:" << fn;
      return false;
    }

  QTextStream stream( &f );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "# KFLog/Cumulus-Task-File V2.0, created at "
         << dtStr << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl << endl;

  for ( int i=0; i < flightTaskList.count(); i++ )
    {
      FlightTask *task = flightTaskList.at(i);
      QList<TaskPoint *> tpList = task->getTpList();

      stream << "TS," << task->getTaskName() << "," << tpList.count() << endl;

      for ( int j=0; j < tpList.count(); j++ )
        {
          // saving each task point ...
          TaskPoint* tp = tpList.at(j);
          stream << "TW,"
                 << tp->wgsPoint.x() << ","
                 << tp->wgsPoint.y() << ","
                 << tp->elevation << ","
                 << tp->name << ","
                 << tp->icao << ","
                 << tp->description << ","
                 << tp->frequency << ","
                 << tp->comment << ","
                 << tp->isLandable << ","
                 << tp->runway << ","
                 << tp->length << ","
                 << tp->surface << ","
                 << tp->type << ","
                 << tp->getActiveTaskPointFigureScheme() << ","
                 << tp->getTaskLineLength().getMeters() << ","
                 << tp->getTaskCircleRadius().getMeters() << ","
                 << tp->getTaskSectorInnerRadius().getMeters() << ","
                 << tp->getTaskSectorOuterRadius().getMeters() << ","
                 << tp->getTaskSectorAngle() << ","
                 << tp->getAutoZoom() << ","
                 << tp->getUserEditFlag()
                 << endl;
        }

      stream << "TE" << endl;
    }

  f.close();

  qDebug() << "TFM:" << flightTaskList.size()
           << "task objects saved to file"
           << fn;

  return true;
}
