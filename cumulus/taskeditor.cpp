/***********************************************************************
**
**   taskeditor.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
#include <QVector>

#include "airfield.h"
#include "taskeditor.h"
#include "mapcontents.h"
#include "flighttask.h"
#include "distance.h"
#include "generalconfig.h"
#include "mainwindow.h"
#include "wplistwidgetparent.h"
#include "airfieldlistwidget.h"
#include "waypointlistwidget.h"

extern MapContents *_globalMapContents;
extern MainWindow  *_globalMainWindow;

TaskEditor::TaskEditor( QWidget* parent,
                        QStringList &taskNamesInUse,
                        FlightTask* task ) :
  QDialog( parent ),
  taskNamesInUse( taskNamesInUse )
{
  setObjectName("TaskEditor");
  setModal(true);
  setAttribute( Qt::WA_DeleteOnClose );

  if( _globalMainWindow )
    {
      // Resize the dialog to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  lastSelectedItem = -1;

  if ( task )
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
  taskName->setBackgroundRole( QPalette::Light );

  taskList = new QTreeWidget( this );
  taskList->setObjectName("taskList");

  taskList->setRootIsDecorated(false);
  taskList->setItemsExpandable(false);
  taskList->setUniformRowHeights(true);
//  taskList->setSortingEnabled(true);
  taskList->setSelectionBehavior(QAbstractItemView::SelectRows);
  taskList->setSelectionMode(QAbstractItemView::SingleSelection);
  taskList->setColumnCount(4);

  QStringList sl;
  sl << "ID" << "Type" << "Waypoint" << "Distance";
  taskList->setHeaderLabels(sl);

  QPushButton* upButton = new QPushButton( this );
  upButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "up.png")) );
  upButton->setToolTip( tr("move selected waypoint up") );

  QPushButton* downButton = new QPushButton( this );
  downButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "down.png")) );
  downButton->setToolTip( tr("move selected waypoint down") );

  QPushButton* invertButton = new QPushButton( this );
  invertButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "resort.png")) );
  invertButton->setToolTip( tr("reverse waypoint order") );

  QPushButton* addButton = new QPushButton( this );
  addButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "left.png")) );
  addButton->setToolTip( tr("add waypoint") );

  QPushButton* delButton = new QPushButton( this );
  delButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "right.png")) );
  delButton->setToolTip( tr("remove waypoint") );

  QPushButton* okButton = new QPushButton( this );
  okButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "ok.png")) );
  okButton->setIconSize(QSize(26,26));
  okButton->setToolTip( tr("save task") );

  QPushButton* cancelButton = new QPushButton( this );
  cancelButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "cancel.png")) );
  cancelButton->setIconSize(QSize(26,26));
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
  waypointList[0] = new WaypointListWidget(this);

  // Airfield list
  QVector<enum MapContents::MapContentsListID> itemList;
  itemList << MapContents::AirfieldList << MapContents::GliderSiteList;
  waypointList[1] = new AirfieldListWidget( itemList, this );

  // outlanding list
  itemList.clear();
  itemList << MapContents::OutLandingList;
  waypointList[2] = new AirfieldListWidget( itemList, this );

  for( int i=0; i<NUM_LISTS; i++ )
    {
      listSelectCB->addItem(listSelectText[i], i);
      waypointList[i]->fillWpList();
      totalLayout->addWidget( waypointList[i], 1, 3, 1, 4 );
    }

  totalLayout->setColumnStretch( 1, 4 );
  totalLayout->setColumnStretch( 4, 2 );

  // first selection is WPList if wp's are defined
  // set index in combo box to selected list
  QList<wayPoint>& wpList = _globalMapContents->getWaypointList();

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

  qDeleteAll(tpList);
  tpList.clear();
}

void TaskEditor::__showTask()
{
  if ( tpList.count() == 0 )
    {
      this->setWindowTitle(tr("New Task"));
      return;
    }

  planTask->setTaskPointList( FlightTask::copyTpList( &tpList ) );

  QString txt = planTask->getTaskTypeString() +
                " / " + planTask->getTaskDistanceString();

  this->setWindowTitle(txt);

  QList<TaskPoint *> tmpList = planTask->getTpList();

  taskList->clear();

  QString typeName, distance, idString;

  for ( int loop = 0; loop < tmpList.count(); loop++ )
    {
      TaskPoint* tp = tmpList.at( loop );
      typeName = tp->getTaskPointTypeString();

      distance = Distance::getText(tp->distance*1000, true, 1);
      idString = QString( "%1").arg( loop, 2, 10, QLatin1Char('0') );

      QStringList rowList;
      rowList << idString << typeName << tp->name << distance;
      taskList->addTopLevelItem( new QTreeWidgetItem(rowList, 0) );

      // reselect last selected item
      if ( lastSelectedItem == (int) loop )
        {
          taskList->setCurrentItem( taskList->topLevelItem(loop) );
          lastSelectedItem = -1;
        }
    }

  taskList->setSortingEnabled(true);
  taskList->sortByColumn(0, Qt::AscendingOrder);
  taskList->setSortingEnabled(false);

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
  wayPoint *wp = waypointList[listSelectCB->currentIndex()]->getSelectedWaypoint();

  if ( wp == 0 )
    {
      return;
    }

  tpList.append( new TaskPoint(*wp) );

  __showTask();
}

void TaskEditor::slotRemoveWaypoint()
{
  QTreeWidgetItem* selected = taskList->currentItem();
  if ( selected == 0 )
    {
      return;
    }

  int id = selected->text(0).toInt();
  delete tpList.takeAt( id );

  delete taskList->takeTopLevelItem( taskList->currentIndex().row() );

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

  __showTask();
}

void TaskEditor::accept()
{
  // qDebug("TaskEditor::accept()");

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

  // emit done();
  // close and destroy dialog
  QDialog::done(QDialog::Accepted);
}

void TaskEditor::reject()
{
  // qDebug("TaskEditor::reject()");

  // delete rejected task object
  delete planTask;
  // close and destroy dialog
  QDialog::done(QDialog::Rejected);
}

void TaskEditor::slotMoveWaypointUp()
{
  if ( taskList->selectedItems().size() == 0 || taskList->topLevelItemCount() <= 2 )
    return;

  int id = taskList->currentItem()->text(0).toInt();
  // we can't move the first item up
  if ( id == 0 )
    return;

  lastSelectedItem = id - 1;

  tpList.move(id, id-1);

  __showTask();
}

void TaskEditor::slotMoveWaypointDown()
{
  if ( taskList->selectedItems().size() == 0 || taskList->topLevelItemCount() <= 2 )
    return;

  int id = taskList->currentItem()->text(0).toInt();
  // we can't move the last item down
  if ( id == taskList->topLevelItemCount() - 1 )
    return;

  lastSelectedItem = id + 1;

  tpList.move(id,  id + 1);

  __showTask();
}

/** Toggle between WP/AF/... list on user request */
void TaskEditor::slotToggleList(int index)
{
  for ( int i=0; i<NUM_LISTS; i++ )
    {
      if (i != index)
        {
          waypointList[i]->hide();
        }
      else
        {
          waypointList[i]->show();
        }
    }
}
