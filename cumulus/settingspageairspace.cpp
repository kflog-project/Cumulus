/***********************************************************************
 **
 **   settingspageairspace.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by Eggert Ehmke
 **                   2009-2020 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
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

#include "airspace.h"
#include "AirspaceFilters.h"
#include "basemapelement.h"
#include "colordialog.h"
#include "distance.h"
#include "generalconfig.h"
#include "helpbrowser.h"
#include "layout.h"
#include "MainWindow.h"
#include "map.h"
#include "mapcontents.h"
#include "mapdefaults.h"
#include "numberEditor.h"

#include "settingspageairspace.h"
#include "settingspageairspacefillingnumpad.h"
#include "settingspageairspacewarningsnumpad.h"
#include "settingspageairspaceloading.h"

#ifdef INTERNET
#include "airspacedownloaddialog.h"
#endif

extern MapContents *_globalMapContents;

SettingsPageAirspace::SettingsPageAirspace(QWidget *parent) :
  QWidget( parent )
{
  setObjectName("SettingsPageAirspace");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Airspaces") );

  if( parent )
    {
      resize( parent->size() );
    }

  // Layout used by scroll area
  QHBoxLayout *sal = new QHBoxLayout;

  // new widget used as container for the dialog layout.
  QWidget* sw = new QWidget;

  // Scroll area
  QScrollArea* sa = new QScrollArea;
  sa->setWidgetResizable( true );
  sa->setFrameStyle( QFrame::NoFrame );
  sa->setWidget( sw );

#ifdef QSCROLLER
  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // Add scroll area to its own layout
  sal->addWidget( sa );

  QHBoxLayout *contentLayout = new QHBoxLayout(this);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal );

  QGridLayout *topLayout = new QGridLayout(sw);
  topLayout->setMargin(3);

  int row=0;

  drawOptions = new QTableWidget(9, 6, this);
  // drawOptions->setShowGrid( false );

  drawOptions->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  drawOptions->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture( drawOptions->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( drawOptions->viewport(), QtScroller::LeftMouseButtonGesture );
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

  cmdAirspaceFilters = new QPushButton(tr("AS Filters"), this);
  hbox->addWidget( cmdAirspaceFilters );
  hbox->addStretch( 10 );
  connect( cmdAirspaceFilters, SIGNAL(clicked()), this, SLOT(slot_editAsFilters()) );

  cmdColorDefaults = new QPushButton(tr("Color Defaults"), this);
  hbox->addWidget( cmdColorDefaults );
  connect( cmdColorDefaults, SIGNAL(clicked()), this, SLOT(slot_setColorDefaults()) );

  topLayout->addLayout( hbox, row, 0, 1, 3 );
  row++;
  topLayout->setRowMinimumHeight( row++, 10 );

  // All buttons are put into a hbox.
  hbox = new QHBoxLayout;

#ifdef INTERNET

  cmdInstall = new QPushButton(tr("Download"), this);
  hbox->addWidget(cmdInstall);
  connect (cmdInstall, SIGNAL(clicked()), this, SLOT(slot_downloadAirspaces()));

  hbox->addSpacing( 10 );

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

  drawAirspaceG = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirG) );
  drawAirspaceG->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceG );

  drawControl = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Ctr) );
  drawControl->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawControl );

  drawAirspaceFir = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirFir) );
  drawAirspaceFir->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceFir );

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

  borderColorAirspaceG = new QWidget();
  borderColorAirspaceG->setAutoFillBackground(true);
  borderColorAirspaceG->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceG );

  borderColorControl = new QWidget();
  borderColorControl->setAutoFillBackground(true);
  borderColorControl->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorControl );

  borderColorAirspaceFir = new QWidget();
  borderColorAirspaceFir->setAutoFillBackground(true);
  borderColorAirspaceFir->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceFir );

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

  fillColorAirspaceG = new QWidget();
  fillColorAirspaceG->setAutoFillBackground(true);
  fillColorAirspaceG->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorAirspaceG );

  fillColorControl = new QWidget();
  fillColorControl->setAutoFillBackground(true);
  fillColorControl->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorControl );

  fillColorAirspaceFir = new QTableWidgetItem(tr("none"), 9999);
  fillColorAirspaceFir->setFlags( Qt::NoItemFlags );
  fillColorAirspaceFir->setTextAlignment(Qt::AlignCenter);
  drawOptions->setItem( row++, col, fillColorAirspaceFir );

  // next column is three
  row = 0;
  col = 3;

  drawRestricted = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Restricted) );
  drawRestricted->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawRestricted );

  drawDanger = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Danger) );
  drawDanger->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawDanger );

  drawProhibited = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Prohibited) );
  drawProhibited->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawProhibited );

  drawRMZ = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Rmz) );
  drawRMZ->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawRMZ );

  drawTMZ = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Tmz) );
  drawTMZ->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawTMZ );

  drawLowFlight = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::LowFlight) );
  drawLowFlight->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawLowFlight );

  drawWaveWindow = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::WaveWindow) );
  drawWaveWindow->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawWaveWindow );

  drawGliderSector = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::GliderSector) );
  drawGliderSector->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawGliderSector );

  drawAirspaceFlarm = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirFlarm) );
  drawAirspaceFlarm->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceFlarm );

  // next column is four
  row = 0;
  col = 4;

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

  borderColorRMZ = new QWidget();
  borderColorRMZ->setAutoFillBackground(true);
  borderColorRMZ->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorRMZ );

  borderColorTMZ = new QWidget();
  borderColorTMZ->setAutoFillBackground(true);
  borderColorTMZ->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorTMZ );

  borderColorLowFlight = new QWidget();
  borderColorLowFlight->setAutoFillBackground(true);
  borderColorLowFlight->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorLowFlight );

  borderColorWaveWindow = new QWidget();
  borderColorWaveWindow->setAutoFillBackground(true);
  borderColorWaveWindow->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorWaveWindow );

  borderColorGliderSector = new QWidget();
  borderColorGliderSector->setAutoFillBackground(true);
  borderColorGliderSector->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorGliderSector );

  borderColorAirspaceFlarm = new QWidget();
  borderColorAirspaceFlarm->setAutoFillBackground(true);
  borderColorAirspaceFlarm->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceFlarm );

  // next column is five
  row = 0;
  col = 5;

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

  fillColorRMZ = new QWidget();
  fillColorRMZ->setAutoFillBackground(true);
  fillColorRMZ->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorRMZ );

  fillColorTMZ = new QWidget();
  fillColorTMZ->setAutoFillBackground(true);
  fillColorTMZ->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorTMZ );

  fillColorLowFlight = new QWidget();
  fillColorLowFlight->setAutoFillBackground(true);
  fillColorLowFlight->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorLowFlight );

  fillColorWaveWindow = new QWidget();
  fillColorWaveWindow->setAutoFillBackground(true);
  fillColorWaveWindow->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorWaveWindow );

  fillColorGliderSector = new QWidget();
  fillColorGliderSector->setAutoFillBackground(true);
  fillColorGliderSector->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorGliderSector );

  fillColorAirspaceFlarm = new QWidget();
  fillColorAirspaceFlarm->setAutoFillBackground(true);
  fillColorAirspaceFlarm->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, fillColorAirspaceFlarm );

  drawOptions->resizeColumnsToContents();

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png"));

  connect(help, SIGNAL(pressed()), this, SLOT(slotHelp()));
  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(help, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix, 0, Qt::AlignCenter);
  contentLayout->addLayout(buttonBox);
  load();
}

SettingsPageAirspace::~SettingsPageAirspace()
{
}

void SettingsPageAirspace::showEvent( QShowEvent *event )
{
  // align all columns to contents before showing
  drawOptions->resizeColumnsToContents();
  drawOptions->setFocus();

  QWidget::showEvent( event );
}

void SettingsPageAirspace::slotHelp()
{
  QString file = "cumulus-settings-airspace.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void SettingsPageAirspace::slotAccept()
{
  save();
  QWidget::close();
}

void SettingsPageAirspace::slotReject()
{
  QWidget::close();
}

void SettingsPageAirspace::load()
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
  drawAirspaceD->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirD) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceE->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirE) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceF->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirF) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceFir->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirFir) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceFlarm->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirFlarm) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceG->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::AirG) ? Qt::Checked : Qt::Unchecked );
  drawControl->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::Ctr) ? Qt::Checked : Qt::Unchecked );
  drawRestricted->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::Restricted) ? Qt::Checked : Qt::Unchecked );
  drawDanger->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::Danger) ? Qt::Checked : Qt::Unchecked );
  drawProhibited->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::Prohibited) ? Qt::Checked : Qt::Unchecked );
  drawRMZ->setCheckState (conf->getItemDrawingEnabled(BaseMapElement::Rmz) ? Qt::Checked : Qt::Unchecked );
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
  borderColorAirspaceFir->setPalette( QPalette(conf->getBorderColorAirspaceFir()));
  borderColorAirspaceFlarm->setPalette( QPalette(conf->getBorderColorAirspaceFlarm()));
  borderColorAirspaceG->setPalette( QPalette(conf->getBorderColorAirspaceG()));
  borderColorWaveWindow->setPalette( QPalette(conf->getBorderColorWaveWindow()));
  borderColorControl->setPalette( QPalette(conf->getBorderColorControlC()));
  borderColorRestricted->setPalette( QPalette(conf->getBorderColorRestricted()));
  borderColorDanger->setPalette( QPalette(conf->getBorderColorDanger()));
  borderColorProhibited->setPalette( QPalette(conf->getBorderColorProhibited()));
  borderColorRMZ->setPalette( QPalette(conf->getBorderColorRMZ()));
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
  fillColorAirspaceFlarm->setPalette( QPalette(conf->getFillColorAirspaceFlarm()));
  fillColorAirspaceG->setPalette( QPalette(conf->getFillColorAirspaceG()));
  fillColorWaveWindow->setPalette( QPalette(conf->getFillColorWaveWindow()));
  fillColorControl->setPalette( QPalette(conf->getFillColorControlC()));
  fillColorRestricted->setPalette( QPalette(conf->getFillColorRestricted()));
  fillColorDanger->setPalette( QPalette(conf->getFillColorDanger()));
  fillColorProhibited->setPalette( QPalette(conf->getFillColorProhibited()));
  fillColorRMZ->setPalette( QPalette(conf->getFillColorRMZ()));
  fillColorTMZ->setPalette( QPalette(conf->getFillColorTMZ()));
  fillColorLowFlight->setPalette( QPalette(conf->getFillColorLowFlight()));
  fillColorGliderSector->setPalette( QPalette(conf->getFillColorGliderSector()));
}

void SettingsPageAirspace::save()
{
  if( checkChanges() == false )
    {
      return;
    }

  GeneralConfig * conf = GeneralConfig::instance();

  conf->setAirspaceDrawingBorder(m_borderDrawingValue->value());
  conf->setAirspaceDrawBorderEnabled(m_enableBorderDrawing->checkState() == Qt::Checked ? true : false);

  conf->setItemDrawingEnabled(BaseMapElement::AirA, drawAirspaceA->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirB, drawAirspaceB->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirC, drawAirspaceC->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirD, drawAirspaceD->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirE, drawAirspaceE->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirF, drawAirspaceF->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirFir, drawAirspaceFir->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirFlarm, drawAirspaceFlarm->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::AirG, drawAirspaceG->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Ctr, drawControl->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Restricted, drawRestricted->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Danger, drawDanger->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Prohibited, drawProhibited->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Rmz, drawRMZ->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::Tmz, drawTMZ->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::LowFlight, drawLowFlight->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::WaveWindow, drawWaveWindow->checkState() == Qt::Checked ? true : false);
  conf->setItemDrawingEnabled(BaseMapElement::GliderSector, drawGliderSector->checkState() == Qt::Checked ? true : false);

  // save border colors
  conf->setBorderColorAirspaceA(borderColorAirspaceA->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceB(borderColorAirspaceB->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceC(borderColorAirspaceC->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceD(borderColorAirspaceD->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceE(borderColorAirspaceE->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceF(borderColorAirspaceF->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceFir(borderColorAirspaceFir->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceFlarm(borderColorAirspaceFlarm->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceG(borderColorAirspaceG->palette().color(QPalette::Window));
  conf->setBorderColorWaveWindow(borderColorWaveWindow->palette().color(QPalette::Window));
  conf->setBorderColorControl(borderColorControl->palette().color(QPalette::Window));
  conf->setBorderColorRestricted(borderColorRestricted->palette().color(QPalette::Window));
  conf->setBorderColorDanger(borderColorDanger->palette().color(QPalette::Window));
  conf->setBorderColorProhibited(borderColorProhibited->palette().color(QPalette::Window));
  conf->setBorderColorRMZ(borderColorRMZ->palette().color(QPalette::Window));
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
  conf->setFillColorAirspaceFlarm(fillColorAirspaceFlarm->palette().color(QPalette::Window));
  conf->setFillColorAirspaceG(fillColorAirspaceG->palette().color(QPalette::Window));
  conf->setFillColorWaveWindow(fillColorWaveWindow->palette().color(QPalette::Window));
  conf->setFillColorControl(fillColorControl->palette().color(QPalette::Window));
  conf->setFillColorRestricted(fillColorRestricted->palette().color(QPalette::Window));
  conf->setFillColorDanger(fillColorDanger->palette().color(QPalette::Window));
  conf->setFillColorProhibited(fillColorProhibited->palette().color(QPalette::Window));
  conf->setFillColorRMZ(fillColorRMZ->palette().color(QPalette::Window));
  conf->setFillColorTMZ(fillColorTMZ->palette().color(QPalette::Window));
  conf->setFillColorLowFlight(fillColorLowFlight->palette().color(QPalette::Window));
  conf->setFillColorGliderSector(fillColorGliderSector->palette().color(QPalette::Window));

  conf->save();

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
  borderColorAirspaceFir->setPalette( QPalette(QColor(AIRFIR_COLOR)) );
  borderColorAirspaceFlarm->setPalette( QPalette(QColor(AIRFLARM_COLOR)) );
  borderColorAirspaceG->setPalette( QPalette(QColor(AIRG_COLOR)) );
  borderColorWaveWindow->setPalette( QPalette(QColor(WAVE_WINDOW_COLOR)) );
  borderColorControl->setPalette( QPalette(QColor(CTR_COLOR)) );
  borderColorRestricted->setPalette( QPalette(QColor(RESTRICTED_COLOR)) );
  borderColorDanger->setPalette( QPalette(QColor(DANGER_COLOR)) );
  borderColorProhibited->setPalette( QPalette(QColor(DANGER_COLOR)) );
  borderColorRMZ->setPalette( QPalette(QColor(RMZ_COLOR)) );
  borderColorTMZ->setPalette( QPalette(QColor(TMZ_COLOR)) );
  borderColorLowFlight->setPalette( QPalette(QColor(LOWF_COLOR)) );
  borderColorGliderSector->setPalette( QPalette(QColor(GLIDER_SECTOR_COLOR)) );

  fillColorAirspaceA->setPalette( QPalette(QColor(AIRA_BRUSH_COLOR)) );
  fillColorAirspaceB->setPalette( QPalette(QColor(AIRB_BRUSH_COLOR)) );
  fillColorAirspaceC->setPalette( QPalette(QColor(AIRC_BRUSH_COLOR)) );
  fillColorAirspaceD->setPalette( QPalette(QColor(AIRD_BRUSH_COLOR)) );
  fillColorAirspaceE->setPalette( QPalette(QColor(AIRE_BRUSH_COLOR)) );
  fillColorAirspaceF->setPalette( QPalette(QColor(AIRF_BRUSH_COLOR)) );
  fillColorAirspaceFlarm->setPalette( QPalette(QColor(AIRFLARM_BRUSH_COLOR)) );
  fillColorAirspaceG->setPalette( QPalette(QColor(AIRG_BRUSH_COLOR)) );
  fillColorWaveWindow->setPalette( QPalette(QColor(WAVE_WINDOW_BRUSH_COLOR)) );
  fillColorControl->setPalette( QPalette(QColor(CTR_BRUSH_COLOR)) );
  fillColorRestricted->setPalette( QPalette(QColor(RESTRICTED_BRUSH_COLOR)) );
  fillColorDanger->setPalette( QPalette(QColor(DANGER_BRUSH_COLOR)) );
  fillColorProhibited->setPalette( QPalette(QColor(DANGER_BRUSH_COLOR)) );
  fillColorRMZ->setPalette( QPalette(QColor(RMZ_BRUSH_COLOR)) );
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

  QTableWidgetItem* it = drawOptions->item( row, column );

  if( it != 0 && it->type() == 9999 )
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
      QColor newColor = ColorDialog::getColor( color, this, title );

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

void SettingsPageAirspace::slot_downloadAirspaces()
{
  AirspaceDownloadDialog *dlg = new AirspaceDownloadDialog( this );

  connect( dlg, SIGNAL(downloadAirspaces(const QStringList& )),
           this, SIGNAL(downloadAirspaces(const QStringList& )));

  dlg->setVisible( true );
}

#endif

/* Called to open the airspace warning dialog. */
void SettingsPageAirspace::slot_openFillDialog()
{
  SettingsPageAirspaceFillingNumPad* dlg = new SettingsPageAirspaceFillingNumPad(this);
  dlg->setVisible( true );
}

/* Called to open the airspace warning dialog. */
void SettingsPageAirspace::slot_openWarningDialog()
{
  SettingsPageAirspaceWarningsNumPad* dlg = new SettingsPageAirspaceWarningsNumPad(this);
  dlg->setVisible( true );
}

/* Called to open the airspace loading selection dialog. */
void SettingsPageAirspace::slot_openLoadDialog()
{
  SettingsPageAirspaceLoading* dlg = new SettingsPageAirspaceLoading(this);

  connect( dlg, SIGNAL(airspaceFileListChanged()),
           _globalMapContents, SLOT(slotReloadAirspaceData()) );

  dlg->setVisible( true );
}

/* Called to open the airspace filters dialog. */
void SettingsPageAirspace::slot_editAsFilters()
{
  AirspaceFilters* dlg = new AirspaceFilters(this);

  dlg->setVisible( true );
}


bool SettingsPageAirspace::checkChanges()
{
  GeneralConfig * conf = GeneralConfig::instance();
  bool changed = false;

  changed |= conf->getAirspaceDrawingBorder() != m_borderDrawingValue->value();
  changed |= conf->getAirspaceDrawBorderEnabled() != (m_enableBorderDrawing->checkState() == Qt::Checked ? true : false);

  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirA) != (drawAirspaceA->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirB) != (drawAirspaceB->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirC) != (drawAirspaceC->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirD) != (drawAirspaceD->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirE) != (drawAirspaceE->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirF) != (drawAirspaceF->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirFir) != (drawAirspaceFir->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirFlarm) != (drawAirspaceFlarm->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::AirG) != (drawAirspaceG->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Ctr) != (drawControl->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Restricted) != (drawRestricted->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Danger) != (drawDanger->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Prohibited) != (drawProhibited->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Rmz) != (drawRMZ->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::Tmz) != (drawTMZ->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::LowFlight) != (drawLowFlight->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::WaveWindow) != (drawWaveWindow->checkState() == Qt::Checked ? true : false);
  changed |= conf->getItemDrawingEnabled(BaseMapElement::GliderSector) != (drawGliderSector->checkState() == Qt::Checked ? true : false);

  changed |= conf->getBorderColorAirspaceA() != borderColorAirspaceA->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceB() != borderColorAirspaceB->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceC() != borderColorAirspaceC->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceD() != borderColorAirspaceD->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceE() != borderColorAirspaceE->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceF() != borderColorAirspaceF->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceFir() != borderColorAirspaceFir->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceFlarm() != borderColorAirspaceFlarm->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceG() != borderColorAirspaceG->palette().color(QPalette::Window);
  changed |= conf->getBorderColorWaveWindow() != borderColorWaveWindow->palette().color(QPalette::Window);
  changed |= conf->getBorderColorControl() != borderColorControl->palette().color(QPalette::Window);
  changed |= conf->getBorderColorRestricted() != borderColorRestricted->palette().color(QPalette::Window);
  changed |= conf->getBorderColorDanger() != borderColorDanger->palette().color(QPalette::Window);
  changed |= conf->getBorderColorProhibited() != borderColorProhibited->palette().color(QPalette::Window);
  changed |= conf->getBorderColorRMZ() != borderColorRMZ->palette().color(QPalette::Window);
  changed |= conf->getBorderColorTMZ() != borderColorTMZ->palette().color(QPalette::Window);
  changed |= conf->getBorderColorLowFlight() != borderColorLowFlight->palette().color(QPalette::Window);
  changed |= conf->getBorderColorGliderSector() != borderColorGliderSector->palette().color(QPalette::Window);

  changed |= conf->getFillColorAirspaceA() != fillColorAirspaceA->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceB() != fillColorAirspaceB->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceC() != fillColorAirspaceC->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceD() != fillColorAirspaceD->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceE() != fillColorAirspaceE->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceF() != fillColorAirspaceF->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceFlarm() != fillColorAirspaceFlarm->palette().color(QPalette::Window);
  changed |= conf->getFillColorAirspaceG() != fillColorAirspaceG->palette().color(QPalette::Window);
  changed |= conf->getFillColorWaveWindow() != fillColorWaveWindow->palette().color(QPalette::Window);
  changed |= conf->getFillColorControl() != fillColorControl->palette().color(QPalette::Window);
  changed |= conf->getFillColorRestricted() != fillColorRestricted->palette().color(QPalette::Window);
  changed |= conf->getFillColorDanger() != fillColorDanger->palette().color(QPalette::Window);
  changed |= conf->getFillColorProhibited() != fillColorProhibited->palette().color(QPalette::Window);
  changed |= conf->getFillColorRMZ() != fillColorRMZ->palette().color(QPalette::Window);
  changed |= conf->getFillColorTMZ() != fillColorTMZ->palette().color(QPalette::Window);
  changed |= conf->getFillColorLowFlight() != fillColorLowFlight->palette().color(QPalette::Window);
  changed |= conf->getFillColorGliderSector() != fillColorGliderSector->palette().color(QPalette::Window);

  return changed;
}

void SettingsPageAirspace::slot_enabledToggled(bool enabled)
{
  m_borderDrawingValue->setEnabled(enabled);
}
