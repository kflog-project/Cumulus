/***********************************************************************
**
**   TaskListStatusView.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2021 by Axel Pauli
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

#include "distance.h"
#include "layout.h"
#include "helpbrowser.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "TaskListStatusView.h"
#include "flighttask.h"

extern MapContents *_globalMapContents;

TaskListStatusView::TaskListStatusView( QWidget *parent ) :
  QWidget(parent),
  rowDelegate(0),
  m_task(0)
{
  setObjectName( "TaskListStatusView" );
  setWindowTitle( tr( "Flight Task Status" ) );
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute( Qt::WA_DeleteOnClose );

  resize( MainWindow::mainWindow()->size() );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  QHBoxLayout *headlineBox = new QHBoxLayout;
  headlineBox->setSpacing( 10 );

  distance = new QLabel( "", this );
  avSpeed = new QLabel( "", this );
  flightTime = new QLabel( "", this );
  startTime = new QLabel( "", this );
  endTime = new QLabel( "", this );

  distance->setAlignment( Qt::AlignCenter );
  avSpeed->setAlignment( Qt::AlignCenter );
  flightTime->setAlignment( Qt::AlignCenter );
  startTime->setAlignment( Qt::AlignCenter );
  endTime->setAlignment( Qt::AlignCenter );

  headlineBox->addWidget( distance );
  headlineBox->addWidget( avSpeed );
  headlineBox->addWidget( flightTime );
  headlineBox->addWidget( startTime );
  headlineBox->addWidget( endTime );
  topLayout->addLayout( headlineBox );

  list = new QTreeWidget( this );
  list->setObjectName( "TaskListStatusView" );
  list->setColumnCount( 5 );
  list->setRootIsDecorated( false );
  list->setItemsExpandable( false );
  list->setUniformRowHeights( true );
  list->setAlternatingRowColors( true );
  list->setSortingEnabled( false );
  list->setSelectionMode( QAbstractItemView::NoSelection );
  list->setFocusPolicy( Qt::NoFocus );
  setHeader();

  list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  list->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  list->setAllColumnsShowFocus( true );
  list->setSelectionMode( QAbstractItemView::SingleSelection );
  list->setSelectionBehavior( QAbstractItemView::SelectRows );
  list->setFocusPolicy( Qt::StrongFocus );
  topLayout->addWidget(list, 10);

  int buttonSize = Layout::getButtonSize();

  // Don't show any buttons, if required
  QBoxLayout *buttonrow = new QHBoxLayout;
  topLayout->addLayout( buttonrow );

  QPushButton *cmdClose = new QPushButton( tr( "Close" ), this );
  cmdClose->setMinimumHeight( buttonSize );
  buttonrow->addWidget( cmdClose );
  buttonrow->addStretch( 5 );

  QPushButton *cmdHelp = new QPushButton( tr( "Help" ), this );
  buttonrow->addWidget( cmdHelp );
  cmdHelp->setMinimumHeight( buttonSize );

  connect( cmdHelp, SIGNAL( clicked() ), this, SLOT( slot_Help() ) );
  connect( cmdClose, SIGNAL( clicked() ), this, SLOT( slot_Close() ) );
}

TaskListStatusView::~TaskListStatusView()
{
}

/** sets the header of the list */
void TaskListStatusView::setHeader()
{
  QStringList sl;

  sl << tr("Leg")
     << tr("From-To")
     << tr("Length")
     << tr("Time")
     << tr("AV-Speed");

  list->setHeaderLabels(sl);

  QTreeWidgetItem* headerItem = list->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 4, Qt::AlignCenter );

  resizeTaskList();
}

void TaskListStatusView::showEvent(QShowEvent *)
{
  // get the current activated task
  FlightTask* ft = _globalMapContents->getCurrentTask();
  slot_setTask( ft );

  resizeTaskList();
  list->setFocus();
}

/** This slot is called if the info button has been clicked */
void TaskListStatusView::slot_Help()
{
  QString file = "cumulus-preflight-settings-task.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
  hb->getTextBrowser()->scrollToAnchor("Taskeditor");
}

/** @ee This slot is called if the listview is closed without selecting */
void TaskListStatusView::slot_Close()
{
  close();
}

/**
 * Retrieves the task points from the task, and fills the list.
 */
void TaskListStatusView::slot_setTask( FlightTask *task )
{
  // At first reset all to default
  clear();

  // Store only the pointer to the task
  m_task = task;

  if( task == static_cast<FlightTask *>(0) )
    {
      // An empty task was passed
      resizeTaskList();
      return;
    }

  slot_updateTask();
}

/**
 * Set or Update the internal task data.
 */
void TaskListStatusView::slot_updateTask()
{
  if( m_task == static_cast<FlightTask *>( 0 ) )
    {
      return;
    }

  // Set the total distance of task
  distance ->setText( "S=" + m_task->getTaskDistanceString().remove(QChar(' ')) );

  // set the total values in the header of this view
  Speed tav = m_task->calAverageSpeed();

  if( tav.isValid() )
    {
      avSpeed->setText( "AV=" + tav.getHorizontalText( true, 1 ) );
    }
  else
    {
      avSpeed->setText( "AV=??" );
    }

  const QDateTime& stime = m_task->getStartTime();

  if( stime.isValid() )
    {
      startTime->setText( "ST=" + stime.time().toString( "hh:mm:ss" ) );list->clear();
    }
  else
    {
      startTime->setText( "ST=??" );
    }

  int ftime = m_task->getFlightTime();

  if( ftime > 0 )
    {
      int hh = ftime / 3600;
      int mm = (ftime % 3600) / 60;
      int ss = (ftime % 3600) % 60;

      QString tstr;
      tstr = QString( "%1:%2:%3" ).arg( hh, 2, 10, QChar('0') )
                                  .arg( mm, 2, 10, QChar('0') )
                                  .arg( ss, 2, 10, QChar('0') );

      flightTime->setText( "FT=" + tstr );
    }

  const QDateTime& etime = m_task->getEndTime();

  if( etime.isValid() )
    {
      endTime->setText( "ET=" + etime.time().toString( "hh:mm:ss" ) );
    }
  else
    {
      endTime->setText( "ET=??" );
    }

  list->clear();

  // set row height at each list fill - has probably changed.
  // Note: rpMargin is a manifold of 2 to ensure symmetry
  int rpMargin = GeneralConfig::instance()->getListDisplayRPMargin();

  if ( rowDelegate )
    {
      rowDelegate->setVerticalMargin( rpMargin );
    }
  else
    {
      rowDelegate = new RowDelegate( list, rpMargin );
      list->setItemDelegate( rowDelegate );
    }

  QList<TaskPoint *> tpList = m_task->getTpList();

  for( int i = 0; i < tpList.size() - 1; i++ )
    {
      TaskPoint* tp1 = tpList.at( i );
      TaskPoint* tp2 = tpList.at( i + 1 );

      QTreeWidgetItem* twi = new QTreeWidgetItem( list );

      if( tpList.size() < 10 )
        {
          twi->setText( 0, QString::number( i + 1 ) );
        }
      else
        {
          QString no = QString( "%1" ).arg( i + 1, 2, 10, QChar( '0' ) );
          twi->setText( 0, no );
        }

      twi->setTextAlignment( 0, Qt::AlignCenter );

      twi->setText( 1, tp1->getWPName() + "-" + tp2->getWPName() );

      // Get the distance to the prevous tp.
      Distance dist = tp2->getDistance2forerunner();
      twi->setText( 2, Distance::getText( dist.getMeters(), true ) );

      // calculate the leg time, if possible
      QDateTime t1 = tp1->getPassTime();
      QDateTime t2 = tp2->getPassTime();

      if( t1.isValid() && t2.isValid() )
        {
          // Leg time in s
          int lt = t1.secsTo( t2 );

          int hh = lt / 3600;
          int mm = (lt % 3600) / 60;
          int ss = (lt % 3600) % 60;

          QString tstr;
          tstr = QString( "%1:%2:%3" ).arg( hh, 2, 10, QChar('0') )
                                      .arg( mm, 2, 10, QChar('0') )
                                      .arg( ss, 2, 10, QChar('0') );
          twi->setText( 3, tstr );

          // calculate leg average speed in m/s
          double av = dist.getMeters() / double( lt );
          Speed speed( av );
          twi->setText( 4, speed.getHorizontalText( true, 1 ) );
        }
      else
        {
          twi->setText( 3, "--" );
          twi->setText( 4, "--" );
        }

      twi->setTextAlignment( 3, Qt::AlignCenter | Qt::AlignRight );
      twi->setTextAlignment( 4, Qt::AlignCenter );
    }

  setHeader();

  // Fire an update timer after 5 seconds.
  QTimer::singleShot( 5000, this, SLOT( slot_updateTask() ) );
}

/** Resizes the columns of the task list to their contents. */
void TaskListStatusView::resizeTaskList()
{
  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->resizeColumnToContents(2);
  list->resizeColumnToContents(3);
  list->resizeColumnToContents(4);
}

void TaskListStatusView::clear()
{
  list->clear();

  distance->setText("");
  avSpeed->setText("");
  flightTime->setText("");
  startTime->setText("");
  endTime->setText("");

  setHeader();
}
