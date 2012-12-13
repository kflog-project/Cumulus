/***********************************************************************
**
**   wpeditdialogpagegeneral.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2012 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QtGui>

#include "altitude.h"
#include "basemapelement.h"
#include "flickcharm.h"
#include "generalconfig.h"
#include "wgspoint.h"
#include "wpeditdialogpagegeneral.h"

WpEditDialogPageGeneral::WpEditDialogPageGeneral(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("WpEditDialogPageGeneral");
  loadedLat = 0;
  loadedLon = 0;

  QGridLayout * topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  // The description maximum length is 25 characters. We calculate
  // the length of a M string of 27 characters. That is the maximum
  // width of the QLineEdit widgets.
  QFontMetrics fm( font() );
  int charWidth = fm.width(QChar('M'));

  int row=0;

  GeneralConfig *conf = GeneralConfig::instance();

  QLabel * lblName = new QLabel(tr("Name(8):"), this);
  topLayout->addWidget(lblName, row, 0);
  edtName = new QLineEdit(this);
  edtName->setMaxLength(8); // limit name to 8 characters
#ifndef ANDROID
  edtName->setMinimumWidth( 27*charWidth );
  edtName->setMaximumWidth( 27*charWidth );
#else
  edtName->setMinimumWidth( 22*charWidth );
  edtName->setMaximumWidth( 22*charWidth );
#endif
  topLayout->addWidget(edtName, row++, 1, 1, 3);

#ifndef ANDROID
  connect( edtName, SIGNAL(textEdited( const QString& )),
           this, SLOT(slot_textEditedName( const QString& )) );
#else
  // Android makes trouble, if word detection is enabled. Therefore the
  // entered string is modified, when the finish signal is emitted.
  connect( edtName, SIGNAL(editingFinished()),
           this, SLOT(slot_textEditedNameFinished()) );
#endif

  QLabel * lblDescription = new QLabel(tr("Description:"), this);
  topLayout->addWidget(lblDescription, row, 0);
  edtDescription = new QLineEdit(this);
  edtDescription->setMaxLength(25); // limit name to 25 characters
#ifndef ANDROID
  edtDescription->setMinimumWidth( 27*charWidth );
  edtDescription->setMaximumWidth( 27*charWidth );
#else
  edtDescription->setMinimumWidth( 22*charWidth );
  edtDescription->setMaximumWidth( 22*charWidth );
#endif
  topLayout->addWidget(edtDescription, row++, 1, 1, 3);

  QLabel * lblCountry = new QLabel(tr("Country(2):"), this);
  topLayout->addWidget(lblCountry, row, 0);
  edtCountry = new QLineEdit(this);
  edtCountry->setMaxLength(2); // limit name to 2 characters
  edtCountry->setMinimumWidth( 3*charWidth );
  edtCountry->setMaximumWidth( 3*charWidth );
  edtCountry->setText( conf->getHomeCountryCode() );
  QRegExp rx("[A-Za-z]{2}");
  edtCountry->setValidator( new QRegExpValidator(rx, this) );

#ifndef ANDROID
  connect( edtCountry, SIGNAL(textEdited( const QString& )),
           this, SLOT(slot_textEditedCountry( const QString& )) );
#else
  // Android makes trouble, if word detection is enabled. Therefore the
  // entered string is modified, when the finish signal is emitted.
  connect( edtCountry, SIGNAL(editingFinished()),
           this, SLOT(slot_textEditedCountryFinished()) );
#endif

  topLayout->addWidget(edtCountry, row++, 1, 1, 2);

  topLayout->setRowMinimumHeight(row++, 10);

  QLabel * lblLat = new QLabel(tr("Latitude:"), this);
  topLayout->addWidget(lblLat, row, 0);
  edtLat = new LatEdit(this, conf->getHomeLat());
  topLayout->addWidget(edtLat, row++, 1, 1, 2);

  QLabel * lblLon = new QLabel(tr("Longitude:"), this);
  topLayout->addWidget(lblLon, row, 0);
  edtLong = new LongEdit(this, conf->getHomeLon());
  topLayout->addWidget(edtLong, row++, 1, 1, 2);

  QLabel * lblElev = new QLabel(tr("Elevation:"), this);
  topLayout->addWidget(lblElev, row, 0);
  QBoxLayout * elevLayout = new QHBoxLayout();
  topLayout->addLayout(elevLayout, row++, 1);

  edtElev = new QLineEdit(this);
  edtElev->setValidator( new QRegExpValidator(QRegExp("[1-9][0-9]*"), this) );

  elevLayout->addWidget(edtElev);
  QLabel * lblElevUnit = new QLabel(Altitude::getUnitText(), this);
  elevLayout->addWidget(lblElevUnit);

  topLayout->setRowMinimumHeight(row++, 10);

  QLabel * lblGReg = new QLabel(tr("Type:"), this);
  topLayout->addWidget(lblGReg, row, 0);
  cmbType = new QComboBox(this);
  cmbType->setObjectName("Type");
  cmbType->setEditable(false);

#ifdef QSCROLLER
  QScroller::grabGesture(cmbType->view(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(cmbType->view());
#endif

  topLayout->addWidget(cmbType, row++, 1);

  // init combo boxes
  QStringList &tlist = BaseMapElement::getSortedTranslationList();

  for( int i=0; i < tlist.size(); i++ )
    {
      cmbType->addItem( tlist.at(i) );
    }

  // The default waypoint type is landmark.
  setWaypointType(BaseMapElement::Landmark );

  QLabel * lblGCall = new QLabel(tr("Importance:"), this);
  topLayout->addWidget(lblGCall, row, 0);
  cmbImportance = new QComboBox(this);
  cmbImportance->setObjectName("Importance");
  cmbImportance->setEditable(false);
  topLayout->addWidget(cmbImportance, row++, 1);
  cmbImportance->addItem(tr("low"));
  cmbImportance->addItem(tr("normal"));
  cmbImportance->addItem(tr("high"));
  cmbImportance->addItem(tr("top"));

  topLayout->setRowStretch(row++, 10);
  topLayout->setColumnStretch(2, 10);
  topLayout->setColumnStretch(4, 10);
}

WpEditDialogPageGeneral::~WpEditDialogPageGeneral()
{
}

/** called if data needs to be loaded */
void WpEditDialogPageGeneral::slot_load(Waypoint *wp)
{
  if( wp )
    { //we don't need to load if the waypoint is not there
      edtName->setText( wp->name.left( 8 ).toUpper() );
      edtDescription->setText( wp->description );
      edtCountry->setText( wp->country );
      edtLat->setKFLogDegree( wp->origP.lat() );
      edtLong->setKFLogDegree( wp->origP.lon() );
      edtElev->setText( Altitude::getText( (wp->elevation), false, -1 ) );
      setWaypointType( wp->type );
      cmbImportance->setCurrentIndex( wp->priority );

      // save loaded values
      loadedLat = wp->origP.lat();
      loadedLon = wp->origP.lon();
    }
}

/** called if data needs to be saved */
void WpEditDialogPageGeneral::slot_save(Waypoint *wp)
{
  if ( wp )
    {
      wp->name = edtName->text().trimmed().toUpper();

      wp->description = edtDescription->text().trimmed();

      wp->country = edtCountry->text().toUpper();

      if( edtLat->isInputChanged() )
        {
          wp->origP.setLat( edtLat->KFLogDegree() );
        }
      else
        {
          wp->origP.setLat( loadedLat );
        }

      if( edtLong->isInputChanged() )
        {
          wp->origP.setLon( edtLong->KFLogDegree() );
        }
      else
        {
          wp->origP.setLon( loadedLon );
        }

      wp->elevation=static_cast<int> (rint(Altitude::convertToMeters(edtElev->text().toDouble())));
      wp->type=getWaypointType();
      wp->priority=( enum Waypoint::Priority ) cmbImportance->currentIndex();
    }
}

void WpEditDialogPageGeneral::slot_textEditedName( const QString& text )
{
  // Change edited text to upper cases
  edtName->setText( text.toUpper() );
}

void WpEditDialogPageGeneral::slot_textEditedCountry( const QString& text )
{
  // Change edited text to upper cases
  edtCountry->setText( text.toUpper() );
}

/**
 * Called to make all text to upper cases.
 */
void WpEditDialogPageGeneral::slot_textEditedNameFinished()
{
  edtName->setText( edtName->text().trimmed().toUpper() );
}

/**
 * Called to make all text to upper cases.
 */
void WpEditDialogPageGeneral::slot_textEditedCountryFinished()
{
  edtCountry->setText( edtCountry->text().trimmed().toUpper() );
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


/** set waypoint type in combo box translate internal id to index */
void WpEditDialogPageGeneral::setWaypointType(int type)
{
  int index = -1;

  if (type != -1)
    {
      index = BaseMapElement::getSortedTranslationList().indexOf(BaseMapElement::item2Text(type));
    }
  else
    {
      index = BaseMapElement::getSortedTranslationList().indexOf(BaseMapElement::item2Text(0));
    }

  cmbType->setCurrentIndex(index);
}

