/***********************************************************************
**
**   settingspagemapobjects.cpp
**
**   This file is part of Cumulus.
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
************************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "layout.h"
#include "numberEditor.h"
#include "settingspagemapobjects.h"

SettingsPageMapObjects::SettingsPageMapObjects(QWidget *parent) :
  QWidget(parent),
  m_autoSip( true )
{
  setObjectName("SettingsPageMapObjects");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Map Objects") );

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

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal, 10 );

  // The parent of the layout is the scroll widget
  QGridLayout* topLayout = new QGridLayout(sw);
  topLayout->setMargin(5);

  int row=0;

  //---------------------------------------------------------------------------
  // The three waypoint scale limits are put in a group box
  QGroupBox *wpGroup = new QGroupBox( tr("Draw Waypoints until this scale"), this );
  topLayout->addWidget( wpGroup, row++, 0 );

  QHBoxLayout *hBox = new QHBoxLayout( wpGroup );
  hBox->setSpacing( 5 );

  QLabel *label = new QLabel( tr("Low"), wpGroup );
  hBox->addWidget( label );

  m_wpLowScaleLimit = new NumberEditor( wpGroup );
  m_wpLowScaleLimit->setDecimalVisible( false );
  m_wpLowScaleLimit->setPmVisible( false );
  m_wpLowScaleLimit->setMaxLength(4);
  m_wpLowScaleLimit->setPrefix( "< ");
  m_wpLowScaleLimit->setMaximum( 1200 );
  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "(0|[1-9][0-9]*)" ), this );
  m_wpLowScaleLimit->setValidator( eValidator );
  hBox->addWidget( m_wpLowScaleLimit );
  connect( m_wpLowScaleLimit, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpLowScaleLimitChanged(int)) );

  hBox->addSpacing( 5 );
  hBox->addStretch( 5 );

  label = new QLabel( tr("Normal"), wpGroup );
  hBox->addWidget( label );

  m_wpNormalScaleLimit = new NumberEditor( wpGroup );
  m_wpNormalScaleLimit->setDecimalVisible( false );
  m_wpNormalScaleLimit->setPmVisible( false );
  m_wpNormalScaleLimit->setMaxLength(4);
  m_wpNormalScaleLimit->setPrefix( "< ");
  m_wpNormalScaleLimit->setMaximum( 1200 );
  eValidator = new QRegExpValidator( QRegExp( "(0|[1-9][0-9]*)" ), this );
  m_wpNormalScaleLimit->setValidator( eValidator );
  hBox->addWidget( m_wpNormalScaleLimit );
  connect( m_wpNormalScaleLimit, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpNormalScaleLimitChanged(int)) );

  hBox->addSpacing( 5 );
  hBox->addStretch( 5 );

  label = new QLabel( tr("High"), wpGroup );
  hBox->addWidget( label );

  m_wpHighScaleLimit = new NumberEditor( wpGroup );
  m_wpHighScaleLimit->setDecimalVisible( false );
  m_wpHighScaleLimit->setPmVisible( false );
  m_wpHighScaleLimit->setMaxLength(4);
  m_wpHighScaleLimit->setPrefix( "< ");
  m_wpHighScaleLimit->setMaximum( 1200 );
  eValidator = new QRegExpValidator( QRegExp( "(0|[1-9][0-9]*)" ), this );
  m_wpHighScaleLimit->setValidator( eValidator );
  hBox->addWidget( m_wpHighScaleLimit );
  connect( m_wpHighScaleLimit, SIGNAL(valueChanged(int)),
           this, SLOT(slot_wpHighScaleLimitChanged(int)) );
  //hBox->addStretch( 10 );

  //---------------------------------------------------------------------------
  // table with 8 rows and 2 columns
  loadOptions = new QTableWidget(8, 2, this);

  loadOptions->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  loadOptions->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture( loadOptions->viewport(), QScroller::QtScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( loadOptions->viewport(), QtScroller::QtScroller::LeftMouseButtonGesture);
#endif

  loadOptions->setShowGrid( true );

  connect( loadOptions, SIGNAL(cellClicked ( int, int )),
           SLOT(slot_toggleCheckBox( int, int )));

  // hide vertical headers
  QHeaderView *vHeader = loadOptions->verticalHeader();
  vHeader->setVisible(false);

  QString header = tr("Load/Draw objects");
  QTableWidgetItem *item = new QTableWidgetItem( header );
  loadOptions->setHorizontalHeaderItem( 0, item );

  header = tr("Draw objects");
  item = new QTableWidgetItem( header );
  loadOptions->setHorizontalHeaderItem( 1, item );

  topLayout->addWidget( loadOptions, row++, 0 );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png"));

  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);

  load();
}

SettingsPageMapObjects::~SettingsPageMapObjects()
{}

void SettingsPageMapObjects::slotAccept()
{
  save();
  QWidget::close();
}

void SettingsPageMapObjects::slotReject()
{
  QWidget::close();
}

/**
 * Called, if the value in the low scale spin box has been changed.
 */
void SettingsPageMapObjects::slot_wpLowScaleLimitChanged( int newValue )
{
  if( newValue > m_wpNormalScaleLimit->value() )
    {
      // Check new value to ensure that current value of normal priority
      // is not exceeded. In such a case a reset is done.
      m_wpLowScaleLimit->setValue( m_wpNormalScaleLimit->value() );
    }

  m_wpLowScaleLimit->setMaximum( m_wpNormalScaleLimit->value() );
}

/**
 * Called, if the value in the normal scale spin box has been changed.
 */
void SettingsPageMapObjects::slot_wpNormalScaleLimitChanged( int newValue )
{
  if( newValue > m_wpHighScaleLimit->value() )
    {
      // Check new value to ensure that current value of high priority
      // is not exceeded. In such a case a reset is done.
      m_wpNormalScaleLimit->setValue( m_wpHighScaleLimit->value() );
    }

  if( m_wpLowScaleLimit->value() > m_wpNormalScaleLimit->value() )
    {
      // Check normal value to ensure that current value of low priority
      // is not exceeded. In such a case a reset is done.
      m_wpLowScaleLimit->setValue( m_wpNormalScaleLimit->value() );
    }

  // set max ranges according to the new passed value
  m_wpNormalScaleLimit->setMaximum( m_wpHighScaleLimit->value() );
  m_wpLowScaleLimit->setMaximum( m_wpNormalScaleLimit->value() );
}

/**
 * Called, if the value in the high scale spin box has been changed.
 */
void SettingsPageMapObjects::slot_wpHighScaleLimitChanged( int newValue )
{
  if( newValue < m_wpNormalScaleLimit->value() )
    {
      // Check new value to ensure that current value of normal priority
      // is not exceeded. In such a case a reset is done.
      m_wpNormalScaleLimit->setValue( newValue );
    }

  if( m_wpLowScaleLimit->value() > m_wpNormalScaleLimit->value() )
    {
      // Check low value to ensure that current value of normal priority
      // is not exceeded. In such a case a reset is done.
      m_wpLowScaleLimit->setValue( m_wpNormalScaleLimit->value() );
    }

  // set the new maxims for low and normal scale limit spin boxes
  m_wpNormalScaleLimit->setMaximum( newValue );
  m_wpLowScaleLimit->setMaximum( newValue );
}


void SettingsPageMapObjects::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  fillLoadOptionList();
  liIsolines->setCheckState( conf->getMapLoadIsoLines() ? Qt::Checked : Qt::Unchecked );
  liIsolineBorders->setCheckState( conf->getMapShowIsoLineBorders() ? Qt::Checked : Qt::Unchecked );
  liWpLabels->setCheckState( conf->getMapShowWaypointLabels() ? Qt::Checked : Qt::Unchecked );
  liLabelsInfo->setCheckState( conf->getMapShowLabelsExtraInfo() ? Qt::Checked : Qt::Unchecked );
  liRoads->setCheckState( conf->getMapLoadRoads() ? Qt::Checked : Qt::Unchecked );
  liMotorways->setCheckState( conf->getMapLoadMotorways() ? Qt::Checked : Qt::Unchecked );
  liRailways->setCheckState( conf->getMapLoadRailways() ? Qt::Checked : Qt::Unchecked );
  liCities->setCheckState( conf->getMapLoadCities() ? Qt::Checked : Qt::Unchecked );
  liWaterways->setCheckState( conf->getMapLoadWaterways() ? Qt::Checked : Qt::Unchecked );
  liForests->setCheckState( conf->getMapLoadForests() ? Qt::Checked : Qt::Unchecked );
  liAfLabels->setCheckState( conf->getMapShowAirfieldLabels() ? Qt::Checked : Qt::Unchecked );
  liTpLabels->setCheckState( conf->getMapShowTaskPointLabels() ? Qt::Checked : Qt::Unchecked );
  liOlLabels->setCheckState( conf->getMapShowOutLandingLabels() ? Qt::Checked : Qt::Unchecked );
  liRelBearingInfo->setCheckState( conf->getMapShowRelBearingInfo() ? Qt::Checked : Qt::Unchecked );
  liFlightTrail->setCheckState( conf->getMapDrawTrail() ? Qt::Checked : Qt::Unchecked );
  // Load scale values for spin boxes. Note! The load order is important because a value change
  // of the spin box will generate a signal.
  m_wpHighScaleLimit->setValue( conf->getWaypointScaleBorder( Waypoint::High ));
  m_wpNormalScaleLimit->setValue( conf->getWaypointScaleBorder( Waypoint::Normal ));
  m_wpLowScaleLimit->setValue( conf->getWaypointScaleBorder( Waypoint::Low ));

  // set maximums
  m_wpLowScaleLimit->setMaximum( m_wpNormalScaleLimit->value() );
  m_wpNormalScaleLimit->setMaximum( m_wpHighScaleLimit->value() );
}

void SettingsPageMapObjects::save()
{
  if( ! checkChanges() )
    {
      return;
    }

  GeneralConfig *conf = GeneralConfig::instance();

  conf->setMapLoadIsoLines( liIsolines->checkState() == Qt::Checked ? true : false );
  conf->setMapShowIsoLineBorders(liIsolineBorders->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadRoads(liRoads->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadMotorways(liMotorways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadRailways(liRailways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadCities(liCities->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadWaterways(liWaterways->checkState() == Qt::Checked ? true : false);
  conf->setMapLoadForests(liForests->checkState() == Qt::Checked ? true : false);
  conf->setMapShowAirfieldLabels(liAfLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowTaskPointLabels(liTpLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowOutLandingLabels(liOlLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowWaypointLabels(liWpLabels->checkState() == Qt::Checked ? true : false);
  conf->setMapShowLabelsExtraInfo(liLabelsInfo->checkState() == Qt::Checked ? true : false);
  conf->setMapShowRelBearingInfo(liRelBearingInfo->checkState() == Qt::Checked ? true : false);
  conf->setMapDrawTrail(liFlightTrail->checkState() == Qt::Checked ? true : false);

  conf->setWaypointScaleBorder( Waypoint::Low, m_wpLowScaleLimit->value() );
  conf->setWaypointScaleBorder( Waypoint::Normal, m_wpNormalScaleLimit->value() );
  conf->setWaypointScaleBorder( Waypoint::High, m_wpHighScaleLimit->value() );

  emit settingsChanged();
}

/** Fills the list with load options */
void SettingsPageMapObjects::fillLoadOptionList()
{
  int row = 0;
  int col = 0;

  liCities = new QTableWidgetItem( tr("Cities") );
  liCities->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liCities );

  liForests = new QTableWidgetItem( tr("Forests") );
  liForests->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liForests );

  liIsolines = new QTableWidgetItem( tr("Isolines") );
  liIsolines->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liIsolines );

  liIsolineBorders = new QTableWidgetItem( tr("Isoline borders") );
  liIsolineBorders->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liIsolineBorders );

  liMotorways = new QTableWidgetItem( tr("Motorways") );
  liMotorways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liMotorways );

  liRailways = new QTableWidgetItem( tr("Railways") );
  liRailways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liRailways );

  liRoads = new QTableWidgetItem( tr("Roads") );
  liRoads->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liRoads );

  liWaterways = new QTableWidgetItem( tr("Rivers & Canals") );
  liWaterways->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liWaterways );

  // next column is one
  row = 0;
  col = 1;

  liAfLabels = new QTableWidgetItem( tr("Airfield labels") );
  liAfLabels->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liAfLabels );

  liTpLabels = new QTableWidgetItem( tr("Taskpoint labels") );
  liTpLabels->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liTpLabels );

  liOlLabels = new QTableWidgetItem( tr("Outlanding labels") );
  liOlLabels->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liOlLabels );

  liWpLabels = new QTableWidgetItem( tr("Waypoint labels") );
  liWpLabels->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liWpLabels );

  liLabelsInfo = new QTableWidgetItem( tr("Labels - Extra info") );
  liLabelsInfo->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liLabelsInfo );

  liRelBearingInfo = new QTableWidgetItem( tr("Relative bearing info") );
  liRelBearingInfo->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liRelBearingInfo );

  liFlightTrail = new QTableWidgetItem( tr("Flight trail") );
  liFlightTrail->setFlags( Qt::ItemIsEnabled );
  loadOptions->setItem( row++, col, liFlightTrail );

  // Set a dummy into the unused cells
  QTableWidgetItem *liDummy = new QTableWidgetItem;
  liDummy->setFlags( Qt::NoItemFlags );
  loadOptions->setItem( row++, col, liDummy );
}

void SettingsPageMapObjects::showEvent( QShowEvent *event )
{
  // align all columns to contents before showing
  loadOptions->resizeColumnsToContents();
  loadOptions->setFocus();

  QWidget::showEvent( event );
}

/**
 * Called to toggle the check box of the clicked table cell.
 */
void SettingsPageMapObjects::slot_toggleCheckBox( int row, int column )
{
  if( column == 1 && row > 6 )
    {
      // Dummy cell was clicked
      return;
    }

  QTableWidgetItem *item = loadOptions->item( row, column );
  item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
}

bool SettingsPageMapObjects::checkChanges()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed |= ( conf->getMapLoadIsoLines() ? Qt::Checked : Qt::Unchecked ) != liIsolines->checkState();
  changed |= ( conf->getMapShowIsoLineBorders() ? Qt::Checked : Qt::Unchecked ) != liIsolineBorders->checkState();
  changed |= ( conf->getMapShowWaypointLabels() ? Qt::Checked : Qt::Unchecked ) != liWpLabels->checkState();
  changed |= ( conf->getMapShowLabelsExtraInfo() ? Qt::Checked : Qt::Unchecked ) != liLabelsInfo->checkState();
  changed |= ( conf->getMapLoadRoads() ? Qt::Checked : Qt::Unchecked ) != liRoads->checkState();
  changed |= ( conf->getMapLoadMotorways() ? Qt::Checked : Qt::Unchecked ) != liMotorways->checkState();
  changed |= ( conf->getMapLoadRailways() ? Qt::Checked : Qt::Unchecked ) != liRailways->checkState();
  changed |= ( conf->getMapLoadCities() ? Qt::Checked : Qt::Unchecked ) != liCities->checkState();
  changed |= ( conf->getMapLoadWaterways() ? Qt::Checked : Qt::Unchecked ) != liWaterways->checkState();
  changed |= ( conf->getMapLoadForests() ? Qt::Checked : Qt::Unchecked ) != liForests->checkState();
  changed |= ( conf->getMapShowAirfieldLabels() ? Qt::Checked : Qt::Unchecked ) != liAfLabels->checkState();
  changed |= ( conf->getMapShowTaskPointLabels() ? Qt::Checked : Qt::Unchecked ) != liTpLabels->checkState();
  changed |= ( conf->getMapShowOutLandingLabels() ? Qt::Checked : Qt::Unchecked ) != liOlLabels->checkState();
  changed |= ( conf->getMapShowRelBearingInfo() ? Qt::Checked : Qt::Unchecked ) != liRelBearingInfo->checkState();
  changed |= ( conf->getMapDrawTrail() ? Qt::Checked : Qt::Unchecked ) != liFlightTrail->checkState();

  changed |= ( conf->getWaypointScaleBorder( Waypoint::Low )    != m_wpLowScaleLimit->value() );
  changed |= ( conf->getWaypointScaleBorder( Waypoint::Normal ) != m_wpNormalScaleLimit->value() );
  changed |= ( conf->getWaypointScaleBorder( Waypoint::High )   != m_wpHighScaleLimit->value() );

  return changed;
}
