/***********************************************************************
**
**   tasklistview.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2009-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   Displays all points of a task as list with different buttons for
**   actions. Can be used in two modes, as display only, buttons for
**   actions are not visible or with command buttons.
**
***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "layout.h"
#include "mainwindow.h"
#include "tasklistview.h"
#include "flighttask.h"
#include "distance.h"
#include "calculator.h"
#include "sonne.h"
#include "TaskListStatusView.h"

extern MapConfig * _globalMapConfig;
extern Calculator* calculator;

TaskListView::TaskListView( QWidget *parent, bool showButtons ) :
  QWidget(parent),
  rowDelegate(0),
  _showButtons(showButtons)
{
  setObjectName("TaskListView");

  _task = 0;
  _currSelectedTp = 0;
  _newSelectedTp = 0;
  _selectText = tr("Select");
  _unselectText = tr("Unselect");

  QVBoxLayout *topLayout = new QVBoxLayout( this );

  if( ! showButtons )
    {
      topLayout->setMargin(0);
    }

  QHBoxLayout *total = new QHBoxLayout;

  speedTotal = new QLabel("", this );
  wind       = new QLabel("", this );
  distTotal  = new QLabel("", this );
  timeTotal  = new QLabel("", this );

  total->addWidget( speedTotal );
  total->addWidget( wind );
  total->addWidget( distTotal );
  total->addWidget( timeTotal );

  headline = new QWidget( this );
  headline->setLayout( total );
  topLayout->addWidget( headline );

  list = new QTreeWidget( this );
  list->setObjectName("TaskListView");
  list->setColumnCount(10);
  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
  list->setAlternatingRowColors(true);
  list->setSortingEnabled(false);
  list->setSelectionMode(QAbstractItemView::NoSelection);
  list->setFocusPolicy(Qt::NoFocus);
  setHeader();

  list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  list->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // calculates the needed icon size
  const int iconSize = Layout::iconSize( font() );

  // Sets the icon size of a list entry
  list->setIconSize( QSize(iconSize, iconSize) );

  topLayout->addWidget(list, 10);

  if ( showButtons )
    {
      total->setSpacing(5);
      list->setAllColumnsShowFocus(true);
      list->setSelectionMode(QAbstractItemView::SingleSelection);
      list->setSelectionBehavior(QAbstractItemView::SelectRows);
      list->setFocusPolicy( Qt::StrongFocus );

      // Don't show any buttons, if required
      QBoxLayout *buttonrow=new QHBoxLayout;
      topLayout->addLayout( buttonrow );

      QPushButton *cmdClose = new QPushButton( tr( "Close" ), this );
      buttonrow->addWidget( cmdClose );

      QPushButton *cmdInfo = new QPushButton( tr( "Info" ), this );
      buttonrow->addWidget( cmdInfo );

      cmdSelect = new QPushButton( _selectText, this );
      buttonrow->addWidget( cmdSelect );

      QPushButton *cmdStart = new QPushButton( tr( "Start" ), this );
      buttonrow->addWidget( cmdStart );

      QPushButton *cmdStatus = new QPushButton( tr( "Status" ), this );
      buttonrow->addWidget( cmdStatus );

      connect( cmdSelect, SIGNAL(clicked()),
               this, SLOT(slot_Select()) );
      connect( cmdInfo, SIGNAL(clicked() ),
               this, SLOT(slot_Info()));
      connect( cmdClose, SIGNAL(clicked() ),
               this, SLOT(slot_Close()) );
      connect( list, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
               this, SLOT(slot_itemClicked(QTreeWidgetItem*, int)));
      connect( cmdStart, SIGNAL(clicked() ),
               this, SLOT(slot_Start()) );
      connect( cmdStatus, SIGNAL(clicked() ),
               this, SLOT(slot_Status()) );

      // activate keyboard shortcut Return as select
      QShortcut* scSelect = new QShortcut( this );
      scSelect->setKey( Qt::Key_Return );
      connect( scSelect, SIGNAL(activated()), this, SLOT( slot_Select() ));
    }
}

TaskListView::~TaskListView()
{
  if ( _task )
    {
      delete _task;
    }
}

/** sets the header of the list */
void TaskListView::setHeader()
{
  QStringList sl;
  QString course;

  if( _task && _task->getWtCalcFlag() == true )
    {
      course = tr("TH"); // true heading
      list->showColumn( 4 );
      list->showColumn( 5 );
    }
  else
    {
      course = tr("TC"); // true course
      list->hideColumn( 4 );
      list->hideColumn( 5 );
    }

  sl << tr("No")
     << tr("Type")
     << tr("Name")
     << tr("Way")
     << tr("GS")
     << tr("WCA")
     << course
     << tr("Time")
     << tr("SS")
     << tr("Description");

  list->setHeaderLabels(sl);

  QTreeWidgetItem* headerItem = list->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 5, Qt::AlignCenter );
  headerItem->setTextAlignment( 6, Qt::AlignCenter );
  headerItem->setTextAlignment( 7, Qt::AlignCenter );
  headerItem->setTextAlignment( 8, Qt::AlignCenter );
  headerItem->setTextAlignment( 9, Qt::AlignCenter );

  resizeTaskList();
}

void TaskListView::slot_Start()
{
  calculator->slot_startTask();
  emit done();
}

/**
 *  Called, if the user clicks into a cell in the listview.
 */
void TaskListView::slot_itemClicked( QTreeWidgetItem* item, int column )
{
  Q_UNUSED(column)

  if( item == static_cast<QTreeWidgetItem *> (0) )
    {
      return;
    }

  cmdSelect->setEnabled(true);

  _newSelectedTp = item;

  if( _newSelectedTp == _currSelectedTp )
    {
      cmdSelect->setText(_unselectText);
    }
  else
    {
      cmdSelect->setText(_selectText);
    }
}

void TaskListView::showEvent(QShowEvent *)
{
  resizeTaskList();

  if ( _showButtons == false )
    {
      // do nothing as display, there are no buttons visible
      return;
    }

  const Waypoint *calcWp = calculator->getTargetWp();
  bool foundWp = false;

  for( int i = 0; i < list->topLevelItemCount(); i++ )
    {
      _TaskPointItem* tpi = static_cast<_TaskPointItem *> (list->topLevelItem(i));
      TaskPoint*       tp = tpi->getTaskPoint();

      // Waypoints can be selected from different windows. We will
      // consider only waypoints for a selection, which are member of a
      // flight task. In this case the taskPointIndex should be unequal to -1.
      if ( calcWp && calcWp->wgsPoint == tp->getWGSPosition() &&
           calcWp->taskPointIndex == tp->getFlightTaskListIndex() )
        {
          list->setCurrentItem( list->topLevelItem(i), 0 );
          _currSelectedTp = tpi;
          _newSelectedTp = tpi;
          cmdSelect->setText(_unselectText);
          foundWp = true;
          break;
        }
    }

  if( foundWp == false )
    {
      // if no calculator waypoint is selected, clear the selection on the listview
      list->clearSelection();
      _currSelectedTp = 0;
      _newSelectedTp = 0;
    }

  list->setFocus();
}

/** This slot is called if the select button has been clicked */
void TaskListView::slot_Select()
{
  if( _newSelectedTp == _currSelectedTp )
    {
      // this was an unselect
      //calculator->slot_WaypointChange(0, true);
      emit newWaypoint(0, true);
      list->clearSelection();
      cmdSelect->setText(_selectText);
      _currSelectedTp = 0;
      _newSelectedTp = 0;
    }
  else
    {
      _currSelectedTp = _newSelectedTp; // save last done selection
      emit newWaypoint(getCurrentEntry(), true);
      emit done();
    }
}

/** This slot is called if the info button has been clicked */
void TaskListView::slot_Info()
{
  Waypoint* wp = getCurrentEntry();

  if( wp != 0 )
    {
      emit info( wp );
    }
}

/**
 * This slot is called, if the status button has been clicked,
 */
void TaskListView::slot_Status()
{
  TaskListStatusView* tlvs = new TaskListStatusView( this );
  tlvs->show();
}

/** @ee This slot is called if the listview is closed without selecting */
void TaskListView::slot_Close ()
{
  if ( _newSelectedTp != _currSelectedTp )
    {
      _newSelectedTp = _currSelectedTp; // selection change was not committed
    }

  emit done();
}

/**
 * Retrieves the task points from the task, and fills the list.
 */
void TaskListView::slot_setTask(const FlightTask *tsk)
{
  // Note! At first clear the list and then delete the task. Otherwise you will
  // get a core dump!
  list->clear();
  _currSelectedTp = 0;
  _newSelectedTp = 0;

  // delete an existing old task
  if ( _task )
    {
      delete _task;
      _task = static_cast<FlightTask *> (0);
    }

  if (tsk == static_cast<FlightTask *> (0) )
    {
      // No new task passed
      _task = static_cast<FlightTask *> (0);
      resizeTaskList();
      return;
    }

  // set row height at each list fill - has probably changed.
  // Note: rpMargin is a manifold of 2 to ensure symmetry
  int rpMargin = GeneralConfig::instance()->getListDisplayRPMargin();

  if ( rowDelegate )
    {
      rowDelegate->setVerticalMargin( rpMargin );
    }
  else
    {
      rowDelegate = new RowDelegate( list, rpMargin );
      list->setItemDelegate( rowDelegate );
    }

  // create a deep task copy
  _task = new FlightTask( *tsk );

  // Check if a waypoint is selected in calculator. In this case set
  // it in task list as selected too.
  const Waypoint *calcWp = calculator->getTargetWp();

  QList<TaskPoint *> tmpList = _task->getTpList();

  for( int loop = 0; loop < tmpList.size(); loop++ )
    {
      TaskPoint* tp = tmpList.at( loop );
      _TaskPointItem* _tpi = new _TaskPointItem( list, tp, _task->getWtCalcFlag(), true );

      if( tmpList.size() < 10 )
        {
          _tpi->setText( 0, QString::number(loop + 1) );
        }
      else
        {
          QString no = QString("%1").arg(loop + 1, 2, 10, QChar('0') );
          _tpi->setText( 0, no );
        }

      if ( calcWp && calcWp->wgsPoint == tp->getWGSPosition() )
        {
          list->setCurrentItem( _tpi, 0 );
          _currSelectedTp = _tpi;
        }
    }

  if ( _showButtons == false )
    {
      QTreeWidgetItem *item = new QTreeWidgetItem( list );

      item->setText( 2, tr("Total") );
      item->setText( 3, _task->getTaskDistanceString( false ) );
      item->setTextAlignment( 3, Qt::AlignRight|Qt::AlignVCenter );

      if( _task->getSpeed() != 0 )
        {
          item->setText( 7, _task->getTotalDistanceTimeString() );
          item->setTextAlignment( 7, Qt::AlignRight|Qt::AlignVCenter );
        }

      list->addTopLevelItem( item );
    }

  // set the total values in the header of this view
  speedTotal->setText( "TAS=" + _task->getSpeedString() );

  // wind direction and speed
  wind->setText( "W=" + _task->getWindString() );

  distTotal->setText( "S=" + _task->getTaskDistanceString().remove(QChar(' ')) );

  if( _task->getSpeed() == 0 )
    {
      // TAS is zero
      timeTotal->setText( "T=" + tr("none") );
    }
  else
    {
      // TAS is not zero
      timeTotal->setText( "T=" + _task->getTotalDistanceTimeString() + "h" );
    }

  setHeader();
}

/**
 * Updates the internal task data. Will be called after
 * configuration changes of task sector items
 */
void TaskListView::slot_updateTask()
{
  if( _task )
    {
      _task->updateTask();
      _task->updateProjection();
    }

  setHeader();
}

/** Returns a pointer to the currently highlighted task point. */
Waypoint *TaskListView::getCurrentEntry()
{
  QTreeWidgetItem *ci = list->currentItem();

  if( ci == static_cast<QTreeWidgetItem *> (0) )
    {
      return static_cast<Waypoint *>(0);
    }

  _TaskPointItem *tpi = dynamic_cast<_TaskPointItem *> (ci);

  if( ! ci )
    {
      // dynamic cast has failed
      return static_cast<Waypoint *>(0);
    }

  return tpi->getTaskPoint()->getWaypointObject();
}

/** Resizes the columns of the task list to their contents. */
void TaskListView::resizeTaskList()
{
  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->resizeColumnToContents(2);
  list->resizeColumnToContents(3);
  list->resizeColumnToContents(4);
  list->resizeColumnToContents(5);
  list->resizeColumnToContents(6);
  list->resizeColumnToContents(7);
  list->resizeColumnToContents(8);
  list->resizeColumnToContents(9);
}

void TaskListView::clear()
{
  list->clear();
  _currSelectedTp = 0;
  _newSelectedTp = 0;

  wind->setText("");
  distTotal->setText("");
  speedTotal->setText("");
  timeTotal->setText("");

  setHeader();
}

TaskListView::_TaskPointItem::_TaskPointItem( QTreeWidget *tpList,
                                              TaskPoint *point,
                                              bool wtCalcFlag,
                                              bool showTpIcon ) :
  QTreeWidgetItem(tpList)
{
  tp = point; // save passed task point for later use

  // calculate sunset for the task point
  QString sr, ss, tz;
  QDate date = QDate::currentDate();
  Sonne::sonneAufUnter( sr, ss, date, tp->getWGSPositionRef(), tz );

  setText(1, tp->getTaskPointTypeString());

  if( showTpIcon )
    {
      const int iconSize = Layout::iconSize( tpList->font() );
      setIcon( 1, tp->getIcon( iconSize ) );
    }

  setText(2, tp->getWPName());
  setText(3, " " + Distance::getText(tp->distance*1000, false, 1));
  setTextAlignment( 3, Qt::AlignRight|Qt::AlignVCenter );

  // If wind calculation is available for all task legs, we do consider that
  // in task display.
  if( wtCalcFlag == true && tp->wtResult == true )
    {
      Speed gs(tp->groundSpeed); // unit is m/s
      int gsInt = static_cast<int> (rint(gs.getHorizontalValue()));
      QString gsString;
      gsString = QString("%1").arg(gsInt);
      setText(4, gsString);
      setTextAlignment( 4, Qt::AlignRight|Qt::AlignVCenter );

      int wca = static_cast<int> (tp->wca);
      QString wcaString;
      wcaString = QString("%1%2").arg(wca).arg(QString(Qt::Key_degree));
      setText(5, wcaString);
      setTextAlignment( 5, Qt::AlignCenter );

      int th = static_cast<int> (tp->trueHeading);
      QString thString;
      thString = QString("%1%2").arg(th).arg(QString(Qt::Key_degree));
      setText(6, thString);
      setTextAlignment( 6, Qt::AlignRight|Qt::AlignVCenter );
    }
  else
    {
      // No wind calculation available, use dash as display value for ground
      // speed and wca.
      setText(4, "-" ); // GS
      setTextAlignment( 3, Qt::AlignCenter );
      setText(5, "-" ); // WCA
      setTextAlignment( 4, Qt::AlignCenter );

      if ( tp->bearing == -1.0 )
        {
          // bearing is undefined
          setText(6, "-");
          setTextAlignment( 6, Qt::AlignCenter );
        }
      else
        {
          int bearing = (int) rint( tp->bearing*180./M_PI );
          setText(6, " " + QString::number( bearing ) + QString(Qt::Key_degree));
          setTextAlignment( 6, Qt::AlignRight|Qt::AlignVCenter );
        }
    }

  setText(7, " " + FlightTask::getDistanceTimeString(tp->distTime));

  if( tp->distTime == 0 )
    {
      setTextAlignment( 7, Qt::AlignCenter );
    }
  else
    {
      setTextAlignment( 7, Qt::AlignRight|Qt::AlignVCenter );
    }

  setText(8, " " + ss + " " + tz);
  setText(9, " " + tp->getName());

  setIcon(2, QIcon(_globalMapConfig->getPixmap(tp->getTypeID(), false)) );
}
