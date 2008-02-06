/***********************************************************************
**
**   wpeditdialogpagegeneral.cpp
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

#include <QLabel>
#include <QGridLayout>

#include "wpeditdialogpagegeneral.h"
#include "generalconfig.h"
#include "altitude.h"
#include "basemapelement.h"

WPEditDialogPageGeneral::WPEditDialogPageGeneral(QWidget *parent, const char *name ):
    QWidget(parent,name, Qt::WStyle_StaysOnTop)
{
  QGridLayout * topLayout = new QGridLayout(this, 9,2,5);
  int row=0;

  GeneralConfig *conf = GeneralConfig::instance();

  QLabel * lblName = new QLabel(tr("Name:"), this);
  topLayout->addWidget(lblName,row,0);
  edtName = new QLineEdit(this);
  topLayout->addWidget(edtName,row++,1);

  QLabel * lblDescription = new QLabel(tr("Description:"), this);
  topLayout->addWidget(lblDescription,row,0);
  edtDescription = new QLineEdit(this);
  topLayout->addWidget(edtDescription,row++,1);

  topLayout->addRowSpacing(row++,10);

  QLabel * lblLat = new QLabel(tr("Latitude:"), this);
  topLayout->addWidget(lblLat,row,0);
  edtLat = new LatEdit(this, "Lat", conf->getHomeLat());
  topLayout->addWidget(edtLat,row++,1);

  QLabel * lblLon = new QLabel(tr("Longitude:"), this);
  topLayout->addWidget(lblLon,row,0);
  edtLong = new LongEdit(this, "Long", conf->getHomeLon());
  topLayout->addWidget(edtLong,row++,1);

  QLabel * lblElev = new QLabel(tr("Elevation:"), this);
  topLayout->addWidget(lblElev,row,0);
  QBoxLayout * elevLayout = new QHBoxLayout();
  topLayout->addLayout(elevLayout,row++,1);
  edtElev = new QLineEdit(this);
  elevLayout->addWidget(edtElev);
  QLabel * lblElevUnit = new QLabel(Altitude::getText(-1,true), this);
  elevLayout->addWidget(lblElevUnit);
  topLayout->addRowSpacing(row++,10);

  QLabel * lblGReg = new QLabel(tr("Type:"), this);
  topLayout->addWidget(lblGReg,row,0);
  cmbType = new QComboBox(false, this, "Type");
  topLayout->addWidget(cmbType,row++,1);

  // init comboboxes
  QStringList &tlist = BaseMapElement::getSortedTranslationList();

  for( int i=0; i < tlist.size(); i++ )
    {
      cmbType->insertItem( tlist.at(i) );
    }

  QLabel * lblGCall = new QLabel(tr("Importance:"), this);
  topLayout->addWidget(lblGCall,row,0);
  cmbImportance = new QComboBox(false, this, "Importance");
  topLayout->addWidget(cmbImportance,row++,1);
  cmbImportance->insertItem(tr("low"));
  cmbImportance->insertItem(tr("normal"));
  cmbImportance->insertItem(tr("high"));

  topLayout->setRowStretch(row++,10);
}


WPEditDialogPageGeneral::~WPEditDialogPageGeneral()
{}


/** called if data needs to be loaded */
void WPEditDialogPageGeneral::slot_load(wayPoint * wp)
{
  if (!wp==0)
    { //we don't need to load if the waypoint is not there
      edtName->setText(wp->name);
      edtDescription->setText(wp->description);
      edtLat->setKFLogDegree(wp->origP.lat());
      edtLong->setKFLogDegree(wp->origP.lon());
      edtElev->setText(Altitude::getText((wp->elevation),false,-1));
      setWaypointType(wp->type);
      cmbImportance->setCurrentItem(wp->importance);
    }
}


/** called if data needs to be saved */
void WPEditDialogPageGeneral::slot_save(wayPoint * wp)
{
  if ( wp )
    {
      wp->name=edtName->text();
      // qDebug("WPEditDialogPageGeneral::slot_save %x %s",wp,(const char *)wp->name );
      wp->description=edtDescription->text();
      if( edtLat->isInputChanged() )
        wp->origP.setLat(edtLat->KFLogDegree());
      if( edtLong->isInputChanged() )
        wp->origP.setLon(edtLong->KFLogDegree());
      wp->elevation=int(Altitude::convertToMeters(edtElev->text().toDouble()));
      wp->type=getWaypointType();
      wp->importance=( enum wayPoint::Importance ) cmbImportance->currentItem();
    }
}


/** return internal type of waypoint */
int WPEditDialogPageGeneral::getWaypointType()
{
  int type = cmbType->currentItem();

  if (type != -1)
    {
      const QString &text = BaseMapElement::getSortedTranslationList().at(type);
      type = BaseMapElement::text2Item( text );
    }

  return type;
}


/** set waypoint type in combo box
translate internal id to index */
void WPEditDialogPageGeneral::setWaypointType(int type)
{
  if (type != -1)
    {
      type = BaseMapElement::getSortedTranslationList().indexOf(BaseMapElement::item2Text(type));
    }
  else
    {
      type = BaseMapElement::getSortedTranslationList().indexOf(BaseMapElement::item2Text(0));
    }

  cmbType->setCurrentItem(type);
}

