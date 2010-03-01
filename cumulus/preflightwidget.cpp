/***********************************************************************
 **
 **   preflightwidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2003      by André Somers
 **                   2008-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QMessageBox>
#include <QShortcut>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolTip>
#include <QLabel>

#include "preflightwidget.h"
#include "mapcontents.h"
#include "preflightgliderpage.h"
#include "preflightmiscpage.h"
#include "calculator.h"

extern MapContents* _globalMapContents;

/** Widget for preflight settings. To reserve the full vertical space for the
 *  the content of the tabulators, tabulators are arranged at the
 *  left side and the ok and cancel buttons are arranged on the right side.
 */

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
  gliderpage->setToolTip(tr("Select a glider to be used"));
  tabWidget->addTab(gliderpage, tr("Glider"));

  taskpage = new TaskList(this);
  tabWidget->addTab(taskpage, tr("Task"));

  miscpage = new PreFlightMiscPage(this);
  miscpage->setToolTip(tr("Define common flight parameters"));
  tabWidget->addTab(miscpage, tr("Common"));

  QShortcut* scLeft = new QShortcut(Qt::Key_Left, this);
  QShortcut* scRight = new QShortcut(Qt::Key_Right, this);
  QShortcut* scReturn = new QShortcut(Qt::Key_Return, this);

  connect(scLeft, SIGNAL(activated()), this, SLOT(slot_keyLeft()));
  connect(scRight, SIGNAL(activated()), this, SLOT(slot_keyRight()));
  connect(scReturn, SIGNAL(activated()), this, SLOT(slot_accept()));

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(26, 26));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(26, 26));
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

  miscpage->load();

  // check to see which tabulator to bring forward
  if (QString(name) == "taskselection")
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(taskpage));
    }
  else
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(gliderpage));
    }

  show();
}

PreFlightWidget::~PreFlightWidget()
{
  // qDebug("PreFlightWidget::~PreFlightWidget()");
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
      int answer = QMessageBox::question(this, tr("Replace previous task?"), tr(
          "<html><b>"
            "Do you want to replace the previous task?<br>"
            "Waypoint selection is reset at start position."
            "</b></html>"), QMessageBox::Yes,
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
      const wayPoint *calcWp = calculator->getselectedWp();

      if (calcWp && calcWp->taskPointIndex != -1)
        {
          // reset taskpoint selection
          emit newWaypoint(static_cast<wayPoint *> (0), true);
        }
    }

  gliderpage->save();
  miscpage->save();

  hide();
  emit settingsChanged();
  emit closeConfig();
  QWidget::close();
}

void PreFlightWidget::slot_reject()
{
  // qDebug("PreFlightWidget::slot_reject()");
  hide();
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
  else if (tabWidget->currentWidget() == miscpage)
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(gliderpage));
    }
}

void PreFlightWidget::slot_keyLeft()
{
  if (tabWidget->currentWidget() == miscpage)
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
