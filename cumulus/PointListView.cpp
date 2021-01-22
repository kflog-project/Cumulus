/***********************************************************************
**
**   PointListView.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014-2017 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <PointListView.h>
#include <QtGui>
#include <QShortcut>
#include <QMessageBox>

#include "generalconfig.h"
#include "calculator.h"
#include "gpsnmea.h"

PointListView::PointListView( ListWidgetParent* lwParent, QWidget *parent ) :
  QWidget( parent ),
  m_listw( lwParent ),
  m_homeChanged( false )
{
  setObjectName("PointListView");

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addWidget(m_listw, 10);

  QBoxLayout *buttonrow=new QHBoxLayout;
  topLayout->addLayout(buttonrow);

  QPushButton *cmdClose = new QPushButton(tr("Close"), this);

  buttonrow->addWidget(cmdClose);

  QPushButton *cmdInfo = new QPushButton(tr("Info"), this);
  buttonrow->addWidget(cmdInfo);

  m_cmdHome = new QPushButton(tr("Home"), this);
  buttonrow->addWidget(m_cmdHome);

  m_cmdSelect = new QPushButton(tr("Select"), this);
  buttonrow->addWidget(m_cmdSelect);

  connect(m_cmdSelect, SIGNAL(clicked()),
          this, SLOT(slot_Select()));
  connect(cmdInfo, SIGNAL(clicked()),
          this, SLOT(slot_Info()));
  connect(cmdClose, SIGNAL(clicked()),
          this, SLOT(slot_Close()));
  connect(m_cmdHome, SIGNAL(clicked()),
          this, SLOT(slot_Home()));
  connect(m_listw, SIGNAL(wpSelectionChanged()),
          this, SLOT(slot_Selected()));
  connect(this, SIGNAL(done()),
          m_listw, SLOT(slot_Done()));

#ifndef ANDROID

  // activate keyboard shortcut Return as select
  QShortcut* scSelect = new QShortcut( this );
  scSelect->setKey( Qt::Key_Return );
  connect( scSelect, SIGNAL(activated()), this, SLOT( slot_Select() ));

#endif

}

PointListView::~PointListView()
{
}

void PointListView::showEvent( QShowEvent *event )
{
  // Show the home button only if we are not to fast in move to avoid
  // usage during flight. The redefinition of the home position will trigger
  // a reload of all loaded lists.
  if( calculator->moving() )
    {
      m_cmdHome->setVisible(false);
    }
  else
    {
      m_cmdHome->setVisible(true);
    }

  // Reset home changed
  m_homeChanged = false;

  QWidget::showEvent( event );
}

/** This signal is called to indicate that a selection has been made. */
void PointListView::slot_Select()
{
  Waypoint *_wp = m_listw->getCurrentWaypoint();

  if ( _wp )
    {
      emit newWaypoint( _wp, true );
      slot_Close();
    }
}

/** This slot is called if the info button has been clicked */
void PointListView::slot_Info()
{
  // qDebug("PointListView::slot_Info");
  Waypoint *_wp = m_listw->getCurrentWaypoint();

  if( _wp )
    {
      emit info( _wp );
    }
}

/** @ee This slot is called if the listview is closed without selecting */
void PointListView::slot_Close()
{
  // That will switch back to the map view. This must be done first to ensure
  // that the home position change does work.
  emit done();

  // Check, if we have not a valid GPS fix. In this case we do move the map
  // to the new home position.
  if( m_homeChanged == true && GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      emit gotoHomePosition();
      m_homeChanged = false;
    }
}

void PointListView::slot_Selected()
{
  m_cmdSelect->setEnabled(true);
  Waypoint* _wp = m_listw->getCurrentWaypoint();

  if (_wp)
    {
      if(_wp->equals(calculator->getTargetWp()))
        {
          m_cmdSelect->setEnabled(false);
        }
    }
}

/** Called to set a new home position. The change of the home position can trigger
 *  a reload of many map data, if option projection follows home is active.
 */
void PointListView::slot_Home()
{
  Waypoint* _wp = m_listw->getCurrentWaypoint();

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
      m_homeChanged = true;
    }
}
