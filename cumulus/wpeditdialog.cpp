/***********************************************************************
 **
 **   wpeditdialog.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QPushButton>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include "wpeditdialog.h"
#include "wpeditdialogpagegeneral.h"
#include "wpeditdialogpageaero.h"
#include "mapcontents.h"
#include "mapmatrix.h"

extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;

WPEditDialog::WPEditDialog(QWidget *parent, wayPoint *wp ):
  QDialog(parent)
{
  setObjectName("WPEditDialog");
  setModal(true);
  setSizeGripEnabled(true);
  
  if (wp==0) {
    setWindowTitle(tr("New Waypoint"));
  } else {
    setWindowTitle(tr("Edit Waypoint"));
  }
  _wp = wp;

  WPEditDialogPageGeneral * pageGeneral=new WPEditDialogPageGeneral(this);
  WPEditDialogPageAero * pageAero=new WPEditDialogPageAero(this);
  comment=new QTextEdit(this);
  comment->setWordWrapMode(QTextOption::WordWrap);

  QTabWidget* tabWidget = new QTabWidget (this);
  tabWidget->addTab(pageGeneral, tr("&General"));
  tabWidget->addTab(pageAero, tr("&Aero"));
  tabWidget->addTab(comment, tr("&Comments"));

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                     | QDialogButtonBox::Cancel);

  connect(this, SIGNAL(load(wayPoint *)),
          pageGeneral, SLOT(slot_load(wayPoint *)));

  connect(this, SIGNAL(load(wayPoint *)),
          pageAero, SLOT(slot_load(wayPoint *)));

  connect(this, SIGNAL(save(wayPoint *)),
          pageGeneral, SLOT(slot_save(wayPoint *)));

  connect(this, SIGNAL(save(wayPoint *)),
          pageAero, SLOT(slot_save(wayPoint *)));

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(tabWidget);
  mainLayout->addWidget(buttonBox);
  setLayout (mainLayout);

  tabWidget->setCurrentWidget(pageGeneral);
  
  // load data into subwidgets
  slot_LoadCurrent();
}


WPEditDialog::~WPEditDialog()
{
  // qDebug("WPEditDialog::~WPEditDialog()");
}

/** This slot is called if the user presses the OK button. */
void WPEditDialog::slot_LoadCurrent()
{
  emit load(_wp);

  if ( _wp ) {
    comment->setText(_wp->comment);
  }
}

/** Called if OK button is pressed? */
void WPEditDialog::accept()
{
  qDebug ("WPEditDialog::accept");
  if (_wp==0) {
    _wp=new wayPoint;
    emit save(_wp);
    _wp->projP = _globalMapMatrix->wgsToMap(_wp->origP);
    //qDebug("new waypoint %s", (const char *)_wp->name);
  } else {
    //qDebug("change waypoint %s", (const char *)_wp->name);
    emit save(_wp);
    _wp->projP = _globalMapMatrix->wgsToMap(_wp->origP);
  }

  _wp->comment=comment->text();
  emit wpListChanged(_wp);
  QDialog::accept();
}
