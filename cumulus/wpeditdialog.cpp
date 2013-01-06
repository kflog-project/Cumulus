/***********************************************************************
 **
 **   WpEditDialog.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by André Somers
 **                   2008-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "wpeditdialog.h"
#include "wpeditdialogpagegeneral.h"
#include "wpeditdialogpageaero.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "generalconfig.h"
#include "mainwindow.h"
#include "layout.h"

#ifdef FLICK_CHARM
#include "flickcharm.h"
#endif

extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;
extern MainWindow  *_globalMainWindow;

WpEditDialog::WpEditDialog(QWidget *parent, Waypoint *wp ) :
  QWidget( parent )
{
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( _globalMainWindow )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  if( wp == 0 )
    {
      setWindowTitle(tr("New Waypoint"));
      m_oldName = "";
    }
  else
    {
      setWindowTitle(tr("Edit Waypoint"));
      m_oldName = wp->name;
    }

  m_wp = wp;

  QTabWidget* tabWidget = new QTabWidget(this);

  // Put all pages into a scroll area. Needed by Maemo Qt, if virtual
  // keyboard is pop up.
  QScrollArea* pgArea = new QScrollArea( tabWidget );
  pgArea->setWidgetResizable( true );
  pgArea->setFrameStyle( QFrame::NoFrame );
  WpEditDialogPageGeneral *pageG = new WpEditDialogPageGeneral(this);
  pgArea->setWidget( pageG );
  tabWidget->addTab( pgArea, tr("General") );
#ifdef QSCROLLER
  QScroller::grabGesture(pgArea, QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(pgArea);
#endif

  QScrollArea* paArea = new QScrollArea( tabWidget );
  paArea->setWidgetResizable( true );
  paArea->setFrameStyle( QFrame::NoFrame );
  WpEditDialogPageAero *pageA = new WpEditDialogPageAero(this);
  paArea->setWidget( pageA );
  tabWidget->addTab( paArea, tr("Aero") );
#ifdef QSCROLLER
  QScroller::grabGesture(paArea, QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  flickCharm = new FlickCharm(this);
  flickCharm->activateOn(paArea);
#endif

  QScrollArea* pcArea = new QScrollArea( tabWidget );
  pcArea->setWidgetResizable( true );
  pcArea->setFrameStyle( QFrame::NoFrame );
  m_comment = new QTextEdit(this);
  m_comment->setWordWrapMode(QTextOption::WordWrap);
  pcArea->setWidget( m_comment );
  tabWidget->addTab( pcArea, tr("Comment") );
#ifdef QSCROLLER
  QScroller::grabGesture(pcArea, QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  flickCharm = new FlickCharm(this);
  flickCharm->activateOn(pcArea);
#endif

  connect(this, SIGNAL(load(Waypoint *)),
          pageG, SLOT(slot_load(Waypoint *)));

  connect(this, SIGNAL(load(Waypoint *)),
          pageA, SLOT(slot_load(Waypoint *)));

  connect(this, SIGNAL(save(Waypoint *)),
          pageG, SLOT(slot_save(Waypoint *)));

  connect(this, SIGNAL(save(Waypoint *)),
          pageA, SLOT(slot_save(Waypoint *)));

  // Add ok and cancel buttons
  QPushButton *cancel = new QPushButton;
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton;
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(close()));

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
}

/** This method is called to load the waypoint data in the tab widgets.
 *  The load will be done only for existing wayppoint objects. */
void WpEditDialog::loadWaypointData()
{
  if( m_wp )
    {
      emit load(m_wp);
      m_comment->setText(m_wp->comment);
    }
}

/** Called if OK button is pressed */
void WpEditDialog::accept()
{
  // get waypoint data from the tab widgets and save them in a new object
  Waypoint newWp;
  emit save( &newWp );
  newWp.projP = _globalMapMatrix->wgsToMap( newWp.origP );
  newWp.comment = m_comment->toPlainText();
  newWp.wpListMember = m_wp ? m_wp->wpListMember : false;

  // Make some mandatory consistency checks
  if( checkWaypointData( newWp ) == false )
    {
      // reject saving due to missing data items
      return;
    }

  if( m_wp == 0 )
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
      if( m_oldName != newWp.name && isWaypointNameInList( newWp.name ) )
        {
          // The waypoint name of the saved object was modified and the new name
          // is already in use in the global list. To avoid multiple entries with
          //the same name, the accept is rejected.
          return;
        }

      // Update old waypoint object
      *m_wp = newWp;

      // The modified waypoint is posted to the subscribers
      emit wpListChanged( *m_wp );
    }

  QWidget::close();
}

/**
 * This method checks, if all mandatory waypoint data have been defined.
 * Returns true on ok otherwise false.
 */
bool WpEditDialog::checkWaypointData( Waypoint& wp )
{
  if( wp.name.isEmpty() )
    {
      QMessageBox mb( QMessageBox::Critical,
                      tr( "Name?" ),
                      tr( "Please add\na waypoint\nname" ),
                      QMessageBox::Ok,
                      this );

    #ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

    #endif

      mb.exec();
      return false;
    }

  if( wp.description.isEmpty() )
    {
      QMessageBox mb( QMessageBox::Critical,
                      tr( "Description?" ),
                      tr( "Please add\na waypoint\ndescription" ),
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      mb.exec();
      return false;
    }

  if( wp.origP == QPoint(0,0) )
      {
        QMessageBox mb( QMessageBox::Warning,
                        tr( "Coordinates?" ),
                        tr( "Waypoint coordinates not set, continue?" ),
                        QMessageBox::Yes | QMessageBox::No,
                        this );

        mb.setDefaultButton( QMessageBox::No );

  #ifdef ANDROID

        mb.show();
        QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                         height()/2 - mb.height()/2 ));
        mb.move( pos );

  #endif

        if( mb.exec() == QMessageBox::Yes )
          {
            // yes was chosen, ignore warning
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
      QMessageBox mb( QMessageBox::Critical,
                      tr( "Name Conflict" ),
                      tr( "Please use another name\nfor your new waypoint" ),
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      mb.exec();
      return true;
    }

  return false;
}
