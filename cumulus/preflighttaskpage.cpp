/***********************************************************************
**
**   preflighttaskpage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2009-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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
#include "layout.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "numberEditor.h"
#include "preflighttaskpage.h"
#include "speed.h"
#include "target.h"
#include "taskeditor.h"
#include "taskfilemanager.h"
#include "wgspoint.h"
#include "rowdelegate.h"

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
  setWindowTitle( tr("PreFlight - Task") );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  int msw = QFontMetrics(font()).width("999 Km/h") + 10;
  int mdw = QFontMetrics(font()).width("999" + QString(Qt::Key_degree)) + 10;

  QVBoxLayout* taskLayout = new QVBoxLayout;
  contentLayout->addLayout( taskLayout, 5 );

  taskLayout->setSpacing(5);
  taskLayout->setMargin(0);

  QHBoxLayout* editrow = new QHBoxLayout;
  editrow->setSpacing(5);
  taskLayout->addLayout( editrow );
  taskLayout->addSpacing( 10 );

  QLabel *label = new QLabel( tr("TAS"), this );
  editrow->addWidget(label);

  m_tas = new NumberEditor( this );
#ifndef ANDROID
  m_tas->setToolTip( tr("True Air Speed") );
#endif
  m_tas->setPmVisible(false);
  m_tas->setRange( 0, 999);
  m_tas->setMaxLength(3);
  m_tas->setValue( GeneralConfig::instance()->getTas() );
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
  m_windDirection->setRange( 0, 360 );
  m_windDirection->setTip("0...360");
  m_windDirection->setMaxLength(3);
  m_windDirection->setValue( GeneralConfig::instance()->getWindDirection() );
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
  m_windSpeed->setRange( 0, 999 );
  m_windSpeed->setMaxLength(3);
  m_windSpeed->setValue( GeneralConfig::instance()->getWindSpeed() );
  m_windSpeed->setSuffix( " " + Speed::getWindUnitText() );
  m_windSpeed->setMinimumWidth( msw );
  editrow->addWidget(m_windSpeed);
  editrow->addStretch(10);

  const int iconSize = Layout::iconSize( font() );

  QPushButton * cmdNew = new QPushButton;
  cmdNew->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("add.png")) );
  cmdNew->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  cmdNew->setToolTip(tr("Define a new task"));
#endif
  editrow->addWidget(cmdNew);

  editrow->addSpacing(20);
  QPushButton * cmdEdit = new QPushButton;
  cmdEdit->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  cmdEdit->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  cmdEdit->setToolTip(tr("Edit selected task"));
#endif
  editrow->addWidget(cmdEdit);

  editrow->addSpacing(20);
  QPushButton * cmdDel = new QPushButton;
  cmdDel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("delete.png")) );
  cmdDel->setIconSize(QSize(iconSize, iconSize));
#ifndef ANDROID
  cmdDel->setToolTip(tr("Remove selected task"));
#endif
  editrow->addWidget(cmdDel);

  //----------------------------------------------------------------------------

  m_taskListWidget = new QWidget( this );
  QVBoxLayout *tlLayout = new QVBoxLayout( m_taskListWidget );
  tlLayout->setMargin( 0 );

  m_taskList = new QTreeWidget;

#ifndef ANDROID
  m_taskList->setToolTip( tr("Select a flight task") );
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

  QPushButton *tlShowButton = new QPushButton( tr("Show") );

#ifndef FLARM
  tlLayout->addWidget( tlShowButton, 0, Qt::AlignRight );
#else
  QPushButton *tlFlarmButton = new QPushButton( tr("Flarm") );
  QHBoxLayout* hbl = new QHBoxLayout;
  hbl->addWidget( tlFlarmButton, 0, Qt::AlignLeft );
  hbl->addWidget( tlShowButton, 0, Qt::AlignRight );
  tlLayout->addLayout( hbl );

  connect( tlFlarmButton, SIGNAL(pressed()), SLOT(slotShowFlarmWidget()) );
#endif

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

  connect(cmdNew, SIGNAL(clicked()), this, SLOT(slotNewTask()));
  connect(cmdEdit, SIGNAL(clicked()), this, SLOT(slotEditTask()));
  connect(cmdDel, SIGNAL(clicked()), this, SLOT(slotDeleteTask()));

  connect( m_taskList, SIGNAL( itemSelectionChanged() ),
           this, SLOT( slotTaskDetails() ) );

  connect( tlShowButton, SIGNAL(pressed()),
           this, SLOT( slotShowTaskViewWidget() ) );

  connect( tvCloseButton, SIGNAL(pressed()),
           this, SLOT( slotShowTaskListWidget() ) );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png"));

  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);

  loadTaskList();
}

PreFlightTaskPage::~PreFlightTaskPage()
{
  // qDebug("PreFlightTaskPage::~PreFlightTaskPage()");
  qDeleteAll(m_flightTaskList);
}

void PreFlightTaskPage::showEvent(QShowEvent *)
{
  m_taskList->resizeColumnToContents(0);
  m_taskList->resizeColumnToContents(1);
  m_taskList->resizeColumnToContents(2);
  m_taskList->resizeColumnToContents(3);
  m_taskList->resizeColumnToContents(4);
}

void PreFlightTaskPage::slotShowTaskListWidget()
{
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

  QTreeWidgetItem* selected = m_taskList->selectedItems().at(0);

  if ( selected->text( 0 ) == " " )
    {
      // Help text is selected
      return;
    }

  m_taskViewWidget->setVisible( true );
  m_taskListWidget->setVisible( false );
}

void PreFlightTaskPage::slotTaskDetails()
{
  QList<QTreeWidgetItem*> selectList = m_taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      return;
    }

  QTreeWidgetItem* selected = m_taskList->selectedItems().at(0);

  if ( selected->text( 0 ) == " " )
    {
      m_taskContent->clear();
      return;
    }

  int id = selected->text( 0 ).toInt() - 1;

  FlightTask *task = m_flightTaskList.at( id );

  // update TAS, can be changed in the meantime by the user
  task->setSpeed( m_tas->value() );

  // update wind parameters, can be changed in the meantime by the user
  task->setWindDirection( m_windDirection->value() % 360 );
  task->setWindSpeed( m_windSpeed->value() );

  m_taskContent->slot_setTask( task );
}

void PreFlightTaskPage::slotNumberEdited( const QString& number )
{
  Q_UNUSED(number)

  updateWayTime();
}

void PreFlightTaskPage::updateWayTime()
{
  if( m_taskList->topLevelItemCount() < 2 )
    {
      // There are no tasks defined.
      return;
    }

  for( int i = 0; i < m_taskList->topLevelItemCount(); i++ )
    {
      QTreeWidgetItem* item = m_taskList->topLevelItem( i );

      if( item == 0 || item->text( 0 ) == " " )
        {
          continue;
        }

      int id = item->text( 0 ).toInt() - 1;

      FlightTask *task = m_flightTaskList.at( id );

      // update TAS, can be changed in the meantime by the user
      task->setSpeed( m_tas->value() );

      // update wind parameters, can be changed in the meantime by the user
      task->setWindDirection( m_windDirection->value() % 360 );
      task->setWindSpeed( m_windSpeed->value() );

      if( task->getSpeed() == 0 )
        {
          // TAS is zero, show nothing
          item->setText(4, "");
        }
      else
        {
          // TAS is not zero, show time total
          item->setText( 4, task->getTotalDistanceTimeString() + "h" );
        }
    }

  // Update task details too
  slotTaskDetails();
}

// This method is called from PreFlightWidget::accept(), to take out
// the selected task from the task list. The ownership of the taken
// FlightTask object goes over the the caller. He has to delete the
// object!!!
FlightTask* PreFlightTaskPage::takeSelectedTask()
{
  // qDebug("PreFlightTaskPage::selectedTask()");

  // save last used TAS, and wind parameters
  GeneralConfig::instance()->setTas( m_tas->value() );
  GeneralConfig::instance()->setWindDirection( m_windDirection->value() );
  GeneralConfig::instance()->setWindSpeed( m_windSpeed->value() );

  QList<QTreeWidgetItem*> selectList = m_taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      return static_cast<FlightTask *> (0);
    }

  QString id( m_taskList->selectedItems().at(0)->text(0) );

  // Special handling for entries with no number, they are system specific.
  if( id == " " )
    {
      GeneralConfig::instance()->setCurrentTask( "" );
      return static_cast<FlightTask *> (0);
    }

  // qDebug("selected Item=%s",id.toLatin1().data());
  GeneralConfig::instance()->setCurrentTask( m_taskList->selectedItems().at(0)->text(1) );

  // Nice trick, take selected element from list to prevent deletion of it, if
  // destruction of list is called.
  int index = id.toInt() - 1;

  FlightTask* task = m_flightTaskList.takeAt( index );

  // Save the task declaration in Flarm format as file.
  createFlarmTaskList( task );

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
  tfm.setTas( m_tas->value() );

  if( tfm.loadTaskList( m_flightTaskList ) == false )
    {
      // could not read file
      rowList << " " << tr("(No tasks defined)");
      m_taskList->addTopLevelItem( new QTreeWidgetItem(m_taskList, rowList, 0) );
      m_taskList->setCurrentItem( m_taskList->itemAt(0,m_taskList->topLevelItemCount()-1) );
      m_taskList->sortItems( 0, Qt::AscendingOrder );

      // reset current task
      GeneralConfig::instance()->setCurrentTask( "" );

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

  if ( m_flightTaskList.count() == 0 )
    {
      rowList << " " << tr("(No tasks defined)");

      // reset current task
      GeneralConfig::instance()->setCurrentTask( "" );
    }
  else
    {
      rowList << " " << tr("(Reset selection)");
    }

  m_taskList->addTopLevelItem( new QTreeWidgetItem(m_taskList, rowList, 0) );
  m_taskList->sortByColumn(0, Qt::AscendingOrder);

  updateWayTime();
  selectLastTask();

  m_taskList->resizeColumnToContents(0);
  m_taskList->resizeColumnToContents(1);
  m_taskList->resizeColumnToContents(2);
  m_taskList->resizeColumnToContents(3);
  m_taskList->resizeColumnToContents(4);

  return true;
}

void PreFlightTaskPage::slotNewTask()
{
  TaskEditor *te = new TaskEditor(this, m_taskNames);

  connect( te, SIGNAL(newTask( FlightTask * )), this,
            SLOT(slotUpdateTaskList( FlightTask * )));

  te->setVisible( true );
}

/**
 * taking over a new flight task from editor
 */
void PreFlightTaskPage::slotUpdateTaskList( FlightTask *newTask)
{
  m_flightTaskList.append( newTask );
  saveTaskList();
  m_taskContent->clear();
  m_taskList->clear();
  loadTaskList();
}

/**
 * pass the selected task to the editor
 */
void PreFlightTaskPage::slotEditTask()
{
  qDebug() << "PreFlightTaskPage::slotEditTask()";

  // fetch selected task item
  QList<QTreeWidgetItem*> selectList = m_taskList->selectedItems();

  if ( selectList.size() == 0 )
    {
      return;
    }

  QString id( m_taskList->selectedItems().at(0)->text(0) );

  if ( id == " ")
    {
      return;
    }

  m_editTask = m_flightTaskList.at(id.toInt() - 1);

  // make a deep copy of fetched task item
  FlightTask* modTask = new FlightTask( m_editTask->getCopiedTpList(),
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
  qDebug("PreFlightTaskPage::slotEditTaskList()");

  // search task item being edited
  int index = m_flightTaskList.indexOf( m_editTask );

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

  saveTaskList();
  m_taskContent->clear();
  m_taskList->clear();
  loadTaskList();
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

  if ( id == " " )
    {
      // Entries with no number are system specific and not deleteable
      return;
    }

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

  m_taskList->sortItems( 0, Qt::AscendingOrder );
  m_taskList->setCurrentItem( m_taskList->topLevelItem(0) );

  // reset last stored selected task
  GeneralConfig::instance()->setCurrentTask( "" );

  // reset task
  extern MapContents* _globalMapContents;
  _globalMapContents->setCurrentTask(0);

  uint no = id.toUInt() - 1;
  delete m_flightTaskList.takeAt( no );
  saveTaskList();
  m_taskContent->clear();
  m_taskList->clear();
  loadTaskList();
}

bool PreFlightTaskPage::saveTaskList()
{
  TaskFileManager tfm;

  return tfm.saveTaskList( m_flightTaskList );
}

/** Creates a task definition file in Flarm format. */
bool PreFlightTaskPage::createFlarmTaskList( FlightTask* flightTask )
{
  if( ! flightTask )
    {
      return false;
    }

  QList<TaskPoint *>& tpList = flightTask->getTpList();

  if( tpList.isEmpty() )
    {
      return false;
    }

  QString fn = GeneralConfig::instance()->getUserDataDirectory() + "/cumulus-flarm.tsk";

  // Save one backup copy.
  QFile::rename( fn, fn + ".bak" );

  QFile f(fn);

  if( !f.open( QIODevice::WriteOnly ) )
    {
      qWarning( "Could not write to task-file %s", f.fileName().toLatin1().data() );
      return false;
    }

  QTextStream stream( &f );
  stream.setCodec( "ISO 8859-15" );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "// Flarm task declaration created at "
         << dtStr
         << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl;

  stream << "$PFLAC,S,NEWTASK," << flightTask->getTaskName() << endl;

  for( int i = 0; i < tpList.count(); i++ )
    {
      // $PFLAC,S,ADDWP,4647900N,01252700E,Lienz Ni
      TaskPoint* tp = tpList.at(i);

      int degree, intMin;
      double min;

      WGSPoint::calcPos( tp->origP.x(), degree, min );

      // Minute is expected as 1/1000
      intMin = static_cast<int> (rint(min * 1000));

      QString lat = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 2, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("S") : QString("N") );

      WGSPoint::calcPos( tp->origP.y(), degree, min );

      intMin = static_cast<int> (rint(min * 1000));

      QString lon = QString("%1%2%3").
                    arg( (degree < 0) ? -degree : degree, 3, 10, QChar('0') ).
                    arg( (intMin < 0) ? -intMin : intMin, 5, 10, QChar('0') ).
                    arg( (degree < 0) ? QString("W") : QString("E") );

      stream << "$PFLAC,S,ADDWP,"
             << lat
             << "," << lon << ","
             << tp->name
             << endl;
    }

  stream << endl;
  f.close();

  return true;
}

/** Select the last stored task */
void PreFlightTaskPage::selectLastTask()
{
  QString lastTask = GeneralConfig::instance()->getCurrentTask();

  int rows = m_taskList->topLevelItemCount();

  for( int i = 0; i < rows; i++ )
    {
      QString taskName = m_taskList->topLevelItem(i)->text(1);
      // qDebug( "taskName(%d)=%s", i, taskName.toLatin1().data() );

      if( taskName == lastTask )
        {
          // last selected task found
          m_taskList->setCurrentItem( m_taskList->topLevelItem(i) );
          return;
        }
    }

  // select first entry in the list, if last selection could not be found
  m_taskList->setCurrentItem( m_taskList->topLevelItem(0) );
}

void PreFlightTaskPage::slotShowFlarmWidget()
{
  FlightTask *task = 0;

  QList<QTreeWidgetItem*> selectList = m_taskList->selectedItems();

  if( selectList.size() > 0 )
    {
      QTreeWidgetItem* selected = m_taskList->selectedItems().at(0);

      if ( selected->text( 0 ) != " " )
        {
          int id = selected->text( 0 ).toInt() - 1;

          task = m_flightTaskList.at( id );
        }
    }

  PreFlightFlarmPage* pffp = new PreFlightFlarmPage( task, this );
  pffp->setVisible( true );
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
  if ( newTask == static_cast<FlightTask *> (0) )
    {
      // No new task has been passed. Check, if a selected waypoint
      // exists and this waypoint belongs to a task. In this case we
      // will reset the selection.
      extern Calculator* calculator;
      const Waypoint *calcWp = calculator->getselectedWp();

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
      if( calculator->getselectedWp() )
        {
          // Reset taskpoint selection in calculator to prevent user query.
          emit newWaypoint(static_cast<Waypoint *> (0), true);

          // Select the start point of the new task.
          calculator->slot_startTask();
        }

      // Inform others about the new task
      emit newTaskSelected();
    }

  emit closingWidget();
  QWidget::close();
}

void PreFlightTaskPage::slotReject()
{
  emit closingWidget();
  QWidget::close();
}