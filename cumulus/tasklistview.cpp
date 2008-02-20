/***********************************************************************
**
**   tasklistview.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
**   Displays all points of a task as list with different buttons for
**   actions. Can be used in two modes, as display only, buttons for
**   actions are not visible or with command buttons.
**
***********************************************************************/

#include <QDesktopWidget>
#include <Q3PtrList>
#include <QFont>

#include "cumulusapp.h"
#include "tasklistview.h"
#include "flighttask.h"
#include "distance.h"
#include "cucalc.h"
#include "sonne.h"

extern MapConfig * _globalMapConfig;
extern CuCalc* calculator;

TaskListView::TaskListView( QWidget *parent, const char *name,
                            bool showButtons, bool bold )
  : QWidget(parent, name)
{
  _task=0;
  _selectedWp = 0;
  _currSelectedTp = 0;
  _newSelectedTp = 0;
  _selectText = tr("Select");
  _unselectText = tr("Unselect");

  // resize(parent->size());

  if( bold )
    {
      QFont fnt( "Helvetica", 12, QFont::Bold  );
      this->setFont(fnt);
    }

  QBoxLayout *topLayout = new QVBoxLayout( this );

  QBoxLayout *total = new QHBoxLayout( topLayout, 0 );
  total->setMargin(2);
  total->setSpacing(5);

  distTotal  = new QLabel("", this );
  speedTotal = new QLabel("", this );
  timeTotal  = new QLabel("", this );

  total->addWidget( distTotal );
  total->addWidget( speedTotal );
  total->addWidget( timeTotal );

  list= new Q3ListView(this, "taskpointList");
  list->addColumn(tr("Type"));
  list->addColumn(tr("Name"));
  list->addColumn(tr("Dist"));
  list->addColumn(tr("Time"));
  list->addColumn(tr("Description"));
  list->addColumn(tr("SS"));

  list->setColumnAlignment( 2, Qt::AlignRight ); 
  list->setColumnAlignment( 3, Qt::AlignRight ); 

  list->setSorting(-1,false);
  list->setAllColumnsShowFocus(false);
  list->setSelectionMode( Q3ListView::NoSelection );
  topLayout->addWidget(list,10);

  if( showButtons )
    {
      list->setAllColumnsShowFocus(true);
      list->setSelectionMode( Q3ListView::Single );

      // Don't show any buttons, if required
      QBoxLayout *buttonrow=new QHBoxLayout(topLayout);
      
      /** @ee add a close button */
      QPushButton *cmdClose = new QPushButton(tr("Close"), this);
      QFont font = cmdClose->font();
      font.setPixelSize(12);
      cmdClose->setFont(font);
      buttonrow->addWidget(cmdClose);
      
      QPushButton *cmdInfo = new QPushButton(tr("Info"), this);
      cmdInfo->setFont(font);
      buttonrow->addWidget(cmdInfo);
      
      cmdSelect = new QPushButton(_selectText, this);
      cmdSelect->setFont(font);
      buttonrow->addWidget(cmdSelect);
      
      connect(cmdSelect, SIGNAL(clicked()),
              this, SLOT(slot_Select()));
      connect(cmdInfo, SIGNAL(clicked()),
              this, SLOT(slot_Info()));
      connect(cmdClose, SIGNAL(clicked()),
              this, SLOT(slot_Close()));
      connect(list, SIGNAL(selectionChanged(Q3ListViewItem*)),
              this, SLOT(slot_Selected(Q3ListViewItem*)));
    }
}

TaskListView::~TaskListView()
{
  // qDebug("TaskListView::~TaskListView()");
  if( _task ) {
    delete _task;
  }
}


void TaskListView::slot_Selected(Q3ListViewItem * itm)
{
  _newSelectedTp = itm;
  _selectedWp = ((_TaskPoint*)itm)->wp;

  if( _selectedWp->taskPointIndex == 0 )
    {
      // Take-off point should not be selectable in taskview
      cmdSelect->setEnabled(false);
      return;
    }

  cmdSelect->setEnabled(true);

  if(_newSelectedTp == _currSelectedTp) {
    cmdSelect->setText(_unselectText);
  } else {
    cmdSelect->setText(_selectText);
  }
  //qDebug("New Selected Waypoint name: %s, Index=%d",
  //       _selectedWp->name.latin1(), _selectedWp->taskPointIndex );
}


void TaskListView::showEvent(QShowEvent *)
{
  // qDebug("TaskListView::showEvent(): name=%s", name());

  if( name() == QString("TaskRouteDisplay") )
    {
      // do nothing as display, there are no buttons avialable
      return;
    }

  const wayPoint *calcWp = calculator->getselectedWp();
  bool foundWp = false;
  Q3ListViewItemIterator it( list );

  // Set the list item to be selected, which is equal to the selected
  // calculator waypoint.
  for ( ; it.current(); ++it ) {

    _TaskPoint* tp = (_TaskPoint*) it.current();
    wayPoint*   wp = tp->wp;

    // Waypoints can be selected from different windows. We will
    // consider only waypoints for a selection, which are member of a
    // flighttask. In this case the taskPointIndex should be unequal to -1.
    if( calcWp && calcWp->origP == wp->origP &&
        calcWp->taskPointIndex == wp->taskPointIndex ) {
      list->setSelected( tp, true );
      _currSelectedTp = tp;
      _newSelectedTp = tp;
      _selectedWp = wp;
      cmdSelect->setText(_unselectText);
      foundWp = true;
      break;
    }
  }

  if( foundWp == false ) {
    // if no calculator waypoint selected, clear selection on listview
    list->clearSelection();
    _selectedWp = 0;
    _currSelectedTp = 0;
    _newSelectedTp = 0;
  }

  list->setFocus();
}


/** This signal is called to indicate that a selection has been made. */
void TaskListView::slot_Select()
{
  // qDebug("TaskListView::slot_Select(): Selected WP= %s, Index=%d",
  //       _selectedWp->name.latin1(), _selectedWp->taskPointIndex );

  if(_newSelectedTp == _currSelectedTp) {
    // this was an unselect
    //calculator->slot_WaypointChange(0, true);
    emit newWaypoint(0, true);
    list->clearSelection();
    cmdSelect->setText(_selectText);
    _selectedWp = 0;
    _currSelectedTp = 0;
    _newSelectedTp = 0;
  } else {
    _currSelectedTp = _newSelectedTp; // save last selection
    emit newWaypoint(getSelectedWaypoint(), true);
    emit done();
  }
}


/** This slot is called if the info button has been clicked */
void TaskListView::slot_Info()
{
  if (getSelectedWaypoint())
    emit info(getSelectedWaypoint());
}


/** @ee This slot is called if the listview is closed without selecting */
void TaskListView::slot_Close ()
{
  if( _newSelectedTp != _currSelectedTp ) {
    _newSelectedTp = _currSelectedTp; // selection change was not committed
  }

  emit done();
}


/**
 * Retrieves the waypoints from the task, and fills the list. 
 */
void TaskListView::slot_setTask(const FlightTask *tsk)
{
  // qDebug("TaskListView::slot_setTask()");

  // delete old task
  if( _task ) {
    delete _task;
    _task = 0;
  }

  list->clear();

  if (tsk == 0 ) {
    // No new task passed
    _task = 0;
    return;
  }

  // create a deep task copy
  _task = new FlightTask(*tsk);

  // Check, if a waypoint is selected in calculator. In this case set
  // it in tasklist as selected too.
  const wayPoint *calcWp = calculator->getselectedWp();

  Q3PtrList<wayPoint> tmpList = _task->getWPList();

  for( uint loop = tmpList.count(); loop > 0; loop-- ) {
    // qDebug("Getting item %d",loop-1);
    wayPoint* wp = tmpList.at( loop-1 );
    _TaskPoint* tp = new _TaskPoint( list, wp );

    if( calcWp && calcWp->origP == wp->origP ) {
      list->setSelected( tp, true );
      _currSelectedTp = tp;
      _selectedWp = wp;
    }
  }

  // set the total values in the header of this view
  distTotal->setText(  "S=" +_task->getTotalDistanceString() );
  speedTotal->setText( "V=" + _task->getSpeedString() );
  timeTotal->setText(  "T=" + _task->getTotalDistanceTimeString() );
}

/**
 * Updates the internal task data. Will be called after
 * configuration changes of task sector items
 */
void TaskListView::slot_updateTask()
{
  if( _task ) {
    _task->updateTask();
  }
}


/** Returns a pointer to the currently highlighted taskpoint. */
wayPoint * TaskListView::getSelectedWaypoint()
{
  return _selectedWp;
}


void TaskListView::clear()
{
  list->clear();
  distTotal->setText("");
  speedTotal->setText("");
  timeTotal->setText("");
}

TaskListView::_TaskPoint::_TaskPoint(Q3ListView *lv, wayPoint *point):Q3ListViewItem(lv)
{
  wp = point;
  QString typeName = wp->getTaskPointTypeString();

  // calculate sunset for the taskpoints
  QString sr, ss;
  QDate date = QDate::currentDate();
  Sonne::sonneAufUnter( sr, ss, date, wp->origP, 0 );

  setText(0, typeName);
  setText(1, wp->name);
  setText(2, " " + Distance::getText(wp->distance*1000,false,1));
  setText(3, " " + FlightTask::getDistanceTimeString(wp->distTime));
  setText(4, " " + wp->description);
  setText(5, " " + ss);
  
  if( QApplication::desktop()->screenGeometry().width() > 240 )
    {
      setPixmap(1, _globalMapConfig->getPixmap(wp->type,false,true));
    }
}
