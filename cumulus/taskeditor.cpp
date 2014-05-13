/***********************************************************************
**
**   taskeditor.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2008-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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

#include "airfield.h"
#include "airfieldlistwidget.h"
#include "distance.h"
#include "flighttask.h"
#include "generalconfig.h"
#include "layout.h"
#include "listwidgetparent.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "rowdelegate.h"
#include "taskeditor.h"
#include "taskpoint.h"
#include "taskpointeditor.h"
#include "waypointlistwidget.h"

extern MapContents *_globalMapContents;

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

  if( MainWindow::mainWindow() )
    {
      // Resize the dialog to the same size as the main window has. That will
      // completely hide the parent window.
      resize( MainWindow::mainWindow()->size() );
    }

  if ( task )
    {
      task2Edit = task;
      editState = TaskEditor::edit;
      setWindowTitle( task2Edit->getTaskTypeString() );
      editedTaskName = task->getTaskName();
    }
  else
    {
      task2Edit = new FlightTask( 0, false, "" );
      editState = TaskEditor::create;
      setWindowTitle(tr("New Task"));
    }

  Qt::InputMethodHints imh;

  taskName = new QLineEdit( this );
  taskName->setBackgroundRole( QPalette::Light );
  imh = (taskName->inputMethodHints() | Qt::ImhNoPredictiveText);
  taskName->setInputMethodHints(imh);

  // The task name maximum length is 10 characters. We calculate
  // the length of a M string of 10 characters. That is the maximum
  // width of the QLineEdit widget.
  QFontMetrics fm( font() );
  int maxInputLength = fm.width("MMMMMMMMMM");
  taskName->setMinimumWidth( maxInputLength );
  taskName->setMaximumWidth( maxInputLength );

  connect( taskName, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

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

  const int iconSize = Layout::iconSize( font() );
  taskList->setIconSize( QSize(iconSize, iconSize) );

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  taskList->setItemDelegate( new RowDelegate( taskList, afMargin ) );

  QStringList sl;
  sl << tr("ID")
     << tr("Type")
     << tr("Waypoint")
     << tr("Length");

  taskList->setHeaderLabels(sl);
#if QT_VERSION >= 0x050000
  taskList->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
#else
  taskList->header()->setResizeMode( QHeaderView::ResizeToContents );
#endif

  taskList->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  taskList->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture( taskList->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( taskList->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  upButton = new QPushButton( this );
  upButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "up.png")) );
  upButton->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  upButton->setToolTip( tr("move selected waypoint up") );
#endif
  downButton = new QPushButton( this );
  downButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "down.png")) );
  downButton->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  downButton->setToolTip( tr("move selected waypoint down") );
#endif
  invertButton = new QPushButton( this );
  invertButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "resort.png")) );
  invertButton->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  invertButton->setToolTip( tr("reverse waypoint order") );
#endif
  addButton = new QPushButton( this );
  addButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "left.png")) );
  addButton->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  addButton->setToolTip( tr("add waypoint") );
#endif
  delButton = new QPushButton( this );
  delButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "right.png")) );
  delButton->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  delButton->setToolTip( tr("remove waypoint") );
#endif
  QPushButton* okButton = new QPushButton( this );
  okButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "ok.png")) );
  okButton->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  okButton->setToolTip( tr("save task") );
#endif
  QPushButton* cancelButton = new QPushButton( this );
  cancelButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "cancel.png")) );
  cancelButton->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  cancelButton->setToolTip( tr("cancel task") );
#endif

  // all single widgets and layouts in this grid
  QGridLayout* totalLayout = new QGridLayout( this );
  totalLayout->setMargin(5);

  QHBoxLayout* headlineLayout = new QHBoxLayout;
  totalLayout->addLayout( headlineLayout, 0, 0, 1, 3 );

  headlineLayout->setMargin(0);
  headlineLayout->addWidget( new QLabel( tr("Name:") ) );
  headlineLayout->addWidget( taskName );

  // Combo box for toggling between waypoint, airfield, outlanding lists
  listSelectCB = new QComboBox(this);
  listSelectCB->setEditable(false);
  headlineLayout->addWidget( listSelectCB );
  //headlineLayout->addSpacing(25);

  QStyle* style = QApplication::style();
  defaultButton = new QPushButton;
  defaultButton->setIcon(style->standardIcon(QStyle::SP_DialogResetButton));
  defaultButton->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  defaultButton->setToolTip(tr("Set task figure default schemas"));
#endif
  headlineLayout->addWidget(defaultButton);
  //headlineLayout->addSpacing(20);

  editButton = new QPushButton;
  editButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  editButton->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  editButton->setToolTip(tr("Edit selected waypoint"));
#endif
  headlineLayout->addWidget(editButton);
  //headlineLayout->setSpacing(20);
  headlineLayout->addWidget(okButton);
  //headlineLayout->addSpacing(20);
  headlineLayout->addWidget(cancelButton);

  totalLayout->addWidget( taskList, 1, 0 );

  // contains the task editor buttons
  QVBoxLayout* buttonLayout = new QVBoxLayout;
  buttonLayout->setMargin(0);
  buttonLayout->addStretch( 10 );
  buttonLayout->addWidget( invertButton );
  buttonLayout->addSpacing(10);
  buttonLayout->addWidget( upButton );
  buttonLayout->addSpacing(10);
  buttonLayout->addWidget( downButton );
  buttonLayout->addSpacing(30);
  buttonLayout->addWidget( addButton  );
  buttonLayout->addSpacing(10);
  buttonLayout->addWidget( delButton );
  buttonLayout->addStretch( 10 );
  totalLayout->addLayout( buttonLayout, 1, 1 );

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
      totalLayout->addWidget( waypointList[i], 1, 2 );
    }

  // first selection is WPList if wp's are defined
  // set index in combo box to selected list
  QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

  listSelectCB->setCurrentIndex( wpList.count() ? 0 : 1 );

  // switch to list to be visible, hide the other one
  slotToggleList( wpList.count() ? 0 : 1 );

  if ( editState == TaskEditor::edit )
    {
      taskName->setText( task2Edit->getTaskName() );

      QList<TaskPoint *> tmpList = task2Edit->getTpList();

      // @AP: Make a deep copy from all elements of the list
      for ( int i=0; i < tmpList.count(); i++ )
        {
          tpList.append( new TaskPoint( *tmpList.at(i)) );
        }
    }

  showTask();

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

  connect( defaultButton, SIGNAL(clicked()),
           this, SLOT(slotSetTaskPointsDefaultSchema()));
  connect( editButton, SIGNAL(clicked()),
           this, SLOT(slotEditTaskPoint()));

  connect( okButton, SIGNAL( clicked() ),
           this, SLOT( slotAccept() ) );
  connect( cancelButton, SIGNAL( clicked() ),
           this, SLOT( slotReject() ) );

  connect( listSelectCB, SIGNAL(activated(int)),
           this, SLOT(slotToggleList(int)));
  connect( taskList, SIGNAL (currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
           this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)) );
}

TaskEditor::~TaskEditor()
{
  qDeleteAll(tpList);
  tpList.clear();
}

void TaskEditor::showTask()
{
  if ( tpList.count() == 0 )
    {
      enableCommandButtons();
      return;
    }

  // That updates all task internal data
  task2Edit->setTaskPointList( FlightTask::copyTpList( &tpList ) );

  QString txt = task2Edit->getTaskTypeString() +
                " / " + task2Edit->getTaskDistanceString();

  setWindowTitle(txt);

  QList<TaskPoint *> tmpList = task2Edit->getTpList();

  taskList->clear();

  QStringList rowList;
  QString typeName, distance, idString;

  double distTotal = 0.0;

  for( int loop = 0; loop < tmpList.size(); loop++ )
    {
      TaskPoint* tp = tmpList.at( loop );
      typeName = tp->getTaskPointTypeString();

      distTotal += tp->distance;

      distance = Distance::getText(tp->distance*1000, true, 1);
      idString = QString( "%1").arg( loop, 2, 10, QLatin1Char('0') );

      rowList.clear();
      rowList << idString << typeName << tp->getWPName() << distance;

      const int iconSize = Layout::iconSize( font() );

      QTreeWidgetItem* item = new QTreeWidgetItem(rowList, 0);

      bool showIcon = true;

      if( tmpList.size() >= 2 &&
          ((loop == 0 && tmpList.at(0)->getWGSPosition() == tmpList.at(1)->getWGSPosition() ) ||
           (loop == tmpList.size()-1 &&
            tmpList.at(tmpList.size()-1)->getWGSPosition() == tmpList.at(tmpList.size()-2)->getWGSPosition() )) )
        {
          // If start and begin point or end and landing point are identical
          // no task figure icon is shown in the list entry.
          showIcon = false;
        }

      if( tmpList.size() > 1 && showIcon )
        {
          item->setIcon ( 1, tp->getIcon( iconSize ) );
        }

      if( tp->getUserEditFlag() == true )
        {
          // Mark the user edited entry
          item->setBackground( 1, QBrush(Qt::yellow) );
        }

      taskList->addTopLevelItem( item );

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
      rowList << "Total" << "" << tr("Total") << distance;

      QTreeWidgetItem* item = new QTreeWidgetItem(rowList, 0);

      // make the total line unselectable
      item->setFlags( Qt::ItemIsEnabled );

      QFont font = item->font(1);
      font.setBold( true );
      item->setFont( 2, font );
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
  Waypoint* wp = waypointList[listSelectCB->currentIndex()]->getCurrentWaypoint();

  if( wp == 0 )
    {
      return;
    }

  QTreeWidgetItem *item = taskList->currentItem();

  if( item == 0 )
    {
      // empty list

      // A taskpoint is only a single point and not more!
      TaskPoint* tp = new TaskPoint( *wp );
      tpList.append( tp );

      // Remember last position.
      lastSelectedItem = 0;
    }
  else
    {
      int id = taskList->indexOfTopLevelItem( item );
      id++;

      // A taskpoint is only a single point and not more!
      TaskPoint* tp = new TaskPoint( *wp );
      tpList.insert( id, tp );

      // Remember last position.
      lastSelectedItem = id;
    }

  setTaskPointFigureSchemas( tpList );
  showTask();
}

void TaskEditor::slotRemoveWaypoint()
{
  QTreeWidgetItem* selected = taskList->currentItem();

  if( selected == 0 )
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

  setTaskPointFigureSchemas( tpList );
  showTask();
}

void TaskEditor::slotInvertWaypoints()
{
  if ( tpList.count() < 4 )
    {
      // not possible to invert order, if elements are less 4
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

  // Swap schema data between begin and and point.
  swapTaskPointSchemas( tpList[1], tpList[ tpList.count() - 2 ] );
  showTask();
}

void TaskEditor::slotEditTaskPoint ()
{
  int id = taskList->indexOfTopLevelItem( taskList->currentItem() );

  if( id < 0 )
  {
    return;
  }

  TaskPoint* modPoint = tpList.at(id);
  TaskPointEditor *tpe = new TaskPointEditor(this, modPoint );

  connect( tpe, SIGNAL(taskPointEdited(TaskPoint*)),
           this, SLOT(slotTaskPointEdited(TaskPoint*)));

  tpe->setVisible( true );
}

void TaskEditor::slotTaskPointEdited( TaskPoint* editedTaskPoint )
{
  Q_UNUSED( editedTaskPoint )

  // That updates the task point list in the flight task.
  showTask();
}

void TaskEditor::slotAccept()
{
  // Check, if a valid task has been defined. Tasks with less than
  // four task points are incomplete
  if ( tpList.count() < 4 )
    {
      QMessageBox mb( QMessageBox::Critical,
                      tr( "Task Incomplete" ),
                      tr( "Task needs at least four waypoints" ),
                      QMessageBox::Ok,
                      this );

    #ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

    #endif

      mb.exec();
      return;
    }

  QString txt = taskName->text();

  // Check if the user has entered a task name
  if ( txt.length() == 0 )
    {
      QMessageBox mb( QMessageBox::Critical,
                      tr("Name Missing"),
                      tr("Enter a name for the task to save it"),
                      QMessageBox::Ok,
                      this );

    #ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

    #endif

      mb.exec();
      return;
    }

  if ( ( editState == TaskEditor::create && taskNamesInUse.contains( txt ) > 0 ) ||
       ( editState != TaskEditor::create && txt != editedTaskName &&
         taskNamesInUse.contains( txt ) > 0 ) )
    {
      // Check if the task name does not conflict with existing onces.
      // The name must be unique in the task name space
      QMessageBox mb( QMessageBox::Critical,
                      tr( "Name in Use"),
                      tr( "Please enter a different name" ),
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif
      mb.exec();
      return;
    }

  // Take over changed task data and publish it
  task2Edit->setTaskName(txt);

  if ( editState == TaskEditor::create )
    {
      emit newTask( task2Edit );
    }
  else
    {
      emit editedTask( task2Edit );
    }

  // closes and destroys window
  close();
}

void TaskEditor::slotReject()
{
  // delete rejected task object
  delete task2Edit;

  // close and destroy window
  close();
}

void TaskEditor::slotMoveWaypointUp()
{
  if( taskList->selectedItems().size() == 0 ||
      taskList->topLevelItemCount() <= 2 )
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

  setTaskPointFigureSchemas( tpList );
  showTask();
}

void TaskEditor::slotMoveWaypointDown()
{
  if( taskList->selectedItems().size() == 0 ||
      taskList->topLevelItemCount() <= 2 )
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

  setTaskPointFigureSchemas( tpList );
  showTask();
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

void TaskEditor::slotCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
  Q_UNUSED(current)
  Q_UNUSED(previous)

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
      editButton->setEnabled (false);
      defaultButton->setEnabled (false);
    }
  else if( tpList.size() == 1 )
    {
      upButton->setEnabled( false );
      downButton->setEnabled( false );
      invertButton->setEnabled( false );
      addButton->setEnabled( true );
      delButton->setEnabled( true );
      editButton->setEnabled (false);
      defaultButton->setEnabled (false);
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

      int id = taskList->indexOfTopLevelItem( taskList->currentItem() );

      bool isNotFirstOrLast = true;

      if( id > 0 )
        {
          upButton->setEnabled( true );
        }
      else
        {
          // At the first position, no up allowed
          upButton->setEnabled( false );
          isNotFirstOrLast = false;
        }


      if( id == -1 || id == taskList->topLevelItemCount() - 1 ||
          ( id == taskList->topLevelItemCount() - 2 &&
            taskList->topLevelItem( taskList->topLevelItemCount() - 1)->text( 0 ) == "Total" ) )
        {
          // At the last allowed down position. No further down allowed.
          downButton->setEnabled( false );
          isNotFirstOrLast = false;
        }
      else
        {
          downButton->setEnabled( true );
        }

      if( tpList.size() >= 4 )
        {
          editButton->setEnabled( isNotFirstOrLast );
          defaultButton->setEnabled( true );
        }
      else
        {
          // Task has not enough points, disable task point editing.
          editButton->setEnabled( false );
          defaultButton->setEnabled( false );
        }
    }
}

void TaskEditor::swapTaskPointSchemas( TaskPoint* tp1, TaskPoint* tp2 )
{
  Distance d1, d2;

  d1 = tp1->getTaskCircleRadius();
  d2 = tp2->getTaskCircleRadius();

  tp1->setTaskCircleRadius( d2 );
  tp2->setTaskCircleRadius( d1 );

  d1 = tp1->getTaskSectorInnerRadius();
  d2 = tp2->getTaskSectorInnerRadius();

  tp1->setTaskSectorInnerRadius( d2 );
  tp2->setTaskSectorInnerRadius( d1 );

  d1 = tp1->getTaskSectorOuterRadius();
  d2 = tp2->getTaskSectorOuterRadius();

  tp1->setTaskSectorOuterRadius( d2 );
  tp2->setTaskSectorOuterRadius( d1 );

  double l1, l2;

  l1 = tp1->getTaskLine().getLineLength();
  l2 = tp2->getTaskLine().getLineLength();

  tp1->getTaskLine().setLineLength( l2 );
  tp2->getTaskLine().setLineLength( l1 );

  int a1, a2;

  a1 = tp1->getTaskSectorAngle();
  a2 = tp2->getTaskSectorAngle();

  tp1->setTaskSectorAngle( a2 );
  tp2->setTaskSectorAngle( a1 );

  enum GeneralConfig::ActiveTaskFigureScheme tfs1, tfs2;

  tfs1 = tp1->getActiveTaskPointFigureScheme();
  tfs2 = tp2->getActiveTaskPointFigureScheme();

  tp1->setActiveTaskPointFigureScheme( tfs2 );
  tp2->setActiveTaskPointFigureScheme( tfs1 );

  bool e1, e2;

  e1 = tp1->getUserEditFlag();
  e2 = tp2->getUserEditFlag();

  tp1->setUserEditFlag( e2 );
  tp2->setUserEditFlag( e1 );
}

void TaskEditor::setTaskPointFigureSchemas( QList<TaskPoint *>& tpList )
{
  // As first set the right task point type
  for( int i = 0; i < tpList.size(); i++ )
    {
      if( i == 0 )
        {
          tpList.at(i)->setTaskPointType(TaskPointTypes::TakeOff);
        }
      else if( i == 1 )
        {
          tpList.at(i)->setTaskPointType(TaskPointTypes::Begin);
        }
      else if( tpList.size() >= 4 && i == tpList.size() - 2 )
        {
          tpList.at(i)->setTaskPointType(TaskPointTypes::End);
        }
      else if( tpList.size() >= 4 && i == tpList.size() - 1 )
        {
          tpList.at(i)->setTaskPointType(TaskPointTypes::Landing);
        }
      else
        {
          tpList.at(i)->setTaskPointType(TaskPointTypes::RouteP);
        }

      // Set task point figure schema to default.
      tpList.at(i)->setConfigurationDefaults();
    }
}

void TaskEditor::slotSetTaskPointsDefaultSchema()
{
  QMessageBox mb( QMessageBox::Question,
                  tr( "Defaults?" ),
                  tr( "Reset all TP schemas to default configuration values?" ),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::Yes )
    {
      setTaskPointFigureSchemas( tpList );
      showTask();
    }
}
