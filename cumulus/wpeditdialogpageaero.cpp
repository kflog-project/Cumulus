/***********************************************************************
**
**   wpeditdialogpageaero.cpp
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
#include <QBoxLayout>

#include "wpeditdialogpageaero.h"
#include "distance.h"
#include "airport.h"

WPEditDialogPageAero::WPEditDialogPageAero(QWidget *parent, const char *name ):
    QWidget(parent,name, Qt::WStyle_StaysOnTop)
{
  QGridLayout * topLayout = new QGridLayout(this, 10,2,5);
  int row=0;

  QLabel * lblIcao = new QLabel(tr("ICAO:"), this);
  topLayout->addWidget(lblIcao,row,0);
  edtICAO = new QLineEdit(this);
  topLayout->addWidget(edtICAO,row++,1);

  QLabel * lblFrequency = new QLabel(tr("Frequency:"), this);
  topLayout->addWidget(lblFrequency,row,0);
  edtFrequency = new QLineEdit(this);
  topLayout->addWidget(edtFrequency,row++,1);

  topLayout->addRowSpacing(row++,10);

  QLabel * lblRun = new QLabel(tr("Runway:"), this);
  topLayout->addWidget(lblRun,row,0);
  edtRunway = new DegreeSpinBox(this, "Runway");
  topLayout->addWidget(edtRunway,row++,1);

  QLabel * lblLen = new QLabel(tr("Length:"), this);
  topLayout->addWidget(lblLen,row,0);
  QBoxLayout * elevLayout = new QHBoxLayout();
  topLayout->addLayout(elevLayout,row++,1);
  edtLength = new QLineEdit(this);
  elevLayout->addWidget(edtLength);
  QLabel * lblLenUnit = new QLabel(Distance::getText(-1,true), this);
  elevLayout->addWidget(lblLenUnit);

  topLayout->addRowSpacing(row++,10);

  chkLandable = new QCheckBox(tr("Landable"), this);
  topLayout->addWidget(chkLandable, row++ , 1);

  QLabel * lblSurface = new QLabel(tr("Surface:"), this);
  topLayout->addWidget(lblSurface,row,0);
  cmbSurface = new QComboBox(false, this, "Surface");
  topLayout->addWidget(cmbSurface,row++,1);

  // init combobox
  QStringList &tlist = Airport::getSortedTranslationList();

  for( int i=0; i < tlist.size(); i++ )
    {
      cmbSurface->insertItem( tlist.at(i) );
    }

  cmbSurface->setCurrentItem(cmbSurface->count()-1);
  topLayout->setRowStretch(row++,10);
}


WPEditDialogPageAero::~WPEditDialogPageAero()
{}


/** Called if the page needs to load data from the waypoint */
void WPEditDialogPageAero::slot_load(wayPoint * wp)
{
  if ( wp )
    {
      edtICAO->setText(wp->icao);
      edtFrequency->setText(QString::number(wp->frequency));
      edtRunway->setValue(wp->runway/10);
      edtLength->setText(Distance::getText(wp->length,false,-1));
      setSurface(wp->surface);
      chkLandable->setChecked(wp->isLandable);
    }
}


/** Called if the data needs to be saved. */
void WPEditDialogPageAero::slot_save(wayPoint * wp)
{
  if ( wp )
    {
      wp->icao=edtICAO->text();
      wp->frequency=edtFrequency->text().toDouble();
      wp->runway=edtRunway->value()*10;
      wp->length=int(Distance::convertToMeters(edtLength->text().toDouble()));
      wp->surface=getSurface();
      wp->isLandable=chkLandable->isChecked();
    }
}


/** return internal type of surface */
int WPEditDialogPageAero::getSurface()
{
  int s = cmbSurface->currentItem();

  if (s != -1)
    {
      const QString &text = Airport::getSortedTranslationList().at(s);
      s = Airport::text2Item( text );
    }

  if (s == 0)
    {
      s = -1;
    }

  return s;
}


/** set surface type in combo box
translate internal id to index */
void WPEditDialogPageAero::setSurface(int s)
{
  if (s != -1)
    {
      s = Airport::getSortedTranslationList().indexOf(Airport::item2Text(s));

    }
  else
    {
      s = Airport::getSortedTranslationList().indexOf(Airport::item2Text(0));
    }

  cmbSurface->setCurrentItem(s);
}
