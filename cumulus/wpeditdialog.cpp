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
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * The WpEditDialog allows the creation of a new waypoint or the modification
 * of an existing waypoint.
 *
 * @author André Somers
 */

#include <QMessageBox>
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

  // load waypoint data into tabulator widgets
  loadWaypointData();
}

WpEditDialog::~WpEditDialog()
{
  // qDebug("WpEditDialog::~WpEditDialog()");
}

/** This method is called to load the waypoint data in the tab widgets.
 *  The load will be done only for existing wayppoint objects. */
void WpEditDialog::loadWaypointData()
{
  if( _wp )
    {
      emit load(_wp);
      comment->setText(_wp->comment);
    }
}

/** Called if OK button is pressed */
void WpEditDialog::accept()
{
  // qDebug ("WpEditDialog::accept");
  if( _wp == 0 )
    {
      // create a new waypoint from edited data
      wayPoint newWp;
      emit save( &newWp );
      newWp.projP = _globalMapMatrix->wgsToMap( newWp.origP );
      newWp.comment = comment->toPlainText();

      if( checkWaypointData( newWp ) == false )
        {
          // reject saving due to missing data items
          return;
        }

      if( _globalMapContents->isInWaypointList( newWp.name ) )
        {
          // The waypoint name is already in use, reject accept.
          QMessageBox::critical( this,tr("Name already in use"),
                                 tr("Please use another name for your new waypoint"),
                                 QMessageBox::Close );
          return;
        }

      emit wpListChanged( newWp );
    }
  else
    {
      // update existing waypoint with edited data
      emit save( _wp );
      _wp->projP = _globalMapMatrix->wgsToMap(_wp->origP);
      _wp->comment=comment->toPlainText();

      if( checkWaypointData( *_wp ) == false )
        {
          // reject saving due to missing data items
          return;
        }

      emit wpListChanged( *_wp );
    }

  QDialog::accept();
}

/**
 * This method checks, if all mandatory waypoint data have been defined.
 * Returns true on ok otherwise false.
 */
bool WpEditDialog::checkWaypointData( wayPoint& wp )
{
  if( wp.name.isEmpty() )
    {
      QMessageBox::critical( this,tr("missing name"),
                             tr("Please add a waypoint name"),
                             QMessageBox::Close );
      return false;
    }

  if( wp.description.isEmpty() )
    {
      QMessageBox::critical( this,tr("missing description"),
                             tr("Please add a waypoint description"),
                             QMessageBox::Close );
      return false;
    }

  if( wp.origP == QPoint(0,0) )
      {
        int answer = QMessageBox::warning(this,tr("missing coordinates"),
                                          tr("Waypoint coordinates not set, continue?"),
                                          QMessageBox::No, QMessageBox::Yes );
        if( answer == QMessageBox::Yes )
          { // yes was chosen, ignore warning
            return true;
          }
        else
          {
            return false;
          }
      }

  return true;
}
