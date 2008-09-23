/***********************************************************************
 **
 **   WpEditDialog.cpp
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

/**
 * The WpEditDialog allows the creation of a new waypoint or the modification
 * of an existing waypoint.
 * @author André Somers
 */


#include <QPushButton>
#include <QTabWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "wpeditdialog.h"
#include "wpeditdialogpagegeneral.h"
#include "wpeditdialogpageaero.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "generalconfig.h"

extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;

WpEditDialog::WpEditDialog(QWidget *parent, wayPoint *wp ):
  QDialog(parent)
{
  setObjectName("WpEditDialog");
  setModal(true);

#ifdef MAEMO
  resize(800,480);
  setSizeGripEnabled(false);
#else
  setSizeGripEnabled(true);
#endif

  if (wp == 0)
    {
      setWindowTitle(tr("New Waypoint"));
    }
  else
    {
      setWindowTitle(tr("Edit Waypoint"));
    }

  _wp = wp;

  WpEditDialogPageGeneral *pageGeneral = new WpEditDialogPageGeneral(this);
  WpEditDialogPageAero    *pageAero    = new WpEditDialogPageAero(this);
  comment = new QTextEdit(this);
  comment->setWordWrapMode(QTextOption::WordWrap);

  QTabWidget* tabWidget = new QTabWidget(this);
  tabWidget->addTab(pageGeneral, tr("&General"));
  tabWidget->addTab(pageAero, tr("&Aero"));
  tabWidget->addTab(comment, tr("&Comments"));

  connect(this, SIGNAL(load(wayPoint *)),
          pageGeneral, SLOT(slot_load(wayPoint *)));

  connect(this, SIGNAL(load(wayPoint *)),
          pageAero, SLOT(slot_load(wayPoint *)));

  connect(this, SIGNAL(save(wayPoint *)),
          pageGeneral, SLOT(slot_save(wayPoint *)));

  connect(this, SIGNAL(save(wayPoint *)),
          pageAero, SLOT(slot_save(wayPoint *)));

  // Add ok and cancel buttons
  QPushButton *cancel = new QPushButton;
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(26, 26));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton;
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(26, 26));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(cancel, 2);
  buttonBox->addSpacing(20);
  buttonBox->addWidget(ok, 2);
  buttonBox->addStretch(2);

  QHBoxLayout *mainLayout = new QHBoxLayout;
  mainLayout->addWidget(tabWidget);
  mainLayout->addLayout(buttonBox);
  setLayout(mainLayout);

  tabWidget->setCurrentWidget(pageGeneral);

  // load data into subwidgets
  slot_LoadCurrent();
}


WpEditDialog::~WpEditDialog()
{
  // qDebug("WpEditDialog::~WpEditDialog()");
}

/** This slot is called if the user presses the OK button. */
void WpEditDialog::slot_LoadCurrent()
{
  emit load(_wp);

  if (_wp)
    {
      comment->setText(_wp->comment);
    }
}

/** Called if OK button is pressed? */
void WpEditDialog::accept()
{
  // qDebug ("WpEditDialog::accept");
  if (_wp == 0)
    {
      _wp = new wayPoint;
      emit
      save(_wp);
      _wp->projP = _globalMapMatrix->wgsToMap(_wp->origP);
      //qDebug("new waypoint %s", (const char *)_wp->name);
    }
  else
    {
      //qDebug("change waypoint %s", (const char *)_wp->name);
      emit
      save(_wp);
      _wp->projP = _globalMapMatrix->wgsToMap(_wp->origP);
    }

  _wp->comment=comment->toPlainText();
  emit wpListChanged(_wp);
  QDialog::accept();
}
