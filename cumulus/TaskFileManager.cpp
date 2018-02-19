/***********************************************************************
**
**   taskfilemanager.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013-2018 Axel Pauli
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
#include "TaskFileManager.h"
#include "taskfilemanager.h"

extern MapMatrix *_globalMapMatrix;

#define UPGRADE_CHECK ".task_upgrade_done"

TaskFileManager::TaskFileManager( const int tas ) :
  m_tas( tas )
{
  // Sets the default task directory
  m_taskFileDirectory = GeneralConfig::instance()->getUserDataDirectory() + "/tasks";
  createTaskDirectory();
}

void TaskFileManager::check4Upgrade()
{
  QString check = m_taskFileDirectory + "/" + UPGRADE_CHECK;
  QDir dir( m_taskFileDirectory );

  if( dir.exists() && QFileInfo(check).exists() == true )
    {
      // upgrade has been done already
      return;
    }

  TaskFileManagerOld tfm;
  QList<FlightTask*> ftl;

  bool ret = tfm.loadTaskListNew( ftl );

  if( ret == false )
    {
      return;
    }

  qDebug() << "TaskFileManager executes upgrade to new structure.";

  saveTaskList( ftl );

  QFile file( check);

  if( !file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
      return;
    }

  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  QTextStream out(&file);
  out << "Task file migration done at " << dtStr << "\n";
  file.close();
}

/**

 Task file syntax:

# A hashmark stands for a comment line

A task file consists of the TS key, several TW keys and a TE key. A pipe symbol
is used as separator between the elements on a single line.

Example:

TS|<TaskName>|<No of task points>
TW|<Latitude>|<Longitude>|<Elevation>|<WpName>|<LongName>|<Waypoint-type>|
   <ActiveTaskPointFigureScheme>|<TaskLineLength>|
   <TaskCircleRadius>|<TaskSectorInnerRadius>|<TaskSectorOuterRadius>|
   <TaskSectorAngle>
...
TE

--------------------------------------------------------------------------------
# Cumulus-Task-File V5.0, created at 2016-01-25 20:50:15 by Cumulus 5.26.0

TS|500 Diamant|4
TW|31488167|8450167|67|Eggersdo|Eggersdorf Muenc|61|1|1000|500|0|3000|90|1|0
TW|31201333|7291667|80|Zerbst|Zerbst|61|1|1000|500|0|3000|90|1|0
TW|30695333|8970167|238|Goerlitz|Goerlitz|61|1|1000|500|0|3000|90|1|0
TW|31488167|8450167|67|Eggersdo|Eggersdorf Muenc|61|1|1000|500|0|3000|90|1|0
TE
--------------------------------------------------------------------------------
*/
bool TaskFileManager::loadTaskList( QList<FlightTask*>& flightTaskList )
{
  while( ! flightTaskList.isEmpty() )
    {
      // Clears the list as first.
      delete flightTaskList.takeFirst();
    }

  QDir dir( m_taskFileDirectory );

  QStringList filters;

  // File extension of task files
  filters << "*.tsk";

  if( dir.exists() )
  {
    // Look, if task files are to find in this directory
    QStringList taskList = dir.entryList( filters, QDir::Files, QDir::Name );

    if( taskList.isEmpty() )
      {
        return true;
      }

    for( int i = 0; i < taskList.size(); i++ )
      {
        // Read in all found task files.
        FlightTask* ft;

        ft = readTaskFile( taskList.at(i) );

        if( ft != static_cast<FlightTask *>(0) )
          {
            flightTaskList.append( ft );
          }
      }

    return true;
  }

  return false;
}

void TaskFileManager::removeTaskFile( QString taskName )
{
  qDebug() << "TaskFileManager::removeTaskFile():" << taskName;

  if( taskName.isEmpty() )
    {
      return;
    }

  QString task = m_taskFileDirectory + "/" + taskName + ".tsk";

  QFile::remove( task + ".bak" );
  QFile::remove( task );
}

FlightTask* TaskFileManager::readTaskFile( QString taskName )
{
  qDebug() << "TaskFileManager::readTaskFile():" << taskName;

  if( taskName.isEmpty() )
    {
      return static_cast<FlightTask *>(0);
    }

  QString task = m_taskFileDirectory + "/" + taskName;

  if( taskName.endsWith( ".tsk") == false )
    {
      task += ".tsk";
    }

  QFile file( task );

  if ( file.open( QIODevice::ReadOnly ) == false )
    {
      qWarning() << __FUNCTION__ << "Could not open task-file:" << task;
      return static_cast<FlightTask *>(0);
    }

  QTextStream stream( &file );

  bool isTask = false;

  QString name;
  QStringList tmpList;
  QList<TaskPoint *> *tpList = 0;
  FlightTask* ft = static_cast<FlightTask *>(0);

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

          tpList = new QList<TaskPoint *>;

          tmpList = line.split( "|", QString::KeepEmptyParts );

          if( tmpList.size() < 2 ) continue;

          name = tmpList.at(1);
        }
      else
        {
          if ( line.mid( 0, 2 ) == "TW" && isTask )
            {
              // new task point
              TaskPoint* tp = new TaskPoint;
              tpList->append( tp );

              tmpList = line.split( "|", QString::KeepEmptyParts );

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

              // Check active task figure schema
              switch( tp->getActiveTaskPointFigureScheme() )
              {
                case GeneralConfig::Circle:
                case GeneralConfig::Sector:
                case GeneralConfig::Line:
                case GeneralConfig::Keyhole:
                  break;
                default:
                  tp->setActiveTaskPointFigureScheme( GeneralConfig::Circle );
                  break;
              }
            }
          else
            {
              if ( line.mid( 0, 2 ) == "TE" && isTask )
                {
                  ft = new FlightTask( tpList, true, name, m_tas );
                  break;
                }
            }
        }
    }

  if( ft == 0 && tpList != 0 )
    {
      // remove all elements from an incomplete task
      qDeleteAll(*tpList);
      delete tpList;
    }

  file.close();
  return ft;
}

QStringList TaskFileManager::getTaskListNames()
{
  QDir dir( m_taskFileDirectory );

  QStringList taskList;
  QStringList filters;

  // File extension of task files
  filters << "*.tsk";

  if( dir.exists() )
  {
    // Look, if task files are to find in this directory
    taskList = dir.entryList( filters, QDir::Files, QDir::Name );
    taskList.sort();
  }

  return taskList;
}

bool TaskFileManager::saveTaskList( QList<FlightTask*>& flightTaskList )
{
  for ( int i=0; i < flightTaskList.count(); i++ )
    {
      FlightTask *task = flightTaskList.at(i);
      writeTaskFile( task );
    }

  return true;
}

bool TaskFileManager::writeTaskFile( FlightTask *task )
{
  qDebug() << "TaskFileManager::writeTaskFile():" << task;

  if( task == 0 && task->getTaskName().isEmpty() )
    {
      return false;
    }

  QString tfn = m_taskFileDirectory + "/" + task->getTaskName() + ".tsk";

  // Save one backup copy. An old backup must be remove before rename otherwise
  // rename fails.
  if( QFileInfo(tfn).exists() )
    {
      QFile::remove( tfn + ".bak" );
      QFile::rename( tfn, tfn + ".bak" );
    }

  QFile f( tfn );

  if ( ! f.open( QIODevice::WriteOnly ) )
    {
      qWarning() << __PRETTY_FUNCTION__ << "Could not create task-file:" << tfn;
      return false;
    }

  QTextStream stream( &f );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "# Cumulus-Task-File V5.0, created at "
         << dtStr << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl
         << "# Task name: " << task->getTaskName() <<endl << endl;

  QList<TaskPoint *> tpList = task->getTpList();

  stream << "TS|" << task->getTaskName() << "|" << tpList.count() << endl;

  for ( int j=0; j < tpList.count(); j++ )
    {
      // saving each task point ...
      TaskPoint* tp = tpList.at(j);
      stream << "TW|"
             << tp->getWGSPosition().x() << "|"
             << tp->getWGSPosition().y() << "|"
             << tp->getElevation() << "|"
             << tp->getWPName() << "|"
             << tp->getName() << "|"
             << tp->getTypeID() << "|"
             << tp->getActiveTaskPointFigureScheme() << "|"
             << tp->getTaskLineLength().getMeters() << "|"
             << tp->getTaskCircleRadius().getMeters() << "|"
             << tp->getTaskSectorInnerRadius().getMeters() << "|"
             << tp->getTaskSectorOuterRadius().getMeters() << "|"
             << tp->getTaskSectorAngle() << "|"
             << tp->getAutoZoom() << "|"
             << tp->getUserEditFlag()
             << endl;
    }

  stream << "TE" << endl;

  f.close();
  return true;
}
