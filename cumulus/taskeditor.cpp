/***********************************************************************
**
**   taskeditor.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2008-2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "airfield.h"
#include "taskeditor.h"
#include "mapcontents.h"
#include "flighttask.h"
#include "distance.h"
#include "generalconfig.h"
#include "mainwindow.h"
#include "listwidgetparent.h"
#include "airfieldlistwidget.h"
#include "waypointlistwidget.h"
#include "layout.h"

extern MapContents *_globalMapContents;
extern MainWindow  *_globalMainWindow;

TaskEditor::TaskEditor( QWidget* parent,
                        QStringList &taskNamesInUse,
                        FlightTask* task ) :
  QWidget( parent ),
  taskNamesInUse( taskNamesInUse ),
  lastSelectedItem(0)
{
  setObjectName("TaskEditor");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute( Qt::WA_DeleteOnClose );

  if( _globalMainWindow )
    {
      // Resize the dialog to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  if ( task )
    {
      planTask = task;
      editState = TaskEditor::edit;
      setWindowTitle(planTask->getTaskTypeString());
      editedTaskName = task->getTaskName();
    }
  else
    {
      planTask = new FlightTask( 0, false, "" );
      editState = TaskEditor::create;
      setWindowTitle(tr("New Task"));
    }

  taskName = new QLineEdit( this );
  taskName->setBackgroundRole( QPalette::Light );

  taskList = new QTreeWidget( this );
  taskList->setObjectName("taskList");

  taskList->setRootIsDecorated(false);
  taskList->setItemsExpandable(false);
  taskList->setUniformRowHeights(true);
  taskList->setAlternatingRowColors(true);
  taskList->setSelectionBehavior(QAbstractItemView::SelectRows);
  taskList->setSelectionMode(QAbstractItemView::SingleSelection);
  taskList->setColumnCount(4);

  taskList->hideColumn( 0 );

  QStringList sl;
  sl << tr("ID")
     << tr("Type")
     << tr("Waypoint")
     << tr("Length");

  taskList->setHeaderLabels(sl);

  taskList->header()->setResizeMode( QHeaderView::ResizeToContents );

  upButton = new QPushButton( this );
  upButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "up.png")) );
  upButton->setIconSize(QSize(IconSize, IconSize));
  upButton->setToolTip( tr("move selected waypoint up") );

  downButton = new QPushButton( this );
  downButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "down.png")) );
  downButton->setIconSize(QSize(IconSize, IconSize));
  downButton->setToolTip( tr("move selected waypoint down") );

  invertButton = new QPushButton( this );
  invertButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "resort.png")) );
  invertButton->setIconSize(QSize(IconSize, IconSize));
  invertButton->setToolTip( tr("reverse waypoint order") );

  addButton = new QPushButton( this );
  addButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "left.png")) );
  addButton->setIconSize(QSize(IconSize, IconSize));
  addButton->setToolTip( tr("add waypoint") );

  delButton = new QPushButton( this );
  delButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "right.png")) );
  delButton->setIconSize(QSize(IconSize, IconSize));
  delButton->setToolTip( tr("remove waypoint") );

  QPushButton* okButton = new QPushButton( this );
  okButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "ok.png")) );
  okButton->setIconSize(QSize(IconSize, IconSize));
  okButton->setToolTip( tr("save task") );

  QPushButton* cancelButton = new QPushButton( this );
  cancelButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "cancel.png")) );
  cancelButton->setIconSize(QSize(IconSize, IconSize));
  cancelButton->setToolTip( tr("cancel task") );

  // all single widgets and layouts in this grid
  QGridLayout* totalLayout = new QGridLayout( this );
  totalLayout->setMargin(5);

  // contains the task editor buttons
  QVBoxLayout* buttonLayout = new QVBoxLayout;
  buttonLayout->setMargin(0);

  totalLayout->addWidget( new QLabel( tr("Name:"), this ), 0, 0 );
  totalLayout->addWidget( taskName, 0, 1 );
  totalLayout->addWidget( taskList, 1, 0, 1, 2 );

  buttonLayout->addStretch( 10 );
  buttonLayout->addWidget( invertButton );
  buttonLayout->addWidget( upButton );
  buttonLayout->addWidget( downButton );
  buttonLayout->addSpacing(20);
  buttonLayout->addWidget( addButton  );
  buttonLayout->addWidget( delButton );
  buttonLayout->addStretch( 10 );
  totalLayout->addLayout( buttonLayout, 0, 2, 2, 1 );

  // Combo box for toggling between waypoint, airfield, outlanding lists
  listSelectCB = new QComboBox(this);
  listSelectCB->setEditable(false);

  totalLayout->addWidget( listSelectCB, 0, 3 );
  totalLayout->addWidget( okButton,     0, 5, Qt::AlignCenter  );
  totalLayout->addWidget( cancelButton, 0, 6, Qt::AlignCenter  );

  // descriptions of combo box selection elements
  listSelectText[0] = tr("Waypoints");
  listSelectText[1] = tr("Airfields");
  listSelectText[2] = tr("Outlandings");

  // create the actual lists
  waypointList[0] = new WaypointListWidget( this, false );

  // Airfield list
  QVector<enum MapContents::MapContentsListID> itemList;
  itemList << MapContents::AirfieldList << MapContents::GliderfieldList;
  waypointList[1] = new AirfieldListWidget( itemList, this, false );

  // outlanding list
  itemList.clear();
  itemList << MapContents::OutLandingList;
  waypointList[2] = new AirfieldListWidget( itemList, this, false );

  for( int i = 0; i < NUM_LISTS; i++ )
    {
      listSelectCB->addItem(listSelectText[i], i);
      totalLayout->addWidget( waypointList[i], 1, 3, 1, 4 );
    }

  totalLayout->setColumnStretch( 1, 4 );
  totalLayout->setColumnStretch( 4, 2 );

  // first selection is WPList if wp's are defined
  // set index in combo box to selected list
  QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

  listSelectCB->setCurrentIndex( wpList.count() ? 0 : 1 );

  // switch to list to be visible, hide the other one
  slotToggleList( wpList.count() ? 0 : 1 );

  if ( editState == TaskEditor::edit )
    {
      taskName->setText( planTask->getTaskName() );

      QList<TaskPoint *> tmpList = planTask->getTpList();

      // @AP: Make a deep copy from all elements of the list
      for ( int i=0; i < tmpList.count(); i++ )
        {
          tpList.append( new TaskPoint( *tmpList.at(i)) );
        }
    }

  __showTask();

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
           this, SLOT( slotAccept() ) );
  connect( cancelButton, SIGNAL( clicked() ),
           this, SLOT( slotReject() ) );

  connect( listSelectCB, SIGNAL(activated(int)),
           this, SLOT(slotToggleList(int)));

  connect( taskList, SIGNAL( itemClicked( QTreeWidgetItem*, int)),
           this, SLOT(slotItemClicked( QTreeWidgetItem*, int)) );
}

TaskEditor::~TaskEditor()
{
  qDeleteAll(tpList);
  tpList.clear();
}

void TaskEditor::__showTask()
{
  if ( tpList.count() == 0 )
    {
      enableCommandButtons();
      return;
    }

  planTask->setTaskPointList( FlightTask::copyTpList( &tpList ) );

  QString txt = planTask->getTaskTypeString() +
                " / " + planTask->getTaskDistanceString();

  setWindowTitle(txt);

  QList<TaskPoint *> tmpList = planTask->getTpList();

  taskList->clear();

  QStringList rowList;
  QString typeName, distance, idString;

  double distTotal = 0.0;

  for( int loop = 0; loop < tmpList.count(); loop++ )
    {
      TaskPoint* tp = tmpList.at( loop );
      typeName = tp->getTaskPointTypeString();

      distTotal += tp->distance;

      distance = Distance::getText(tp->distance*1000, true, 1);
      idString = QString( "%1").arg( loop, 2, 10, QLatin1Char('0') );

      rowList.clear();
      rowList << idString << typeName << tp->name << distance;
      taskList->addTopLevelItem( new QTreeWidgetItem(rowList, 0) );

      // reselect last selected item
      if( lastSelectedItem == loop )
        {
          taskList->setCurrentItem( taskList->topLevelItem(loop) );
        }
    }

  enableCommandButtons();
  lastSelectedItem = -1;

  if( distTotal > 0.0 )
    {
      distance = Distance::getText( distTotal*1000, true, 1 );

      rowList.clear();
      rowList << "Total" << tr("Total") << "" << distance;

      QTreeWidgetItem* item = new QTreeWidgetItem(rowList, 0);

      QFont font = item->font(1);
      font.setBold( true );
      item->setFont( 1, font );
      item->setFont( 3, font );
      taskList->addTopLevelItem( item );
    }

  resizeTaskListColumns();
}

/**
 * aligns the task list columns to their contents
*/
void TaskEditor::resizeTaskListColumns()
{
  taskList->resizeColumnToContents(0);
  taskList->resizeColumnToContents(1);
  taskList->resizeColumnToContents(2);
  taskList->resizeColumnToContents(3);
}

void TaskEditor::slotAddWaypoint()
{
  Waypoint *wp = waypointList[listSelectCB->currentIndex()]->getCurrentWaypoint();

  if( wp == 0 )
    {
      return;
    }

  QTreeWidgetItem *item = taskList->currentItem();

  if( item == 0 )
    {
      // empty list
      tpList.append( new TaskPoint(*wp) );

      // Remember last position.
      lastSelectedItem = 0;
    }
  else
    {
      int id = taskList->indexOfTopLevelItem( item );
      id++;
      tpList.insert( id, new TaskPoint(*wp) );

      // Remember last position.
      lastSelectedItem = id;
    }

  __showTask();
}

void TaskEditor::slotRemoveWaypoint()
{
  QTreeWidgetItem* selected = taskList->currentItem();

  if( selected == 0 || selected->text( 0 ) == "Total" )
    {
      return;
    }

  int id = taskList->indexOfTopLevelItem( taskList->currentItem() );

  delete taskList->takeTopLevelItem( taskList->currentIndex().row() );
  delete tpList.takeAt( id );

  // Remember last position.
  if( id >= tpList.size() )
    {
      lastSelectedItem = tpList.size() - 1;
    }
  else
    {
      lastSelectedItem = id;
    }

  __showTask();
}

void TaskEditor::slotInvertWaypoints()
{
  if ( tpList.count() < 2 )
    {
      // not possible to invert order, if elements are less 2
      return;
    }

  // invert list order
  for ( int i= tpList.count()-2; i >= 0; i-- )
    {
      TaskPoint* tp = tpList.at(i);
      tpList.removeAt(i);
      tpList.append( tp );
    }

  // After an invert the first task item is selected.
  lastSelectedItem = 0;

  __showTask();
}

void TaskEditor::slotAccept()
{
  // Check, if a valid task has been defined. Tasks with less than
  // four task points are incomplete
  if ( tpList.count() < 4 )
    {
      QMessageBox::critical(this,tr("Task Incomplete"),
                           tr("Task needs at least four waypoints"),
                           QMessageBox::Ok );
      return;
    }

  QString txt = taskName->text();

  // Check if the user has entered a task name
  if ( txt.length() == 0 )
    {
      QMessageBox::critical(this,tr("Name Missing"),
                           tr("Enter a name for the task to save it"),
                           QMessageBox::Ok );
      return;
    }

  if ( editState == TaskEditor::create )
    {
      // Check if the task name does not conflict with existing onces.
      // The name must be unique in the task name space

      if ( taskNamesInUse.contains( txt ) > 0 )
        {
          QMessageBox::critical(this,tr("Name in Use"),
                               tr("Please enter a different name"),
                               QMessageBox::Ok );
          return;
        }
    }
  else
    {
      // Check if the name of the edited task has been changed. In
      // that case we have to check if the new name is unique

      if ( txt != editedTaskName && taskNamesInUse.contains( txt ) > 0 )
        {
          QMessageBox::critical(this,tr("Name in Use"),
                               tr("Please enter a different name"),
                               QMessageBox::Ok );
          return;
        }
    }

  // Take over changed task data and publish it
  planTask->setTaskName(txt);

  if ( editState == TaskEditor::create )
    {
      emit newTask( planTask );
    }
  else
    {
      emit editedTask( planTask );
    }

  // closes and destroys window
  close();
}

void TaskEditor::slotReject()
{
  // qDebug("TaskEditor::reject()");

  // delete rejected task object
  delete planTask;
  // closes and destroys window
  close();
}

void TaskEditor::slotMoveWaypointUp()
{
  if( taskList->selectedItems().size() == 0 ||
      taskList->topLevelItemCount() <= 2 ||
      taskList->currentItem()->text( 0 ) == "Total" )
    {
      return;
    }

  int id = taskList->indexOfTopLevelItem( taskList->currentItem() );

  // we can't move the first item up
  if( id <= 0 )
    {
      return;
    }

  lastSelectedItem = id - 1;

  tpList.move( id, id - 1 );

  __showTask();
}

void TaskEditor::slotMoveWaypointDown()
{
  if( taskList->selectedItems().size() == 0 ||
      taskList->topLevelItemCount() <= 2 ||
      taskList->currentItem()->text(0) == "Total" )
    {
      return;
    }

  int id = taskList->indexOfTopLevelItem( taskList->currentItem() );

  // we can't move the last item down
  if( id == -1 || id == taskList->topLevelItemCount() - 1 ||
      ( id == taskList->topLevelItemCount() - 2 &&
        taskList->topLevelItem( taskList->topLevelItemCount() - 1)->text( 0 ) == "Total" ) )
    {
      return;
    }

  lastSelectedItem = id + 1;

  tpList.move(id,  id + 1);

  __showTask();
}

/** Toggle between WP/AF/... list on user request */
void TaskEditor::slotToggleList(int index)
{
  for( int i = 0; i < NUM_LISTS; i++ )
    {
      if( i != index )
        {
          waypointList[i]->hide();
        }
      else
        {
          waypointList[i]->show();
        }
    }
}

void TaskEditor::slotItemClicked( QTreeWidgetItem* item, int column )
{
  Q_UNUSED( column )

  if( item && item->text(0) == "Total" )
    {
      // It is not allowed to select the last row. We move the selection
      // one row up.
      taskList->setCurrentItem( taskList->itemAbove( item ) );
    }

  enableCommandButtons();
}

void TaskEditor::enableCommandButtons()
{
  if( tpList.size() == 0 )
    {
      upButton->setEnabled( false );
      downButton->setEnabled( false );
      invertButton->setEnabled( false );
      addButton->setEnabled( true );
      delButton->setEnabled( false );
    }
  else if( tpList.size() == 1 )
    {
      upButton->setEnabled( false );
      downButton->setEnabled( false );
      invertButton->setEnabled( false );
      addButton->setEnabled( true );
      delButton->setEnabled( true );
    }
  else
    {
      invertButton->setEnabled( true );
      addButton->setEnabled( true );
      delButton->setEnabled( true );

      if( taskList->topLevelItemCount() && taskList->currentItem() == 0 )
        {
          // If no item is selected we select the first one.
          taskList->setCurrentItem(taskList->topLevelItem(taskList->indexOfTopLevelItem(0)));
        }

      if( taskList->indexOfTopLevelItem(taskList->currentItem()) > 0 )
        {
          upButton->setEnabled( true );
        }
      else
        {
          // At the first position, no up allowed
          upButton->setEnabled( false );
        }

      int id = taskList->indexOfTopLevelItem( taskList->currentItem() );

      if( id == -1 || id == taskList->topLevelItemCount() - 1 ||
          ( id == taskList->topLevelItemCount() - 2 &&
            taskList->topLevelItem( taskList->topLevelItemCount() - 1)->text( 0 ) == "Total" ) )
        {
          // At the last allowed down position. No further down allowed.
          downButton->setEnabled( false );
        }
      else
        {
          downButton->setEnabled( true );
        }
    }
}
