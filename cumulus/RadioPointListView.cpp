/***********************************************************************
**
**   RadioPointListView.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include "RadioPointListView.h"

#include <QtGui>
#include <QShortcut>
#include <QMessageBox>

#include "generalconfig.h"
#include "calculator.h"
#include "airfield.h"
#include "gpsnmea.h"

RadioPointListView::RadioPointListView( QWidget *parent ) :
  QWidget(parent),
  homeChanged( false )
{
  setObjectName("RadioPointListView");

  QBoxLayout *topLayout = new QVBoxLayout( this );

  listw = new RadioPointListWidget;
  topLayout->addWidget(listw, 10);

  QBoxLayout *buttonrow=new QHBoxLayout;
  topLayout->addLayout(buttonrow);

  QPushButton *cmdClose = new QPushButton(tr("Close"), this);

  buttonrow->addWidget(cmdClose);

  QPushButton *cmdInfo = new QPushButton(tr("Info"), this);
  buttonrow->addWidget(cmdInfo);

  cmdHome = new QPushButton(tr("Home"), this);
  buttonrow->addWidget(cmdHome);

  cmdSelect = new QPushButton(tr("Select"), this);
  buttonrow->addWidget(cmdSelect);

  connect(cmdSelect, SIGNAL(clicked()),
          this, SLOT(slot_Select()));
  connect(cmdInfo, SIGNAL(clicked()),
          this, SLOT(slot_Info()));
  connect(cmdClose, SIGNAL(clicked()),
          this, SLOT(slot_Close()));
  connect(cmdHome, SIGNAL(clicked()),
          this, SLOT(slot_Home()));
  connect(listw, SIGNAL(wpSelectionChanged()),
          this, SLOT(slot_Selected()));
  connect(this, SIGNAL(done()),
          listw, SLOT(slot_Done()));

  // activate keyboard shortcut Return as select
  QShortcut* scSelect = new QShortcut( this );
  scSelect->setKey( Qt::Key_Return );
  connect( scSelect, SIGNAL(activated()), this, SLOT( slot_Select() ));

#ifdef ANDROID
  // Activate keyboard shortcut Close
  QShortcut* scClose = new QShortcut( this );
  scClose->setKey( Qt::Key_Close );
  connect( scClose, SIGNAL(activated()), this, SLOT( slot_Close() ));
#endif

}

RadioPointListView::~RadioPointListView()
{
}

void RadioPointListView::showEvent(QShowEvent *)
{
  // Show the home button only if we are not to fast in move to avoid
  // usage during flight. The redefinition of the home position will trigger
  // a reload of the airfield list.
  if( calculator->moving() )
    {
      cmdHome->setVisible(false);
    }
  else
    {
      cmdHome->setVisible(true);
    }

  // Reset home changed
  homeChanged = false;
}

/** This signal is called to indicate that a selection has been made. */
void RadioPointListView::slot_Select()
{
  Waypoint *_wp = listw->getCurrentWaypoint();

  if ( _wp )
    {
      emit newWaypoint( _wp, true );
      slot_Close();
    }
}

/** This slot is called if the info button has been clicked */
void RadioPointListView::slot_Info()
{
  // qDebug("RadioPointListView::slot_Info");

  Waypoint *_wp = listw->getCurrentWaypoint();

  if( _wp )
    {
      emit info( _wp );
    }
}

/** @ee This slot is called if the listview is closed without selecting */
void RadioPointListView::slot_Close ()
{
  // That will switch back to the map view. This must be done first to ensure
  // that the home position change does work.
  emit done();

  // Check, if we have not a valid GPS fix. In this case we do move the map
  // to the new home position.
  if( homeChanged == true && GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      emit gotoHomePosition();
      homeChanged = false;
    }
}

void RadioPointListView::slot_Selected()
{
  cmdSelect->setEnabled(true);
  Waypoint* _wp = listw->getCurrentWaypoint();

  if (_wp)
    {
      if(_wp->equals(calculator->getselectedWp()))
        {
          cmdSelect->setEnabled(false);
        }
    }
}

/** Called to set a new home position. The change of the home position can trigger
 *  a reload of many map data, if Welt2000 has radius option set or option projection
 *  follows home is active.
 */
void RadioPointListView::slot_Home()
{
  Waypoint* _wp = listw->getCurrentWaypoint();

  if( _wp == static_cast<Waypoint *> ( 0 ) )
    {
      return;
    }

  GeneralConfig *conf = GeneralConfig::instance();

  if( conf->getHomeCoord() == _wp->wgsPoint )
    {
      // no new coordinates, ignore request
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Set home site" ),
                  tr( "Use point<br><b>%1</b><br>as your home site?").arg(_wp->name) +
                  tr("<br>Change can take<br>a few seconds and more."),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::Yes );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::Yes )
    {
      // save new home position and elevation
      conf->setHomeCountryCode( _wp->country );
      conf->setHomeName( _wp->name );
      conf->setHomeCoord( _wp->wgsPoint );
      conf->setHomeElevation( Distance(_wp->elevation) );

      emit newHomePosition( _wp->wgsPoint );
      homeChanged = true;
    }
}
