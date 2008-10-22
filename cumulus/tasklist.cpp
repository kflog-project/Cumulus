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
#include <QList>

#include "target.h"
#include "tasklist.h"
#include "generalconfig.h"
#include "mapmatrix.h"
#include "mapcontents.h"
#include "taskeditor.h"
#include "distance.h"
#include "speed.h"

// Initialize static member
uint TaskList::lastSelection = 0;

TaskList::TaskList( QWidget* parent ) :
  QWidget( parent ),
  editTask(0)
{
  setObjectName("TaskList");
  QVBoxLayout* taskLayout = new QVBoxLayout( this );
  taskLayout->setSpacing(5);
  taskLayout->setMargin(5);

  QHBoxLayout* editrow = new QHBoxLayout;
  editrow->setSpacing(2);
  taskLayout->addLayout( editrow );

  cruisingSpeed = new QSpinBox( this );
  cruisingSpeed->setButtonSymbols(QSpinBox::PlusMinus);
  cruisingSpeed->setRange( 0, 1000);
  cruisingSpeed->setSingleStep( 5 );
  cruisingSpeed->setValue( GeneralConfig::instance()->getCruisingSpeed() );
  cruisingSpeed->setSuffix( QString(" ") + Speed::getHorizontalUnitText() );

  editrow->addSpacing(10);
  editrow->addStretch(10);

  QPushButton * cmdNew = new QPushButton(this);
  cmdNew->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("add.png")) );
  cmdNew->setIconSize(QSize(26,26));
  editrow->addWidget(cmdNew,1);

  editrow->addSpacing(10);
  QPushButton * cmdEdit = new QPushButton(this);
  cmdEdit->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  cmdEdit->setIconSize(QSize(26,26));
  editrow->addWidget(cmdEdit,1);

  editrow->addSpacing(10);
  QPushButton * cmdDel = new QPushButton(this);
  cmdDel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("delete.png")) );
  cmdDel->setIconSize(QSize(26,26));
  editrow->addWidget(cmdDel,1);

  splitter = new QSplitter( Qt::Vertical, this );
  splitter->setOpaqueResize( true );
  splitter->setHandleWidth(10);

  taskListWidget = new QTreeWidget( splitter );

  taskListWidget->setRootIsDecorated(false);
  taskListWidget->setItemsExpandable(false);
  taskListWidget->setUniformRowHeights(true);
  taskListWidget->setSortingEnabled(true);
  taskListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  taskListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  taskListWidget->setColumnCount(4);

  QStringList sl;
  sl << "No." << "Name" << "Type" << "Distance";
  taskListWidget->setHeaderLabels(sl);

  taskListWidget->setColumnWidth( 0, 70 );
  taskListWidget->setColumnWidth( 1, 222 );
  taskListWidget->setColumnWidth( 2, 200 );

  taskListWidget->setFocus();

  taskContent = new TaskListView( splitter, false );

  taskLayout->addWidget( splitter );

  connect(cmdNew, SIGNAL(clicked()), this, SLOT(slotNewTask()));
  connect(cmdEdit, SIGNAL(clicked()), this, SLOT(slotEditTask()));
  connect(cmdDel, SIGNAL(clicked()), this, SLOT(slotDeleteTask()));
  connect(cruisingSpeed, SIGNAL(valueChanged(int)), this, SLOT(slotCruisingSpeedChanged(int)));

  connect( taskListWidget, SIGNAL( itemSelectionChanged() ),
           this, SLOT( slotTaskDetails() ) );

  if( ! slotLoadTask() ) {
    return;
  }
}


TaskList::~TaskList()
{
  // qDebug("TaskList::~TaskList()");
  qDeleteAll(taskList);
}

void TaskList::showEvent(QShowEvent *)
{
  static bool first = true;

  // @AP: With the first show event we set the splitter line to our
  // desired place. Found no other way to do it better.

  if( first )
    {
      first = false;

      // get the heights of the two widgets in the splitter
      QList<int> sizeList = splitter->sizes();

      int sum = sizeList[0] + sizeList[1];

      if( sum >= 200 )
        {
          sizeList[0] = 150;
          sizeList[1] = sum-150;

          // set the splitter line to a new place
          splitter->setSizes(sizeList);
        }
    }
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
  QList<QTreeWidgetItem*> selectList = taskListWidget->selectedItems();
  if( selectList.size() == 0 )
    return;

  QTreeWidgetItem * selected = taskListWidget->selectedItems().at(0);

  if ( selected->text( 0 ) == " " ) {
    if ( selected->text( 1 ) == tr("(No tasks defined)") ) {
      QMessageBox::information( this,
                        tr("Create New Task"),
                        tr("Push <b>Plus</b> button to add a task") );
    }

    taskContent->clear();
    return;
  }

  int id = selected->text( 0 ).toInt() - 1;

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

  QList<QTreeWidgetItem*> selectList = taskListWidget->selectedItems();
  if( selectList.size() == 0 )
    return (FlightTask *) 0;

  QString id( taskListWidget->selectedItems().at(0)->text(0) );

  // Special handling for entries with no number, they are system
  // specific.
  if (id==" ") {
    lastSelection = 0;
    return 0;
  }

  // qDebug("selected Item=%s",id.toLatin1().data());
  lastSelection = id.toUInt();

  // Nice trick, take selected element from list to prevent deletion of it, if
  // destruction of list is called.
  int index = id.toInt() - 1;
  return taskList.takeAt( index );
}


/** load tasks from file*/
bool TaskList::slotLoadTask()
{
  extern MapMatrix * _globalMapMatrix;
  QStringList rowList;

  while( !taskList.isEmpty() ) delete taskList.takeFirst();
  taskNames.clear();

#warning task list file 'tasks.tsk' is stored at User Data Directory

  // currently hardcoded file name
  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/tasks.tsk" );

  if( !f.open( QIODevice::ReadOnly ) ) {
    // could not read file ...
    rowList << " " << tr("(No tasks defined)");
    taskListWidget->addTopLevelItem( new QTreeWidgetItem(taskListWidget, rowList, 0) );
    taskListWidget->setCurrentItem( taskListWidget->itemAt(0,taskListWidget->topLevelItemCount()-1) );
    taskListWidget->sortItems( 0, Qt::AscendingOrder );
    return false;
  }

  QTextStream stream( &f );
  QString line;
  bool isTask( false );
  QString numTask, taskName;
  QStringList tmpList;
  QList<wayPoint*> *wpList = 0;

  while( !stream.atEnd() ) {
    line = stream.readLine();

    if( line.mid( 0, 1 ) == "#" )
      continue;

    if( line.mid( 0, 2 ) == "TS" ) {
      // new task ...
      isTask = true;

      if( wpList != 0 ) {
        // remove all elements from previous uncomplete step
        qDeleteAll(*wpList);
        wpList->clear();
      } else {
        wpList = new QList<wayPoint*>;
      }

      tmpList = line.split( ",", QString::KeepEmptyParts );
      taskName = tmpList.at(1);
    } else {
      if( line.mid( 0, 2 ) == "TW" && isTask ) {
        // new waypoint
        wayPoint* wp = new wayPoint;
        wpList->append( wp );

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
      } else {
        if( line.mid( 0, 2 ) == "TE" && isTask ) {
          // task complete
          isTask = false;
          FlightTask* task = new FlightTask( wpList, true,
                                             taskName, cruisingSpeed->value() );
          taskList.append( task );

          wpList = 0; // ownership was overtaken by FlighTask
          numTask.sprintf( "%02d", taskList.count() );

          rowList << numTask
                  << taskName
                  << task->getTaskTypeString()
                  << task->getTaskDistanceString();

          taskListWidget->addTopLevelItem( new QTreeWidgetItem(taskListWidget, rowList, 0) );
          rowList.clear();

          // save task name
          taskNames << taskName;
        }
      }
    }
  }

  f.close();

  if( wpList != 0 ) {
    // remove all elements from previous uncomplete step
    qDeleteAll(*wpList);
    delete wpList;
  }

  if ( taskList.count() == 0 ) {
    rowList << " " << tr("(No tasks defined)");
  } else {
    rowList << " " << tr("(Reset selection)") << tr("none");
    taskListWidget->setCurrentItem( taskListWidget->itemAt(0,0) );
  }
  taskListWidget->addTopLevelItem( new QTreeWidgetItem(taskListWidget, rowList, 0) );
  taskListWidget->sortByColumn(0, Qt::AscendingOrder);

  return true;
}


void TaskList::slotNewTask()
{
  TaskEditor *te = new TaskEditor(this, taskNames);

  connect( te, SIGNAL(newTask( FlightTask * )), this,
           SLOT(slotUpdateTaskList( FlightTask * )));

  te->show();
}


/**
 * taking over a new flight task from editor
 */
void TaskList::slotUpdateTaskList( FlightTask *newTask)
{
  taskList.append( newTask );
  saveTaskList();
  taskContent->clear();
  taskListWidget->clear();
  slotLoadTask();
}


/**
 * pass the selected task to the editor
 */
void TaskList::slotEditTask()
{
  // fetch selected task item
  QList<QTreeWidgetItem*> selectList = taskListWidget->selectedItems();

  if( selectList.size() == 0 )
    return;

  QString id( taskListWidget->selectedItems().at(0)->text(0) );

  if( id == " ") {
    return;
  }

  lastSelection = id.toUInt();
  editTask = taskList.at(id.toInt() - 1);

  // make a deep copy of fetched task item
  FlightTask* modTask = new FlightTask( editTask->getCopiedWPList(),
                                        true,
                                        editTask->getTaskName() );

  TaskEditor *te = new TaskEditor(this, taskNames, modTask  );

  connect( te, SIGNAL(editedTask( FlightTask * )),
           this, SLOT(slotEditTaskList( FlightTask * )));

#ifdef MAEMO
  te->showMaximized();
#else
  te->show();
#endif

}


/**
 * taking over an edited flight task from editor
 */
void TaskList::slotEditTaskList( FlightTask *editedTask)
{
  // qDebug("TaskList::slotEditTaskList()");

  // search task item being edited
  int index = taskList.indexOf( editTask );

  if( index != -1 )
    {
      // remove old item
      delete taskList.takeAt( index );
      // put new item on old position
      taskList.insert( index, editedTask );
    }
  else
    {
      // no old position available, append it at end of list
      taskList.append( editedTask );
    }

  saveTaskList();
  taskContent->clear();
  taskListWidget->clear();
  slotLoadTask();
}


/**
 * remove the selected task from the list
 */
void TaskList::slotDeleteTask()
{
  QTreeWidgetItem* selected = taskListWidget->currentItem();
  if( selected == 0 )
    return;

  QString id( selected->text(0) );

  if( id == " " ) {
    // Entries with no number are system specific and not deleteable
    return;
  }

  int answer= QMessageBox::question(this,tr("Delete Task"),
                         tr("Delete the selected task?"),
                         QMessageBox::No, QMessageBox::Yes );

  if( answer != QMessageBox::Yes ) {
    return;
  }

  delete taskListWidget->takeTopLevelItem( taskListWidget->currentIndex().row() );

  taskListWidget->sortItems( 0, Qt::AscendingOrder );
  taskListWidget->setCurrentItem( 0 );

  lastSelection = 0;

  // reset task
  extern MapContents* _globalMapContents;
  _globalMapContents->setCurrentTask(0);

  uint no = id.toUInt() - 1;
  delete taskList.takeAt( no );
  saveTaskList();
  taskContent->clear();
  taskListWidget->clear();
  slotLoadTask();
}


bool TaskList::saveTaskList()
{
  // currently hardcoded ...
  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/tasks.tsk" );

  if( !f.open( QIODevice::WriteOnly ) )
    {
      qWarning( "Could not write to task-file %s", f.fileName().toLatin1().data() );
      return false;
    }

  QTextStream stream( &f );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString(Qt::ISODate);

  stream << "# KFLog/Cumulus-Task-File created at "
         << dtStr << " by Cumulus " << CU_VERSION << endl;

  for( int i=0; i < taskList.count(); i++ )
    {
      FlightTask *task = taskList.at(i);
      QList<wayPoint*> wpList = task->getWPList();

      stream << "TS," << task->getTaskName() << "," << wpList.count() << endl;

      for( int j=0; j < wpList.count(); j++ )
        {
          // saving each taskpoint ...
          wayPoint* wp = wpList.at(j);
          stream << "TW," << wp->origP.x() << "," << wp->origP.y() << ","
          << wp->elevation << "," << wp->name << "," << wp->icao << ","
          << wp->description << "," << wp->frequency << ","
          << wp->comment << "," << wp->isLandable << "," << wp->runway << ","
          << wp->length << "," << wp->surface << "," << wp->type << endl;
        }

      stream << "TE" << endl;
    }

  f.close();

  return true;
}
