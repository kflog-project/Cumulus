/***********************************************************************
**
**   tasklistview.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2009-2014 by Axel Pauli
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

extern MapConfig * _globalMapConfig;
extern Calculator* calculator;

TaskListView::TaskListView( QWidget *parent, bool showButtons ) :
  QWidget(parent),
  rowDelegate(0),
  _showButtons(showButtons)
{
  setObjectName("TaskListView");

  _task = 0;
  _selectedTp = 0;
  _currSelectedTp = 0;
  _newSelectedTp = 0;
  _selectText = tr("Select");
  _unselectText = tr("Unselect");

  QVBoxLayout *topLayout = new QVBoxLayout( this );

  if ( ! showButtons )
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

      QPushButton *cmdClose = new QPushButton(tr("Close"), this);
      buttonrow->addWidget(cmdClose);

      QPushButton *cmdInfo = new QPushButton(tr("Info"), this);
      buttonrow->addWidget(cmdInfo);

      cmdSelect = new QPushButton(_selectText, this);
      buttonrow->addWidget(cmdSelect);

      QPushButton *cmdStart = new QPushButton(tr("Start"), this);
      buttonrow->addWidget(cmdStart);

      connect( cmdSelect, SIGNAL(clicked()),
               this, SLOT(slot_Select()) );
      connect( cmdInfo, SIGNAL(clicked() ),
               this, SLOT(slot_Info()));
      connect( cmdClose, SIGNAL(clicked() ),
               this, SLOT(slot_Close()) );
      connect( list, SIGNAL(itemSelectionChanged() ),
               this, SLOT(slot_Selected()) );
      connect( cmdStart, SIGNAL(clicked() ),
               this, SLOT(slot_Start()) );

      // activate keyboard shortcut Return as select
      QShortcut* scSelect = new QShortcut( this );
      scSelect->setKey( Qt::Key_Return );
      connect( scSelect, SIGNAL(activated()), this, SLOT( slot_Select() ));
    }
}

TaskListView::~TaskListView()
{
  // qDebug("TaskListView::~TaskListView()");
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

  sl << tr("")
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

void TaskListView::slot_Selected()
{
  // Selected item has been changed in the task list by the user.
  _newSelectedTp = list->currentItem();

  if ( _newSelectedTp == static_cast<QTreeWidgetItem *> (0) )
    {
      return;
    }

  _TaskPointItem *tpi = dynamic_cast<_TaskPointItem *> (_newSelectedTp);

  if( ! tpi )
    {
      // dynamic cast has failed
      return;
    }

  _selectedTp = tpi->getTaskPoint();

  if ( _selectedTp->getFlightTaskListIndex() == 0 )
    {
      QTreeWidgetItem* tpBelow = list->itemBelow( _newSelectedTp );
      _TaskPointItem *tpiBelow = 0;
      TaskPoint *below = 0;

      if( tpBelow )
        {
          tpiBelow = dynamic_cast<_TaskPointItem *> (tpBelow);

          if( tpiBelow )
            {
              below = tpiBelow->getTaskPoint();
            }
        }

      if( ! (below && below->getFlightTaskListIndex() == 1 &&
             below->getWGSPosition() != _selectedTp->getWGSPosition()) )
        {
          // Take-off point should not be selectable in taskview, if it is
          // identical to the start point.
          cmdSelect->setEnabled(false);
          return;
        }
    }

  cmdSelect->setEnabled(true);

  if (_newSelectedTp == _currSelectedTp)
    {
      cmdSelect->setText(_unselectText);
    }
  else
    {
      cmdSelect->setText(_selectText);
    }

  // qDebug( "New Selected Task point name: %s, Index=%d",
  //         _selectedTp->name.toLatin1().data(), _selectedTp->taskPointIndex );
}

void TaskListView::showEvent(QShowEvent *)
{
  resizeTaskList();

  if ( _showButtons == false )
    {
      // do nothing as display, there are no buttons visible
      return;
    }

  const Waypoint *calcWp = calculator->getselectedWp();
  bool foundWp = false;

  for( int i = 0; i < list->topLevelItemCount(); i++ )
    {
      _TaskPointItem* _tp = static_cast<_TaskPointItem *> (list->topLevelItem(i));
      TaskPoint*       tp = _tp->getTaskPoint();

      // Waypoints can be selected from different windows. We will
      // consider only waypoints for a selection, which are member of a
      // flight task. In this case the taskPointIndex should be unequal to -1.
      if ( calcWp && calcWp->wgsPoint == tp->getWGSPosition() &&
           calcWp->taskPointIndex == tp->getFlightTaskListIndex() )
        {
          list->setCurrentItem( list->topLevelItem(i), 0 );
          _currSelectedTp = _tp;
          _newSelectedTp = _tp;
          _selectedTp = tp;
          cmdSelect->setText(_unselectText);
          foundWp = true;
          break;
        }
    }

  if ( foundWp == false )
    {
      // if no calculator waypoint selected, clear selection on listview
      list->clearSelection();
      _selectedTp = 0;
      _currSelectedTp = 0;
      _newSelectedTp = 0;
    }

  list->setFocus();
}

/** This slot is called if the select button has been clicked */
void TaskListView::slot_Select()
{
  //  qDebug() << "TaskListView::slot_Select(): Selected WP=" << _selectedTp->name
  //            << "Index=" << _selectedTp->taskPointIndex;

  if (_newSelectedTp == _currSelectedTp)
    {
      // this was an unselect
      //calculator->slot_WaypointChange(0, true);
      emit newWaypoint(0, true);
      list->clearSelection();
      cmdSelect->setText(_selectText);
      _selectedTp = 0;
      _currSelectedTp = 0;
      _newSelectedTp = 0;
    }
  else
    {
      _currSelectedTp = _newSelectedTp; // save last selection
      emit newWaypoint(getCurrentEntry(), true);
      emit done();
    }
}

/** This slot is called if the info button has been clicked */
void TaskListView::slot_Info()
{
  if (getCurrentEntry())
    {
      emit info(getCurrentEntry());
    }
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
  // delete old task
  if ( _task )
    {
      delete _task;
      _task = static_cast<FlightTask *> (0);
    }

  list->clear();

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

  // qDebug( "rpMargin=%d", rpMargin );

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
  const Waypoint *calcWp = calculator->getselectedWp();

  QList<TaskPoint *> tmpList = _task->getTpList();

  for( int loop = 0; loop < tmpList.size(); loop++ )
    {
      bool showTpIcon = true;

      if( tmpList.size() >= 2 &&
          ((loop == 0 && tmpList.at(0)->getWGSPosition() == tmpList.at(1)->getWGSPosition() ) ||
           (loop == tmpList.size()-1 &&
            tmpList.at(tmpList.size()-1)->getWGSPosition() == tmpList.at(tmpList.size()-2)->getWGSPosition() )) )
        {
          // If start and begin point or end and landing point are identical
          // no task figure icon is shown in the list entry.
          showTpIcon = false;
        }

      TaskPoint* tp = tmpList.at( loop );
      _TaskPointItem* _tpi = new _TaskPointItem( list, tp, _task->getWtCalcFlag(), showTpIcon );

      if( tmpList.size() < 10 )
        {
          _tpi->setText( 0, QString::number(loop) );
        }
      else
        {
          QString no = QString("%1").arg(loop, 2, 10, QChar('0') );
          _tpi->setText( 0, no );
        }

      if ( calcWp && calcWp->wgsPoint == tp->getWGSPosition() )
        {
          list->setCurrentItem( _tpi, 0 );
          _currSelectedTp = _tpi;
          _selectedTp = tp;
        }
    }

  if ( _showButtons == false )
    {
      QTreeWidgetItem *item = new QTreeWidgetItem( list );

      item->setText( 2, tr("Total") );
      item->setText( 3, _task->getTotalDistanceString( false ) );
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

  distTotal->setText( "S=" + _task->getTotalDistanceString().remove(QChar(' ')) );

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
  // qDebug("TaskListView::slot_updateTask()");

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
  if( _selectedTp )
    {
      return _selectedTp->getWaypointObject();
    }

  return static_cast<Waypoint *>(0);
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

  setIcon(2, QIcon(_globalMapConfig->getPixmap(tp->getTypeID(), false, false)) );
}
