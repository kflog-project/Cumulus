/***********************************************************************
**
**   airfieldlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "airfieldlistview.h"

#include <QMessageBox>
#include <QShortcut>

#include "generalconfig.h"
#include "calculator.h"
#include "airfield.h"

AirfieldListView::AirfieldListView( QVector<enum MapContents::MapContentsListID> &itemList,
                                    QMainWindow *parent ) : QWidget(parent)
{
  setObjectName("AirfieldListView");

  par=parent;
  QBoxLayout *topLayout = new QVBoxLayout( this );

  listw = new AirfieldListWidget( itemList, this );

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
}

AirfieldListView::~AirfieldListView()
{
}

void AirfieldListView::showEvent(QShowEvent *)
{
  // Show the home button only if we are not to fast in move to avoid
  // wrong usage. The redefinition of the home position can trigger
  // a reload of the airfield list.
  if( calculator->moving() )
    {
      cmdHome->setVisible(false);
    }
  else
    {
      cmdHome->setVisible(true);
    }
}

/** This signal is called to indicate that a selection has been made. */
void AirfieldListView::slot_Select()
{
  wayPoint *_wp = listw->getSelectedWaypoint();

  if ( _wp )
    {
      emit newWaypoint( _wp, true );
      emit done();
    }
}

/** This slot is called if the info button has been clicked */
void AirfieldListView::slot_Info()
{
  // qDebug("AirfieldListView::slot_Info");

  wayPoint *_wp = listw->getSelectedWaypoint();

  if ( _wp )
    {
      emit info( _wp );
    }
}

/** @ee This slot is called if the listview is closed without selecting */
void AirfieldListView::slot_Close ()
{
  // qDebug("AirfieldListView::slot_Close");
  emit done();
}

void AirfieldListView::slot_Selected()
{
  cmdSelect->setEnabled(true);
  wayPoint* _wp = listw->getSelectedWaypoint();

  if (_wp)
    {
      if(_wp->equals(calculator->getselectedWp()))
        {
          cmdSelect->setEnabled(false);
        }
    }
}

void AirfieldListView::slot_Home()
{
  wayPoint* _wp = listw->getSelectedWaypoint();

  if ( _wp == 0 )
    {
      return;
    }

  GeneralConfig *conf = GeneralConfig::instance();

  if( conf->getHomeLat() == _wp->origP.lat() &&
      conf->getHomeLon() == _wp->origP.lon() )
    {
      // no new coordinates, ignore request
      return;
    }

  int answer= QMessageBox::question(this,
                                   tr("Set home site"),
                                   tr("Use point<br>%1<br>as your home site?").arg(_wp->name),
                                   QMessageBox::No, QMessageBox::Yes );
  if( answer == QMessageBox::Yes )
    {
      // Save new data as home position
      conf->setHomeLat(_wp->origP.lat());
      conf->setHomeLon(_wp->origP.lon());
      conf->save();

      emit newHomePosition( _wp->origP );
    }
}
