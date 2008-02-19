/***********************************************************************
**
**   tasklist.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <limits.h>

#include <QSplitter>
#include <QDir>
#include <QTextStream>
#include <QPushButton>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "tasklist.h"
#include "generalconfig.h"
#include "mapmatrix.h"
#include "mapcontents.h"
#include "taskdialog.h"
#include "distance.h"
#include "speed.h"

// Initialize static member
uint TaskList::lastSelection = 0;


TaskList::TaskList( QWidget* parent, const char* name )
    : QWidget( parent, name ),
    editTask(0)
{
  taskList.setAutoDelete(true);

  QVBoxLayout* taskLayout = new QVBoxLayout( this, 5 );
  QHBoxLayout *editrow=new QHBoxLayout(taskLayout, 0);

  cruisingSpeed = new QSpinBox(0, 1000, 5, this, "CruisingSpeed" );
  cruisingSpeed->setButtonSymbols(QSpinBox::PlusMinus);
  editrow->addWidget(cruisingSpeed);
  editrow->addWidget(new QLabel( Speed::getHorizontalUnitText(), this));
  cruisingSpeed->setValue( GeneralConfig::instance()->getCruisingSpeed() );
  editrow->addSpacing(10);
  editrow->addStretch(10);

  QPushButton * cmdNew = new QPushButton(this);
  cmdNew->setPixmap(GeneralConfig::instance()->loadPixmap("new.png"));
  cmdNew->setFlat(true);
  editrow->addWidget(cmdNew);

  QPushButton * cmdEdit = new QPushButton(this);
  cmdEdit->setPixmap(GeneralConfig::instance()->loadPixmap("edit.png"));
  cmdEdit->setFlat(true);
  editrow->addWidget(cmdEdit);

  QPushButton * cmdDel = new QPushButton(this);
  cmdDel->setPixmap(GeneralConfig::instance()->loadPixmap("trash.png"));
  cmdDel->setFlat(true);
  editrow->addWidget(cmdDel);

  QSplitter* splitter = new QSplitter( Qt::Vertical, this, "Splitter" );
  splitter->setOpaqueResize( true );
  taskListView = new Q3ListView( splitter );

  taskListView->addColumn( tr("No") );
  taskListView->addColumn( tr("Name") );
  taskListView->addColumn( tr("Type") );
  taskListView->setColumnAlignment(taskListView->addColumn( tr("Distance") ), Qt::AlignRight);
  taskListView->setAllColumnsShowFocus( true );
  taskListView->setMinimumHeight(20);
  taskListView->setFocus();

  taskContent = new TaskListView( splitter, "TaskRouteDisplay", false, false );

  // taskContent->setMinimumSize(1,100); //somehow, the layout alone doesn't cut it.

  taskLayout->addWidget( splitter, 1 );

  connect(cmdNew, SIGNAL(clicked()), this, SLOT(slotNewTask()));
  connect(cmdEdit, SIGNAL(clicked()), this, SLOT(slotEditTask()));
  connect(cmdDel, SIGNAL(clicked()), this, SLOT(slotDeleteTask()));
  connect(cruisingSpeed, SIGNAL(valueChanged(int)), this, SLOT(slotCruisingSpeedChanged(int)));

  connect( taskListView, SIGNAL( selectionChanged() ),
           this, SLOT( slotTaskDetails() ) );

  if( ! slotLoadTask() )
    {
      return;
    }
}


TaskList::~TaskList()
{
  // qDebug("TaskList::~TaskList()");
}

/** new value in spin box set */
void TaskList::slotCruisingSpeedChanged( int /* value */ )
{
  // The user has changed the value in the cruising speed spin box. We
  // have to initiate an update of the task details.
  slotTaskDetails();
  return;
}

void TaskList::slotTaskDetails()
{
  if( taskListView->selectedItem() == 0 )
    {
      return;
    }

  if ((QString)taskListView->selectedItem()->text( 0 )==" ")
    {
      if (taskListView->selectedItem()->text( 1 )==tr("(No tasks defined)"))
        {
          QMessageBox::information( this, 
                                    tr("Defime a Task"),
                                    tr("Tip on editor icon, to define a task.") );
        };

      taskContent->clear();
      return;
    }

  int id( ((QString)taskListView->selectedItem()->text( 0 ) ).toInt() - 1 );

  FlightTask *task = taskList.at( id );

  // update speed, can be changed in the meantime by the user
  task->setSpeed( cruisingSpeed->value() );

  taskContent->slot_setTask( task );
}


// This method is called from PreFlightDialog::accept(), to take out
// the selected task from the task list. The ownership of the taken
// FlightTask object goes over the the caller. He has to delete the
// object!!!
FlightTask* TaskList::takeSelectedTask()
{
  // qDebug("TaskList::selectedTask()");

  // save last used cruising speed
  GeneralConfig::instance()->setCruisingSpeed( cruisingSpeed->value() );

  if( taskListView->selectedItem() == 0 )
    return (FlightTask *) 0;

  QString id( taskListView->selectedItem()->text(0) );

  // Special handling for entries with no number, they are system
  // specific.
  if (id==" ")
    {
      lastSelection = 0;
      return 0;
    }

  // qDebug("selected Item=%s",id.toLatin1().data());
  lastSelection = id.toUInt();

  // Nice trick, take selected element from list to prevent deletion of it, if
  // destruction of list is called.
  FlightTask* task = taskList.take( id.toInt() - 1 );

  return task;
}


/** load tasks from file*/
bool TaskList::slotLoadTask()
{
  extern MapMatrix * _globalMapMatrix;

  taskList.clear();
  taskNames.clear();

# warning task list file 'tasks.tsk' is stored  at $HOME/cumulus/tasks.tsk

  // currently hardcoded ...
  QFile f( QDir::homeDirPath() + "cumulus/tasks.tsk" );

  if( !f.open( IO_ReadOnly ) )
    {
      // could not read file ...
      taskListView->insertItem(new Q3ListViewItem( taskListView, " ",
                               tr("(No tasks defined)")));
      return false;
    }

  QTextStream stream( &f );
  QString line;
  bool isTask( false );
  QString numTask, taskName;
  QStringList tmpList;
  wayPoint* wp;
  Q3PtrList<wayPoint> *wpList = 0;

  while( !stream.atEnd() )
    {
      line = stream.readLine();

      if( line.mid( 0, 1 ) == "#" )
        continue;

      if( line.mid( 0, 2 ) == "TS" )
        {
          // new task ...
          isTask = true;

          if( wpList != 0 )
            {
              // remove all elements from previous uncomplete step
              wpList->clear();
            }
          else
            {
              wpList = new Q3PtrList<wayPoint>;
              wpList->setAutoDelete(true);
            }

          tmpList = line.split( ",", QString::KeepEmptyParts );
          taskName = tmpList.at(1);
        }
      else if( line.mid( 0, 2 ) == "TW" && isTask )
        {
          // new wp ...
          wpList->append( new wayPoint );
          wp = wpList->current();

          tmpList = line.split( ",", QString::KeepEmptyParts );

          wp->origP.setLat( tmpList.at( 1 ).toInt() );
          wp->origP.setLon( tmpList.at( 2 ) .toInt() );
          wp->projP = _globalMapMatrix->wgsToMap( wp->origP );
          wp->elevation = tmpList.at( 3 ).toInt();
          wp->name = tmpList.at( 4 );
          wp->icao = tmpList.at( 5 );
          wp->description = tmpList.at( 6 );
          wp->frequency = tmpList.at( 7 ).toDouble();
          wp->comment = tmpList.at( 8 );
          wp->isLandable = tmpList.at( 9 ).toInt();
          wp->runway = tmpList.at( 10 ).toInt();
          wp->length = tmpList.at( 11 ).toInt();
          wp->surface = tmpList.at( 12 ).toInt();
          wp->type = tmpList.at( 13 ).toInt();
        }
      else if( line.mid( 0, 2 ) == "TE" && isTask )
        {
          // task complete
          isTask = false;
          taskList.append( new FlightTask( wpList, true,
                                           taskName, cruisingSpeed->value() ) );

          wpList = 0; // ownership was overtaken by FlighTask
          numTask.sprintf( "%02d", taskList.count() );

          Q3ListViewItem *newItem = new Q3ListViewItem( taskListView, numTask,
                                    taskName, taskList.current()->getTaskTypeString(),
                                    taskList.current()->getTaskDistanceString() );

          taskListView->insertItem(newItem);
          // save task name
          taskNames << taskName;

          if( taskList.count() == lastSelection )
            {
              // restore last selection
              taskListView->setSelected( newItem, true );
            }
        }
    }

  f.close();

  if( wpList != 0 )
    {
      // remove all elements from previous uncomplete step
      wpList->clear();
      delete wpList;
    }

  if (taskList.count()==0)
    {
      taskListView->insertItem(new Q3ListViewItem( taskListView, " ", tr("(No tasks defined)")));
    }
  else
    {
      taskListView->insertItem(new Q3ListViewItem( taskListView, " ", tr("(Reset selection)"), tr("none")));
    }

  return true;
}


void TaskList::slotNewTask()
{
  TaskDialog *td = new TaskDialog(this, "TaskListDialog", taskNames);

  connect( td, SIGNAL(newTask( FlightTask * )), this,
           SLOT(slotUpdateTaskList( FlightTask * )));

  td->show();
}


/**
 * overtake a new flight task from editor
 */
void TaskList::slotUpdateTaskList( FlightTask *newTask)
{
  taskList.append( newTask );
  saveTaskList();
  taskContent->clear();
  taskListView->clear();
  slotLoadTask();
}


/**
 * pass the selected task to the editor
 */
void TaskList::slotEditTask()
{
  // fetch selected task item
  if( taskListView->selectedItem() == 0 )
    {
      return;
    }

  QString id( taskListView->selectedItem()->text(0) );

  if( id == " ")
    {
      return;
    }

  lastSelection = id.toUInt();
  editTask = taskList.at(id.toInt() - 1);

  // make a deep copy of fetched task item
  FlightTask* modTask = new FlightTask( editTask->getCopiedWPList(),
                                        true,
                                        editTask->getTaskName() );

  TaskDialog *td = new TaskDialog(this, "TaskListDialog", taskNames, modTask  );

  connect( td, SIGNAL(editedTask( FlightTask * )),
           this, SLOT(slotEditTaskList( FlightTask * )));

  td->show();
}


/**
 * overtake an edited flight task from editor
 */
void TaskList::slotEditTaskList( FlightTask *editedTask)
{
  // qDebug("TaskList::slotEditTaskList()");

  // search task item being edited
  int index = taskList.findRef( editTask );

  if( index != -1 )
    {
      // remove old item
      taskList.remove( (uint) index );
      // put new item on old position
      taskList.insert( (uint) index, editedTask );
    }
  else
    {
      // no old position available, append it at end of list
      taskList.append( editedTask );
    }

  saveTaskList();
  taskContent->clear();
  taskListView->clear();
  slotLoadTask();
}


/**
 * remove the selected task from the list
 */
void TaskList::slotDeleteTask()
{
  if( taskListView->selectedItem() == 0 )
    {
      return;
    }

  QString id( taskListView->selectedItem()->text(0) );

  if( id == " " )
    {
      // Entries with no number are system specific and not deleteable
      return;
    }

  int answer=
    QMessageBox::warning(this,tr("Delete task?"),
                         tr("<qt>"
                            "Do you want to delete the selected task?"
                            "</qt>"),
                         QMessageBox::Ok | QMessageBox::Default,
                         QMessageBox::Cancel | QMessageBox::Escape );

  if( answer != QMessageBox::Ok )
    {
      return;
    }

  lastSelection = 0;

  // reset task
  extern MapContents* _globalMapContents;
  _globalMapContents->setCurrentTask(0);

  uint no = id.toUInt() - 1;
  taskList.remove( no );
  saveTaskList();
  taskContent->clear();
  taskListView->clear();
  slotLoadTask();
}


bool TaskList::saveTaskList()
{
  // currently hardcoded ...
  QFile f( QDir::homeDirPath() + "/cumulus/tasks.tsk" );

  if( !f.open( IO_WriteOnly ) )
    {
      qWarning( "Could not write to task-file %s", f.name().toLatin1().data() );
      return false;
    }

  QTextStream stream( &f );

  // writing file-header
  stream << "# KFLog/Cumulus-Task-File" << endl;

  for( uint i=0; i < taskList.count(); i++ )
    {
      FlightTask *task = taskList.at(i);
      Q3PtrList<wayPoint> wpList = task->getWPList();

      stream << "TS," << task->getTaskName() << "," << wpList.count() << endl;

      for( uint j=0; j < wpList.count(); j++ )
        {
          // saving each waypoint ...
          wayPoint* wp = wpList.at(j);
          stream << "TW," << wp->origP.x() << "," << wp->origP.y() << ","
          << wp->elevation << "," << wp->name << "," << wp->icao << ","
          << wp->description << "," << wp->frequency << ","
          << wp->comment << "," << wp->isLandable << "," << wp->runway << ","
          << wp->length << "," << wp->surface << "," << wp->type << /* ","
                                << wp->taskPointType << */ endl;
          //AS: The taskpoint type is implied by the order of the entries, so removed.
        }

      stream << "TE" << endl;
    }

  f.close();

  return true;
}
