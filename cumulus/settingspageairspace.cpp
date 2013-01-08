/***********************************************************************
 **
 **   settingspageairspace.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by Eggert Ehmke
 **                   2009-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#include <QtGui>

#include "airspace.h"
#include "basemapelement.h"
#include "distance.h"
#include "generalconfig.h"
#include "mainwindow.h"
#include "map.h"
#include "mapdefaults.h"
#include "mapcontents.h"
#include "settingspageairspace.h"
#include "settingspageairspaceloading.h"
#include "settingspageairspacewarnings.h"
#include "varspinbox.h"

#ifdef FLICK_CHARM
#include "flickcharm.h"
#endif

#ifdef INTERNET
#include "airspacedownloaddialog.h"
#endif

#ifdef USE_NUM_PAD
#include "numberEditor.h"
#include "settingspageairspacefillingnumpad.h"
#include "settingspageairspacewarningsnumpad.h"
#else
#include "settingspageairspacefilling.h"
#endif

extern MapContents *_globalMapContents;

SettingsPageAirspace::SettingsPageAirspace(QWidget *parent) :
  QWidget( parent ),
  m_autoSip( true )
{
  setObjectName("SettingsPageAirspace");
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // save current altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.

  altUnit = Altitude::getUnit();
  QString unit = (altUnit == Altitude::meters) ? " m" : " ft";

  QGridLayout *topLayout = new QGridLayout(this);
  topLayout->setMargin(3);

  int row=0;

  drawOptions = new QTableWidget(8, 6, this);
  // drawOptions->setShowGrid( false );

#ifdef QSCROLLER
  QScroller::grabGesture(drawOptions, QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(drawOptions);
#endif

  connect( drawOptions, SIGNAL(cellClicked ( int, int )),
           SLOT(slot_toggleCheckBox( int, int )));

  // hide vertical headers
  QHeaderView *vHeader = drawOptions->verticalHeader();
  vHeader->setVisible(false);

  QTableWidgetItem *item = new QTableWidgetItem( tr("Airspace") );
  drawOptions->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr("Border") );
  drawOptions->setHorizontalHeaderItem( 1, item );

  item = new QTableWidgetItem( tr("Area") );
  drawOptions->setHorizontalHeaderItem( 2, item );

  item = new QTableWidgetItem( tr("Airspace") );
  drawOptions->setHorizontalHeaderItem( 3, item );

  item = new QTableWidgetItem( tr("Border") );
  drawOptions->setHorizontalHeaderItem( 4, item );

  item = new QTableWidgetItem( tr("Area") );
  drawOptions->setHorizontalHeaderItem( 5, item );

  topLayout->addWidget(drawOptions, row, 0, 1, 3);
  topLayout->setRowStretch( row, 5 );
  row++;

  QHBoxLayout *hbox = new QHBoxLayout;

  m_enableBorderDrawing = new QCheckBox(tr("Ignore AS"), this);
  m_enableBorderDrawing->setChecked(false);
  hbox->addWidget( m_enableBorderDrawing );
  connect( m_enableBorderDrawing, SIGNAL(toggled(bool)),
           SLOT(slot_enabledToggled(bool)));

#ifdef USE_NUM_PAD
  m_borderDrawingValue = new NumberEditor;
  m_borderDrawingValue->setDecimalVisible( false );
  m_borderDrawingValue->setPmVisible( false );
  m_borderDrawingValue->setMaxLength(3);
  m_borderDrawingValue->setPrefix( ">FL ");
  m_borderDrawingValue->setMaximum( 500 );
  m_borderDrawingValue->setTip("50...500");
  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "([5-9][0-9]|[1-4][0-9][0-9]|500)" ), this );
  m_borderDrawingValue->setValidator( eValidator );
  hbox->addWidget( m_borderDrawingValue );
#else
  m_borderDrawingValue = new QSpinBox;
  m_borderDrawingValue->setPrefix(">FL ");
  m_borderDrawingValue->setRange( 50, 500 );
  m_borderDrawingValue->setSingleStep( 1 );
  VarSpinBox* hspin = new VarSpinBox( m_borderDrawingValue );
  hbox->addWidget( hspin );
#endif

  hbox->addStretch( 10 );

  cmdColorDefaults = new QPushButton(tr("Color Defaults"), this);
  hbox->addWidget( cmdColorDefaults );
  connect( cmdColorDefaults, SIGNAL(clicked()), this, SLOT(slot_setColorDefaults()) );

  topLayout->addLayout( hbox, row, 0, 1, 3 );
  row++;

#if 0
  hbox = new QHBoxLayout;
  hbox->addWidget( new QLabel(tr("Line Width:"), this ));

  spinAsLineWidth = new QSpinBox;
  spinAsLineWidth->setRange( 1, 5 );
  spinAsLineWidth->setSingleStep( 1 );
  hspin = new VarSpinBox( spinAsLineWidth );
  hbox->addWidget( hspin );
  hbox->addStretch( 10 );
  hbox->setEnabled( false );

  topLayout->addLayout( hbox, row, 0, 1, 2 );
  row++;
#endif

  topLayout->setRowMinimumHeight( row++, 10 );

  // All buttons are put into a hbox.
  hbox = new QHBoxLayout;

#ifdef INTERNET
#ifndef ANDROID

  cmdInstall = new QPushButton(tr("Download"), this);
  hbox->addWidget(cmdInstall);
  connect (cmdInstall, SIGNAL(clicked()), this, SLOT(slot_installAirspace()));

  hbox->addSpacing( 10 );

#endif
#endif

  cmdLoading = new QPushButton(tr("Load"), this);
  hbox->addWidget(cmdLoading);
  connect (cmdLoading, SIGNAL(clicked()), this, SLOT(slot_openLoadDialog()));

  hbox->addSpacing( 10 );

  cmdWarning = new QPushButton(tr("Warnings"), this);
  hbox->addWidget(cmdWarning);
  connect (cmdWarning, SIGNAL(clicked()), this, SLOT(slot_openWarningDialog()));

  hbox->addSpacing( 10 );

  cmdFilling = new QPushButton(tr("Filling"), this);
  hbox->addWidget(cmdFilling);
  connect (cmdFilling, SIGNAL(clicked()), this, SLOT(slot_openFillDialog()));

  topLayout->addLayout( hbox, row, 0, 1, 3 );

  row = 0;
  int col = 0;

  drawAirspaceA = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirA) );
  drawAirspaceA->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceA );

  drawAirspaceB = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirB) );
  drawAirspaceB->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceB );

  drawAirspaceC = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirC) );
  drawAirspaceC->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceC );

  drawAirspaceD = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirD) );
  drawAirspaceD->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceD );

  drawAirspaceE = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirE) );
  drawAirspaceE->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceE );

  drawAirspaceF = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirF) );
  drawAirspaceF->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceF );

  drawWaveWindow = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::WaveWindow) );
  drawWaveWindow->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawWaveWindow );

  drawGliderSector = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::GliderSector) );
  drawGliderSector->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawGliderSector );

  // next column is one
  row = 0;
  col = 1;

  borderColorAirspaceA = new QWidget();
  borderColorAirspaceA->setAutoFillBackground(true);
  borderColorAirspaceA->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceA );

  borderColorAirspaceB = new QWidget();
  borderColorAirspaceB->setAutoFillBackground(true);
  borderColorAirspaceB->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceB );

  borderColorAirspaceC = new QWidget();
  borderColorAirspaceC->setAutoFillBackground(true);
  borderColorAirspaceC->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceC );

  borderColorAirspaceD = new QWidget();
  borderColorAirspaceD->setAutoFillBackground(true);
  borderColorAirspaceD->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceD );

  borderColorAirspaceE = new QWidget();
  borderColorAirspaceE->setAutoFillBackground(true);
  borderColorAirspaceE->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceE );

  borderColorAirspaceF = new QWidget();
  borderColorAirspaceF->setAutoFillBackground(true);
  borderColorAirspaceF->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceF );

  borderColorWaveWindow = new QWidget();
  borderColorWaveWindow->setAutoFillBackground(true);
  borderColorWaveWindow->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorWaveWindow );

  borderColorGliderSector = new QWidget();
  borderColorGliderSector->setAutoFillBackground(true);
  borderColorGliderSector->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorGliderSector );

  // next column is two
  row = 0;
  col = 2;

  fillColorAirspaceA = new QWidget();
  fillColorAirspaceA->setAutoFillBackground(true);
  fillColorAirspaceA->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorAirspaceA );

  fillColorAirspaceB = new QWidget();
  fillColorAirspaceB->setAutoFillBackground(true);
  fillColorAirspaceB->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorAirspaceB );

  fillColorAirspaceC = new QWidget();
  fillColorAirspaceC->setAutoFillBackground(true);
  fillColorAirspaceC->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorAirspaceC );

  fillColorAirspaceD = new QWidget();
  fillColorAirspaceD->setAutoFillBackground(true);
  fillColorAirspaceD->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorAirspaceD );

  fillColorAirspaceE = new QWidget();
  fillColorAirspaceE->setAutoFillBackground(true);
  fillColorAirspaceE->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorAirspaceE );

  fillColorAirspaceF = new QWidget();
  fillColorAirspaceF->setAutoFillBackground(true);
  fillColorAirspaceF->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorAirspaceF );

  fillColorWaveWindow = new QWidget();
  fillColorWaveWindow->setAutoFillBackground(true);
  fillColorWaveWindow->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorWaveWindow );

  fillColorGliderSector = new QWidget();
  fillColorGliderSector->setAutoFillBackground(true);
  fillColorGliderSector->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorGliderSector );

  // next column is three
  row = 0;
  col = 3;

  drawControlC = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::ControlC) );
  drawControlC->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawControlC );

  drawControlD = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::ControlD) );
  drawControlD->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawControlD );

  drawRestricted = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Restricted) );
  drawRestricted->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawRestricted );

  drawDanger = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Danger) );
  drawDanger->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawDanger );

  drawProhibited = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Prohibited) );
  drawProhibited->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawProhibited );

  drawTMZ = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Tmz) );
  drawTMZ->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawTMZ );

  drawLowFlight = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::LowFlight) );
  drawLowFlight->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawLowFlight );

  // Set a dummy into the unused cells
  QTableWidgetItem *liDummy = new QTableWidgetItem;
  liDummy->setFlags( Qt::NoItemFlags );
  drawOptions->setItem( row++, col, liDummy );

  // next column is four
  row = 0;
  col = 4;

  borderColorControlC = new QWidget();
  borderColorControlC->setAutoFillBackground(true);
  borderColorControlC->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorControlC );

  borderColorControlD = new QWidget();
  borderColorControlD->setAutoFillBackground(true);
  borderColorControlD->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorControlD );

  borderColorRestricted = new QWidget();
  borderColorRestricted->setAutoFillBackground(true);
  borderColorRestricted->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorRestricted );

  borderColorDanger = new QWidget();
  borderColorDanger->setAutoFillBackground(true);
  borderColorDanger->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorDanger );

  borderColorProhibited = new QWidget();
  borderColorProhibited->setAutoFillBackground(true);
  borderColorProhibited->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorProhibited );

  borderColorTMZ = new QWidget();
  borderColorTMZ->setAutoFillBackground(true);
  borderColorTMZ->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorTMZ );

  borderColorLowFlight = new QWidget();
  borderColorLowFlight->setAutoFillBackground(true);
  borderColorLowFlight->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorLowFlight );

  // Set a dummy into the unused cells
  liDummy = new QTableWidgetItem;
  liDummy->setFlags( Qt::NoItemFlags );
  drawOptions->setItem( row++, col, liDummy );

  // next column is five
  row = 0;
  col = 5;

  fillColorControlC = new QWidget();
  fillColorControlC->setAutoFillBackground(true);
  fillColorControlC->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorControlC );

  fillColorControlD = new QWidget();
  fillColorControlD->setAutoFillBackground(true);
  fillColorControlD->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorControlD );

  fillColorRestricted = new QWidget();
  fillColorRestricted->setAutoFillBackground(true);
  fillColorRestricted->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorRestricted );

  fillColorDanger = new QWidget();
  fillColorDanger->setAutoFillBackground(true);
  fillColorDanger->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorDanger );

  fillColorProhibited = new QWidget();
  fillColorProhibited->setAutoFillBackground(true);
  fillColorProhibited->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorProhibited );

  fillColorTMZ = new QWidget();
  fillColorTMZ->setAutoFillBackground(true);
  fillColorTMZ->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorTMZ );

  fillColorLowFlight = new QWidget();
  fillColorLowFlight->setAutoFillBackground(true);
  fillColorLowFlight->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorLowFlight );

  // Set a dummy into the unused cells
  liDummy = new QTableWidgetItem;
  liDummy->setFlags( Qt::NoItemFlags );
  drawOptions->setItem( row++, col, liDummy );

  drawOptions->resizeColumnsToContents();
}

SettingsPageAirspace::~SettingsPageAirspace()
{
}

void SettingsPageAirspace::showEvent(QShowEvent *)
{
  // align all columns to contents before showing
  drawOptions->resizeColumnsToContents();
  drawOptions->setFocus();

  // Switch off automatic software input panel popup
  m_autoSip = qApp->autoSipEnabled();
  qApp->setAutoSipEnabled( true );
}

void SettingsPageAirspace::hideEvent( QHideEvent *)
{
  qApp->setAutoSipEnabled( m_autoSip );
}

void SettingsPageAirspace::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();
  bool enabled = conf->getAirspaceDrawBorderEnabled();

  m_enableBorderDrawing->setChecked(enabled);
  slot_enabledToggled(enabled);
  m_borderDrawingValue->setValue( conf->getAirspaceDrawingBorder() );

  // spinAsLineWidth->setValue( spinAsLineWidthValue );

  drawAirspaceA->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirA) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceB->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirB) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceC->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirC) ? Qt::Checked : Qt::Unchecked );
  drawControlC->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::ControlC) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceD->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirD) ? Qt::Checked : Qt::Unchecked );
  drawControlD->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::ControlD) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceE->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirE) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceF->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirF) ? Qt::Checked : Qt::Unchecked );
  drawRestricted->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::Restricted) ? Qt::Checked : Qt::Unchecked );
  drawDanger->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::Danger) ? Qt::Checked : Qt::Unchecked );
  drawProhibited->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::Prohibited) ? Qt::Checked : Qt::Unchecked );
  drawTMZ->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::Tmz) ? Qt::Checked : Qt::Unchecked );
  drawLowFlight->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::LowFlight) ? Qt::Checked : Qt::Unchecked );
  drawWaveWindow->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::WaveWindow) ? Qt::Checked : Qt::Unchecked );
  drawGliderSector->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::GliderSector) ? Qt::Checked : Qt::Unchecked );

  // load border colors
  borderColorAirspaceA->setPalette( QPalette(conf->getBorderColorAirspaceA()));
  borderColorAirspaceB->setPalette( QPalette(conf->getBorderColorAirspaceB()));
  borderColorAirspaceC->setPalette( QPalette(conf->getBorderColorAirspaceC()));
  borderColorAirspaceD->setPalette( QPalette(conf->getBorderColorAirspaceD()));
  borderColorAirspaceE->setPalette( QPalette(conf->getBorderColorAirspaceE()));
  borderColorAirspaceF->setPalette( QPalette(conf->getBorderColorAirspaceF()));
  borderColorWaveWindow->setPalette( QPalette(conf->getBorderColorWaveWindow()));
  borderColorControlC->setPalette( QPalette(conf->getBorderColorControlC()));
  borderColorControlD->setPalette( QPalette(conf->getBorderColorControlD()));
  borderColorRestricted->setPalette( QPalette(conf->getBorderColorRestricted()));
  borderColorDanger->setPalette( QPalette(conf->getBorderColorDanger()));
  borderColorProhibited->setPalette( QPalette(conf->getBorderColorProhibited()));
  borderColorTMZ->setPalette( QPalette(conf->getBorderColorTMZ()));
  borderColorLowFlight->setPalette( QPalette(conf->getBorderColorLowFlight()));
  borderColorGliderSector->setPalette( QPalette(conf->getBorderColorGliderSector()));

  // load fill colors
  fillColorAirspaceA->setPalette( QPalette(conf->getFillColorAirspaceA()));
  fillColorAirspaceB->setPalette( QPalette(conf->getFillColorAirspaceB()));
  fillColorAirspaceC->setPalette( QPalette(conf->getFillColorAirspaceC()));
  fillColorAirspaceD->setPalette( QPalette(conf->getFillColorAirspaceD()));
  fillColorAirspaceE->setPalette( QPalette(conf->getFillColorAirspaceE()));
  fillColorAirspaceF->setPalette( QPalette(conf->getFillColorAirspaceF()));
  fillColorWaveWindow->setPalette( QPalette(conf->getFillColorWaveWindow()));
  fillColorControlC->setPalette( QPalette(conf->getFillColorControlC()));
  fillColorControlD->setPalette( QPalette(conf->getFillColorControlD()));
  fillColorRestricted->setPalette( QPalette(conf->getFillColorRestricted()));
  fillColorDanger->setPalette( QPalette(conf->getFillColorDanger()));
  fillColorProhibited->setPalette( QPalette(conf->getFillColorProhibited()));
  fillColorTMZ->setPalette( QPalette(conf->getFillColorTMZ()));
  fillColorLowFlight->setPalette( QPalette(conf->getFillColorLowFlight()));
  fillColorGliderSector->setPalette( QPalette(conf->getFillColorGliderSector()));
}

void SettingsPageAirspace::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();
  AirspaceWarningDistance awd;

  conf->setAirspaceDrawingBorder(m_borderDrawingValue->value());
  conf->setAirspaceDrawBorderEnabled(m_enableBorderDrawing->checkState() == Qt::Checked ? true : false);

  // conf->setAirspaceLineWidth( spinAsLineWidth->value() );

  conf->setItemDrawingEnabled(BaseMapElement::AirA,drawAirspaceA->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirB,drawAirspaceB->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirC,drawAirspaceC->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::ControlC,drawControlC->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirD,drawAirspaceD->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::ControlD,drawControlD->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirE,drawAirspaceE->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirF,drawAirspaceF->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Restricted,drawRestricted->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Danger,drawDanger->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Prohibited,drawProhibited->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Tmz,drawTMZ->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::LowFlight,drawLowFlight->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::WaveWindow,drawWaveWindow->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::GliderSector,drawGliderSector->checkState() == Qt::Checked ? true : false);

  // save border colors
  conf->setBorderColorAirspaceA(borderColorAirspaceA->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceB(borderColorAirspaceB->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceC(borderColorAirspaceC->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceD(borderColorAirspaceD->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceE(borderColorAirspaceE->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceF(borderColorAirspaceF->palette().color(QPalette::Window));
  conf->setBorderColorWaveWindow(borderColorWaveWindow->palette().color(QPalette::Window));
  conf->setBorderColorControlC(borderColorControlC->palette().color(QPalette::Window));
  conf->setBorderColorControlD(borderColorControlD->palette().color(QPalette::Window));
  conf->setBorderColorRestricted(borderColorRestricted->palette().color(QPalette::Window));
  conf->setBorderColorDanger(borderColorDanger->palette().color(QPalette::Window));
  conf->setBorderColorProhibited(borderColorProhibited->palette().color(QPalette::Window));
  conf->setBorderColorTMZ(borderColorTMZ->palette().color(QPalette::Window));
  conf->setBorderColorLowFlight(borderColorLowFlight->palette().color(QPalette::Window));
  conf->setBorderColorGliderSector(borderColorGliderSector->palette().color(QPalette::Window));

  // save fill colors
  conf->setFillColorAirspaceA(fillColorAirspaceA->palette().color(QPalette::Window));
  conf->setFillColorAirspaceB(fillColorAirspaceB->palette().color(QPalette::Window));
  conf->setFillColorAirspaceC(fillColorAirspaceC->palette().color(QPalette::Window));
  conf->setFillColorAirspaceD(fillColorAirspaceD->palette().color(QPalette::Window));
  conf->setFillColorAirspaceE(fillColorAirspaceE->palette().color(QPalette::Window));
  conf->setFillColorAirspaceF(fillColorAirspaceF->palette().color(QPalette::Window));
  conf->setFillColorWaveWindow(fillColorWaveWindow->palette().color(QPalette::Window));
  conf->setFillColorControlC(fillColorControlC->palette().color(QPalette::Window));
  conf->setFillColorControlD(fillColorControlD->palette().color(QPalette::Window));
  conf->setFillColorRestricted(fillColorRestricted->palette().color(QPalette::Window));
  conf->setFillColorDanger(fillColorDanger->palette().color(QPalette::Window));
  conf->setFillColorProhibited(fillColorProhibited->palette().color(QPalette::Window));
  conf->setFillColorTMZ(fillColorTMZ->palette().color(QPalette::Window));
  conf->setFillColorLowFlight(fillColorLowFlight->palette().color(QPalette::Window));
  conf->setFillColorGliderSector(fillColorGliderSector->palette().color(QPalette::Window));

  emit airspaceColorsUpdated();

  // @AP: initiate a redraw of airspaces on the map due to color modifications.
  //      Not the best solution but it is working ;-)
  Map::getInstance()->scheduleRedraw(Map::airspaces);
}

/**
  * Called to set all colors to their default value.
  */
void SettingsPageAirspace::slot_setColorDefaults()
{
  borderColorAirspaceA->setPalette( QPalette(QColor(AIRA_COLOR)) );
  borderColorAirspaceB->setPalette( QPalette(QColor(AIRB_COLOR)) );
  borderColorAirspaceC->setPalette( QPalette(QColor(AIRC_COLOR)) );
  borderColorAirspaceD->setPalette( QPalette(QColor(AIRD_COLOR)) );
  borderColorAirspaceE->setPalette( QPalette(QColor(AIRE_COLOR)) );
  borderColorAirspaceF->setPalette( QPalette(QColor(AIRF_COLOR)) );
  borderColorWaveWindow->setPalette( QPalette(QColor(WAVE_WINDOW_COLOR)) );
  borderColorControlC->setPalette( QPalette(QColor(CTRC_COLOR)) );
  borderColorControlD->setPalette( QPalette(QColor(CTRD_COLOR)) );
  borderColorRestricted->setPalette( QPalette(QColor(RESTRICTED_COLOR)) );
  borderColorDanger->setPalette( QPalette(QColor(DANGER_COLOR)) );
  borderColorProhibited->setPalette( QPalette(QColor(DANGER_COLOR)) );
  borderColorTMZ->setPalette( QPalette(QColor(TMZ_COLOR)) );
  borderColorLowFlight->setPalette( QPalette(QColor(LOWF_COLOR)) );
  borderColorGliderSector->setPalette( QPalette(QColor(GLIDER_SECTOR_COLOR)) );

  fillColorAirspaceA->setPalette( QPalette(QColor(AIRA_BRUSH_COLOR)) );
  fillColorAirspaceB->setPalette( QPalette(QColor(AIRB_BRUSH_COLOR)) );
  fillColorAirspaceC->setPalette( QPalette(QColor(AIRC_BRUSH_COLOR)) );
  fillColorAirspaceD->setPalette( QPalette(QColor(AIRD_BRUSH_COLOR)) );
  fillColorAirspaceE->setPalette( QPalette(QColor(AIRE_BRUSH_COLOR)) );
  fillColorAirspaceF->setPalette( QPalette(QColor(AIRF_BRUSH_COLOR)) );
  fillColorWaveWindow->setPalette( QPalette(QColor(WAVE_WINDOW_BRUSH_COLOR)) );
  fillColorControlC->setPalette( QPalette(QColor(CTRC_BRUSH_COLOR)) );
  fillColorControlD->setPalette( QPalette(QColor(CTRD_BRUSH_COLOR)) );
  fillColorRestricted->setPalette( QPalette(QColor(RESTRICTED_BRUSH_COLOR)) );
  fillColorDanger->setPalette( QPalette(QColor(DANGER_BRUSH_COLOR)) );
  fillColorProhibited->setPalette( QPalette(QColor(DANGER_BRUSH_COLOR)) );
  fillColorTMZ->setPalette( QPalette(QColor(TMZ_BRUSH_COLOR)) );
  fillColorLowFlight->setPalette( QPalette(QColor(LOWF_BRUSH_COLOR)) );
  fillColorGliderSector->setPalette( QPalette(QColor(GLIDER_SECTOR_BRUSH_COLOR)) );
}

/**
 * Called to toggle the check box of the clicked table cell.
 */
void SettingsPageAirspace::slot_toggleCheckBox( int row, int column )
{
  // qDebug("row=%d, column=%d", row, column);

  if( row == 7 && column >= 3 )
    {
      // Ignore dummy cells
      return;
    }

  if( column % 3 ) // only every third columns are QTableWidgetItems
    {
      QString title = "none";
      QTableWidgetItem *asItem;
      QTableWidgetItem *hItem;

      // compose a title for the color dialog
      if( column == 1 )
        {
          asItem = drawOptions->item( row, 0 );
          hItem  = drawOptions->horizontalHeaderItem( 1 );
          title = hItem->text() + " " + asItem->text();
        }
      else if( column == 2 )
        {
          asItem = drawOptions->item( row, 0 );
          hItem  = drawOptions->horizontalHeaderItem( 2 );
          title = hItem->text() + " " + asItem->text();
        }
      else if( column == 4 )
        {
          asItem = drawOptions->item( row, 3 );
          hItem  = drawOptions->horizontalHeaderItem( 4 );
          title = hItem->text() + " " + asItem->text();
        }
      else if( column == 5 )
        {
          asItem = drawOptions->item( row, 3 );
          hItem  = drawOptions->horizontalHeaderItem( 5 );
          title = hItem->text() + " " + asItem->text();
        }

      QWidget *cw = drawOptions->cellWidget( row, column );
      QPalette palette = cw->palette();
      QColor color = palette.color(QPalette::Window);

      // Open color chooser dialog
      QColor newColor = QColorDialog::getColor( color, this, title );

      if( newColor.isValid() && color != newColor )
        {
          // set new color in widget
          cw->setPalette( QPalette(newColor));
        }
    }
  else
    {
      QTableWidgetItem *item = drawOptions->item( row, column );
      item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
    }
}

#ifdef INTERNET

/**
 * Called to request the download an airspace file.
 */
void SettingsPageAirspace::slot_installAirspace()
{
  AirspaceDownloadDialog *dlg = new AirspaceDownloadDialog( this );

  connect( dlg, SIGNAL(downloadAirspace( QString& )),
           this, SLOT(slot_startDownload( QString& )));

  dlg->setVisible( true );
}

/**
 * Called to start a download of an airspace file.
 */
void SettingsPageAirspace::slot_startDownload( QString &url )
{
  emit downloadAirspace( url );
}

#endif

/* Called to open the airspace warning dialog. */
void SettingsPageAirspace::slot_openFillDialog()
{
#ifdef USE_NUM_PAD
  SettingsPageAirspaceFillingNumPad* dlg = new SettingsPageAirspaceFillingNumPad(this);
  dlg->setVisible( true );
#else
  SettingsPageAirspaceFilling* dlg = new SettingsPageAirspaceFilling(this);
  dlg->setVisible( true );

#ifdef ANDROID

  QSize ms = dlg->minimumSizeHint();

  ms += QSize(10, 10);

  // A dialog is not centered over the parent and not limited in
  // its size under Android. Therefore this must be done by our self.
  dlg->setGeometry( (MainWindow::mainWindow()->width() - ms.width()) / 2,
                    (MainWindow::mainWindow()->height() - ms.height()) / 2,
                     ms.width(), ms.height() );

#endif
#endif
}

/* Called to open the airspace warning dialog. */
void SettingsPageAirspace::slot_openWarningDialog()
{
#ifdef USE_NUM_PAD
  SettingsPageAirspaceWarningsNumPad* dlg = new SettingsPageAirspaceWarningsNumPad(this);
  dlg->setVisible( true );
#else
  SettingsPageAirspaceWarnings* dlg = new SettingsPageAirspaceWarnings(this);
  dlg->setVisible( true );

#ifdef ANDROID

  QSize ms = dlg->minimumSizeHint();

  ms += QSize(10, 10);

  // A dialog is not centered over the parent and not limited in
  // its size under Android. Therefore this must be done by our self.
  dlg->setGeometry( (MainWindow::mainWindow()->width() - ms.width()) / 2,
                    (MainWindow::mainWindow()->height() - ms.height()) / 2,
                     ms.width(), ms.height() );

#endif
#endif
}

/* Called to open the airspace loading selection dialog. */
void SettingsPageAirspace::slot_openLoadDialog()
{
  SettingsPageAirspaceLoading* dlg = new SettingsPageAirspaceLoading(this);

  connect( dlg, SIGNAL(airspaceFileListChanged()),
           _globalMapContents, SLOT(slotReloadAirspaceData()) );

  dlg->setVisible( true );
}

/* Called to ask is confirmation on the close is needed. */
void SettingsPageAirspace::slot_query_close(bool& warn, QStringList& warnings)
{
  GeneralConfig * conf = GeneralConfig::instance();
  bool changed = false;

  // changed |= spinAsLineWidthValue != spinAsLineWidth->value();
  QString where;

  changed |= conf->getAirspaceDrawingBorder() != m_borderDrawingValue->value();
  changed |= conf->getAirspaceDrawBorderEnabled() != (m_enableBorderDrawing->checkState() == Qt::Checked ? true : false);

  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirA) != (drawAirspaceA->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirB) != (drawAirspaceB->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirC) != (drawAirspaceC->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::ControlC) != (drawControlC->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirD) != (drawAirspaceD->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::ControlD) != (drawControlD->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirE) != (drawAirspaceE->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirF) != (drawAirspaceF->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Restricted) != (drawRestricted->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Danger) != (drawDanger->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Prohibited) != (drawProhibited->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Tmz) != (drawTMZ->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::LowFlight) != (drawLowFlight->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::WaveWindow) != (drawWaveWindow->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::GliderSector) != (drawGliderSector->checkState() == Qt::Checked ? true : false);

  if( changed )
    {
      where += tr("drawing");
    }

  changed = false;

  changed |= conf->getBorderColorAirspaceA() != borderColorAirspaceA->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceB() != borderColorAirspaceB->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceC() != borderColorAirspaceC->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceD() != borderColorAirspaceD->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceE() != borderColorAirspaceE->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceF() != borderColorAirspaceF->palette().color(QPalette::Window);
  changed |= conf->getBorderColorWaveWindow() != borderColorWaveWindow->palette().color(QPalette::Window);
  changed |= conf->getBorderColorControlC() != borderColorControlC->palette().color(QPalette::Window);
  changed |= conf->getBorderColorControlD() != borderColorControlD->palette().color(QPalette::Window);
  changed |= conf->getBorderColorRestricted() != borderColorRestricted->palette().color(QPalette::Window);
  changed |= conf->getBorderColorDanger() != borderColorDanger->palette().color(QPalette::Window);
  changed |= conf->getBorderColorProhibited() != borderColorProhibited->palette().color(QPalette::Window);
  changed |= conf->getBorderColorTMZ() != borderColorTMZ->palette().color(QPalette::Window);
  changed |= conf->getBorderColorLowFlight() != borderColorLowFlight->palette().color(QPalette::Window);
  changed |= conf->getBorderColorGliderSector() != borderColorGliderSector->palette().color(QPalette::Window);

  if( changed )
    {
      if( ! where.isEmpty() )
        {
          where += ", ";
        }

      where += tr("border");
    }

  changed = false;

  changed |= conf->getFillColorAirspaceA() != fillColorAirspaceA->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceB() != fillColorAirspaceB->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceC() != fillColorAirspaceC->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceD() != fillColorAirspaceD->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceE() != fillColorAirspaceE->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceF() != fillColorAirspaceF->palette().color(QPalette::Window);
  changed |= conf->getFillColorWaveWindow() != fillColorWaveWindow->palette().color(QPalette::Window);
  changed |= conf->getFillColorControlC() != fillColorControlC->palette().color(QPalette::Window);
  changed |= conf->getFillColorControlD() != fillColorControlD->palette().color(QPalette::Window);
  changed |= conf->getFillColorRestricted() != fillColorRestricted->palette().color(QPalette::Window);
  changed |= conf->getFillColorDanger() != fillColorDanger->palette().color(QPalette::Window);
  changed |= conf->getFillColorProhibited() != fillColorProhibited->palette().color(QPalette::Window);
  changed |= conf->getFillColorTMZ() != fillColorTMZ->palette().color(QPalette::Window);
  changed |= conf->getFillColorLowFlight() != fillColorLowFlight->palette().color(QPalette::Window);
  changed |= conf->getFillColorGliderSector() != fillColorGliderSector->palette().color(QPalette::Window);

  if( changed )
    {
      if( ! where.isEmpty() )
        {
          where += ", ";
        }

      where += tr("fill");
    }

  if( ! where.isEmpty() )
  {
    warn=true;
    warnings.append(tr("The Airspace settings") + " (" + where + ")" );
  }
}

void SettingsPageAirspace::slot_enabledToggled(bool enabled)
{
  m_borderDrawingValue->setEnabled(enabled);
}
