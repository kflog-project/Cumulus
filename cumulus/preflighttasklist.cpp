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
#include "rowdelegate.h"

#ifdef FLARM
#include "preflightflarmpage.h"
#endif

PreFlightTaskList::PreFlightTaskList( QWidget* parent ) :
  QWidget( parent ),
  editTask(0),
  lastFocusWidget(0)
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
#ifndef ANDROID
  tas->setToolTip( tr("True Air Speed") );
#endif
  tas->setButtonSymbols(QSpinBox::NoButtons);
  tas->setFocusPolicy(Qt::StrongFocus);
  tas->setRange( 0, 900);
  tas->setSingleStep( 5 );
  tas->setValue( GeneralConfig::instance()->getTas() );
  tas->setSuffix( Speed::getHorizontalUnitText() );
  editrow->addWidget(tas);

  label = new QLabel( tr("WD"), this );
  editrow->addWidget(label);

  windDirection = new QSpinBox( this );
#ifndef ANDROID
  windDirection->setToolTip( tr("Wind Direction") );
#endif
  windDirection->setButtonSymbols(QSpinBox::NoButtons);
  windDirection->setFocusPolicy(Qt::StrongFocus);
  windDirection->setRange( 0, 360 );
  windDirection->setWrapping(true);
  windDirection->setSingleStep( 10 );
  windDirection->setValue( GeneralConfig::instance()->getWindDirection() );
  windDirection->setSuffix( QString(Qt::Key_degree) );
  editrow->addWidget(windDirection);

  label = new QLabel( tr("WS"), this );
  editrow->addWidget(label);

  windSpeed = new QSpinBox( this );
#ifndef ANDROID
  windSpeed->setToolTip( tr("Wind Speed") );
#endif
  windSpeed->setButtonSymbols(QSpinBox::NoButtons);
  windSpeed->setFocusPolicy(Qt::StrongFocus);
  windSpeed->setRange( 0, 900 );
  windSpeed->setValue( GeneralConfig::instance()->getWindSpeed() );
  windSpeed->setSuffix( Speed::getWindUnitText() );

  if( Speed::getWindUnit() != Speed::metersPerSecond )
    {
      windSpeed->setSingleStep( 5 );
    }

  editrow->addWidget(windSpeed);

  // take a bold font for the plus and minus sign
  QFont bFont = font();
  bFont.setBold(true);

  plus  = new QPushButton("+");
  minus = new QPushButton("-");

#ifndef ANDROID
  plus->setToolTip( tr("Increase number value") );
  minus->setToolTip( tr("Decrease number value") );
#endif

  plus->setFont(bFont);
  minus->setFont(bFont);

  // The buttons have no focus policy to avoid a focus change during click of them.
  plus->setFocusPolicy(Qt::NoFocus);
  minus->setFocusPolicy(Qt::NoFocus);

  editrow->addSpacing(10);
  editrow->addWidget(plus);
  editrow->addSpacing(10);
  editrow->addWidget(minus);
  editrow->addStretch(10);

  QPushButton * cmdNew = new QPushButton;
  cmdNew->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("add.png")) );
  cmdNew->setIconSize(QSize(IconSize, IconSize));
#ifndef ANDROID
  cmdNew->setToolTip(tr("Define a new task"));
#endif
  editrow->addWidget(cmdNew);

  editrow->addSpacing(10);
  QPushButton * cmdEdit = new QPushButton;
  cmdEdit->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  cmdEdit->setIconSize(QSize(IconSize, IconSize));
#ifndef ANDROID
  cmdEdit->setToolTip(tr("Edit selected task"));
#endif
  editrow->addWidget(cmdEdit);

  editrow->addSpacing(10);
  QPushButton * cmdDel = new QPushButton;
  cmdDel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("delete.png")) );
  cmdDel->setIconSize(QSize(IconSize, IconSize));
#ifndef ANDROID
  cmdDel->setToolTip(tr("Remove selected task"));
#endif
  editrow->addWidget(cmdDel);

  int size = cmdDel->sizeHint().height();

  plus->setMinimumSize(size, size);
  plus->setMaximumSize(size, size);
  minus->setMinimumSize(size, size);
  minus->setMaximumSize(size, size);

  //----------------------------------------------------------------------------

  taskListWidget = new QWidget( this );
  QVBoxLayout *tlLayout = new QVBoxLayout( taskListWidget );

  taskList = new QTreeWidget;

#ifndef ANDROID
  taskList->setToolTip( tr("Select a flight task") );
#endif
  taskList->setRootIsDecorated(false);
  taskList->setItemsExpandable(false);
  taskList->setUniformRowHeights(true);
  taskList->setAlternatingRowColors(true);
  taskList->setSortingEnabled(true);
  taskList->setSelectionBehavior(QAbstractItemView::SelectRows);
  taskList->setSelectionMode(QAbstractItemView::SingleSelection);
  taskList->setColumnCount(5);
  taskList->setFocus();

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  taskList->setItemDelegate( new RowDelegate( taskList, afMargin ) );

#ifdef QSCROLLER
  QScroller::grabGesture(taskList, QScroller::LeftMouseButtonGesture);
#endif

  QStringList sl;
  sl << tr("No.")
     << tr("Name")
     << tr("Type")
     << tr("Distance")
     << tr("Time");

  taskList->setHeaderLabels(sl);

  QTreeWidgetItem* headerItem = taskList->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 4, Qt::AlignCenter );

  tlLayout->addWidget( taskList, 10 );

  QPushButton *tlShowButton = new QPushButton( tr("Show") );

#ifndef FLARM
  tlLayout->addWidget( tlShowButton, 0, Qt::AlignRight );
#else
  QPushButton *tlFlarmButton = new QPushButton( tr("Flarm") );
  QHBoxLayout* hbl = new QHBoxLayout;
  hbl->addWidget( tlFlarmButton, 0, Qt::AlignLeft );
  hbl->addWidget( tlShowButton, 0, Qt::AlignRight );
  tlLayout->addLayout( hbl );

  connect( tlFlarmButton, SIGNAL(pressed()), SLOT(slotShowFlarmWidget()) );
#endif

  taskLayout->addWidget( taskListWidget );

  //----------------------------------------------------------------------------

  taskViewWidget = new QWidget( this );
  QVBoxLayout *tvLayout = new QVBoxLayout( taskViewWidget );

  taskContent = new TaskListView( this, false );
  taskContent->setHeadlineVisible( false );

#ifndef ANDROID
  taskContent->setToolTip( tr("Task display") );
#endif

  tvLayout->addWidget( taskContent, 10 );

  QPushButton *tvCloseButton = new QPushButton( tr("Close") );
  tvLayout->addWidget( tvCloseButton, 0, Qt::AlignRight );
  taskLayout->addWidget( taskViewWidget );
  taskViewWidget->setVisible( false );

  connect(cmdNew, SIGNAL(clicked()), this, SLOT(slotNewTask()));
  connect(cmdEdit, SIGNAL(clicked()), this, SLOT(slotEditTask()));
  connect(cmdDel, SIGNAL(clicked()), this, SLOT(slotDeleteTask()));
  connect(plus, SIGNAL(pressed()), this, SLOT(slotIncrementBox()));
  connect(minus, SIGNAL(pressed()), this, SLOT(slotDecrementBox()));

  /**
   * If the plus or minus button is clicked, the focus is changed to the main
   * window. I don't know why. Therefore the previous focused widget must be
   * saved, to have an indication, if a spinbox entry should be modified.
   */
  connect( QCoreApplication::instance(), SIGNAL(focusChanged( QWidget*, QWidget*)),
            this, SLOT( slotFocusChanged( QWidget*, QWidget*)) );

  connect( taskList, SIGNAL( itemSelectionChanged() ),
            this, SLOT( slotTaskDetails() ) );

  connect( tlShowButton, SIGNAL(pressed()),
            this, SLOT( slotShowTaskViewWidget() ) );

  connect( tvCloseButton, SIGNAL(pressed()),
            this, SLOT( slotShowTaskListWidget() ) );

  loadTaskList();
}

PreFlightTaskList::~PreFlightTaskList()
{
  // qDebug("PreFlightTaskList::~PreFlightTaskList()");
  qDeleteAll(flightTaskList);
}

void PreFlightTaskList::showEvent(QShowEvent *)
{
  taskList->resizeColumnToContents(0);
  taskList->resizeColumnToContents(1);
  taskList->resizeColumnToContents(2);
  taskList->resizeColumnToContents(3);
  taskList->resizeColumnToContents(4);
}

void PreFlightTaskList::slotShowTaskListWidget()
{
  taskViewWidget->setVisible( false );
  taskListWidget->setVisible( true );
}

void PreFlightTaskList::slotShowTaskViewWidget()
{
  QList<QTreeWidgetItem*> selectList = taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      // Nothing is selected
      return;
    }

  QTreeWidgetItem* selected = taskList->selectedItems().at(0);

  if ( selected->text( 0 ) == " " )
    {
      // Help text is selected
      return;
    }

  taskViewWidget->setVisible( true );
  taskListWidget->setVisible( false );
}

void PreFlightTaskList::slotIncrementBox()
{
  if( ! plus->isDown() )
    {
      return;
    }

  // Look which spin box has the focus. Note, focus can be changed by clicking
  // the connected button. Therefore take old focus widget under account and
  // set the focus back to the spinbox.
  QAbstractSpinBox* spinBoxList[3] = {tas, windDirection, windSpeed};

  for( uint i = 0; i < (sizeof(spinBoxList) / sizeof(spinBoxList[0])); i++ )
    {
      if( QApplication::focusWidget() == spinBoxList[i] || lastFocusWidget == spinBoxList[i] )
        {
          spinBoxList[i]->stepUp();
          spinBoxList[i]->setFocus();
          updateWayTime();
          slotTaskDetails();

          // Start repetition timer, to check, if button is longer pressed.
           QTimer::singleShot(250, this, SLOT(slotIncrementBox()));
          return;
        }
    }
}

void PreFlightTaskList::slotDecrementBox()
{
  if( ! minus->isDown() )
    {
      return;
    }

  // Look which spin box has the focus. Note, focus can be changed by clicking
  // the connected button. Therefore take old focus widget under account and
  // set the focus back to the spinbox.
  QAbstractSpinBox* spinBoxList[3] = {tas, windDirection, windSpeed};

  for( uint i = 0; i < (sizeof(spinBoxList) / sizeof(spinBoxList[0])); i++ )
    {
      if( QApplication::focusWidget() == spinBoxList[i] || lastFocusWidget == spinBoxList[i] )
        {
          spinBoxList[i]->stepDown();
          spinBoxList[i]->setFocus();
          updateWayTime();
          slotTaskDetails();

          // Start repetition timer, to check, if button is longer pressed.
          QTimer::singleShot(250, this, SLOT(slotDecrementBox()));
          return;
        }
    }
}

void PreFlightTaskList::slotFocusChanged( QWidget* oldWidget, QWidget* newWidget)
{
  Q_UNUSED(newWidget)

  // We save the old widget, which has just lost the focus.
  lastFocusWidget = oldWidget;
}

void PreFlightTaskList::slotTaskDetails()
{
  QList<QTreeWidgetItem*> selectList = taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      return;
    }

  QTreeWidgetItem* selected = taskList->selectedItems().at(0);

  if ( selected->text( 0 ) == " " )
    {
      taskContent->clear();
      return;
    }

  int id = selected->text( 0 ).toInt() - 1;

  FlightTask *task = flightTaskList.at( id );

  // update TAS, can be changed in the meantime by the user
  task->setSpeed( tas->value() );

  // update wind parameters, can be changed in the meantime by the user
  task->setWindDirection( windDirection->value() % 360 );
  task->setWindSpeed( windSpeed->value() );

  taskContent->slot_setTask( task );
}

void PreFlightTaskList::updateWayTime()
{
  if( taskList->topLevelItemCount() < 2 )
    {
      // There are no tasks defined.
      return;
    }

  for( int i = 0; i < taskList->topLevelItemCount(); i++ )
    {
      QTreeWidgetItem* item = taskList->topLevelItem( i );

      if( item == 0 || item->text( 0 ) == " " )
        {
          continue;
        }

      int id = item->text( 0 ).toInt() - 1;

      FlightTask *task = flightTaskList.at( id );

      // update TAS, can be changed in the meantime by the user
      task->setSpeed( tas->value() );

      // update wind parameters, can be changed in the meantime by the user
      task->setWindDirection( windDirection->value() % 360 );
      task->setWindSpeed( windSpeed->value() );

      if( task->getSpeed() == 0 )
        {
          // TAS is zero, show nothing
          item->setText(4, "");
        }
      else
        {
          // TAS is not zero, show time total
          item->setText( 4, task->getTotalDistanceTimeString() + "h" );
        }
    }
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

  QList<QTreeWidgetItem*> selectList = taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      return static_cast<FlightTask *> (0);
    }

  QString id( taskList->selectedItems().at(0)->text(0) );

  // Special handling for entries with no number, they are system specific.
  if( id == " " )
    {
      GeneralConfig::instance()->setCurrentTask( "" );
      return static_cast<FlightTask *> (0);
    }

  // qDebug("selected Item=%s",id.toLatin1().data());
  GeneralConfig::instance()->setCurrentTask( taskList->selectedItems().at(0)->text(1) );

  // Nice trick, take selected element from list to prevent deletion of it, if
  // destruction of list is called.
  int index = id.toInt() - 1;

  FlightTask* task = flightTaskList.takeAt( index );

  // Save the task declaration in Flarm format as file.
  createFlarmTaskList( task );

  return task;
}

/** load tasks from file*/
bool PreFlightTaskList::loadTaskList()
{
  extern MapMatrix *_globalMapMatrix;
  QStringList rowList;

  while ( !flightTaskList.isEmpty() )
    {
      delete flightTaskList.takeFirst();
    }

  taskNames.clear();

#warning "task list file 'tasks.tsk' is stored at User Data Directory"

  // currently hard coded file name
  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/tasks.tsk" );

  if ( ! f.open( QIODevice::ReadOnly ) )
    {
      // could not read file ...
      rowList << " " << tr("(No tasks defined)");
      taskList->addTopLevelItem( new QTreeWidgetItem(taskList, rowList, 0) );
      taskList->setCurrentItem( taskList->itemAt(0,taskList->topLevelItemCount()-1) );
      taskList->sortItems( 0, Qt::AscendingOrder );

      // reset current task
      GeneralConfig::instance()->setCurrentTask( "" );

      taskList->resizeColumnToContents(0);
      taskList->resizeColumnToContents(1);
      taskList->resizeColumnToContents(2);
      taskList->resizeColumnToContents(3);
      taskList->resizeColumnToContents(4);

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
                  flightTaskList.append( task );

                  tpList = 0; // ownership was overtaken by FlighTask
                  numTask.sprintf( "%02d", flightTaskList.count() );

                  rowList << numTask
                          << taskName
                          << task->getTaskTypeString()
                          << task->getTaskDistanceString()
                          << "";

                  QTreeWidgetItem *item = new QTreeWidgetItem( taskList, rowList, 0 );
                  item->setTextAlignment( 0, Qt::AlignCenter);
                  item->setTextAlignment( 3, Qt::AlignRight);
                  item->setTextAlignment( 4, Qt::AlignRight);

                  taskList->addTopLevelItem( item );
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

  if ( flightTaskList.count() == 0 )
    {
      rowList << " " << tr("(No tasks defined)");

      // reset current task
      GeneralConfig::instance()->setCurrentTask( "" );
    }
  else
    {
      rowList << " " << tr("(Reset selection)") << tr("none");
    }

  taskList->addTopLevelItem( new QTreeWidgetItem(taskList, rowList, 0) );
  taskList->sortByColumn(0, Qt::AscendingOrder);

  updateWayTime();
  selectLastTask();

  taskList->resizeColumnToContents(0);
  taskList->resizeColumnToContents(1);
  taskList->resizeColumnToContents(2);
  taskList->resizeColumnToContents(3);
  taskList->resizeColumnToContents(4);

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
  flightTaskList.append( newTask );
  saveTaskList();
  taskContent->clear();
  taskList->clear();
  loadTaskList();
}

/**
 * pass the selected task to the editor
 */
void PreFlightTaskList::slotEditTask()
{
  // fetch selected task item
  QList<QTreeWidgetItem*> selectList = taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      return;
    }

  QString id( taskList->selectedItems().at(0)->text(0) );

  if ( id == " ")
    {
      return;
    }

  editTask = flightTaskList.at(id.toInt() - 1);

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
  int index = flightTaskList.indexOf( editTask );

  if ( index != -1 )
    {
      // remove old item
      delete flightTaskList.takeAt( index );
      // put new item on old position
      flightTaskList.insert( index, editedTask );
    }
  else
    {
      // no old position available, append it at end of list
      flightTaskList.append( editedTask );
    }

  saveTaskList();
  taskContent->clear();
  taskList->clear();
  loadTaskList();
}

/**
 * remove the selected task from the list
 */
void PreFlightTaskList::slotDeleteTask()
{
  QTreeWidgetItem* selected = taskList->currentItem();

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

  QMessageBox mb( QMessageBox::Question,
                  tr( "Delete Task?" ),
                  tr( "Delete the selected task?" ),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if ( mb.exec() != QMessageBox::Yes )
    {
      return;
    }

  delete taskList->takeTopLevelItem( taskList->currentIndex().row() );

  taskList->sortItems( 0, Qt::AscendingOrder );
  taskList->setCurrentItem( taskList->topLevelItem(0) );

  // reset last stored selected task
  GeneralConfig::instance()->setCurrentTask( "" );

  // reset task
  extern MapContents* _globalMapContents;
  _globalMapContents->setCurrentTask(0);

  uint no = id.toUInt() - 1;
  delete flightTaskList.takeAt( no );
  saveTaskList();
  taskContent->clear();
  taskList->clear();
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

  for ( int i=0; i < flightTaskList.count(); i++ )
    {
      FlightTask *task = flightTaskList.at(i);
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
             << tp->name
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

  int rows = taskList->topLevelItemCount();

  for( int i = 0; i < rows; i++ )
    {
      QString taskName = taskList->topLevelItem(i)->text(1);
      // qDebug( "taskName(%d)=%s", i, taskName.toLatin1().data() );

      if( taskName == lastTask )
        {
          // last selected task found
          taskList->setCurrentItem( taskList->topLevelItem(i) );
          return;
        }
    }

  // select first entry in the list, if last selection could not be found
  taskList->setCurrentItem( taskList->topLevelItem(0) );
}

void PreFlightTaskList::slotShowFlarmWidget()
{
  FlightTask *task = 0;

  QList<QTreeWidgetItem*> selectList = taskList->selectedItems();

  if( selectList.size() > 0 )
    {
      QTreeWidgetItem* selected = taskList->selectedItems().at(0);

      if ( selected->text( 0 ) != " " )
        {
          int id = selected->text( 0 ).toInt() - 1;

          task = flightTaskList.at( id );
        }
    }

  PreFlightFlarmPage* pffp = new PreFlightFlarmPage( task, this );
  pffp->setVisible( true );
}
