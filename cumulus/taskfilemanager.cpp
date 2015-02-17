/***********************************************************************
**
**   taskfilemanager.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013-2015 Axel Pauli
**
**   Created on: 16.01.2013
**
**   Author: Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
TW,<Latitude>,<Longitude>,<Elevation>,<WpName>,<LongName>,<Waypoint-type>,
   <ActiveTaskPointFigureScheme>,<TaskLineLength>,
   <TaskCircleRadius>,<TaskSectorInnerRadius>,<TaskSectorOuterRadius>,
   <TaskSectorAngle>
...
TE

--------------------------------------------------------------------------------
# KFLog/Cumulus-Task-File V3.0, created at 2013-05-03 14:39:49 by Cumulus 5.2.0

TS,500 Diamant,6
TW,31488167,8450167,67,Eggersdo,Eggersdorf Muenc,61,1,1000,500,0,3000,90,1,0
TW,31488167,8450167,67,Eggersdo,Eggersdorf Muenc,61,1,1000,500,0,3000,90,1,0
TW,31201333,7291667,80,Zerbst,Zerbst,61,1,1000,500,0,3000,90,1,0
TW,30695333,8970167,238,Goerlitz,Goerlitz,61,1,1000,500,0,3000,90,1,0
TW,31488167,8450167,67,Eggersdo,Eggersdorf Muenc,61,1,1000,500,0,3000,90,1,0
TW,31488167,8450167,67,Eggersdo,Eggersdorf Muenc,61,1,1000,500,0,3000,90,1,0
TE
TS,ul robin,6
TW,31488167,8450167,67,Eggersdo,Eggersdorf Muenc,61,0,0,500,0,0,0,1,0
TW,31488167,8450167,67,Eggersdo,Eggersdorf Muenc,61,2,2000,500,0,3000,90,1,1
TW,31140500,7917667,102,Reinsdor,Reinsdorf,61,1,0,500,0,3000,90,1,0
TW,31139833,7831833,85,Oehna Ze,Oehna Zellendorf,61,1,0,500,500,3000,90,1,1
TW,31488167,8450167,67,Eggersdo,Eggersdorf Muenc,61,2,1000,3000,0,3000,90,1,0
TW,31488167,8450167,67,Eggersdo,Eggersdorf Muenc,61,0,0,500,0,0,0,1,0
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

  // Read the first line to decide, which format version should be read.
  QString line = stream.readLine().trimmed();

  f.close();

  if( line.startsWith( "# KFLog/Cumulus-Task-File V2.0") ||
      line.startsWith( "# KFLog/Cumulus-Task-File created") ||
      line.startsWith( "TS," ) ||
      line.isEmpty() )
    {
      // Old format has to be read
      return loadTaskListOld( flightTaskList, fileName );
    }

  // New format has to be read
  return loadTaskListNew( flightTaskList, fileName );
}

bool TaskFileManager::loadTaskListOld( QList<FlightTask*>& flightTaskList,
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

              WGSPoint wgsp( tmpList.at( 1 ).toInt(), tmpList.at( 2 ) .toInt() );

              tp->setWGSPosition( wgsp );
              tp->setPosition( _globalMapMatrix->wgsToMap( wgsp ) );
              tp->setElevation( tmpList.at( 3 ).toInt() );
              tp->setWPName( tmpList.at( 4 ) );
              // tp->icao = tmpList.at( 5 );
              tp->setName( tmpList.at( 6 ) );
              // tp->frequency = tmpList.at( 7 ).toDouble();
              tp->setComment( "" );
              // tp->isLandable = tmpList.at( 9 ).toInt();
              // tp->runway = tmpList.at( 10 ).toInt();
              // tp->length = tmpList.at( 11 ).toInt();
              // tp->surface = tmpList.at( 12 ).toInt();
              // tp->type = tmpList.at( 13 ).toInt();
              tp->setTypeID( BaseMapElement::Turnpoint ) ;

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

bool TaskFileManager::loadTaskListNew( QList<FlightTask*>& flightTaskList,
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

  bool isTask = false;

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
          isTask = true;

          if ( tpList != 0 )
            {
              // remove all elements from previous incomplete steps
              qDeleteAll(*tpList);
              tpList->clear();
            }
          else
            {
              tpList = new QList<TaskPoint *>;
            }

          tmpList = line.split( ",", QString::KeepEmptyParts );

          if( tmpList.size() < 2 ) continue;

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

              if( tmpList.size() < 14 ) continue;

              WGSPoint wgsp( tmpList.at( 1 ).toInt(), tmpList.at( 2 ) .toInt() );

              int i = 3;

              tp->setWGSPosition( wgsp );
              tp->setPosition( _globalMapMatrix->wgsToMap( wgsp ) );
              tp->setElevation( tmpList.at( i++ ).toInt() );
              tp->setWPName( tmpList.at( i++ ) );
              tp->setName( tmpList.at( i++ ) );
              tp->setTypeID( (BaseMapElement::objectType) tmpList.at( i++ ).toShort() ) ;

              tp->setActiveTaskPointFigureScheme( static_cast<enum GeneralConfig::ActiveTaskFigureScheme> (tmpList.at( i++ ).toInt()) );
              tp->setTaskLineLength( tmpList.at( i++ ).toDouble() );
              tp->setTaskCircleRadius( tmpList.at( i++ ).toDouble() );
              tp->setTaskSectorInnerRadius( tmpList.at( i++ ).toDouble() );
              tp->setTaskSectorOuterRadius( tmpList.at( i++ ).toDouble() );
              tp->setTaskSectorAngle( tmpList.at( i++ ).toInt() );
              tp->setAutoZoom( tmpList.at( i++ ).toInt() > 0 ? true : false );
              tp->setUserEditFlag( tmpList.at( i++ ).toInt() > 0 ? true : false );
            }
          else
            {
              if ( line.mid( 0, 2 ) == "TE" && isTask )
                {
                  // task complete
                  isTask = false;

                  FlightTask* task = new FlightTask( tpList, true, taskName, m_tas );
                  flightTaskList.append( task );

                  // ownership about the list is taken over by FlighTask
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

FlightTask* TaskFileManager::loadTask( QString taskName, QString fileName )
{
  QList<FlightTask*> ftl;

  if( loadTaskList( ftl, fileName ) == false )
    {
      return 0;
    }

  FlightTask* ft = 0;

  // Search desired task by name in list.
  for( int i = 0; i < ftl.size(); i++ )
    {
      ft = ftl.at(i);

      if( ft->getTaskName() == taskName )
	{
	  // Take found element from the list.
	  ft = ftl.takeAt(i);
	  break;
	}
    }

  // Remove all allocated list members
  qDeleteAll(ftl);
  return ft;
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

  stream << "# KFLog/Cumulus-Task-File V3.0, created at "
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
                 << tp->getWGSPosition().x() << ","
                 << tp->getWGSPosition().y() << ","
                 << tp->getElevation() << ","
                 << tp->getWPName() << ","
                 << tp->getName() << ","
                 << tp->getTypeID() << ","
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
