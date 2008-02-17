/***********************************************************************
**
**   preflightdialog.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003 by Andr� Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QMessageBox>
#include <QDialogButtonBox>
#include <QShortcut>

#include "preflightdialog.h"
#include "mapcontents.h"
#include "preflightgliderpage.h"
#include "preflightmiscpage.h"
#include "cucalc.h"

extern MapContents * _globalMapContents;

PreFlightDialog::PreFlightDialog(QWidget * parent, const char * name):
    QDialog(parent, "PreFlightDialog", true, Qt::WStyle_StaysOnTop)
{
  setWindowTitle(tr("Cumulus Preflight settings"));

  tabWidget = new QTabWidget (this);

  gliderpage = new PreFlightGliderPage(this,"gliderpage");
  tabWidget->addTab(gliderpage, tr("&Glider"));

  taskpage=new TaskList(this, "taskpage");
  tabWidget->addTab(taskpage, tr("&Task"));

  miscpage =new PreFlightMiscPage(this,"miscpage");
  tabWidget->addTab(miscpage, tr("&Common"));

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                 | QDialogButtonBox::Cancel);

  QShortcut* scLeft = new QShortcut(this);
  QShortcut* scRight = new QShortcut(this);
  QShortcut* scSpace = new QShortcut(this);
  scLeft->setKey (Qt::Key_Left);
  scRight->setKey (Qt::Key_Right);
  scSpace->setKey (Qt::Key_Space);
  connect(scLeft,    SIGNAL(activated()),this, SLOT(keyLeft()));
  connect(scRight,   SIGNAL(activated()),this, SLOT(keyRight()));
  connect(scSpace,   SIGNAL(activated()),this, SLOT(accept()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabWidget);
  mainLayout->addWidget(buttonBox);
  setLayout(mainLayout);

  miscpage->load();

  //check to see which tab to bring forward
  if (QString (name) == "taskselection")
  {
    tabWidget->showPage (taskpage);
  }
  else
  {
    tabWidget->showPage (gliderpage);
  }

  show();
}


PreFlightDialog::~PreFlightDialog()
{
  // qDebug("PreFlightDialog::~PreFlightDialog()");
}


void PreFlightDialog::accept()
{

  FlightTask *curTask = _globalMapContents->getCurrentTask();

  // Note we have overtaken the ownership about this object!
  FlightTask *newTask = taskpage->takeSelectedTask();

  bool newTaskPassed = true;

  // Check, if a new task has been passed for accept.
  if( curTask && newTask &&
      curTask->getTaskName() == newTask->getTaskName() )
    {
      newTaskPassed = false; // task names identical
    }

  if( curTask && newTask && newTaskPassed )
    {
      int answer=
        QMessageBox::warning(this,tr("Replace previous task?"),
                             tr("<qt>"
                                "Do you want to replace the previous task?<br>"
                                "Waypoint selection is reset at start position."
                                "</qt>"),
                             QMessageBox::Ok | QMessageBox::Default,
                             QMessageBox::Cancel | QMessageBox::Escape );

      if( answer != QMessageBox::Ok )
        {
          // do nothing change
          delete newTask;
          reject();
          return;
        }
    }

  // Forward of new task in every case, user can have modified
  // content. MapContent will overtake the ownership of the task
  // object.
  _globalMapContents->setCurrentTask( newTask );

  // @AP: Open problem with waypoint selection, if user has modified
  // task content. We ignore that atm.

 if ( newTask == (FlightTask *) 0 )
    {
      // No new task has been passed. Check, if a selected waypoint
      // exists and this waypoint belongs to a task. In this case we
      // will reset the selection.
      extern CuCalc* calculator;
      const wayPoint *calcWp = calculator->getselectedWp();

      if( calcWp && calcWp->taskPointIndex != -1 )
        {
          // reset taskpoint selection
          emit newWaypoint( (wayPoint *) 0, true);
        }
    }

  gliderpage->save();
  miscpage->save();

  emit settingsChanged();
  QDialog::accept();
  delete this;
}


void PreFlightDialog::reject()
{
  // qDebug("PreFlightDialog::reject()");
  QDialog::reject();
  delete this;
}


void PreFlightDialog::keyRight()
{
  if( tabWidget->currentPage() == gliderpage )
    {
      tabWidget->showPage(taskpage);
    }
  else if( tabWidget->currentPage() == taskpage )
    {
      tabWidget->showPage(miscpage);
    }
  else if( tabWidget->currentPage() == miscpage )
    {
      tabWidget->showPage(gliderpage);
    }
}


void PreFlightDialog::keyLeft()
{
  if(  tabWidget->currentPage() == miscpage )
    {
      tabWidget->showPage(taskpage);
    }
  else if(  tabWidget->currentPage() == taskpage )
    {
      tabWidget->showPage(gliderpage);
    }
  else if(  tabWidget->currentPage() == gliderpage )
    {
      tabWidget->showPage(miscpage);
    }
}
