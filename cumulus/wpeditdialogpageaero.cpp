/***********************************************************************
**
**   wpeditdialogpageaero.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers,
**                   2008-2012 by Axel Pauli
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
#include "airfield.h"
#include "flickcharm.h"
#include "wpeditdialogpageaero.h"

WpEditDialogPageAero::WpEditDialogPageAero(QWidget *parent) :
  QWidget(parent, Qt::WindowStaysOnTopHint)
{
  setObjectName("WpEditDialogPageAero");

  QGridLayout *topLayout = new QGridLayout(this);
  topLayout->setMargin(5);
  int row=0;

  QLabel *lblIcao = new QLabel(tr("ICAO:"),  this);
  topLayout->addWidget(lblIcao, row, 0);
  edtICAO = new QLineEdit(this);
  edtICAO->setMaxLength(4); // limit name to 4 characters
  topLayout->addWidget(edtICAO, row++, 1);

  QLabel *lblFrequency = new QLabel(tr("Frequency:"),  this);
  topLayout->addWidget(lblFrequency, row, 0);
  edtFrequency = new QLineEdit(this);
  edtFrequency->setMaxLength(7); // limit name to 7 characters
  edtFrequency->setInputMask("999.999");
  edtFrequency->setText("000.000");

  topLayout->addWidget(edtFrequency, row++, 1);

  topLayout->setRowMinimumHeight(row++,  10);

  QLabel *lblLen = new QLabel(tr("Length:"),  this);
  topLayout->addWidget(lblLen, row, 0);
  QBoxLayout *elevLayout = new QHBoxLayout();
  topLayout->addLayout(elevLayout, row++, 1);
  edtLength = new QLineEdit(this);
  QRegExp rx("[0-9]*");
  edtLength->setValidator( new QRegExpValidator(rx, this) );

  elevLayout->addWidget(edtLength);

  // Note! We take as runway length unit the altitude unit (m/ft)
  QLabel *lblLenUnit = new QLabel(Altitude::getUnitText(),  this);
  elevLayout->addWidget(lblLenUnit);

  QLabel *lblRun = new QLabel(tr("Runway heading1:"),  this);
  topLayout->addWidget(lblRun, row, 0);
  edtRunway1 = new QComboBox(this);
  edtRunway1->setEditable(false);

#ifdef QSCROLLER
  QScroller::grabGesture(edtRunway1->view(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(edtRunway1->view());
#endif

  topLayout->addWidget(edtRunway1, row++, 1);

  QLabel *lblRun1 = new QLabel(tr("Runway heading2:"),  this);
  topLayout->addWidget(lblRun1, row, 0);
  edtRunway2 = new QComboBox(this);
  edtRunway2->setEditable(false);

#ifdef QSCROLLER
  QScroller::grabGesture(edtRunway2->view(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  flickCharm = new FlickCharm(this);
  flickCharm->activateOn(edtRunway2->view());
#endif

  topLayout->addWidget(edtRunway2, row++, 1);

  QLabel *lblSurface = new QLabel(tr("Surface:"),  this);
  topLayout->addWidget(lblSurface, row, 0);
  cmbSurface = new QComboBox(this);
  cmbSurface->setObjectName("Surface");
  cmbSurface->setEditable(false);
  topLayout->addWidget(cmbSurface, row++, 1);

  // init combo boxes
  edtRunway1->addItem( "--" );
  edtRunway2->addItem( "--" );

  for( int i = 1; i <= 36; i++ )
    {
      QString item;
      item = QString("%1").arg(i, 2, 10, QLatin1Char('0'));

      edtRunway1->addItem( item );
      edtRunway2->addItem( item );
    }

  QStringList &tlist = Runway::getSortedTranslationList();

  for( int i=0; i < tlist.size(); i++ )
    {
      cmbSurface->addItem( tlist.at(i) );
    }

  cmbSurface->setCurrentIndex(cmbSurface->count()-1);

  QLabel *lblLand = new QLabel(tr("Landable:"),  this);
  topLayout->addWidget(lblLand, row, 0);

  // Set spaces as label to make the checkbox better operable.
  chkLandable = new QCheckBox("          ", this);
  topLayout->addWidget(chkLandable,  row++ ,  1);

  topLayout->setRowStretch(row++, 10);
  topLayout->setColumnStretch(2,  10);

}

WpEditDialogPageAero::~WpEditDialogPageAero()
{}

/** Called if the page needs to load data from the waypoint */
void WpEditDialogPageAero::slot_load(Waypoint *wp)
{
  if ( wp )
    {
      edtICAO->setText(wp->icao);
      edtRunway1->setCurrentIndex(wp->runway / 256);
      edtRunway2->setCurrentIndex(wp->runway % 256);
      edtLength->setText(Altitude::getText((wp->length), false, -1));
      setSurface(wp->surface);
      chkLandable->setChecked(wp->isLandable);

      QString tmp = QString("%1").arg(wp->frequency, 0, 'f', 3);

      while( tmp.size() < 7 )
        {
          // Add leading zeros, that is not supported by floating point
          // formatting.
          tmp.insert(0, "0");
        }

      edtFrequency->setText(tmp);
    }
}

/** Called if the data needs to be saved. */
void WpEditDialogPageAero::slot_save(Waypoint *wp)
{
  if (wp)
    {
      wp->icao = edtICAO->text();
      wp->runway = (edtRunway1->currentIndex() * 256) + edtRunway2->currentIndex();
      wp->length = static_cast<float> (Altitude::convertToMeters(edtLength->text().toDouble()));
      wp->surface = getSurface();
      wp->isLandable = chkLandable->isChecked();

      bool ok;
      wp->frequency = edtFrequency->text().toFloat(&ok);

      if( ! ok )
        {
          wp->frequency = 0.0;
        }
    }
}

/** return internal type of surface */
int WpEditDialogPageAero::getSurface()
{
  int s = cmbSurface->currentIndex();

  if (s != -1)
    {
      const QString &text = Runway::getSortedTranslationList().at(s);
      s = Runway::text2Item( text );
    }

  if (s == 0)
    {
      s = -1;
    }

  return s;
}

/** set surface type in combo box
translate internal id to index */
void WpEditDialogPageAero::setSurface(int s)
{
  if (s != -1)
    {
      s = Runway::getSortedTranslationList().indexOf(Runway::item2Text(s));
    }
  else
    {
      s = Runway::getSortedTranslationList().indexOf(Runway::item2Text(0));
    }

  cmbSurface->setCurrentIndex(s);
}
