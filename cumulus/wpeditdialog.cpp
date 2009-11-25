/***********************************************************************
 **
 **   WpEditDialog.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by André Somers
 **                   2008-2009 by Axel Pauli
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
#include <QScrollArea>

#include "wpeditdialog.h"
#include "wpeditdialogpagegeneral.h"
#include "wpeditdialogpageaero.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "generalconfig.h"
#include "mainwindow.h"

extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;
extern MainWindow  *_globalMainWindow;

WpEditDialog::WpEditDialog(QWidget *parent, wayPoint *wp ) : QDialog(parent)
{
  setObjectName("WpEditDialog");
  setAttribute(Qt::WA_DeleteOnClose);
  setModal(true);

  if( _globalMainWindow )
    {
      // Resize the dialog to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  if( wp == 0 )
    {
      setWindowTitle(tr("New Waypoint"));
      oldName = "";
    }
  else
    {
      setWindowTitle(tr("Edit Waypoint"));
      oldName = wp->name;
    }

  _wp = wp;

  QTabWidget* tabWidget = new QTabWidget(this);

  // Put all pages into a scroll area. Needed by Maemo Qt, if virtual
  // keyboard is pop up.
  QScrollArea* pgArea = new QScrollArea( tabWidget );
  pgArea->setWidgetResizable( true );
  pgArea->setFrameStyle( QFrame::NoFrame );
  WpEditDialogPageGeneral *pageG = new WpEditDialogPageGeneral(this);
  pgArea->setWidget( pageG );
  tabWidget->addTab( pgArea, tr("General") );

  QScrollArea* paArea = new QScrollArea( tabWidget );
  paArea->setWidgetResizable( true );
  paArea->setFrameStyle( QFrame::NoFrame );
  WpEditDialogPageAero *pageA = new WpEditDialogPageAero(this);
  paArea->setWidget( pageA );
  tabWidget->addTab( paArea, tr("Aero") );

  QScrollArea* pcArea = new QScrollArea( tabWidget );
  pcArea->setWidgetResizable( true );
  pcArea->setFrameStyle( QFrame::NoFrame );
  comment = new QTextEdit(this);
  comment->setWordWrapMode(QTextOption::WordWrap);
  pcArea->setWidget( comment );
  tabWidget->addTab( pcArea, tr("Comment") );

  connect(this, SIGNAL(load(wayPoint *)),
          pageG, SLOT(slot_load(wayPoint *)));

  connect(this, SIGNAL(load(wayPoint *)),
          pageA, SLOT(slot_load(wayPoint *)));

  connect(this, SIGNAL(save(wayPoint *)),
          pageG, SLOT(slot_save(wayPoint *)));

  connect(this, SIGNAL(save(wayPoint *)),
          pageA, SLOT(slot_save(wayPoint *)));

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

  tabWidget->setCurrentWidget(pageG);

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

  // get waypoint data from the tab widgets and save them in a new object
  wayPoint newWp;
  emit save( &newWp );
  newWp.projP = _globalMapMatrix->wgsToMap( newWp.origP );
  newWp.comment = comment->toPlainText();

  // Make some mandatory consistency checks
  if( checkWaypointData( newWp ) == false )
    {
      // reject saving due to missing data items
      return;
    }

  if( _wp == 0 )
    {
      if( isWaypointNameInList( newWp.name ) )
        {
          // Waypoint name is already to find in the global list.
          // To avoid multiple entries with the same name, the
          // accept is rejected.
          return;
        }

      // The new waypoint is posted to the subscribers
      emit wpListChanged( newWp );
    }
  else
    {
      // Update existing waypoint item with edited data. Note that the object in the
      // global waypoint list is updated because we work with a reference to it!
      // Therefore we use a temporary object for saving and checking.
      if( oldName != newWp.name && isWaypointNameInList( newWp.name ) )
        {
          // The waypoint name of the saved object was modified and the new name
          // is already in use in the global list. To avoid multiple entries with
          //the same name, the accept is rejected.
          return;
        }

      // Update old waypoint object
      *_wp = newWp;

      // The modified waypoint is posted to the subscribers
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
      QMessageBox::critical( this,tr("Name?"),
                             tr("Please add\na waypoint\nname"),
                             QMessageBox::Close );
      return false;
    }

  if( wp.description.isEmpty() )
    {
      QMessageBox::critical( this,tr("Description?"),
                             tr("Please add\na waypoint\ndescription"),
                             QMessageBox::Close );
      return false;
    }

  if( wp.origP == QPoint(0,0) )
      {
        int answer = QMessageBox::warning(this,tr("Coordinates?"),
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

/**
 * This method checks, if the passed waypoint name is already to find
 * in the global waypoint list. If yes the user is informed with a
 * message box about this fact.
 * Returns true if yes otherwise false.
 */
bool WpEditDialog::isWaypointNameInList( QString& wpName )
{
  if( _globalMapContents->isInWaypointList( wpName ) )
    {
      // The waypoint name is already in use
      QMessageBox::critical( this,tr("Name Conflict"),
                             tr("Please use another name\nfor your new waypoint"),
                             QMessageBox::Close );
      return true;
    }

  return false;
}

/**
  * This method checks, if the passed waypoint name is multiple to find
  * in the global waypoint list. If yes the user is informed with a
  * message box about this fact.
  * Returns true if yes otherwise false.
  */
bool WpEditDialog::countWaypointNameInList( QString& wpName )
{
  if( _globalMapContents->countNameInWaypointList( wpName ) > 1 )
    {
        // The waypoint name is more than one to find in the list
      QMessageBox::critical( this,tr("Name Conflict"),
                            tr("Please use another name\nfor your new waypoint"),
                            QMessageBox::Close );
      return true;
    }

  return false;
}
