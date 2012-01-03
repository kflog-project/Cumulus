/***********************************************************************
**
**   preflighttasklist.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2009-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <limits.h>

#include <QtGui>

#include "target.h"
#include "preflighttasklist.h"
#include "generalconfig.h"
#include "mapmatrix.h"
#include "mapcontents.h"
#include "taskeditor.h"
#include "distance.h"
#include "speed.h"
#include "layout.h"
#include "wgspoint.h"

PreFlightTaskList::PreFlightTaskList( QWidget* parent ) :
  QWidget( parent ),
  editTask(0)
{
  setObjectName("TaskList");
  QVBoxLayout* taskLayout = new QVBoxLayout( this );
  taskLayout->setSpacing(5);
  taskLayout->setMargin(5);

  QHBoxLayout* editrow = new QHBoxLayout;
  editrow->setSpacing(5);
  taskLayout->addLayout( editrow );

  QLabel *label = new QLabel( tr("TAS"), this );
  editrow->addWidget(label);

  tas = new QSpinBox( this );
  tas->setToolTip( tr("True Air Speed") );
  tas->setButtonSymbols(QSpinBox::PlusMinus);
  tas->setRange( 0, 1000);
  tas->setSingleStep( 5 );
  tas->setValue( GeneralConfig::instance()->getTas() );
  tas->setSuffix( Speed::getHorizontalUnitText() );
  editrow->addWidget(tas);

  label = new QLabel( tr("WD"), this );
  editrow->addWidget(label);

  windDirection = new QSpinBox( this );
  windDirection->setToolTip( tr("Wind Direction") );
  windDirection->setButtonSymbols(QSpinBox::PlusMinus);
  windDirection->setRange( 0, 359 );
  windDirection->setWrapping(true);
  windDirection->setSingleStep( 10 );
  windDirection->setValue( GeneralConfig::instance()->getWindDirection() );
  windDirection->setSuffix( QString(Qt::Key_degree) );
  editrow->addWidget(windDirection);

  label = new QLabel( tr("WS"), this );
  editrow->addWidget(label);

  windSpeed = new QSpinBox( this );
  windSpeed->setToolTip( tr("Wind Speed") );
  windSpeed->setButtonSymbols(QSpinBox::PlusMinus);
  windSpeed->setRange( 0, 1000 );
  windSpeed->setValue( GeneralConfig::instance()->getWindSpeed() );
  windSpeed->setSuffix( Speed::getWindUnitText() );

  if( Speed::getWindUnit() != Speed::metersPerSecond )
    {
      windSpeed->setSingleStep( 5 );
    }

  editrow->addWidget(windSpeed);

  editrow->addStretch(10);

  QPushButton * cmdNew = new QPushButton(this);
  cmdNew->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("add.png")) );
  cmdNew->setIconSize(QSize(IconSize, IconSize));
  cmdNew->setToolTip(tr("Define a new task"));
  editrow->addWidget(cmdNew);

  editrow->addSpacing(10);
  QPushButton * cmdEdit = new QPushButton(this);
  cmdEdit->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  cmdEdit->setIconSize(QSize(IconSize, IconSize));
  cmdEdit->setToolTip(tr("Edit selected task"));
  editrow->addWidget(cmdEdit);

  editrow->addSpacing(10);
  QPushButton * cmdDel = new QPushButton(this);
  cmdDel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("delete.png")) );
  cmdDel->setIconSize(QSize(IconSize, IconSize));
  cmdDel->setToolTip(tr("Remove selected task"));
  editrow->addWidget(cmdDel);

  splitter = new QSplitter( Qt::Vertical, this );
  splitter->setOpaqueResize( true );
  splitter->setHandleWidth(10);

  taskListWidget = new QTreeWidget( splitter );

  taskListWidget->setToolTip( tr("Select a flight task") );
  taskListWidget->setRootIsDecorated(false);
  taskListWidget->setItemsExpandable(false);
  taskListWidget->setUniformRowHeights(true);
  taskListWidget->setSortingEnabled(true);
  taskListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  taskListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  taskListWidget->setColumnCount(4);

  QStringList sl;
  sl << tr("No.")
     << tr("Name")
     << tr("Type")
     << tr("Distance");

  taskListWidget->setHeaderLabels(sl);
  taskListWidget->setFocus();

  taskContent = new TaskListView( splitter, false );
  taskContent->setToolTip( tr("Task display") );

  taskLayout->addWidget( splitter );

  connect(cmdNew, SIGNAL(clicked()), this, SLOT(slotNewTask()));
  connect(cmdEdit, SIGNAL(clicked()), this, SLOT(slotEditTask()));
  connect(cmdDel, SIGNAL(clicked()), this, SLOT(slotDeleteTask()));
  connect(tas, SIGNAL(valueChanged(int)), this, SLOT(slotTasChanged(int)));
  connect(windDirection, SIGNAL(valueChanged(int)), this, SLOT(slotWDChanged(int)));
  connect(windSpeed, SIGNAL(valueChanged(int)), this, SLOT(slotWSChanged(int)));

  connect( taskListWidget, SIGNAL( itemSelectionChanged() ),
           this, SLOT( slotTaskDetails() ) );

  loadTaskList();
}

PreFlightTaskList::~PreFlightTaskList()
{
  // qDebug("PreFlightTaskList::~PreFlightTaskList()");
  qDeleteAll(taskList);
}

void PreFlightTaskList::showEvent(QShowEvent *)
{
  static bool first = true;

  // @AP: With the first show event we set the splitter line to our
  // desired place. Found no other way to do it better.
  if ( first )
    {
      first = false;

      // get the heights of the two widgets in the splitter
      QList<int> sizeList = splitter->sizes();

      int sum = sizeList[0] + sizeList[1];

      if ( sum >= 200 )
        {
          sizeList[0] = 150;
          sizeList[1] = sum-150;

          // set the splitter line to a new place
          splitter->setSizes(sizeList);
        }

      // Notice user how to add a new task
      QTreeWidgetItem* selected = taskListWidget->selectedItems().at(0);

      if ( selected->text( 0 ) == " " &&
           selected->text( 1 ) == tr("(No tasks defined)") )
        {
          QMessageBox::information( this,
                                    tr("Create New Task"),
                                    tr("Push <b>Plus</b> button to add a task") );
        }
    }

  taskListWidget->resizeColumnToContents(0);
  taskListWidget->resizeColumnToContents(1);
  taskListWidget->resizeColumnToContents(2);
}

/** new value set in TAS spin box */
void PreFlightTaskList::slotTasChanged( int /* value */ )
{
  // The user has changed the value in the TAS spin box. We
  // have to initiate an update of the task details.
  slotTaskDetails();
  return;
}

/** value in wind direction spin box has been changed, do update of task list. */
void PreFlightTaskList::slotWDChanged( int /* value */ )
{
  // The user has changed the value in the wind direction spin box. We
  // have to initiate an update of the task details.
  slotTaskDetails();
  return;
}

/** value in wind speed spin box has been changed, do update of task list. */
void PreFlightTaskList::slotWSChanged( int /* value */ )
{
  // The user has changed the value in the wind speed spin box. We
  // have to initiate an update of the task details.
  slotTaskDetails();
  return;
}

void PreFlightTaskList::slotTaskDetails()
{
  QList<QTreeWidgetItem*> selectList = taskListWidget->selectedItems();

  if ( selectList.size() == 0 )
    {
      return;
    }

  QTreeWidgetItem* selected = taskListWidget->selectedItems().at(0);

  if ( selected->text( 0 ) == " " )
    {
      taskContent->clear();
      return;
    }

  int id = selected->text( 0 ).toInt() - 1;

  FlightTask *task = taskList.at( id );

  // update TAS, can be changed in the meantime by the user
  task->setSpeed( tas->value() );

  // update wind parameters, can be changed in the meantime by the user
  task->setWindDirection( windDirection->value() );
  task->setWindSpeed( windSpeed->value() );

  taskContent->slot_setTask( task );
}

// This method is called from PreFlightWidget::accept(), to take out
// the selected task from the task list. The ownership of the taken
// FlightTask object goes over the the caller. He has to delete the
// object!!!
FlightTask* PreFlightTaskList::takeSelectedTask()
{
  // qDebug("PreFlightTaskList::selectedTask()");

  // save last used TAS, and wind parameters
  GeneralConfig::instance()->setTas( tas->value() );
  GeneralConfig::instance()->setWindDirection( windDirection->value() );
  GeneralConfig::instance()->setWindSpeed( windSpeed->value() );

  QList<QTreeWidgetItem*> selectList = taskListWidget->selectedItems();

  if ( selectList.size() == 0 )
    {
      return static_cast<FlightTask *> (0);
    }

  QString id( taskListWidget->selectedItems().at(0)->text(0) );

  // Special handling for entries with no number, they are system specific.
  if( id == " " )
    {
      GeneralConfig::instance()->setCurrentTask( "" );
      return static_cast<FlightTask *> (0);
    }

  // qDebug("selected Item=%s",id.toLatin1().data());
  GeneralConfig::instance()->setCurrentTask( taskListWidget->selectedItems().at(0)->text(1) );

  // Nice trick, take selected element from list to prevent deletion of it, if
  // destruction of list is called.
  int index = id.toInt() - 1;

  FlightTask* task = taskList.takeAt( index );

  // Save the task declaration in Flarm format as file.
  createFlarmTaskList( task );

  return task;
}

/** load tasks from file*/
bool PreFlightTaskList::loadTaskList()
{
  extern MapMatrix *_globalMapMatrix;
  QStringList rowList;

  while ( !taskList.isEmpty() )
    {
      delete taskList.takeFirst();
    }

  taskNames.clear();

#warning task list file 'tasks.tsk' is stored at User Data Directory

  // currently hard coded file name
  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/tasks.tsk" );

  if ( ! f.open( QIODevice::ReadOnly ) )
    {
      // could not read file ...
      rowList << " " << tr("(No tasks defined)");
      taskListWidget->addTopLevelItem( new QTreeWidgetItem(taskListWidget, rowList, 0) );
      taskListWidget->setCurrentItem( taskListWidget->itemAt(0,taskListWidget->topLevelItemCount()-1) );
      taskListWidget->sortItems( 0, Qt::AscendingOrder );

      // reset current task
      GeneralConfig::instance()->setCurrentTask( "" );

      taskListWidget->resizeColumnToContents(0);
      taskListWidget->resizeColumnToContents(1);
      taskListWidget->resizeColumnToContents(2);

      return false;
    }

  QTextStream stream( &f );
  QString line;
  bool isTask( false );
  QString numTask, taskName;
  QStringList tmpList;
  QList<TaskPoint *> *tpList = 0;

  while ( !stream.atEnd() )
    {
      line = stream.readLine();

      if ( line.mid( 0, 1 ) == "#" )
        {
          continue;
        }

      if ( line.mid( 0, 2 ) == "TS" )
        {
          // new task ...
          isTask = true;

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

              tp->origP.setLat( tmpList.at( 1 ).toInt() );
              tp->origP.setLon( tmpList.at( 2 ) .toInt() );
              tp->projP = _globalMapMatrix->wgsToMap( tp->origP );
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
            }
          else
            {
              if ( line.mid( 0, 2 ) == "TE" && isTask )
                {
                  // task complete
                  isTask = false;
                  FlightTask* task = new FlightTask( tpList, true,
                                                     taskName, tas->value() );
                  taskList.append( task );

                  tpList = 0; // ownership was overtaken by FlighTask
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

  if ( tpList != 0 )
    {
      // remove all elements from previous incomplete step
      qDeleteAll(*tpList);
      delete tpList;
    }

  if ( taskList.count() == 0 )
    {
      rowList << " " << tr("(No tasks defined)");

      // reset current task
      GeneralConfig::instance()->setCurrentTask( "" );
    }
  else
    {
      rowList << " " << tr("(Reset selection)") << tr("none");
    }

  taskListWidget->addTopLevelItem( new QTreeWidgetItem(taskListWidget, rowList, 0) );
  taskListWidget->sortByColumn(0, Qt::AscendingOrder);

  selectLastTask();

  taskListWidget->resizeColumnToContents(0);
  taskListWidget->resizeColumnToContents(1);
  taskListWidget->resizeColumnToContents(2);

  return true;
}

void PreFlightTaskList::slotNewTask()
{
  TaskEditor *te = new TaskEditor(this, taskNames);

  connect( te, SIGNAL(newTask( FlightTask * )), this,
           SLOT(slotUpdateTaskList( FlightTask * )));

  te->setVisible( true );
}

/**
 * taking over a new flight task from editor
 */
void PreFlightTaskList::slotUpdateTaskList( FlightTask *newTask)
{
  taskList.append( newTask );
  saveTaskList();
  taskContent->clear();
  taskListWidget->clear();
  loadTaskList();
}

/**
 * pass the selected task to the editor
 */
void PreFlightTaskList::slotEditTask()
{
  // fetch selected task item
  QList<QTreeWidgetItem*> selectList = taskListWidget->selectedItems();

  if ( selectList.size() == 0 )
    {
      return;
    }

  QString id( taskListWidget->selectedItems().at(0)->text(0) );

  if ( id == " ")
    {
      return;
    }

  editTask = taskList.at(id.toInt() - 1);

  // make a deep copy of fetched task item
  FlightTask* modTask = new FlightTask( editTask->getCopiedTpList(),
                                        true,
                                        editTask->getTaskName() );

  TaskEditor *te = new TaskEditor(this, taskNames, modTask  );

  connect( te, SIGNAL(editedTask( FlightTask * )),
           this, SLOT(slotEditTaskList( FlightTask * )));

  te->setVisible( true );
}

/**
 * taking over an edited flight task from editor
 */
void PreFlightTaskList::slotEditTaskList( FlightTask *editedTask)
{
  // qDebug("PreFlightTaskList::slotEditTaskList()");

  // search task item being edited
  int index = taskList.indexOf( editTask );

  if ( index != -1 )
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
  loadTaskList();
}

/**
 * remove the selected task from the list
 */
void PreFlightTaskList::slotDeleteTask()
{
  QTreeWidgetItem* selected = taskListWidget->currentItem();

  if ( selected == 0 )
    {
      return;
    }

  QString id( selected->text(0) );

  if ( id == " " )
    {
      // Entries with no number are system specific and not deleteable
      return;
    }

  int answer= QMessageBox::question(this,tr("Delete Task"),
                                    tr("Delete the selected task?"),
                                    QMessageBox::No, QMessageBox::Yes );

  if ( answer != QMessageBox::Yes )
    {
      return;
    }

  delete taskListWidget->takeTopLevelItem( taskListWidget->currentIndex().row() );

  taskListWidget->sortItems( 0, Qt::AscendingOrder );
  taskListWidget->setCurrentItem( taskListWidget->topLevelItem(0) );

  // reset last stored selected task
  GeneralConfig::instance()->setCurrentTask( "" );

  // reset task
  extern MapContents* _globalMapContents;
  _globalMapContents->setCurrentTask(0);

  uint no = id.toUInt() - 1;
  delete taskList.takeAt( no );
  saveTaskList();
  taskContent->clear();
  taskListWidget->clear();
  loadTaskList();
}

bool PreFlightTaskList::saveTaskList()
{
  // currently hard coded ...
  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/tasks.tsk" );

  if ( !f.open( QIODevice::WriteOnly ) )
    {
      qWarning( "Could not write to task-file %s", f.fileName().toLatin1().data() );
      return false;
    }

  QTextStream stream( &f );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "# KFLog/Cumulus-Task-File created at "
         << dtStr << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl;

  for ( int i=0; i < taskList.count(); i++ )
    {
      FlightTask *task = taskList.at(i);
      QList<TaskPoint *> tpList = task->getTpList();

      stream << "TS," << task->getTaskName() << "," << tpList.count() << endl;

      for ( int j=0; j < tpList.count(); j++ )
        {
          // saving each task point ...
          TaskPoint* tp = tpList.at(j);
          stream << "TW," << tp->origP.x() << "," << tp->origP.y() << ","
          << tp->elevation << "," << tp->name << "," << tp->icao << ","
          << tp->description << "," << tp->frequency << ","
          << tp->comment << "," << tp->isLandable << "," << tp->runway << ","
          << tp->length << "," << tp->surface << "," << tp->type << endl;
        }

      stream << "TE" << endl;
    }

  f.close();

  return true;
}

/** Creates a task definition file in Flarm format. */
bool PreFlightTaskList::createFlarmTaskList( FlightTask* flightTask )
{
  if( ! flightTask )
    {
      return false;
    }

  QList<TaskPoint *>& tpList = flightTask->getTpList();

  if( tpList.isEmpty() )
    {
      return false;
    }

  QString fn = GeneralConfig::instance()->getUserDataDirectory() + "/cumulus-flarm.tsk";

  // Save one backup copy.
  QFile::rename( fn, fn + ".bak" );

  QFile f(fn);

  if( !f.open( QIODevice::WriteOnly ) )
    {
      qWarning( "Could not write to task-file %s", f.fileName().toLatin1().data() );
      return false;
    }

  QTextStream stream( &f );
  stream.setCodec( "ISO 8859-15" );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "// Flarm task declaration created at "
         << dtStr
         << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl;

  stream << "$PFLAC,S,NEWTASK," << flightTask->getTaskName() << endl;

  for( int i = 0; i < tpList.count(); i++ )
    {
      // $PFLAC,S,ADDWP,4647900N,01252700E,Lienz Ni
      TaskPoint* tp = tpList.at(i);

      int degree, intMin;
      double min;

      WGSPoint::calcPos( tp->origP.x(), degree, min );

      // Minute is expected as 1/1000
      intMin = static_cast<int> (rint(min * 1000));

      QString lat = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 2, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("S") : QString("N") );

      WGSPoint::calcPos( tp->origP.y(), degree, min );

      intMin = static_cast<int> (rint(min * 1000));

      QString lon = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 3, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("W") : QString("E") );

      stream << "$PFLAC,S,ADDWP,"
             << lat
             << "," << lon << ","
             << tp->name << " - " << tp->description
             << endl;
    }

  stream << endl;
  f.close();

  return true;
}

/** Select the last stored task */
void PreFlightTaskList::selectLastTask()
{
  QString lastTask = GeneralConfig::instance()->getCurrentTask();

  int rows = taskListWidget->topLevelItemCount();

  for( int i = 0; i < rows; i++ )
    {
      QString taskName = taskListWidget->topLevelItem(i)->text(1);
      // qDebug( "taskName(%d)=%s", i, taskName.toLatin1().data() );

      if( taskName == lastTask )
        {
          // last selected task found
          taskListWidget->setCurrentItem( taskListWidget->topLevelItem(i) );
          return;
        }
    }

  // select first entry in the list, if last selection could not be found
  taskListWidget->setCurrentItem( taskListWidget->topLevelItem(0) );
}
