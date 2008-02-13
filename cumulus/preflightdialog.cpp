/***********************************************************************
**
**   preflightdialog.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <Q3Accel>
#include <QMessageBox>
#include <Q3TabDialog>

#include "preflightdialog.h"
#include "mapcontents.h"
#include "preflightgliderpage.h"
#include "preflightmiscpage.h"
#include "cucalc.h"

extern MapContents * _globalMapContents;

PreFlightDialog::PreFlightDialog(QWidget * parent, const char * name):
    Q3TabDialog(parent, "PreFlightDialog", true, Qt::WStyle_StaysOnTop)
{
  setCaption(tr("Cumulus Preflight settings"));
  setOkButton();
  setCancelButton();

  gliderpage = new PreFlightGliderPage(this,"gliderpage");
  addTab(gliderpage, tr("&Glider"));

  taskpage=new TaskList(this, "taskpage");
  addTab(taskpage, tr("&Task"));

  miscpage =new PreFlightMiscPage(this,"miscpage");
  addTab(miscpage, tr("&Common"));

  Q3Accel *acc = new Q3Accel(this);
  acc->connectItem(acc->insertItem(Qt::Key_Space),
                   this, SLOT(accept()));
  acc->connectItem(acc->insertItem(Qt::Key_Left),
                   this, SLOT(keyLeft()));
  acc->connectItem(acc->insertItem(Qt::Key_Right),
                   this, SLOT(keyRight()));

  miscpage->load();

  //check to see which tab to bring forward
  if (QString (name) == "taskselection")
    {
      showPage (taskpage);
    }
  else
    {
      showPage (gliderpage);
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
  if( currentPage() == gliderpage )
    {
      showPage(taskpage);
    }
  else if( currentPage() == taskpage )
    {
      showPage(miscpage);
    }
  else if( currentPage() == miscpage )
    {
      showPage(gliderpage);
    }
}


void PreFlightDialog::keyLeft()
{
  if(  currentPage() == miscpage )
    {
      showPage(taskpage);
    }
  else if(  currentPage() == taskpage )
    {
      showPage(gliderpage);
    }
  else if(  currentPage() == gliderpage )
    {
      showPage(miscpage);
    }
}
