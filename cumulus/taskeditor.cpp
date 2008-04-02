/***********************************************************************
**
**   taskeditor.cpp
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
**   Description: This class handles creation and modification of
**   flight tasks in a simple editor.
**
***********************************************************************/

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QToolTip>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "airport.h"
#include "taskeditor.h"
#include "mapcontents.h"
#include "flighttask.h"
#include "distance.h"
#include "generalconfig.h"
#include "cumulusapp.h"

extern MapContents *_globalMapContents;
extern CumulusApp  *_globalCumulusApp;

TaskEditor::TaskEditor( QWidget* parent, QStringList &taskNamesInUse,
                        FlightTask* task ) :
    QDialog( parent ),
    taskNamesInUse( taskNamesInUse )
{
  setObjectName("TaskEditor");
  setModal(true);
  setSizeGripEnabled(true);

  wpList = _globalMapContents->getWaypointList();
  lastSelectedItem = -1;

  if( task )
    {
      planTask = task;
      editState = TaskEditor::edit;
      this->setWindowTitle(planTask->getTaskTypeString());
      editedTaskName = task->getTaskName();
    }
  else
    {
      planTask = new FlightTask( 0, false, "" );
      editState = TaskEditor::create;
      this->setWindowTitle(tr("New Task"));
    }

  taskName = new QLineEdit( this );
  taskName->setBackgroundMode( Qt::PaletteLight );

  taskList = new Q3ListView( this, "taskList" );
  taskList->addColumn( tr("Id") );
  taskList->addColumn( tr("Type") );
  taskList->addColumn( tr("Waypoint") );
  taskList->addColumn( tr("Distance") );

  taskList->setColumnAlignment( 3, Qt::AlignRight );

  taskList->setAllColumnsShowFocus( true );

  QPushButton* upButton = new QPushButton( this );
  upButton->setPixmap( GeneralConfig::instance()->loadPixmap( "moveup.png") );
  upButton->setFlat(true);
  upButton->setToolTip( tr("move selected waypoint up") );

  QPushButton* downButton = new QPushButton( this );
  downButton->setPixmap( GeneralConfig::instance()->loadPixmap( "movedown.png") );
  downButton->setFlat(true);
  downButton->setToolTip( tr("move selected waypoint down") );

  QPushButton* invertButton = new QPushButton( this );
  invertButton->setPixmap( GeneralConfig::instance()->loadPixmap( "invert.png") );
  invertButton->setFlat(true);
  invertButton->setToolTip( tr("reverse waypoint order") );

  QPushButton* addButton = new QPushButton( this );
  addButton->setPixmap( GeneralConfig::instance()->loadPixmap( "moveleft.png") );
  addButton->setFlat(true);
  addButton->setToolTip( tr("add waypoint") );

  QPushButton* delButton = new QPushButton( this );
  delButton->setPixmap( GeneralConfig::instance()->loadPixmap( "moveright.png") );
  delButton->setFlat(true);
  delButton->setToolTip( tr("remove waypoint") );

  QPushButton* okButton = new QPushButton( this );
  okButton->setPixmap( GeneralConfig::instance()->loadPixmap( "standardbutton-apply-16.png") );
  okButton->setFlat(true);
  okButton->setToolTip( tr("save task") );

  QPushButton* cancelButton = new QPushButton( this );
  cancelButton->setPixmap( GeneralConfig::instance()->loadPixmap( "standardbutton-cancel-16.png") );
  cancelButton->setFlat(true);
  cancelButton->setToolTip( tr("cancel task") );

  // all single layouts are put in this horizontal box
  QHBoxLayout* totalLayout = new QHBoxLayout( this );
  totalLayout->setMargin(5);

  // contains the task part
  QGridLayout* taskLayout = new QGridLayout;
  taskLayout->setMargin(0);

  // contains the task editor buttons
  QVBoxLayout* buttonLayout = new QVBoxLayout;
  buttonLayout->setMargin(0);

  // contains the selection list part
  QGridLayout* listLayout = new QGridLayout;
  listLayout->setMargin(0);

  totalLayout->addLayout( taskLayout );
  totalLayout->addLayout( buttonLayout );
  totalLayout->addLayout( listLayout );

  taskLayout->addWidget( new QLabel( tr("Name:"), this ), 0, 0 );
  taskLayout->addWidget( taskName, 0, 1 );
  taskLayout->addWidget( taskList, 1, 0, 1, 2 );
  taskLayout->setRowStretch( 1, 10 );
  
  buttonLayout->addStretch( 10 );
  buttonLayout->addWidget( invertButton );
  buttonLayout->addWidget( upButton );
  buttonLayout->addWidget( downButton );
  buttonLayout->addSpacing(20);
  buttonLayout->addWidget( addButton  );
  buttonLayout->addWidget( delButton );
  buttonLayout->addStretch( 10 );

  // Combo box for toggling between waypoint and airfield lists
  listSelectCB = new QComboBox(this);
  listSelectCB->setEditable(false);
  
  // descriptions of combo box selection elements
  listSelectText[0] = tr("Waypoints");
  listSelectText[1] = tr("Airfields");

  listLayout->addWidget( listSelectCB, 0, 0 );
  listLayout->addWidget( okButton,     0, 2, Qt::AlignCenter  );
  listLayout->addWidget( cancelButton, 0, 3, Qt::AlignCenter  );

  listLayout->setColumnStretch( 1, 10 );

  for(int i=0; i<NUM_LISTS; i++)
    {
      listSelectCB->addItem(listSelectText[i], i);
      waypointList[i] = new Q3ListView( this, "waypointList" );
      waypointList[i]->addColumn( tr("Name") );
      waypointList[i]->addColumn( tr("Description") );
      waypointList[i]->addColumn( tr("ICAO") );
      waypointList[i]->setAllColumnsShowFocus( true );
      waypointList[i]->setFocus();
      waypointList[i]->setMinimumWidth( 420 );
      
      filter[i] = new ListViewFilter(waypointList[i], this, "listfilter");

      listLayout->addWidget( filter[i], 1, 0, 1, 4 );
      listLayout->addWidget( waypointList[i], 2, 0, 1, 4 );
    }

  _globalCumulusApp->viewWP->fillWpList(wpList, waypointList[0], filter[0]);
  _globalCumulusApp->viewAF->fillWpList(waypointList[1], filter[1]);

  // first selection is WPList if wp's are defined
  // set index in combo box to selected list
  listSelectCB->setCurrentIndex( wpList->count() ? 0 : 1 );
  
  // switch to list to be visible, hide the other one
  slotToggleList( wpList->count() ? 0 : 1 );

  if( editState == TaskEditor::edit )
    {
      taskName->setText( planTask->getTaskName() );

      QList<wayPoint*> tmpList = planTask->getWPList();

      // @AP: Make a deep copy from all elements of the list
      for( int i=0; i < tmpList.count(); i++ )
        {
          taskWPList.append( new wayPoint(*tmpList.at(i)) );
        }

      __showTask();
    }

  connect( addButton,    SIGNAL( clicked() ),
           this, SLOT( slotAddWaypoint() ) );
  connect( delButton,    SIGNAL( clicked() ),
           this, SLOT( slotRemoveWaypoint() ) );
  connect( upButton,     SIGNAL( clicked() ),
           this, SLOT( slotMoveWaypointUp() ) );
  connect( downButton,   SIGNAL( clicked() ),
           this, SLOT( slotMoveWaypointDown() ) );
  connect( invertButton, SIGNAL( clicked() ),
           this, SLOT( slotInvertWaypoints() ) );

  connect( okButton, SIGNAL( clicked() ),
           this, SLOT( accept() ) );
  connect( cancelButton, SIGNAL( clicked() ),
           this, SLOT( reject() ) );

  connect( listSelectCB, SIGNAL(activated(int)),
           this, SLOT(slotToggleList(int)));
}

TaskEditor::~TaskEditor()
{
  // qDebug("TaskEditor::~TaskEditor()");

  qDeleteAll(taskWPList);
  taskWPList.clear();

  for(int i=0; i<NUM_LISTS; i++)
    {
      waypointList[i]->clear();
    }
}

void TaskEditor::__showTask()
{
  if( taskWPList.count() == 0 )
    {
      this->setWindowTitle(tr("New Task"));
      return;
    }

  planTask->setWaypointList( FlightTask::copyWpList( &taskWPList ) );

  QString txt = planTask->getTaskTypeString() +
                " / " + planTask->getTaskDistanceString();

  this->setWindowTitle(txt);

  QList<wayPoint*> tmpList = planTask->getWPList();

  taskList->clear();

  QString typeName, distance, idString;

  for( int loop = 0; loop < tmpList.count(); loop++ )
    {
      wayPoint* wp = tmpList.at( loop );
      typeName = wp->getTaskPointTypeString();

      distance = Distance::getText(wp->distance*1000, true, 1);
      idString.sprintf( "%02d", loop );

      Q3ListViewItem *newLvi =
        new Q3ListViewItem( taskList,
                            idString, typeName, wp->name, distance );

      taskList->insertItem( newLvi );

      // reselect last selected item
      if( lastSelectedItem == (int) loop )
        {
          taskList->setSelected( newLvi, true );
          lastSelectedItem = -1;
        }
    }
}

void TaskEditor::slotAddWaypoint()
{
  if( waypointList[listSelectCB->currentItem()]->selectedItem() == 0 )
    {
      return;
    }

  if(listSelectCB->currentItem() == 0)
    {
      wayPoint *wp = _globalCumulusApp->viewWP->getSelectedWaypoint(waypointList[listSelectCB->currentItem()]);
      // if this is not a copy, the global wpList gets corrupted
      // since taskWPList has setAutoDelete to true
      taskWPList.append( new wayPoint(*wp) );
    }
  else if(listSelectCB->currentItem() == 1)
    {
      wayPoint *wp = _globalCumulusApp->viewAF->getSelectedAirfield(waypointList[listSelectCB->currentItem()]);
      // if this is not a copy, all taskpoints are the same since the
      // returned wp is (always the same) local variable of AirfieldListView
      taskWPList.append( new wayPoint(*wp) );
    }

  __showTask();
}

void TaskEditor::slotRemoveWaypoint()
{
  if( taskList->selectedItem() == 0 )
    {
      return;
    }

  int id( taskList->selectedItem()->text(0).toInt() );

  delete taskWPList.takeAt( id );
  taskList->takeItem( taskList->selectedItem() );

  __showTask();
}

void TaskEditor::slotInvertWaypoints()
{
  if( taskWPList.count() < 2 )
    {
      // not possible to invert order, if elements are less 2
      return;
    }

  // invert list order
  for( int i= (int) taskWPList.count()-2; i >= 0; i-- )
    {
      wayPoint* wp = taskWPList.at(i);
      taskWPList.removeAt(i);
      taskWPList.append( wp );
    }

  __showTask();
}

void TaskEditor::accept()
{
  // qDebug("TaskEditor::accept()");

  // Check, if a sensefull task has been defined. Tasks with less as
  // four waypoints are incomplete and will be ignored.
  if( taskWPList.count() < 4 )
    {
      QMessageBox::warning(this,tr("Task incomplete!"),
                           tr("<html><b>"
                              "Task is incomplete<br>"
                              "Please add some more waypoints!"
                              "</b></html>"),
                           QMessageBox::Ok | QMessageBox::Default, 0 );
      return;
    }

  QString txt = taskName->text();

  // Check, if the user has entered a task name. Nameless task will
  // be ignored.
  if( txt.length() == 0 )
    {
      QMessageBox::warning(this,tr("Missing name?"),
                           tr("<html><b>"
                              "Please enter a task name before exit!"
                              "</b></html>"),
                           QMessageBox::Ok | QMessageBox::Default, 0 );
      return;
    }

  if( editState == TaskEditor::create )
    {
      // Check the name of the task, if it does not conflictct with
      // exististing onces. The name must be unique in the task name
      // space.

      if( taskNamesInUse.contains( txt ) > 0 )
        {
          QMessageBox::warning(this,tr("Name already in use!"),
                               tr("<html><b>"
                                  "Task name already in use<br>"
                                  "Please enter another name before exit!"
                                  "</b></html>"),
                               QMessageBox::Ok | QMessageBox::Default, 0 );
          return;
        }
    }
  else
    {
      // Check, if the name of the edited task has been changed. In
      // this case we have to look, if the new name is unique in the
      // task name space.

      if( txt != editedTaskName && taskNamesInUse.contains( txt ) > 0 )
        {
          QMessageBox::warning(this,tr("Name already in use!"),
                               tr("<html><b>"
                                  "Task name already in use<br>"
                                  "Please enter another name before exit!"
                                  "</b></html>"),
                               QMessageBox::Ok | QMessageBox::Default, 0 );
          return;
        }
    }

  // Overtake changed task data and publish it
  QDialog::accept();
  planTask->setTaskName(txt);

  if( editState == TaskEditor::create )
    {
      emit newTask( planTask );
    }
  else
    {
      emit editedTask( planTask );
    }

  planTask = 0;
  delete this;
}

void TaskEditor::reject()
{
  // qDebug("TaskEditor::reject()");
  QDialog::reject();

  delete planTask;
  delete this;
}

void TaskEditor::slotMoveWaypointUp()
{
  if( taskList->selectedItem() == 0 || taskList->childCount() <= 2 )
    return;

  unsigned int id( taskList->selectedItem()->text( 0 ).toInt() );
  // we can't  move the first item
  if( id == 0 )
    return;

  lastSelectedItem = id - 1;

  taskWPList.move(id, id-1);

  __showTask();
}

void TaskEditor::slotMoveWaypointDown()
{
  if( taskList->selectedItem() == 0 || taskList->childCount() <= 2 )
    return;

  int id( taskList->selectedItem()->text( 0 ).toInt() );
  // we can't move the last item
  if( id == taskList->childCount() - 1 )
    return;

  lastSelectedItem = id + 1;

  taskWPList.move(id,  id + 1);

  __showTask();
}

/** Toggle between WP/AF/... list on user request */
void TaskEditor::slotToggleList(int index)
{
  for( int i=0; i<NUM_LISTS; i++ )
    {
      if(i != index)
        {
          waypointList[i]->hide();
          filter[i]->hide();
        }
      else
        {
          waypointList[i]->show();
          filter[i]->show();
        }
    }
}
