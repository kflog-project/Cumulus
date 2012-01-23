/***********************************************************************
 **
 **   preflightwidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2003      by André Somers
 **                   2008-2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "preflightwidget.h"
#include "map.h"
#include "mapcontents.h"
#include "preflightgliderpage.h"
#include "preflighttasklist.h"
#include "preflightmiscpage.h"
#include "preflightwaypointpage.h"

#include "calculator.h"
#include "layout.h"

extern MapContents* _globalMapContents;

PreFlightWidget::PreFlightWidget(QWidget* parent, const char* name) :
  QWidget(parent)
{
  // qDebug("PreFlightWidget::PreFlightWidget()");
  setObjectName("PreFlightWidget");
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(tr("Preflight settings"));

  tabWidget = new QTabWidget(this);
  tabWidget->setTabPosition(QTabWidget::West);

  gliderpage = new PreFlightGliderPage(this);
  tabWidget->addTab(gliderpage, "");

  taskpage = new PreFlightTaskList(this);
  tabWidget->addTab(taskpage, "");

  wppage = new PreFlightWaypointPage(this);
  tabWidget->addTab(wppage, "");

  connect( wppage, SIGNAL(waypointsAdded()),
           Map::getInstance(), SLOT(slotRedraw()) );

  miscpage = new PreFlightMiscPage(this);
  tabWidget->addTab(miscpage, "");

  QShortcut* scLeft = new QShortcut(Qt::Key_Left, this);
  QShortcut* scRight = new QShortcut(Qt::Key_Right, this);

  connect(scLeft, SIGNAL(activated()), this, SLOT(slot_keyLeft()));
  connect(scRight, SIGNAL(activated()), this, SLOT(slot_keyRight()));

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

  connect(ok, SIGNAL(clicked()), this, SLOT(slot_accept()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(slot_reject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(cancel, 2);
  buttonBox->addSpacing(20);
  buttonBox->addWidget(ok, 2);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix, 1);

  QHBoxLayout *contentLayout = new QHBoxLayout;
  contentLayout->addWidget(tabWidget);
  contentLayout->addLayout(buttonBox);

  setLayout(contentLayout);

  // Activate keyboard shortcuts as close button
  QShortcut* scCancel = new QShortcut( this );

  scCancel->setKey( Qt::Key_Escape );
  scCancel->setKey( Qt::Key_Close );

  connect( scCancel, SIGNAL( activated() ), this, SLOT( slot_reject() ) );

  miscpage->load();
  wppage->load();

  // check to see which tabulator to bring forward
  if (QString(name) == "taskselection")
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(taskpage));
    }
  else
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(gliderpage));
    }

  setLabels();
  setVisible( true );
}

PreFlightWidget::~PreFlightWidget()
{
  // qDebug("PreFlightWidget::~PreFlightWidget()");
}

/** Sets all widget labels, which need a translation. */
void PreFlightWidget::setLabels()
{
  tabWidget->setTabText( 0, tr("Glider") );
  tabWidget->setTabText( 1, tr("Task") );
  tabWidget->setTabText( 2, tr("Waypoints") );
  tabWidget->setTabText( 3, tr("Common") );
}

/** Used to handle language change events */
void PreFlightWidget::changeEvent( QEvent* event )
{
  if( event->type() == QEvent::LanguageChange )
    {
      setLabels();
    }
  else
    {
      QWidget::changeEvent( event );
    }
}

void PreFlightWidget::slot_accept()
{
  FlightTask *curTask = _globalMapContents->getCurrentTask();

  // Note we have overtaken the ownership about this object!
  FlightTask *newTask = taskpage->takeSelectedTask();

  bool newTaskPassed = true;

  // Check, if a new task has been passed for accept.
  if (curTask && newTask && curTask->getTaskName() == newTask->getTaskName())
    {
      newTaskPassed = false; // task names identical
    }

  if (curTask && newTask && newTaskPassed)
    {
      int answer = QMessageBox::question(this, tr("Replace previous task?"),
          tr( "<html>"
              "Do you want to replace the previous task?<br>"
              "A selected target is reset at task start."
              "</html>"), QMessageBox::Yes,
          QMessageBox::No | QMessageBox::Escape);

      if (answer != QMessageBox::Yes)
        {
          // do nothing change
          delete newTask;
          slot_reject();
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

      if (calcWp && calcWp->taskPointIndex != -1)
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

  gliderpage->save();
  miscpage->save();
  wppage->save();

  setVisible( false );
  emit settingsChanged();
  emit closeConfig();
  QWidget::close();
}

void PreFlightWidget::slot_reject()
{
  // qDebug("PreFlightWidget::slot_reject()");
  setVisible( false );
  emit closeConfig();
  QWidget::close();
}

void PreFlightWidget::slot_keyRight()
{
  if (tabWidget->currentWidget() == gliderpage)
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(taskpage));
    }
  else if (tabWidget->currentWidget() == taskpage)
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(miscpage));
    }
  else if (tabWidget->currentWidget() == wppage)
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(miscpage));
    }
  else if (tabWidget->currentWidget() == miscpage)
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(gliderpage));
    }
}

void PreFlightWidget::slot_keyLeft()
{
  if (tabWidget->currentWidget() == miscpage)
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(wppage));
    }
  else if (tabWidget->currentWidget() == wppage)
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(taskpage));
    }
  else if (tabWidget->currentWidget() == taskpage)
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(gliderpage));
    }
  else if (tabWidget->currentWidget() == gliderpage)
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(miscpage));
    }
}
