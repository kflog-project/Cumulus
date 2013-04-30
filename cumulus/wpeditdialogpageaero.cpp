/***********************************************************************
**
**   wpeditdialogpageaero.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers,
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "altitude.h"
#include "airfield.h"
#include "doubleNumberEditor.h"
#include "numberEditor.h"
#include "wpeditdialogpageaero.h"

WpEditDialogPageAero::WpEditDialogPageAero(QWidget *parent) :
  QWidget(parent, Qt::WindowStaysOnTopHint)
{
  setObjectName("WpEditDialogPageAero");

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  topLayout->setMargin(10);

  QGridLayout *qgl = new QGridLayout;
  qgl->setMargin(0);
  topLayout->addLayout( qgl );

  Qt::InputMethodHints imh;

  edtFrequency = new DoubleNumberEditor( this );
  edtFrequency->setDecimalVisible( true );
  edtFrequency->setPmVisible( false );
  edtFrequency->setMaxLength(7);
  edtFrequency->setSuffix( tr(" MHz") );
  edtFrequency->setDecimals( 3 );
  edtFrequency->setAlignment( Qt::AlignLeft );
  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,3}\\.[0-9]{1,3})" ), this );
  edtFrequency->setValidator( eValidator );
  edtFrequency->setText("0.0");

  QFormLayout *qfl = new QFormLayout;
  qfl->addRow(tr("Frequency:"), edtFrequency);
  qgl->addLayout(qfl, 0, 0);

  edtICAO = new QLineEdit;
  imh = (edtICAO->inputMethodHints() | Qt::ImhNoPredictiveText);
  edtICAO->setInputMethodHints(imh);
  edtICAO->setMaxLength(4); // limit name to 4 characters

  qfl = new QFormLayout;
  qfl->addRow(tr("ICAO:"), edtICAO);
  qgl->addLayout(qfl, 0, 2);

  qgl->setColumnStretch(1, 10);
  qgl->setRowMinimumHeight(1, 20);

  //----------------------------------------------------------------------------
  QHBoxLayout* hboxEnabRwy = new QHBoxLayout;

  chkRwy1Enable = new QCheckBox( tr("Runway 1") );
  hboxEnabRwy->addWidget( chkRwy1Enable );

  chkRwy2Enable = new QCheckBox( tr("Runway 2") );
  hboxEnabRwy->addWidget( chkRwy2Enable );

  topLayout->addLayout( hboxEnabRwy );

  //----------------------------------------------------------------------------
  gboxRunway1 = new QGroupBox( tr("Runway 1"), this );
  gboxRunway1->setEnabled(false);
  topLayout->addWidget( gboxRunway1 );

  QHBoxLayout* grpLayout1 = new QHBoxLayout;
  gboxRunway1->setLayout( grpLayout1 );

  QHBoxLayout* hbox = new QHBoxLayout;

  chkRwy1Both = new QCheckBox( tr("bidirectional") );
  hbox->addWidget( chkRwy1Both );

  qgl = new QGridLayout;
  qgl->setMargin(0);
  grpLayout1->addLayout(qgl);

  edtRwy1Heading = createRunwayHeadingEditor();
  slot_rwy1HeadingEdited( "0" );

  qfl = new QFormLayout;
  qfl->addRow(tr("Heading:"), edtRwy1Heading);

  qgl->addLayout( qfl, 0, 0 );

  edtRwy1Length = createRunwayLengthEditor();
  qfl = new QFormLayout;
  qfl->addRow(tr("Length:"), edtRwy1Length);
  qgl->addLayout( qfl, 0, 1 );

  cmbRwy1Surface = new QComboBox;
  cmbRwy1Surface->setEditable(false);
  cmbRwy1Surface->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  cmbRwy1Surface->view()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

#ifdef QSCROLLER
  QScroller::grabGesture( cmbRwy1Surface->view()->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( cmbRwy1Surface->view()->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  qfl = new QFormLayout;
  qfl->addRow(tr("Surface:"), cmbRwy1Surface);
  qgl->addLayout( qfl, 0, 2 );

  chkRwy1Both = new QCheckBox( tr("both") );
  chkRwy1Both->setChecked( true );
  qgl->addWidget( chkRwy1Both, 1, 0, 1, 2 );

  chkRwy1Usable = new QCheckBox( tr("usable") );
  chkRwy1Usable->setChecked( true );
  qgl->addWidget( chkRwy1Usable, 1, 2, 1, 2 );

  //----------------------------------------------------------------------------
  gboxRunway2 = new QGroupBox( tr("Runway 2"), this );
  gboxRunway2->setEnabled(false);
  topLayout->addWidget( gboxRunway2 );

  QHBoxLayout* grpLayout2 = new QHBoxLayout;
  gboxRunway2->setLayout( grpLayout2 );

  hbox = new QHBoxLayout;

  chkRwy2Both = new QCheckBox( tr("bidirectional") );
  hbox->addWidget( chkRwy2Both );

  qgl = new QGridLayout;
  qgl->setMargin(0);
  grpLayout2->addLayout(qgl);

  edtRwy2Heading = createRunwayHeadingEditor();
  slot_rwy2HeadingEdited( "0" );
  qfl = new QFormLayout;
  qfl->addRow(tr("Heading:"), edtRwy2Heading);
  qgl->addLayout( qfl, 0, 0 );

  edtRwy2Length = createRunwayLengthEditor();

  qfl = new QFormLayout;
  qfl->addRow(tr("Length:"), edtRwy2Length);
  qgl->addLayout( qfl, 0, 1 );

  cmbRwy2Surface = new QComboBox;
  cmbRwy2Surface->setEditable(false);
  cmbRwy2Surface->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  cmbRwy2Surface->view()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

#ifdef QSCROLLER
  QScroller::grabGesture( cmbRwy2Surface->view()->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( cmbRwy2Surface->view()->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  qfl = new QFormLayout;
  qfl->addRow(tr("Surface:"), cmbRwy2Surface);
  qgl->addLayout( qfl, 0, 2 );

  chkRwy2Both = new QCheckBox( tr("both") );
  chkRwy2Both->setChecked( true );
  qgl->addWidget( chkRwy2Both, 1, 0, 1, 2 );

  chkRwy2Usable = new QCheckBox( tr("usable") );
  chkRwy2Usable->setChecked( true );
  qgl->addWidget( chkRwy2Usable, 1, 2, 1, 2 );

  topLayout->addStretch( 10 );

  //----------------------------------------------------------------------------

  // initialize surface combo boxes
  QStringList &tlist = Runway::getSortedTranslationList();

  for( int i=0; i < tlist.size(); i++ )
    {
      cmbRwy1Surface->addItem( tlist.at(i) );
      cmbRwy2Surface->addItem( tlist.at(i) );
    }

  cmbRwy1Surface->setCurrentIndex(cmbRwy1Surface->count()-1);
  cmbRwy2Surface->setCurrentIndex(cmbRwy2Surface->count()-1);

  connect( chkRwy1Enable, SIGNAL(stateChanged(int)),
           SLOT(slot_stateChangedRwy1Enable(int)) );

  connect( chkRwy2Enable, SIGNAL(stateChanged(int)),
           SLOT(slot_stateChangedRwy2Enable(int)) );

  connect( edtRwy1Heading, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_rwy1HeadingEdited(const QString&)) );

  connect( edtRwy2Heading, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_rwy2HeadingEdited(const QString&)) );
}

WpEditDialogPageAero::~WpEditDialogPageAero()
{}

NumberEditor* WpEditDialogPageAero::createRunwayHeadingEditor( QWidget* parent )
{
  NumberEditor* ne = new NumberEditor( parent );
  ne->setDecimalVisible( false );
  ne->setPmVisible( false );
  ne->setMaxLength(2);
  ne->setAlignment( Qt::AlignLeft );
  ne->setSpecialValueText( "--" );
  ne->setValue( 0 );
  ne->setRange( 0, 36 );
  ne->setTitle( tr("Set Runway heading") );
  ne->setTip( tr("00=unknown") + ", 01...36");
  return ne;
}

NumberEditor* WpEditDialogPageAero::createRunwayLengthEditor( QWidget* parent )
{
  NumberEditor* ne = new NumberEditor( parent );
  ne->setDecimalVisible( false );
  ne->setPmVisible( false );
  ne->setMaxLength(6);
  ne->setAlignment( Qt::AlignLeft );

  // Note! We take as runway length unit the altitude unit (m/ft)
  ne->setSuffix( " " + Altitude::getUnitText() );
  ne->setSpecialValueText( "-" );
  ne->setRange( 0, 999999 );
  ne->setValue( 0 );
  ne->setTitle( tr("Set Runway length") );
  ne->setTip( tr("0=unknown") + ", 1-9...");
  return ne;
}

void WpEditDialogPageAero::slot_rwy1HeadingEdited( const QString& value )
{
  QString iv = QString("%1").arg(value, 2, QChar('0') );

  edtRwy1Heading->setText( iv );
}

void WpEditDialogPageAero::slot_rwy2HeadingEdited( const QString& value )
{
  QString iv = QString("%1").arg(value, 2, QChar('0') );

  edtRwy2Heading->setText( iv );
}

void WpEditDialogPageAero::slot_load( Waypoint *wp )
{
  if( ! wp )
    {
      return;
    }

  edtICAO->setText(wp->icao.toUpper());

  QString tmp = QString("%1").arg(wp->frequency, 0, 'f', 3);

  while( tmp.size() < 7 )
    {
      // Add leading zeros, that is not supported by floating point
      // formatting.
      tmp.insert(0, "0");
    }

  edtFrequency->setText(tmp);

  // switch off both, that is the default.
  chkRwy1Enable->setCheckState( Qt::Unchecked );
  chkRwy2Enable->setCheckState( Qt::Unchecked );

  // Set headings to unknown as default
  slot_rwy1HeadingEdited( "0" );
  slot_rwy2HeadingEdited( "0" );

  if( wp->rwyList.size() > 0 )
    {
      for( int i = 0; i < wp->rwyList.size() && i < 2; i++ )
        {
          Runway rwy = wp->rwyList.at(i);

          if( i == 0 )
            {
              chkRwy1Enable->setCheckState( Qt::Checked );

              slot_rwy1HeadingEdited( QString::number( rwy.heading / 256 ) );
              edtRwy1Length->setText( Altitude::getText((rwy.length), false, -1) );
              setSurface( cmbRwy1Surface, rwy.surface );
              chkRwy1Usable->setChecked( rwy.isOpen );

              if( rwy.isBidirectional || ( rwy.heading / 256 != rwy.heading % 256) )
                {
                  chkRwy1Both->setChecked( true );
                }
              else
                {
                  chkRwy1Both->setChecked( false );
                }
            }
          else if( i == 1 )
            {
              chkRwy2Enable->setCheckState( Qt::Checked );

              slot_rwy2HeadingEdited( QString::number( rwy.heading / 256 ) );
              edtRwy2Length->setText( Altitude::getText((rwy.length), false, -1) );
              setSurface( cmbRwy2Surface, rwy.surface );
              chkRwy2Usable->setChecked( rwy.isOpen );

              if( rwy.isBidirectional || ( rwy.heading / 256 != rwy.heading % 256) )
                {
                  chkRwy2Both->setChecked( true );
                }
              else
                {
                  chkRwy2Both->setChecked( false );
                }
            }
        }
    }
}

void WpEditDialogPageAero::slot_save( Waypoint *wp )
{
  if( ! wp )
    {
      return;
    }

  wp->icao = edtICAO->text().trimmed().toUpper();

  bool ok;
  wp->frequency = edtFrequency->text().toFloat(&ok);

  if( ! ok )
    {
      wp->frequency = 0.0;
    }

  wp->rwyList.clear();

  // If the runway heading is not defined, all data are ignored!
  if( chkRwy1Enable->isChecked() && edtRwy1Heading->value() > 0 )
    {
      Runway rwy;

      rwy.length = static_cast<float> (Altitude::convertToMeters(edtRwy1Length->text().toDouble()));
      rwy.surface = getSurface( cmbRwy1Surface );
      rwy.isOpen = chkRwy1Usable->isChecked();
      rwy.isBidirectional = chkRwy1Both->isChecked();

      int hdg1 = edtRwy1Heading->value();
      int hdg2 = hdg1 > 18 ? hdg1 - 18 : hdg1 + 18;

      if( rwy.isBidirectional )
        {
          rwy.heading = (hdg1 * 256) + hdg2;
        }
      else
        {
          rwy.heading = (hdg1 * 256) + hdg1;
        }

      wp->rwyList.append( rwy );
    }

  // If the runway heading is not defined, all data are ignored!
  if( chkRwy2Enable->isChecked() && edtRwy2Heading->value() > 0 )
    {
      Runway rwy;

      rwy.length = static_cast<float> (Altitude::convertToMeters(edtRwy2Length->text().toDouble()));
      rwy.surface = getSurface( cmbRwy2Surface );
      rwy.isOpen = chkRwy2Usable->isChecked();
      rwy.isBidirectional = chkRwy2Both->isChecked();

      int hdg1 = edtRwy2Heading->value();
      int hdg2 = hdg1 > 18 ? hdg1 - 18 : hdg1 + 18;

      if( rwy.isBidirectional )
        {
          rwy.heading = (hdg1 * 256) + hdg2;
        }
      else
        {
          rwy.heading = (hdg1 * 256) + hdg1;
        }

      wp->rwyList.append( rwy );
    }
}

int WpEditDialogPageAero::getSurface(QComboBox* cbox)
{
  if( ! cbox )
    {
      return -1;
    }

  int s = cbox->currentIndex();

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

void WpEditDialogPageAero::setSurface(QComboBox* cbox, int s)
{
  if( ! cbox )
    {
      return;
    }

  if (s != -1)
    {
      s = Runway::getSortedTranslationList().indexOf(Runway::item2Text(s));
    }
  else
    {
      s = Runway::getSortedTranslationList().indexOf(Runway::item2Text(0));
    }

  cbox->setCurrentIndex(s);
}
