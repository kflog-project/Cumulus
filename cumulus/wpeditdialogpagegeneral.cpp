/***********************************************************************
**
**   wpeditdialogpagegeneral.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by Andr� Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <math.h>

#include <QLabel>
#include <QGridLayout>

#include "wpeditdialogpagegeneral.h"
#include "generalconfig.h"
#include "altitude.h"
#include "basemapelement.h"

WpEditDialogPageGeneral::WpEditDialogPageGeneral(QWidget *parent) :
 QWidget(parent)
{
  setObjectName("WpEditDialogPageGeneral");

  QGridLayout * topLayout = new QGridLayout(this);
  topLayout->setMargin(5);
  topLayout->setColumnMinimumWidth( 2, 50 );

  int row=0;

  GeneralConfig *conf = GeneralConfig::instance();

  QLabel * lblName = new QLabel(tr("Name(9):"), this);
  topLayout->addWidget(lblName, row, 0);
  edtName = new QLineEdit(this);
  edtName->setMaxLength(9); // limit name to 9 characters
  topLayout->addWidget(edtName, row++, 1, 1, 2);

  QLabel * lblDescription = new QLabel(tr("Description:"), this);
  topLayout->addWidget(lblDescription, row, 0);
  edtDescription = new QLineEdit(this);
  edtDescription->setMaxLength(25); // limit name to 25 characters
  topLayout->addWidget(edtDescription, row++, 1, 1, 2);

  topLayout->setRowMinimumHeight(row++, 10);

  QLabel * lblLat = new QLabel(tr("Latitude:"), this);
  topLayout->addWidget(lblLat, row, 0);
  edtLat = new LatEdit(this, conf->getHomeLat());
  topLayout->addWidget(edtLat, row++, 1);

  QLabel * lblLon = new QLabel(tr("Longitude:"), this);
  topLayout->addWidget(lblLon, row, 0);
  edtLong = new LongEdit(this, conf->getHomeLon());
  topLayout->addWidget(edtLong, row++, 1);

  QLabel * lblElev = new QLabel(tr("Elevation:"), this);
  topLayout->addWidget(lblElev, row, 0);
  QBoxLayout * elevLayout = new QHBoxLayout();
  topLayout->addLayout(elevLayout, row++, 1);
  edtElev = new QLineEdit(this);
  elevLayout->addWidget(edtElev);
  QLabel * lblElevUnit = new QLabel(Altitude::getText(-1,true), this);
  elevLayout->addWidget(lblElevUnit);

  topLayout->setRowMinimumHeight(row++, 10);

  QLabel * lblGReg = new QLabel(tr("Type:"), this);
  topLayout->addWidget(lblGReg, row, 0);
  cmbType = new QComboBox(this);
  cmbType->setObjectName("Type");
  cmbType->setEditable(false);
  topLayout->addWidget(cmbType, row++, 1);

  // init comboboxes
  QStringList &tlist = BaseMapElement::getSortedTranslationList();

  for( int i=0; i < tlist.size(); i++ )
    {
      cmbType->addItem( tlist.at(i) );
    }

  QLabel * lblGCall = new QLabel(tr("Importance:"), this);
  topLayout->addWidget(lblGCall, row, 0);
  cmbImportance = new QComboBox(this);
  cmbImportance->setObjectName("Importance");
  cmbImportance->setEditable(false);
  topLayout->addWidget(cmbImportance, row++, 1);
  cmbImportance->addItem(tr("low"));
  cmbImportance->addItem(tr("normal"));
  cmbImportance->addItem(tr("high"));

  topLayout->setRowStretch(row++, 10);
  topLayout->setColumnStretch(2, 10);
}


WpEditDialogPageGeneral::~WpEditDialogPageGeneral()
{}


/** called if data needs to be loaded */
void WpEditDialogPageGeneral::slot_load(wayPoint * wp)
{
  if ( wp )
    { //we don't need to load if the waypoint is not there
      edtName->setText(wp->name);
      edtDescription->setText(wp->description);
      edtLat->setKFLogDegree(wp->origP.lat());
      edtLong->setKFLogDegree(wp->origP.lon());
      edtElev->setText(Altitude::getText((wp->elevation),false,-1));
      setWaypointType(wp->type);
      cmbImportance->setCurrentIndex(wp->importance);
    }
}


/** called if data needs to be saved */
void WpEditDialogPageGeneral::slot_save(wayPoint * wp)
{
  if ( wp )
    {
      wp->name = edtName->text().trimmed();
      // qDebug("WpEditDialogPageGeneral::slot_save %x %s",wp,(const char *)wp->name );
      wp->description = edtDescription->text().trimmed();

      if( edtLat->isInputChanged() )
        wp->origP.setLat(edtLat->KFLogDegree());

      if( edtLong->isInputChanged() )
        wp->origP.setLon(edtLong->KFLogDegree());

      wp->elevation=static_cast<int> (rint(Altitude::convertToMeters(edtElev->text().toDouble())));
      wp->type=getWaypointType();
      wp->importance=( enum wayPoint::Importance ) cmbImportance->currentIndex();
    }
}


/** return internal type of waypoint */
int WpEditDialogPageGeneral::getWaypointType()
{
  int type = cmbType->currentIndex();

  if (type != -1)
    {
      const QString &text = BaseMapElement::getSortedTranslationList().at(type);
      type = BaseMapElement::text2Item( text );
    }

  return type;
}


/** set waypoint type in combo box
translate internal id to index */
void WpEditDialogPageGeneral::setWaypointType(int type)
{
  if (type != -1)
    {
      type = BaseMapElement::getSortedTranslationList().indexOf(BaseMapElement::item2Text(type));
    }
  else
    {
      type = BaseMapElement::getSortedTranslationList().indexOf(BaseMapElement::item2Text(0));
    }

  cmbType->setCurrentIndex(type);
}

