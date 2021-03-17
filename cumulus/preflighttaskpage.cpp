/***********************************************************************
**
**   preflighttaskpage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2009-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <climits>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "calculator.h"
#include "distance.h"
#include "generalconfig.h"
#include "helpbrowser.h"
#include "layout.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "numberEditor.h"
#include "preflighttaskpage.h"
#include "speed.h"
#include "target.h"
#include "taskeditor.h"
#include "TaskFileManager.h"
#include "wgspoint.h"
#include "rowdelegate.h"
#include "XCSoar.h"

#ifdef FLARM
#include "preflightflarmpage.h"
#endif

PreFlightTaskPage::PreFlightTaskPage( QWidget* parent ) :
  QWidget( parent ),
  m_editTask(0)
{
  setObjectName("PreFlightTaskPage");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("PreFlight - Task/Route") );

  if( MainWindow::mainWindow() )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( MainWindow::mainWindow()->size() );

#ifdef ANDROID
      // On Galaxy S3 there are size problems observed
      setMinimumSize( MainWindow::mainWindow()->size() );
      setMaximumSize( MainWindow::mainWindow()->size() );
#endif
    }

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  int msw = QFontMetrics(font()).width("999 Km/h") + 10;
  int mdw = QFontMetrics(font()).width("999" + QString(Qt::Key_degree)) + 10;

  const int iconSize = Layout::iconSize( font() );
  const int Scaling = Layout::getIntScaledDensity();

  QVBoxLayout* taskLayout = new QVBoxLayout;
  contentLayout->addLayout( taskLayout, 5 );

  taskLayout->setSpacing(5);
  taskLayout->setMargin(0);

  QHBoxLayout* editrow = new QHBoxLayout;
  editrow->setSpacing(5 * Scaling);
  taskLayout->addLayout( editrow );
  taskLayout->addSpacing( 10 );

  QLabel *label = new QLabel( tr("TAS"), this );
  editrow->addWidget(label);

  m_tas = new NumberEditor( this );
#ifndef ANDROID
  m_tas->setToolTip( tr("True Air Speed") );
#endif
  m_tas->setPmVisible(false);
  m_tas->setDecimalVisible(false);
  m_tas->setRange( 0, 999);
  m_tas->setMaxLength(3);

  const Speed& tas = GeneralConfig::instance()->getTas();
  m_tas->setText( tas.getHorizontalText( false, 0 ) );
  m_tas->setSuffix( " " + Speed::getHorizontalUnitText() );
  m_tas->setMinimumWidth( msw );
  editrow->addWidget(m_tas);

  label = new QLabel( tr("WD"), this );
  editrow->addWidget(label);

  m_windDirection = new NumberEditor( this );
#ifndef ANDROID
  m_windDirection->setToolTip( tr("Wind Direction") );
#endif
  m_windDirection->setPmVisible(false);
  m_windDirection->setDecimalVisible(false);
  m_windDirection->setRange( 0, 360 );
  m_windDirection->setTip("0...360");
  m_windDirection->setMaxLength(3);
  m_windDirection->setValue( GeneralConfig::instance()->getManualWindDirection() );
  m_windDirection->setSuffix( QString(Qt::Key_degree) );
  m_windDirection->setMinimumWidth( mdw );
  editrow->addWidget(m_windDirection);

  label = new QLabel( tr("WS"), this );
  editrow->addWidget(label);

  m_windSpeed = new NumberEditor( this );
#ifndef ANDROID
  m_windSpeed->setToolTip( tr("Wind Speed") );
#endif
  m_windSpeed->setPmVisible(false);
  m_windSpeed->setDecimalVisible(false);
  m_windSpeed->setRange( 0, 999 );
  m_windSpeed->setMaxLength(3);

  const Speed& wv = GeneralConfig::instance()->getManualWindSpeed();
  m_windSpeed->setText( wv.getWindText( false, 0 ) );

  m_windSpeed->setSuffix( " " + Speed::getWindUnitText() );
  m_windSpeed->setMinimumWidth( msw );
  editrow->addWidget(m_windSpeed);
  editrow->addStretch(10);

  m_cmdNew = new QPushButton;
  m_cmdNew->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("add.png", true)) );
  m_cmdNew->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  m_cmdNew->setToolTip(tr("Define a new task"));
#endif
  editrow->addWidget(m_cmdNew);

  editrow->addSpacing(20 * Scaling);
  m_cmdEdit = new QPushButton;
  m_cmdEdit->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png", true)) );
  m_cmdEdit->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  m_cmdEdit->setToolTip(tr("Edit selected task"));
#endif
  editrow->addWidget(m_cmdEdit);

  editrow->addSpacing(20 * Scaling);
  m_cmdDel = new QPushButton;
  m_cmdDel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("delete.png", true)) );
  m_cmdDel->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  m_cmdDel->setToolTip(tr("Remove selected task"));
#endif
  editrow->addWidget(m_cmdDel);

  //----------------------------------------------------------------------------

  m_taskListWidget = new QWidget( this );
  QVBoxLayout *tlLayout = new QVBoxLayout( m_taskListWidget );
  tlLayout->setMargin( 0 );

  m_taskList = new QTreeWidget;

  connect( m_taskList, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
           SLOT(slotItemClicked(QTreeWidgetItem*, int)) );

  connect( m_taskList, SIGNAL(itemSelectionChanged()),
           SLOT(slotItemSelectionChanged()) );

#ifndef ANDROID
  m_taskList->setToolTip( tr("Choose a flight task to be flown") );
#endif
  m_taskList->setRootIsDecorated(false);
  m_taskList->setItemsExpandable(false);
  m_taskList->setUniformRowHeights(true);
  m_taskList->setAlternatingRowColors(true);
  m_taskList->setSortingEnabled(true);
  m_taskList->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_taskList->setSelectionMode(QAbstractItemView::SingleSelection);
  m_taskList->setColumnCount(5);
  m_taskList->setFocus();
  m_taskList->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_taskList->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture(m_taskList->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_taskList->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  m_taskList->setItemDelegate( new RowDelegate( m_taskList, afMargin ) );

  QStringList sl;
  sl << tr("No.")
     << tr("Name")
     << tr("Type")
     << tr("Distance")
     << tr("Time");

  m_taskList->setHeaderLabels(sl);

  QTreeWidgetItem* headerItem = m_taskList->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 4, Qt::AlignCenter );

  tlLayout->addWidget( m_taskList, 10 );

  QHBoxLayout *tlButtonLayout = new QHBoxLayout;
  tlButtonLayout->setMargin( 0 );
  tlLayout->addLayout( tlButtonLayout );

  m_deactivateButton = new QPushButton( tr("Deactivate Task") );
#ifndef ANDROID
  m_deactivateButton->setToolTip(tr("Deactivate the currently activated task"));
#endif
  tlButtonLayout->addWidget( m_deactivateButton, 0, Qt::AlignLeft );
  tlButtonLayout->addSpacing( 30 );

  m_importButton = new QPushButton( tr("Import") );
#ifndef ANDROID
  m_importButton->setToolTip(tr("Import WeGlide task"));
#endif
  tlButtonLayout->addWidget( m_importButton );

  tlButtonLayout->addStretch( 5 );

  m_showButton = new QPushButton( tr("Show") );
#ifndef ANDROID
  m_showButton->setToolTip(tr("Show details of selected task"));
#endif
  tlButtonLayout->addWidget( m_showButton, 0, Qt::AlignRight );

  taskLayout->addWidget( m_taskListWidget );

  //----------------------------------------------------------------------------

  m_taskViewWidget = new QWidget( this );
  QVBoxLayout *tvLayout = new QVBoxLayout( m_taskViewWidget );

  m_taskContent = new TaskListView( this, false );
  m_taskContent->setHeadlineVisible( false );

#ifndef ANDROID
  m_taskContent->setToolTip( tr("Task display") );
#endif

  tvLayout->addWidget( m_taskContent, 10 );

  QPushButton *tvCloseButton = new QPushButton( tr("Close") );
  tvLayout->addWidget( tvCloseButton, 0, Qt::AlignRight );
  taskLayout->addWidget( m_taskViewWidget );
  m_taskViewWidget->setVisible( false );

  connect( m_tas, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slotNumberEdited(const QString&)) );
  connect( m_windDirection, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slotNumberEdited(const QString&)) );
  connect( m_windSpeed, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slotNumberEdited(const QString&)) );

  connect(m_cmdNew, SIGNAL(clicked()), this, SLOT(slotNewTask()));
  connect(m_cmdEdit, SIGNAL(clicked()), this, SLOT(slotEditTask()));
  connect(m_cmdDel, SIGNAL(clicked()), this, SLOT(slotDeleteTask()));

  connect( m_taskList, SIGNAL( itemSelectionChanged() ),
           this, SLOT( slotTaskDetails() ) );

  connect( m_deactivateButton, SIGNAL(pressed()),
           this, SLOT( slotDeactivateTask() ) );

  connect( m_importButton, SIGNAL(pressed()),
           this, SLOT( slotImportTask() ) );

  connect( m_showButton, SIGNAL(pressed()),
           this, SLOT( slotShowTaskViewWidget() ) );

  connect( tvCloseButton, SIGNAL(pressed()),
           this, SLOT( slotShowTaskListWidget() ) );

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_cancel = new QPushButton(this);
  m_cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png", true)));
  m_cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  m_cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_ok = new QPushButton(this);
  m_ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png", true)));
  m_ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  m_ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_titlePix = new QLabel(this);
  m_titlePix->setAlignment( Qt::AlignCenter );
  m_titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

  connect(help, SIGNAL(pressed()), this, SLOT(slotHelp()));
  connect(m_ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(m_cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setMargin(10 * Scaling);
  buttonBox->setSpacing(0);
  buttonBox->addWidget(help, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(m_cancel, 1);
  buttonBox->addSpacing(30 * Scaling);
  buttonBox->addWidget(m_ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(m_titlePix);
  contentLayout->addLayout(buttonBox);

  loadTaskList();
}

PreFlightTaskPage::~PreFlightTaskPage()
{
  qDeleteAll(m_flightTaskList);
}

void PreFlightTaskPage::showEvent( QShowEvent *event )
{
  m_taskList->resizeColumnToContents(0);
  m_taskList->resizeColumnToContents(1);
  m_taskList->resizeColumnToContents(2);
  m_taskList->resizeColumnToContents(3);
  m_taskList->resizeColumnToContents(4);
  m_taskList->sortByColumn(0, Qt::AscendingOrder);

  if( m_taskList->topLevelItemCount() > 0 && m_taskList->currentItem() != 0 )
    {
      m_taskList->scrollToItem( m_taskList->currentItem(),
				QAbstractItemView::PositionAtCenter );
    }

  enableButtons();
  QWidget::showEvent( event );
}

void PreFlightTaskPage::enableButtons()
{
  QList<QTreeWidgetItem*> selectList = m_taskList->selectedItems();

  if( m_taskList->topLevelItemCount() == 0 || selectList.size() == 0 )
    {
      m_deactivateButton->setEnabled( false );
      m_showButton->setEnabled( false );
    }
  else
    {
      m_deactivateButton->setEnabled( true );
      m_showButton->setEnabled( true );
    }
}

void PreFlightTaskPage::slotShowTaskListWidget()
{
  // Show all buttons in this view.
  m_cmdNew->setVisible( true );
  m_cmdEdit->setVisible( true );
  m_cmdDel->setVisible( true );
  m_cancel->setVisible( true );
  m_ok->setVisible( true );
  m_titlePix->setVisible( true );

  m_taskViewWidget->setVisible( false );
  m_taskListWidget->setVisible( true );
}

void PreFlightTaskPage::slotShowTaskViewWidget()
{
  QList<QTreeWidgetItem*> selectList = m_taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      // Nothing is selected
      return;
    }

  // Ensure visibility of selected item after return to list.
  m_taskList->scrollToItem( m_taskList->selectedItems().at(0),
			    QAbstractItemView::PositionAtCenter );

  // Hide buttons which shall not usable.
  m_cmdNew->hide();
  m_cmdEdit->hide();
  m_cmdDel->hide();
  m_cancel->hide();
  m_ok->hide();
  m_titlePix->hide();

  m_taskViewWidget->setVisible( true );
  m_taskListWidget->setVisible( false );
}

void PreFlightTaskPage::slotTaskDetails()
{
  QList<QTreeWidgetItem*> selectList = m_taskList->selectedItems();

  if( selectList.size() == 0 )
    {
      return;
    }

  QTreeWidgetItem* selectedItem = m_taskList->selectedItems().at(0);

  int id = selectedItem->text(0).toInt() - 1;

  FlightTask *task = m_flightTaskList.at( id );

  // update TAS, can be changed in the meantime by the user
  Speed tas;
  tas.setHorizontalValue( m_tas->doubleValue() );
  task->setSpeed( tas );

  // update wind parameters, can be changed in the meantime by the user
  task->setWindDirection( m_windDirection->value() % 360 );

  Speed ws;
  ws.setWindValue( m_windSpeed->doubleValue() );
  task->setWindSpeed( ws );

  m_taskContent->slot_setTask( task );
}

void PreFlightTaskPage::slotNumberEdited( const QString& number )
{
  Q_UNUSED(number)

  updateWayTime();
}

void PreFlightTaskPage::updateWayTime()
{
  if( m_taskList->topLevelItemCount() == 0 )
    {
      // There are no tasks defined.
      return;
    }

  for( int i = 0; i < m_taskList->topLevelItemCount(); i++ )
    {
      QTreeWidgetItem* item = m_taskList->topLevelItem( i );

      if( item == 0 )
        {
          continue;
        }

      int id = item->text(0).toInt() - 1;
      FlightTask *task = m_flightTaskList.at( id );

      // update TAS, can be changed in the meantime by the user
      Speed tas;
      tas.setHorizontalValue( m_tas->doubleValue() );
      task->setSpeed( tas );

      // update wind parameters, can be changed in the meantime by the user
      task->setWindDirection( m_windDirection->value() % 360 );

      Speed ws;
      ws.setWindValue( m_windSpeed->doubleValue() );
      task->setWindSpeed( ws );

      if( task->getSpeed().getMps() == 0.0 )
        {
          // TAS is zero, show nothing
          item->setText(4, "");
        }
      else
        {
          // TAS is not zero, show time total
          item->setText( 4, task->getTotalDistanceTimeString() + "h " );
        }
    }

  // Update task details too
  slotTaskDetails();
}

// This method is called from PreFlightWidget::accept(), to take out
// the selected task from the task list. The ownership of the taken
// FlightTask object goes over to the caller. He has to delete the
// object!!!
FlightTask* PreFlightTaskPage::takeSelectedTask()
{
  // save last used TAS, and wind parameters
  Speed tas;
  tas.setValueInUnit( m_tas->doubleValue(), Speed::getHorizontalUnit() );

  GeneralConfig::instance()->setTas( tas );
  GeneralConfig::instance()->setManualWindDirection( m_windDirection->value() );

  Speed wv;
  wv.setValueInUnit( m_windSpeed->text().toDouble(), Speed::getWindUnit() );
  GeneralConfig::instance()->setManualWindSpeed( wv );

  QList<QTreeWidgetItem*> selectList = m_taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      GeneralConfig::instance()->setCurrentTaskName( "" );
      return static_cast<FlightTask *> (0);
    }

  QString id( m_taskList->selectedItems().at(0)->text(0) );

  GeneralConfig::instance()->setCurrentTaskName( m_taskList->selectedItems().at(0)->text(1) );

  // Nice trick, take selected element from list to prevent deletion of it, if
  // destruction of list is called.
  int index = id.toInt() - 1;

  FlightTask* task = m_flightTaskList.takeAt( index );

#ifdef FLARM
  // Save the task declaration in Flarm format as file.
  PreFlightFlarmPage::createFlarmTaskList( task );
#endif

  return task;
}

/** load tasks from file*/
bool PreFlightTaskPage::loadTaskList()
{
  QStringList rowList;

  while ( !m_flightTaskList.isEmpty() )
    {
      delete m_flightTaskList.takeFirst();
    }

  m_taskNames.clear();

  TaskFileManager tfm;
  Speed tas;
  tas.setValueInUnit( m_tas->doubleValue(), Speed::getHorizontalUnit() );
  tfm.setTas( tas );

  if( tfm.loadTaskList( m_flightTaskList ) == false || m_flightTaskList.size() == 0 )
    {
      // no task has been read, reset current task
      GeneralConfig::instance()->setCurrentTaskName( "" );

      m_taskList->resizeColumnToContents(0);
      m_taskList->resizeColumnToContents(1);
      m_taskList->resizeColumnToContents(2);
      m_taskList->resizeColumnToContents(3);
      m_taskList->resizeColumnToContents(4);
      return false;
    }

  for( int i = 0; i < m_flightTaskList.size(); i++ )
    {
      FlightTask* task = m_flightTaskList.at(i);

      QString taskNum = QString("%1").arg(i + 1, 2, 10, QLatin1Char('0'));

      rowList << taskNum
              << task->getTaskName()
              << task->getTaskTypeString()
              << task->getTaskDistanceString()
              << "";

      QTreeWidgetItem *item = new QTreeWidgetItem( m_taskList, rowList, 0 );
      item->setTextAlignment( 0, Qt::AlignCenter);
      item->setTextAlignment( 3, Qt::AlignRight);
      item->setTextAlignment( 4, Qt::AlignRight);

      m_taskList->addTopLevelItem( item );
      rowList.clear();

      // save task name
      m_taskNames << task->getTaskName();
    }

  updateWayTime();
  selectLastTask();

  m_taskList->resizeColumnToContents(0);
  m_taskList->resizeColumnToContents(1);
  m_taskList->resizeColumnToContents(2);
  m_taskList->resizeColumnToContents(3);
  m_taskList->resizeColumnToContents(4);

  return true;
}

void PreFlightTaskPage::slotItemClicked(QTreeWidgetItem*, int)
{
  enableButtons();
}

void PreFlightTaskPage::slotItemSelectionChanged()
{
  enableButtons();
}

void PreFlightTaskPage::slotNewTask()
{
  // Ensure visibility of selected item after return to list.
  if( m_taskList->topLevelItemCount() > 0 && m_taskList->currentItem() != 0 )
    {
      m_taskList->scrollToItem( m_taskList->currentItem(),
                                QAbstractItemView::PositionAtCenter );
    }

  TaskEditor *te = new TaskEditor(this, m_taskNames);

  connect( te, SIGNAL(newTask( FlightTask * )), this,
            SLOT(slotUpdateTaskList( FlightTask * )));

  te->setVisible( true );
}

/**
 * Taking over a new flight task from the editor or importer
 */
void PreFlightTaskPage::slotUpdateTaskList( FlightTask *newTask)
{
  QString taskName = newTask->getTaskName();

  m_flightTaskList.append( newTask );
  saveTask( newTask );
  m_taskContent->clear();
  m_taskList->clear();
  loadTaskList();

  QList<QTreeWidgetItem *> items = m_taskList->findItems( taskName,
                                                          Qt::MatchExactly,
                                                          1 );
  if( items.size () > 0 )
    {
      m_taskList->setCurrentItem( items.at(0) );
    }
  else
    {
      m_taskList->setCurrentItem( m_taskList->topLevelItem(m_taskList->topLevelItemCount() - 1 ) );
    }

  m_taskList->scrollToItem( m_taskList->currentItem(),
                            QAbstractItemView::PositionAtCenter );
  enableButtons();
}

/**
 * pass the selected task to the editor
 */
void PreFlightTaskPage::slotEditTask()
{
  // fetch selected task item
  QList<QTreeWidgetItem*> selectList = m_taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      return;
    }

  // Ensure visibility of selected item after list update.
  m_taskList->scrollToItem( m_taskList->currentItem(),
                            QAbstractItemView::PositionAtCenter );

  QString id( m_taskList->selectedItems().at(0)->text(0) );

  m_editTask = m_flightTaskList.at(id.toInt() - 1);

  FlightTask* modTask = new FlightTask( m_editTask->getTpList(),
                                        true,
                                        m_editTask->getTaskName() );

  TaskEditor *te = new TaskEditor(this, m_taskNames, modTask  );

  connect( te, SIGNAL(editedTask( FlightTask * )),
           this, SLOT(slotEditTaskList( FlightTask * )));

  te->setVisible( true );
}

/**
 * taking over an edited flight task from editor
 */
void PreFlightTaskPage::slotEditTaskList( FlightTask *editedTask)
{
  // search task item being edited
  int index = m_flightTaskList.indexOf( m_editTask );

  QString taskName = editedTask->getTaskName();

  if ( index != -1 )
    {
      // remove old item
      delete m_flightTaskList.takeAt( index );
      // put new item on old position
      m_flightTaskList.insert( index, editedTask );
    }
  else
    {
      // no old position available, append it at end of list
      m_flightTaskList.append( editedTask );
    }

  saveTask( editedTask );
  m_taskContent->clear();
  m_taskList->clear();
  loadTaskList();

  QList<QTreeWidgetItem *> items = m_taskList->findItems( taskName,
                                                          Qt::MatchExactly,
                                                          1 );
  if( items.size () > 0 )
    {
      m_taskList->setCurrentItem( items.at(0) );
    }
  else
    {
      m_taskList->setCurrentItem( m_taskList->topLevelItem(m_taskList->topLevelItemCount() - 1 ) );
    }

  // Ensure visibility of selected item after list update.
  m_taskList->scrollToItem( m_taskList->currentItem(),
                            QAbstractItemView::PositionAtCenter );
  enableButtons();
}

/**
 * remove the selected task from the list
 */
void PreFlightTaskPage::slotDeleteTask()
{
  QTreeWidgetItem* selected = m_taskList->currentItem();

  if ( selected == 0 )
    {
      return;
    }

  QString id( selected->text(0) );
  QString taskName( selected->text(1).trimmed() );

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

  delete m_taskList->takeTopLevelItem( m_taskList->currentIndex().row() );

  if( m_taskList->topLevelItemCount() > 0 )
    {
      m_taskList->sortItems( 0, Qt::AscendingOrder );
      m_taskList->setCurrentItem( m_taskList->topLevelItem(0) );
    }

  // reset last stored selected task
  GeneralConfig::instance()->setCurrentTaskName( "" );

  // reset task
  extern MapContents* _globalMapContents;
  _globalMapContents->setCurrentTask(0);

  uint no = id.toUInt() - 1;
  delete m_flightTaskList.takeAt( no );

  TaskFileManager tfm;
  tfm.removeTaskFile( taskName );
  m_taskContent->clear();
  m_taskList->clear();
  loadTaskList();
  enableButtons();
}

bool PreFlightTaskPage::saveTaskList()
{
  TaskFileManager tfm;
  return tfm.saveTaskList( m_flightTaskList );
}

bool PreFlightTaskPage::saveTask( FlightTask *task )
{
  TaskFileManager tfm;
  return tfm.writeTaskFile( task );
}

/** Select the last stored task */
void PreFlightTaskPage::selectLastTask()
{
  QString lastTask = GeneralConfig::instance()->getCurrentTaskName();

  int rows = m_taskList->topLevelItemCount();

  for( int i = 0; i < rows; i++ )
    {
      QString taskName = m_taskList->topLevelItem(i)->text(1);

      if( taskName == lastTask )
        {
          // last selected task found
          m_taskList->setCurrentItem( m_taskList->topLevelItem(i) );
          return;
        }
    }
}

void PreFlightTaskPage::slotAccept()
{
  extern MapContents *_globalMapContents;

  FlightTask *curTask = _globalMapContents->getCurrentTask();

  // Note we have taken over the ownership about this object!
  FlightTask *newTask = takeSelectedTask();

  bool newTaskPassed = true;

  // Check, if a new task has been passed for accept.
  if (curTask && newTask && curTask->getTaskName() == newTask->getTaskName())
    {
      newTaskPassed = false; // task names identical
    }

  if( curTask && newTask && newTaskPassed )
    {
      QMessageBox mb( QMessageBox::Question,
                      tr( "Replace current task?" ),
                      tr( "<html>"
                          "Do you want to replace the current task?<br>"
                          "A selected target is reset to task start."
                          "</html>" ),
                      QMessageBox::Yes | QMessageBox::No,
                      this );

      mb.setDefaultButton( QMessageBox::No );

    #ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

    #endif

      if( mb.exec() != QMessageBox::Yes )
        {
          // do nothing change
          delete newTask;
          slotReject();
          return;
        }
    }

  // Forward of new task in every case, user can have modified
  // content. MapContent will overtake the ownership of the task
  // object.
  _globalMapContents->setCurrentTask(newTask);

  // @AP: Open problem with waypoint selection, if user has modified
  // task content. We ignore that atm.
  if( newTask == static_cast<FlightTask *> (0) )
    {
      // No new task has been passed. Check, if a selected waypoint
      // exists and this waypoint belongs to a task. In this case we
      // will reset the selection.
      extern Calculator* calculator;
      const Waypoint *calcWp = calculator->getTargetWp();

      if( calcWp && calcWp->taskPointIndex != -1 )
        {
          // reset taskpoint selection
          emit newWaypoint(static_cast<Waypoint *> (0), true);
        }
    }
  else
    {
      extern Calculator* calculator;

      // If a waypoint selection exists, we do overwrite it with the begin
      // point of the new flight task.
      if( calculator->getTargetWp() )
        {
          // Reset taskpoint selection in calculator to prevent user query.
          emit newWaypoint(static_cast<Waypoint *> (0), true);

          // Select the start point of the new task.
          calculator->slot_startTask();
        }

      // Inform others about the new task
      emit newTaskSelected();
    }

  GeneralConfig *conf = GeneralConfig::instance();
  conf->setManualWindDirection( m_windDirection->value() );

  Speed wv;
  wv.setValueInUnit( m_windSpeed->text().toDouble(), Speed::getWindUnit() );
  conf->setManualWindSpeed( wv );

  if( conf->isManualWindEnabled() == true )
    {
      // Inform about a wind parameter change, if manual wind is enabled.
      emit manualWindStateChange( true );
    }

  emit closingWidget();
  QWidget::close();
}

void PreFlightTaskPage::slotReject()
{
  emit closingWidget();
  QWidget::close();
}

void PreFlightTaskPage::slotHelp()
{
  QString file = "cumulus-tasks.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void PreFlightTaskPage::slotDeactivateTask()
{
  m_taskList->clearSelection();
}

/**
 * Called, to import a WeGilde task.
 */
void PreFlightTaskPage::slotImportTask()
{
  QString wayPointDir = GeneralConfig::instance()->getUserDataDirectory();

  QString filter;
  filter.append(tr("TSK") + " (*.tsk *.TSK);;");

  QString fName = QFileDialog::getOpenFileName( this,
                                                tr("Import task"),
                                                wayPointDir,
                                                filter );
  if( fName.isEmpty() )
    {
      return;
    }

  QString errorInfo;

  FlightTask* ft = XCSoar::reakTaskFile( fName, errorInfo );

  if( ft == nullptr )
    {
      QMessageBox mb( QMessageBox::Critical,
                       tr("Error in file ") + QFileInfo( fName ).fileName(),
                       errorInfo,
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

  slotUpdateTaskList( ft );
}

