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

#include <QMessageBox>
#include <QShortcut>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolTip>

#include "preflightdialog.h"
#include "mapcontents.h"
#include "preflightgliderpage.h"
#include "preflightmiscpage.h"
#include "cucalc.h"

extern MapContents* _globalMapContents;

/** Dialog for pre-flight settings. To reserve the full vertical space for the
 *  the content of the tabulators, tabulators are arranged at the
 *  left side and the ok and cancel buttons are arranged on the right side.
 */

PreFlightDialog::PreFlightDialog(QWidget* parent, const char* name) :
  QWidget(parent)
{
  qDebug("PreFlightDialog::PreFlightDialog()");
  setObjectName("PreFlightDialog");
//  setModal(true);
//  setWindowTitle(tr("Preflight settings"));
//  setSizeGripEnabled(true);

  QVBoxLayout *topLayout = new QVBoxLayout(this);
  setLayout(topLayout);
 
  title = new QLabel("<b>Pre-Flight Settings</b>", this);
  topLayout->addWidget(title);
  title->hide();

  tabWidget = new QTabWidget(this);
  tabWidget->setTabPosition( QTabWidget::West );

  gliderpage = new PreFlightGliderPage(this);
  gliderpage->setToolTip(tr("Select a glider to be used"));
  tabWidget->addTab(gliderpage, tr("Glider"));

  taskpage = new TaskList(this);
  taskpage->setToolTip(tr("Select or define a flight task"));
  tabWidget->addTab(taskpage, tr("Task"));

  miscpage = new PreFlightMiscPage(this);
  miscpage->setToolTip(tr("Define common flight parameters"));
  tabWidget->addTab(miscpage, tr("Common"));

  QShortcut* scLeft  = new QShortcut(Qt::Key_Left, this);
  QShortcut* scRight = new QShortcut(Qt::Key_Right, this);
  QShortcut* scSpace = new QShortcut(Qt::Key_Space, this);

  connect(scLeft,  SIGNAL(activated()),this, SLOT(keyLeft()));
  connect(scRight, SIGNAL(activated()),this, SLOT(keyRight()));
  connect(scSpace, SIGNAL(activated()),this, SLOT(accept()));

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "cancel.png") ) );
  cancel->setIconSize(QSize(26,26));
  cancel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "ok.png") ) );
  ok->setIconSize(QSize(26,26));
  ok->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::QSizePolicy::Preferred);

  connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget( cancel, 2 );
  buttonBox->addSpacing(20);
  buttonBox->addWidget( ok, 2 );
  buttonBox->addStretch(2);

  QHBoxLayout *contentLayout = new QHBoxLayout;
  contentLayout->addWidget(tabWidget);
  contentLayout->addLayout(buttonBox);

  topLayout->addLayout(contentLayout);

  miscpage->load();

  //check to see which tab to bring forward
  if (QString (name) == "taskselection")
  {
    tabWidget->setCurrentIndex( tabWidget->indexOf(taskpage) );
  }
  else
  {
    tabWidget->setCurrentIndex( tabWidget->indexOf(gliderpage) );
  }

//#ifdef MAEMO
//  resize( 640, 480 );
//#endif

  show();
  setWindowState(windowState() ^ Qt::WindowFullScreen);
}

PreFlightDialog::~PreFlightDialog()
{
  qDebug("PreFlightDialog::~PreFlightDialog()");
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
                             tr("<html><b>"
                                "Do you want to replace the previous task?<br>"
                                "Waypoint selection is reset at start position."
                                "</b></html>"),
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

  hide();
  emit settingsChanged();
  emit closeConfig();
//  QDialog::accept();
}


void PreFlightDialog::reject()
{
  // qDebug("PreFlightDialog::reject()");
//  QDialog::reject();
  hide();
  emit closeConfig();
}


void PreFlightDialog::keyRight()
{
  if( tabWidget->currentWidget() == gliderpage )
    {
      tabWidget->setCurrentIndex( tabWidget->indexOf(taskpage) );
    }
  else if( tabWidget->currentWidget() == taskpage )
    {
      tabWidget->setCurrentIndex( tabWidget->indexOf(miscpage) );
    }
  else if( tabWidget->currentWidget() == miscpage )
    {
      tabWidget->setCurrentIndex( tabWidget->indexOf(gliderpage) );
    }
}


void PreFlightDialog::keyLeft()
{
  if(  tabWidget->currentWidget() == miscpage )
    {
      tabWidget->setCurrentIndex( tabWidget->indexOf(taskpage) );
    }
  else if(  tabWidget->currentWidget() == taskpage )
    {
      tabWidget->setCurrentIndex( tabWidget->indexOf(gliderpage) );
    }
  else if(  tabWidget->currentWidget() == gliderpage )
    {
      tabWidget->setCurrentIndex( tabWidget->indexOf(miscpage) );
    }
}

void PreFlightDialog::resizeEvent(QResizeEvent*)
{
  if ( ( (QWidget*)parent() )->windowState() == Qt::WindowFullScreen )
    title->show();
  else
    title->hide();
}
