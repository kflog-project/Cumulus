/***********************************************************************
**
**   tasklistview.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2009-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
**   Displays all points of a task as list with different buttons for
**   actions. Can be used in two modes, as display only, buttons for
**   actions are not visible or with command buttons.
**
***********************************************************************/

#include <QDesktopWidget>
#include <QShortcut>

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
  rowDelegate(0)
{
  setObjectName("TaskListView");

  _showButtons = showButtons;
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
  topLayout->addLayout( total );

  speedTotal = new QLabel("", this );
  wind       = new QLabel("", this );
  distTotal  = new QLabel("", this );
  timeTotal  = new QLabel("", this );

  total->addWidget( speedTotal );
  total->addWidget( wind );
  total->addWidget( distTotal );
  total->addWidget( timeTotal );

  list = new QTreeWidget( this );
  list->setObjectName("TaskListView");
  list->setColumnCount(9);
  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
  list->setAlternatingRowColors(true);
  list->setSortingEnabled(false);
  list->setSelectionMode(QAbstractItemView::NoSelection);
  list->setFocusPolicy(Qt::NoFocus);
//  list->setEnabled(false);

  setHeader();

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

      connect( cmdSelect, SIGNAL(clicked()),
               this, SLOT(slot_Select()) );
      connect( cmdInfo, SIGNAL(clicked() ),
               this, SLOT(slot_Info()));
      connect( cmdClose, SIGNAL(clicked() ),
               this, SLOT(slot_Close()) );
      connect( list, SIGNAL(itemSelectionChanged()),
               this, SLOT(slot_Selected()) );

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

  if( _task && _task->getWtCalcFlag() ==true )
    {
      course = tr("TH"); // true heading
    }
  else
    {
      course = tr("TC"); // true course
    }

  sl << tr("Type")
     << tr("Name")
     << tr("Dist.")
     << tr("GS")
     << tr("WCA")
     << course
     << tr("Time")
     << tr("SS")
     << tr("Description");

  list->setHeaderLabels(sl);
}

void TaskListView::slot_Selected()
{
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

  if ( _selectedTp->taskPointIndex == 0 )
    {
      QTreeWidgetItem* tpBelow = list->itemBelow( _newSelectedTp );
      _TaskPointItem *tpiBelow = 0;;
      TaskPoint *below = 0;

      if( tpBelow )
        {
          tpiBelow = dynamic_cast<_TaskPointItem *> (tpBelow);

          if( tpiBelow )
            {
              below = tpiBelow->getTaskPoint();
            }
        }

      if( ! (below && below->taskPointIndex == 1 &&
             below->origP != _selectedTp->origP) )
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

  // qDebug("New Selected Task point name: %s, Index=%d",
  //        _selectedTp->name.toLatin1().data(), _selectedTp->taskPointIndex );
}

void TaskListView::showEvent(QShowEvent *)
{
  if ( _showButtons == false )
    {
      // do nothing as display, there are no buttons visible
      return;
    }

  const wayPoint *calcWp = calculator->getselectedWp();
  bool foundWp = false;

  for ( int i = 0; i < list->topLevelItemCount(); i++)
    {

      _TaskPointItem* _tp = static_cast<_TaskPointItem *> (list->topLevelItem(i));
      TaskPoint*   tp = _tp->getTaskPoint();

      // Waypoints can be selected from different windows. We will
      // consider only waypoints for a selection, which are member of a
      // flight task. In this case the taskPointIndex should be unequal to -1.
      if ( calcWp && calcWp->origP == tp->origP &&
           calcWp->taskPointIndex == tp->taskPointIndex )
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

/** This signal is called to indicate that a selection has been made. */
void TaskListView::slot_Select()
{
  // qDebug("TaskListView::slot_Select(): Selected WP= %s, Index=%d",
  //       _selectedTp->name.latin1(), _selectedTp->taskPointIndex );

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
      emit newWaypoint(getSelectedWaypoint(), true);
      emit done();
    }
}

/** This slot is called if the info button has been clicked */
void TaskListView::slot_Info()
{
  if (getSelectedWaypoint())
    {
      emit info(getSelectedWaypoint());
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
      return;
    }

  if ( _showButtons == true )
    {
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
    }

  // create a deep task copy
  _task = new FlightTask( *tsk );

  // Check if a waypoint is selected in calculator. In this case set
  // it in task list as selected too.
  const wayPoint *calcWp = calculator->getselectedWp();

  QList<TaskPoint *> tmpList = _task->getTpList();

  for ( int loop = 0; loop < tmpList.count(); loop++ )
    {
      TaskPoint* tp = tmpList.at( loop );
      _TaskPointItem* _tp = new _TaskPointItem( list, tp, _task->getWtCalcFlag() );

      if ( calcWp && calcWp->origP == tp->origP )
        {
          list->setCurrentItem( _tp, 0 );
          _currSelectedTp = _tp;
          _selectedTp = tp;
        }
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
      timeTotal->setText( "T=" + _task->getTotalDistanceTimeString() + "h" );
    }

  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->resizeColumnToContents(2);
  list->resizeColumnToContents(3);
  list->resizeColumnToContents(4);
  list->resizeColumnToContents(5);
  list->resizeColumnToContents(6);
  list->resizeColumnToContents(7);
  list->resizeColumnToContents(8);

  setHeader();
}

/**
 * Updates the internal task data. Will be called after
 * configuration changes of task sector items
 */
void TaskListView::slot_updateTask()
{
  // qDebug("TaskListView::slot_updateTask()");

  if ( _task )
    {
      setHeader();
      _task->updateTask();
      _task->updateProjection();
    }
}

/** Returns a pointer to the currently highlighted task point. */
wayPoint *TaskListView::getSelectedWaypoint()
{
  return _selectedTp;
}

void TaskListView::clear()
{
  list->clear();
  wind->setText("");
  distTotal->setText("");
  speedTotal->setText("");
  timeTotal->setText("");
}

/**
 * This class constructor sets all data of a QTreeWidgetItem.
 *
 * @param tpList Parent of QTreeWidget.
 * @param point Data of task point to be set.
 * @param wtCalcFlag Flag to indicate if wind triangle calculation
 *        was successful for all task legs or not.
 */
TaskListView::_TaskPointItem::_TaskPointItem( QTreeWidget *tpList,
                                              TaskPoint *point,
                                              bool wtCalcFlag ) :
  QTreeWidgetItem(tpList)
{
  tp = point; // save passed task point for later use

  // calculate sunset for the task point
  QString sr, ss, tz;
  QDate date = QDate::currentDate();
  Sonne::sonneAufUnter( sr, ss, date, tp->origP, tz );

  setText(0, tp->getTaskPointTypeString());
  setText(1, tp->name);
  setText(2, " " + Distance::getText(tp->distance*1000, false, 1));
  setTextAlignment( 2, Qt::AlignRight|Qt::AlignVCenter );

  // If wind calculation is available for all task legs, we do consider that
  // in task display.
  if( wtCalcFlag == true && tp->wtResult == true )
    {
      Speed gs(tp->groundSpeed); // unit is m/s
      int gsInt = static_cast<int> (rint(gs.getHorizontalValue()));
      QString gsString;
      gsString = QString("%1").arg(gsInt);
      setText(3, gsString);
      setTextAlignment( 3, Qt::AlignRight|Qt::AlignVCenter );

      int wca = static_cast<int> (tp->wca);
      QString wcaString;
      wcaString = QString("%1%2").arg(wca).arg(QString(Qt::Key_degree));
      setText(4, wcaString);
      setTextAlignment( 4, Qt::AlignCenter );

      int th = static_cast<int> (tp->trueHeading);
      QString thString;
      thString = QString("%1%2").arg(th).arg(QString(Qt::Key_degree));
      setText(5, thString);
      setTextAlignment( 5, Qt::AlignRight|Qt::AlignVCenter );
    }
  else
    {
      // No wind calculation available, use dash as display value for ground
      // speed and wca.
      setText(3, "-" ); // GS
      setTextAlignment( 3, Qt::AlignCenter );
      setText(4, "-" ); // WCA
      setTextAlignment( 4, Qt::AlignCenter );

      if ( tp->bearing == -1.0 )
        {
          // bearing is undefined
          setText(5, "-");
          setTextAlignment( 5, Qt::AlignCenter );
        }
      else
        {
          int bearing = (int) rint( tp->bearing*180./M_PI );
          setText(5, " " + QString::number( bearing ) + QString(Qt::Key_degree));
          setTextAlignment( 5, Qt::AlignRight|Qt::AlignVCenter );
        }
    }

  setText(6, " " + FlightTask::getDistanceTimeString(tp->distTime));

  if( tp->distTime == 0 )
    {
      setTextAlignment( 6, Qt::AlignCenter );
    }
  else
    {
      setTextAlignment( 6, Qt::AlignRight|Qt::AlignVCenter );
    }

  setText(7, " " + ss + " " + tz);
  setText(8, " " + tp->description);

  setIcon(1, QIcon(_globalMapConfig->getPixmap(tp->type, false, true)) );
}
