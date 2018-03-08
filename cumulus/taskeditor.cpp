/***********************************************************************
**
**   taskeditor.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2008-2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
#include "AirfieldListWidget.h"
#include "distance.h"
#include "flighttask.h"
#include "generalconfig.h"
#include "layout.h"
#include "listwidgetparent.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "radiopoint.h"
#include "SinglePointListWidget.h"
#include "RadioPointListWidget.h"
#include "rowdelegate.h"
#include "taskeditor.h"
#include "taskpoint.h"
#include "taskpointeditor.h"
#include "waypointlistwidget.h"
#include "wpeditdialog.h"

extern MapContents *_globalMapContents;

TaskEditor::TaskEditor( QWidget* parent,
                        QStringList &taskNamesInUse,
                        FlightTask* task ) :
  QWidget( parent ),
  taskNamesInUse( taskNamesInUse ),
  lastSelectedItem(0),
  afButton(0),
  hsButton(0),
  naButton(0),
  olButton(0),
  wpButton(0),
  afSelectionList(0),
  hsSelectionList(0),
  naSelectionList(0),
  olSelectionList(0),
  wpSelectionList(0),
  m_lastEditedTP(-1)
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

  if( task )
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

  int scale = Layout::getIntScaledDensity();

  Qt::InputMethodHints imh;

  taskName = new QLineEdit( this );
  taskName->setBackgroundRole( QPalette::Light );
  imh = (taskName->inputMethodHints() | Qt::ImhNoPredictiveText);
  taskName->setInputMethodHints(imh);

  // The task name minimum length is 10 characters. We calculate
  // the length of a M string of 10 characters.
  QFontMetrics fm( font() );
  int maxInputLength = fm.width("MMMMMMMMMM");
  taskName->setMinimumWidth( maxInputLength );

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

  int iconButtonSize = Layout::getButtonSize(12);

  upButton = new QPushButton( this );
  upButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "up.png", true )) );
  upButton->setIconSize(QSize(iconButtonSize, iconButtonSize));
#ifndef ANDROID
  upButton->setToolTip( tr("move selected waypoint up") );
#endif
  downButton = new QPushButton( this );
  downButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "down.png", true )) );
  downButton->setIconSize(QSize(iconButtonSize, iconButtonSize));
#ifndef ANDROID
  downButton->setToolTip( tr("move selected waypoint down") );
#endif
  invertButton = new QPushButton( this );
  invertButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "resort.png", true )) );
  invertButton->setIconSize(QSize(iconButtonSize, iconButtonSize));
#ifndef ANDROID
  invertButton->setToolTip( tr("reverse waypoint order") );
#endif
  cloneButton = new QPushButton( this );
  cloneButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "clone.png", true )) );
  cloneButton->setIconSize(QSize(iconButtonSize, iconButtonSize));
#ifndef ANDROID
  cloneButton->setToolTip( tr("clone waypoint") );
#endif
  delButton = new QPushButton( this );
  delButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "delete.png", true )) );
  delButton->setIconSize(QSize(iconButtonSize, iconButtonSize));
#ifndef ANDROID
  delButton->setToolTip( tr("remove waypoint") );
#endif

  QPushButton* cancelButton = new QPushButton( this );
  cancelButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancelButton->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancelButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
#ifndef ANDROID
  cancelButton->setToolTip( tr("cancel task") );
#endif

  QPushButton* okButton = new QPushButton( this );
  okButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  okButton->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  okButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
#ifndef ANDROID
  okButton->setToolTip( tr("save task") );
#endif

  // all single widgets and layouts in this grid
  QGridLayout* totalLayout = new QGridLayout( this );
  totalLayout->setMargin(5 * scale);

  QHBoxLayout* headlineLayout = new QHBoxLayout;
  totalLayout->addLayout( headlineLayout, 0, 0, 1, 3 );

  headlineLayout->setMargin(0);
  headlineLayout->addWidget( new QLabel( tr("Name:") ) );
  headlineLayout->addWidget( taskName, 5 );
  headlineLayout->addSpacing(10 * scale);

  defaultButton = new QPushButton;
  // defaultButton->setIcon(style->standardIcon(QStyle::SP_DialogResetButton));
  defaultButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("clear-32.png")) );
  defaultButton->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
#ifndef ANDROID
  defaultButton->setToolTip(tr("Set task figure default schemas"));
#endif
  headlineLayout->addWidget(defaultButton);
  headlineLayout->addSpacing(10 * scale);

  editButton = new QPushButton;
  editButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  editButton->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
#ifndef ANDROID
  editButton->setToolTip(tr("Edit selected waypoint"));
#endif
  headlineLayout->addWidget(editButton);
  totalLayout->addWidget( taskList, 1, 0 );

  // contains the task editor buttons
  QVBoxLayout* buttonLayout = new QVBoxLayout;
  buttonLayout->setMargin(0);
  buttonLayout->addStretch( 10 );
  buttonLayout->addWidget( invertButton );
  buttonLayout->addSpacing(10 * scale);
  buttonLayout->addWidget( upButton );
  buttonLayout->addSpacing(10 * scale);
  buttonLayout->addWidget( downButton );
  buttonLayout->addSpacing(30 * scale);
  buttonLayout->addWidget( cloneButton  );
  buttonLayout->addSpacing(10 * scale);
  buttonLayout->addWidget( delButton );
  buttonLayout->addStretch( 10 );
  totalLayout->addLayout( buttonLayout, 1, 1 );

  // The access buttons to the lists are only shown, if the lists are not empty.
  if( _globalMapContents->getAirfieldList().size() > 0 ||
      _globalMapContents->getGliderfieldList().size() > 0 )
    {
      afButton = new QPushButton( tr("Airfields") );
      connect( afButton, SIGNAL( clicked() ), SLOT(slotOpenAfSelectionList()) );
    }

  if( _globalMapContents->getHotspotList().size() > 0 )
    {
      hsButton = new QPushButton( tr("Hotspots") );
      connect( hsButton, SIGNAL( clicked() ), SLOT(slotOpenHsSelectionList()) );
    }

  if( _globalMapContents->getRadioPointList().size() > 0 )
    {
      naButton = new QPushButton( "Navaids" );
      connect( naButton, SIGNAL( clicked() ), SLOT(slotOpenNaSelectionList()) );
    }

  if( _globalMapContents->getQutlandingList().size() > 0 )
    {
      olButton = new QPushButton( "Outlandings" );
      connect( olButton, SIGNAL( clicked() ), SLOT(slotOpenOlSelectionList()) );
    }

  if( _globalMapContents->getWaypointList().size() > 0 )
    {
      wpButton = new QPushButton( "Waypoints" );
      connect( wpButton, SIGNAL( clicked() ), SLOT(slotOpenWpSelectionList()) );
    }

  if( afButton || hsButton || naButton || olButton || wpButton )
    {
      buttonLayout = new QVBoxLayout;
      buttonLayout->setMargin(10);

      QPushButton* bList[5];
      bList[0] = afButton;
      bList[1] = hsButton;
      bList[2] = naButton;
      bList[3] = olButton;
      bList[4] = wpButton;

      for( int i = 0; i < 5; i++ )
        {
	  if( bList[i] != 0 )
	    {
	      buttonLayout->addWidget( bList[i] );
	    }
        }

      totalLayout->addLayout( buttonLayout, 1, 2 );
    }
  else
    {
      QLabel* label = new QLabel( tr("No data\navailable") );
      totalLayout->addWidget( label, 1, 2 );
    }

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancelButton, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(okButton, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  totalLayout->addLayout(buttonBox, 0, 3, 2, 1);

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

  connect( cloneButton, SIGNAL( clicked() ),
           this, SLOT( slotCloneTaskpoint() ) );
  connect( delButton, SIGNAL( clicked() ),
           this, SLOT( slotRemoveTaskpoint() ) );
  connect( upButton, SIGNAL( clicked() ),
           this, SLOT( slotMoveTaskpointUp() ) );
  connect( downButton, SIGNAL( clicked() ),
           this, SLOT( slotMoveTaskpointDown() ) );
  connect( invertButton, SIGNAL( clicked() ),
           this, SLOT( slotInvertTaskpoints() ) );

  connect( defaultButton, SIGNAL(clicked()),
           this, SLOT(slotSetTaskPointsDefaultSchema()));
  connect( editButton, SIGNAL(clicked()),
           this, SLOT(slotEditTaskPoint()));

  connect( okButton, SIGNAL( clicked() ),
           this, SLOT( slotAccept() ) );
  connect( cancelButton, SIGNAL( clicked() ),
           this, SLOT( slotReject() ) );

  connect( taskList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
           this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)) );
}

TaskEditor::~TaskEditor()
{
  qDeleteAll(tpList);
  tpList.clear();

  // All allocated widgets for selections have to be removed.
  TaskPointSelectionList* tsl[] = { afSelectionList,
                                    naSelectionList,
				    naSelectionList,
				    wpSelectionList };
  for( int i = 0; i < 4; i++ )
    {
      if( tsl[i] != 0 )
        {
          delete tsl[i];
        }
    }
}

void TaskEditor::slotOpenAfSelectionList()
{
  if( afSelectionList == 0 )
    {
      afSelectionList = new TaskPointSelectionList( this, tr("Airfields") );
      afSelectionList->fillSelectionListWithAirfields();

      connect( afSelectionList, SIGNAL(takeThisPoint(const SinglePoint*)),
	       SLOT(slotAddTaskpoint( const SinglePoint*)) );
    }

  afSelectionList->show();
}

void TaskEditor::slotOpenHsSelectionList()
{
  if( hsSelectionList == 0 )
    {
      hsSelectionList = new TaskPointSelectionList( this, tr("Hotspots") );
      hsSelectionList->fillSelectionListWithHotspots();

      connect( hsSelectionList, SIGNAL(takeThisPoint(const SinglePoint*)),
 	       SLOT(slotAddTaskpoint( const SinglePoint*)) );
    }

  hsSelectionList->show();
}

void TaskEditor::slotOpenNaSelectionList()
{
  if( naSelectionList == 0 )
    {
      naSelectionList = new TaskPointSelectionList( this, tr("Navaids") );
      naSelectionList->fillSelectionListWithNavaids();

      connect( naSelectionList, SIGNAL(takeThisPoint(const SinglePoint*)),
  	       SLOT(slotAddTaskpoint( const SinglePoint*)) );
    }

  naSelectionList->show();
}

void TaskEditor::slotOpenOlSelectionList()
{
  if( olSelectionList == 0 )
    {
      olSelectionList = new TaskPointSelectionList( this, tr("Outlandings") );
      olSelectionList->fillSelectionListWithOutlandings();

      connect( olSelectionList, SIGNAL(takeThisPoint(const SinglePoint*)),
  	       SLOT(slotAddTaskpoint( const SinglePoint*)) );
    }

  olSelectionList->show();
}

void TaskEditor::slotOpenWpSelectionList()
{
  if( wpSelectionList == 0 )
    {
      wpSelectionList = new TaskPointSelectionList( this, tr("Waypoints") );
      wpSelectionList->fillSelectionListWithWaypoints();

      connect( wpSelectionList, SIGNAL(takeThisPoint(const SinglePoint*)),
  	       SLOT(slotAddTaskpoint( const SinglePoint*)) );
    }

  wpSelectionList->show();
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

      if( tmpList.size() > 1 )
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

void TaskEditor::slotAddTaskpoint( const SinglePoint* sp )
{
  if( sp == 0 )
    {
      return;
    }

  QTreeWidgetItem *item = taskList->currentItem();

  if( item == 0 )
    {
      // empty list

      // A taskpoint is only a single point and not more!
      TaskPoint* tp = new TaskPoint( *sp );
      tpList.append( tp );

      // Remember last position.
      lastSelectedItem = 0;
    }
  else
    {
      int id = taskList->indexOfTopLevelItem( item );
      id++;

      // A taskpoint is only a single point and not more!
      TaskPoint* tp = new TaskPoint( *sp );
      tpList.insert( id, tp );

      // Remember last position.
      lastSelectedItem = id;
    }

  setTaskPointFigureSchemas( tpList, false );
  showTask();
}

void TaskEditor::slotCloneTaskpoint()
{
  // Clone the current selected TaskPoint and insert it behind the current
  // position.
  QTreeWidgetItem* selected = taskList->currentItem();

  if( selected == static_cast<QTreeWidgetItem *>(0) )
    {
      return;
    }

  int id = taskList->indexOfTopLevelItem( taskList->currentItem() );

  if( id != -1 && id < tpList.size() )
    {
      // Add current taskpoint again to the list.
      slotAddTaskpoint( tpList.at(id) );
    }
}

void TaskEditor::slotRemoveTaskpoint()
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

  setTaskPointFigureSchemas( tpList, false );
  showTask();
}

void TaskEditor::slotInvertTaskpoints()
{
  if ( tpList.count() < 2 )
    {
      // not possible to invert order, if elements are less than 2
      return;
    }

  // invert list order
  for ( int i = tpList.count() - 2; i >= 0; i-- )
    {
      TaskPoint* tp = tpList.at(i);
      tpList.removeAt(i);
      tpList.append( tp );
    }

  // If start and end point have the same coordinates, the taskpoint figure
  // schemes should be switched also.
  WGSPoint* start = tpList.first()->getWGSPositionPtr();
  WGSPoint* end   = tpList.last()->getWGSPositionPtr();

  if( *start == *end )
    {
      TaskPoint* tps = tpList.first();
      TaskPoint* tpe = tpList.last();

      enum GeneralConfig::ActiveTaskFigureScheme ss =
          tps->getActiveTaskPointFigureScheme();

      enum GeneralConfig::ActiveTaskFigureScheme es =
          tpe->getActiveTaskPointFigureScheme();

      if( ss != es )
        {
          tps->setActiveTaskPointFigureScheme( es );
          tpe->setActiveTaskPointFigureScheme( ss );
        }
    }

  // After an invert the first task item is selected.
  lastSelectedItem = 0;
  showTask();
}

void TaskEditor::slotEditTaskPoint ()
{
  int id = taskList->indexOfTopLevelItem( taskList->currentItem() );

  if( id < 0 )
  {
    return;
  }

  m_lastEditedTP = id;

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

  if( m_lastEditedTP >= 0 )
    {
      // Set selection back to the state before editing
      QTreeWidgetItem* item = taskList->topLevelItem( m_lastEditedTP );

      if( item != 0 )
	{
	  taskList->setCurrentItem( item );
	}
    }
}

void TaskEditor::slotAccept()
{
  // Check, if a valid task has been defined. Tasks with less than
  // four task points are incomplete
  if ( tpList.count() < 2 )
    {
      QMessageBox mb( QMessageBox::Critical,
                      tr( "Task Incomplete" ),
                      tr( "Task needs at least a start and a finish point!" ),
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

  // Check, if in the task are not two waypoints with the same coordinates
  // in order.
  for( int i = 0; i < tpList.count() - 1; i++ )
    {
      if( tpList.at(i)->getWGSPositionRef() == tpList.at(i+1)->getWGSPositionRef() )
	{
	  QMessageBox mb( QMessageBox::Critical,
			  tr("Double points in order"),
			  QString(tr("Points %1 and %2 have the same coordinates.\nPlease remove one of them!")).arg(i+1).arg(i+2),
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
                      tr( "Task name in use"),
                      tr( "Task name in use." ) + "\n\n" + tr( "Overwrite existing task?" ),
                      QMessageBox::Yes|QMessageBox::No,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      int ret = mb.exec();

      if( ret == QMessageBox::No )
	{
	  return;
	}
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

  // close and destroy window
  close();
}

void TaskEditor::slotReject()
{
  // delete rejected task object
  delete task2Edit;

  // close and destroy window
  close();
}

void TaskEditor::slotMoveTaskpointUp()
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

  setTaskPointFigureSchemas( tpList, false );
  showTask();
}

void TaskEditor::slotMoveTaskpointDown()
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

  setTaskPointFigureSchemas( tpList, false );
  showTask();
}

/** Toggle between the point data lists on user request */
void TaskEditor::slotToggleList(int index)
{
  for( int i = 0; i < pointDataList.size(); i++ )
    {
      if( i != index )
        {
          pointDataList[i]->hide();
        }
      else
        {
          pointDataList[i]->show();
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
      cloneButton->setEnabled( false );
      upButton->setEnabled( false );
      downButton->setEnabled( false );
      invertButton->setEnabled( false );
      delButton->setEnabled( false );
      editButton->setEnabled (false);
      defaultButton->setEnabled (false);
    }
  else if( tpList.size() == 1 )
    {
      upButton->setEnabled( false );
      downButton->setEnabled( false );
      invertButton->setEnabled( false );
      cloneButton->setEnabled( true );
      delButton->setEnabled( true );
      editButton->setEnabled (true);
      defaultButton->setEnabled (false);
    }
  else
    {
      invertButton->setEnabled( true );
      cloneButton->setEnabled( true );
      delButton->setEnabled( true );
      editButton->setEnabled( true );
      defaultButton->setEnabled( true );

      if( taskList->topLevelItemCount() && taskList->currentItem() == 0 )
        {
          // If no item is selected we select the first one.
          taskList->setCurrentItem(taskList->topLevelItem(taskList->indexOfTopLevelItem(0)));
        }

      int id = taskList->indexOfTopLevelItem( taskList->currentItem() );

      if( id > 0 )
        {
          upButton->setEnabled( true );
        }
      else
        {
          // At the first position, no up allowed
          upButton->setEnabled( false );
        }

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

void TaskEditor::swapTaskPointSchemas( TaskPoint* tp1, TaskPoint* tp2 )
{
  qDebug() << "TaskEditor::swapTaskPointSchemas";

  qDebug() << "tp1=" << tp1->getName() << tp1->getActiveTaskPointFigureScheme();
  qDebug() << "tp2=" << tp2->getName() << tp2->getActiveTaskPointFigureScheme();

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

void TaskEditor::setTaskPointFigureSchemas( QList<TaskPoint *>& tpList,
					    const bool setDefaultFigure )
{
  // As first set the right task point type
  for( int i = 0; i < tpList.size(); i++ )
    {
      if( i == 0 )
        {
          tpList.at(i)->setTaskPointType(TaskPointTypes::Start);
        }
      else if( tpList.size() >= 2 && i == tpList.size() - 1 )
        {
          tpList.at(i)->setTaskPointType(TaskPointTypes::Finish);
        }
      else
        {
          tpList.at(i)->setTaskPointType(TaskPointTypes::Turn);
        }

      // Set task point figure schema to default, if the user has not edited the
      // task point.
      if( setDefaultFigure == true || tpList.at(i)->getUserEditFlag() == false )
	{
	  tpList.at(i)->setConfigurationDefaults();
	}
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
      setTaskPointFigureSchemas( tpList, true );
      showTask();
    }
}

void TaskEditor::slotWpEdited( Waypoint &editedWp )
{
  QTreeWidgetItem *item = taskList->currentItem();

  if( item == 0 )
    {
      return;
    }

  int idx = taskList->indexOfTopLevelItem( item );

  if( idx == -1 )
    {
      return;
    }

  if( idx >= tpList.size() )
    {
      return;
    }

  // Take old point from the list
  TaskPoint* tp = tpList.takeAt( idx );
  delete tp;

  tp = new TaskPoint( editedWp );
  tpList.insert( idx, tp );

  // Remember last position.
  lastSelectedItem = idx;

  setTaskPointFigureSchemas( tpList, false );
  showTask();
}
