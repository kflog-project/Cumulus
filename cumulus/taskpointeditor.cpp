/***********************************************************************
**
**   taskpointeditor.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-2016 Eggert Ehmke, Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "doubleNumberEditor.h"
#include "flighttask.h"
#include "generalconfig.h"
#include "layout.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "numberEditor.h"
#include "taskpointeditor.h"

extern MapContents  *_globalMapContents;

TaskPointEditor::TaskPointEditor( QWidget *parent, TaskPoint* tp) :
  QWidget( parent ),
  m_loadedCircleRadius(0),
  m_loadedInnerSectorRadius(0),
  m_loadedOuterSectorRadius(0),
  m_loadedLineLength (0),
  m_taskPoint(tp)
{
  setObjectName("TaskPointEditor");

  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowTitle( tr("Taskpoint Editor") );

  if( tp )
    {
      // We use a working copy, created from the passed task point for the
      // user modifications. That is necessary because the user can exit the
      // dialog with reject, to leave the original task point untouched.
      m_workTp = *tp;
    }

  if( MainWindow::mainWindow() )
  {
    // Resize the dialog to the same size as the main window has. That will
    // completely hide the parent window.
    resize( MainWindow::mainWindow()->size() );
  }

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  GeneralConfig *conf = GeneralConfig::instance();

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
  contentLayout->addLayout( sal, 10 );

  QGridLayout *topLayout = new QGridLayout(sw);
  topLayout->setMargin(5);
  topLayout->setSpacing( topLayout->spacing() * Layout::getIntScaledDensity() );

  // The input size of a QLineEditor is calculated a one M.
  QFontMetrics fm( font() );
  int charWidth = fm.width(QChar('M'));

  Qt::InputMethodHints imh;

  int row = 0;

  QHBoxLayout* hbl = new QHBoxLayout;
  hbl->setMargin(0);
  hbl->setSpacing( topLayout->spacing() * Layout::getIntScaledDensity() );

  m_pointShortLabel = new QLabel;
  hbl->addWidget( m_pointShortLabel );

  m_pointShortNameEditor = new QLineEdit;
  m_pointShortNameEditor->setInputMethodHints( Qt::ImhNoPredictiveText );
  m_pointShortNameEditor->setMaxLength(8); // limit name to 8 characters
  m_pointShortNameEditor->setMinimumWidth( 10 * charWidth );
  m_pointShortNameEditor->setMaximumWidth( 10 * charWidth );

  hbl->addWidget( m_pointShortNameEditor );
  hbl->addStretch( 10 );

  connect( m_pointShortNameEditor, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  topLayout->addLayout( hbl, row++, 0, 1, 3 );

  QLabel * lblDescription = new QLabel(tr("Description:"), this);
  topLayout->addWidget(lblDescription, row, 0);

  m_pointLongNameEditor = new QLineEdit;
  imh = (m_pointLongNameEditor->inputMethodHints() | Qt::ImhNoPredictiveText);
  m_pointLongNameEditor->setInputMethodHints(imh);
  m_pointLongNameEditor->setMaxLength(25); // limit name to 25 characters
#ifndef ANDROID
  m_pointLongNameEditor->setMinimumWidth( 18*charWidth );
  m_pointLongNameEditor->setMaximumWidth( 18*charWidth );
#else
  m_pointLongNameEditor->setMinimumWidth( 18*charWidth );
  m_pointLongNameEditor->setMaximumWidth( 18*charWidth );
#endif

  connect( m_pointLongNameEditor, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  topLayout->addWidget(m_pointLongNameEditor, row++, 1, 1, 2);

  QLabel* lblLat = new QLabel(tr("Latitude:"), this);
  topLayout->addWidget(lblLat, row, 0);

  m_latEditor = new LatEditNumPad(this, conf->getHomeLat());
  topLayout->addWidget(m_latEditor, row++, 1, 1, 1);

  QLabel* lblLon = new QLabel(tr("Longitude:"), this);
  topLayout->addWidget(lblLon, row, 0);

  m_lonEditor = new LongEditNumPad(this, conf->getHomeLon());
  topLayout->addWidget(m_lonEditor, row++, 1, 1, 1);

  QLabel* lblElev = new QLabel(tr("Elevation:"), this);
  topLayout->addWidget(lblElev, row, 0);

  m_elevetionEditor = new NumberEditor;
  m_elevetionEditor->setDecimalVisible( false );
  m_elevetionEditor->setSuffix( " " + Altitude::getUnitText() );
  m_elevetionEditor->setMaxLength(6);
  m_elevetionEditor->setAlignment( Qt::AlignLeft );
  m_elevetionEditor->setRange(-1000, 999999);
  m_elevetionEditor->setText("0");
  m_elevetionEditor->setMinimumWidth( 10 * charWidth );
  m_elevetionEditor->setMaximumWidth( 10 * charWidth );

  topLayout->addWidget(m_elevetionEditor, row++, 1);

  topLayout->setRowMinimumHeight( row++, 10 * Layout::getIntScaledDensity() );

  QGroupBox *tpsBox = new QGroupBox( tr("TP Scheme"), this );
  topLayout->addWidget( tpsBox, row, 0 );

  m_circle = new QRadioButton( tr("Circle"), this );
  m_sector = new QRadioButton( tr("Sector"), this );
  m_line   = new QRadioButton( tr("Line"), this);

  m_cslScheme = new QButtonGroup(this);
  m_cslScheme->addButton( m_circle, 0 );
  m_cslScheme->addButton( m_sector, 1 );
  m_cslScheme->addButton( m_line, 2);

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget( m_circle );
  vbox->addWidget( m_sector );
  vbox->addWidget( m_line);
  vbox->addStretch(1);
  tpsBox->setLayout(vbox);

  m_circle->setEnabled(true);
  m_circle->setChecked(false);
  m_sector->setEnabled(true);
  m_sector->setChecked(false);
  m_line->setEnabled(true);
  m_line->setChecked(false);

  //--------------------------------------------------------------
  // as next circle group is added
  m_circleGroup = new QGroupBox( tr("Circle"), this );
  topLayout->addWidget( m_circleGroup, row, 1 );

  QHBoxLayout *hbox = new QHBoxLayout;

  QLabel *lbl = new QLabel( tr("Radius:"), this );
  hbox->addWidget( lbl );

  QString unitText = Distance::getUnitText();

  m_circleRadius = new DoubleNumberEditor( this );
  m_circleRadius->setDecimalVisible( true );
  m_circleRadius->setPmVisible( false );
  m_circleRadius->setMaxLength(10);
  m_circleRadius->setSuffix( unitText );
  m_circleRadius->setDecimals( 3 );
  hbox->addWidget( m_circleRadius );
  hbox->addStretch(10);

  m_circleGroup->setLayout(hbox);

  //--------------------------------------------------------------
  // as next m_sector group is added
  m_sectorGroup = new QGroupBox( tr("Sector"), this );
  topLayout->addWidget( m_sectorGroup, row, 1 );

  QGridLayout *sectorLayout = new QGridLayout( m_sectorGroup );
  sectorLayout->setMargin(10);
  sectorLayout->setSpacing(3);

  int row1 = 0;

  lbl = new QLabel( tr("Radius 1:"), m_sectorGroup );
  sectorLayout->addWidget( lbl, row1, 0 );

  m_innerSectorRadius = new DoubleNumberEditor( m_sectorGroup );
  m_innerSectorRadius->setDecimalVisible( true );
  m_innerSectorRadius->setPmVisible( false );
  m_innerSectorRadius->setMaxLength(10);
  m_innerSectorRadius->setSuffix( unitText );
  m_innerSectorRadius->setDecimals( 3 );
  sectorLayout->addWidget( m_innerSectorRadius, row1, 1 );
  row1++;

  lbl = new QLabel( tr("Radius 2:"), m_sectorGroup );
  sectorLayout->addWidget( lbl, row1, 0 );

  m_outerSectorRadius = new DoubleNumberEditor( m_sectorGroup );
  m_outerSectorRadius->setDecimalVisible( true );
  m_outerSectorRadius->setPmVisible( false );
  m_outerSectorRadius->setMaxLength(10);
  m_outerSectorRadius->setSuffix( unitText );
  m_outerSectorRadius->setDecimals( 3 );
  sectorLayout->addWidget( m_outerSectorRadius, row1, 1 );

  // m_line selection must be invisible.
  if( m_workTp.getTaskPointType() != TaskPointTypes::Start &&
      m_workTp.getTaskPointType() != TaskPointTypes::Finish )
    {
      m_line->hide();
    }

  row1++;

  lbl = new QLabel( tr("Angle:"), m_sectorGroup );
  sectorLayout->addWidget( lbl, row1, 0 );

  m_sectorAngle = new NumberEditor( m_sectorGroup );
  m_sectorAngle->setDecimalVisible( false );
  m_sectorAngle->setPmVisible( false );
  m_sectorAngle->setMaxLength(3);
  m_sectorAngle->setSuffix( QString(Qt::Key_degree) );
  m_sectorAngle->setRange(1, 360 );
  m_sectorAngle->setTip("1...360");
  m_sectorAngle->setValue( m_workTp.getTaskSectorAngle() );
  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "[1-9][0-9]{0,2}" ), this );
  m_sectorAngle->setValidator( eValidator );
  sectorLayout->addWidget( m_sectorAngle, row1, 1 );
  sectorLayout->setColumnStretch(2, 10);

  //--------------------------------------------------------------
  // as next m_line group is added
  m_lineGroup = new QGroupBox( tr("Line"), this );
  topLayout->addWidget( m_lineGroup, row, 1 );

  QGridLayout *lineLayout = new QGridLayout( m_lineGroup );
  lineLayout->setMargin(10);
  lineLayout->setSpacing(3);

  row1 = 0;

  lbl = new QLabel( tr("Length:"), m_lineGroup );
  lineLayout->addWidget( lbl, row1, 0 );

  m_lineLength = new DoubleNumberEditor( m_lineGroup );
  m_lineLength->setDecimalVisible( true );
  m_lineLength->setPmVisible( false );
  m_lineLength->setMaxLength(10);
  m_lineLength->setSuffix( unitText );
  m_lineLength->setDecimals( 3 );
  lineLayout->addWidget( m_lineLength, row1, 1 );

  //--------------------------------------------------------------
  // as next auto zoom group is added
  QGroupBox* zoomGroup = new QGroupBox( tr("Zoom"), this );

  QVBoxLayout *zoomLayout = new QVBoxLayout (zoomGroup);
  zoomLayout->setMargin(10);
  zoomLayout->setSpacing (3);

  m_autoZoom = new QCheckBox( tr("Auto"), zoomGroup );
  m_autoZoom->setChecked( conf->getTaskPointAutoZoom() );
  zoomLayout->addWidget(m_autoZoom, 0);

  topLayout->addWidget( zoomGroup, row++, 2 );
  topLayout->setColumnStretch( 3, 10 );
  topLayout->setRowStretch( row++, 10 );

  // QStyle* style = QApplication::style();
  QPushButton *defaults = new QPushButton(tr("Defaults"));
  // defaults->setIcon(style->standardIcon(QStyle::SP_DialogResetButton));
  defaults->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("clear-32.png")) );

  const int iconSize = Layout::iconSize( font() );
  defaults->setIconSize(QSize(iconSize, iconSize));
  topLayout->addWidget( defaults, row, 0 );

  connect( defaults, SIGNAL(pressed()), SLOT(slot_schemaConfigurationDefaults()) );

  //--------------------------------------------------------------
  connect( m_cslScheme, SIGNAL(buttonClicked(int)),
           this, SLOT(slot_buttonPressedCSL(int)) );

  connect( m_outerSectorRadius, SIGNAL(numberEdited(const QString& )),
           this, SLOT(slot_outerSectorRadiusChanged(const QString& )) );

  //----------------------------------------------------------------------------
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
  titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

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

TaskPointEditor::~TaskPointEditor()
{
}

void TaskPointEditor::showEvent(QShowEvent *event)
{
  QWidget::showEvent( event );
}

void TaskPointEditor::slotAccept()
{
  save();
  emit taskPointEdited( m_taskPoint );
  close ();
}

void TaskPointEditor::slotReject()
{
  close ();
}

// value of outer spin box changed
void TaskPointEditor::slot_outerSectorRadiusChanged( double /* value */ )
{
  // set max range of inner m_sector to current value of outer m_sector
  m_innerSectorRadius->setMaximum( m_outerSectorRadius->value() );

  if( m_innerSectorRadius->value() > m_outerSectorRadius->value() )
    {
      // inner spin box value must be less equal outer spin box value
      m_innerSectorRadius->setValue( m_outerSectorRadius->value() );
    }
}

void TaskPointEditor::slot_outerSectorRadiusChanged( const QString& /* value */ )
{
  slot_outerSectorRadiusChanged( 1.0 );
}

// Set the passed scheme widget as active and the other one to inactive
void TaskPointEditor::slot_buttonPressedCSL( int newScheme )
{
  m_selectedCSLScheme = newScheme;

  switch (newScheme)
  {
    case GeneralConfig::Sector:
      m_circle->setChecked(false);
      m_sector->setChecked(true);
      m_line->setChecked(false);
      m_sectorGroup->setVisible(true);
      m_circleGroup->setVisible(false);
      m_lineGroup->setVisible(false);
      break;
    case GeneralConfig::Circle:
      m_circle->setChecked(true);
      m_sector->setChecked(false);
      m_line->setChecked(false);
      m_sectorGroup->setVisible(false);
      m_circleGroup->setVisible(true);
      m_lineGroup->setVisible(false);
      break;
    case GeneralConfig::Line:
      m_circle->setChecked(false);
      m_sector->setChecked(false);
      m_line->setChecked(true);
      m_sectorGroup->setVisible(false);
      m_circleGroup->setVisible(false);
      m_lineGroup->setVisible(true);
      break;
  }
}

void TaskPointEditor::load()
{
  m_pointShortLabel->setText( tr("Waypoint No %1 (%2):")
			      .arg(m_workTp.getFlightTaskListIndex() + 1)
			      .arg(m_workTp.getTaskPointTypeString(true)) );

  m_pointShortNameEditor->setText( m_workTp.getWPName() );

  m_pointLongNameEditor->setText( m_workTp.getName() );

  m_latEditor->setKFLogDegree(m_workTp.getWGSPositionPtr()->lat() );

  m_lonEditor->setKFLogDegree(m_workTp.getWGSPositionPtr()->lon() );

  m_elevetionEditor->setText( Altitude::getText( m_workTp.getElevation(), false, -1 ) );

  loadSchema();
}

void TaskPointEditor::loadSchema()
{
  // set active button as selected
  m_selectedCSLScheme = (int) m_workTp.getActiveTaskPointFigureScheme();

  slot_buttonPressedCSL( (int) m_workTp.getActiveTaskPointFigureScheme() );

  m_loadedCSLScheme = m_selectedCSLScheme;

  // @AP: radius is always fetched in meters.
  Distance cRadius = m_workTp.getTaskCircleRadius();
  Distance iRadius = m_workTp.getTaskSectorInnerRadius();
  Distance oRadius = m_workTp.getTaskSectorOuterRadius();
  Distance lLength = m_workTp.getTaskLineLength();

  Distance::distanceUnit distUnit = Distance::getUnit();

  if( distUnit == Distance::kilometers ) // user gets meters
    {
      m_circleRadius->setValue( cRadius.getKilometers() );
      m_innerSectorRadius->setValue( iRadius.getKilometers() );
      m_outerSectorRadius->setValue( oRadius.getKilometers() );
      m_lineLength->setValue(lLength.getKilometers());
    }
  else if( distUnit == Distance::miles ) // user gets miles
    {
      m_circleRadius->setValue( cRadius.getMiles() );
      m_innerSectorRadius->setValue( iRadius.getMiles() );
      m_outerSectorRadius->setValue( oRadius.getMiles() );
      m_lineLength->setValue(lLength.getMiles());
    }
  else // ( distUnit == Distance::nautmiles )
    {
      m_circleRadius->setValue( cRadius.getNautMiles() );
      m_innerSectorRadius->setValue( iRadius.getNautMiles() );
      m_outerSectorRadius->setValue( oRadius.getNautMiles() );
      m_lineLength->setValue(lLength.getNautMiles());
    }

  m_sectorAngle->setValue( m_workTp.getTaskSectorAngle() );

  // Save the loaded values from the configuration
  m_loadedCircleRadius    = m_circleRadius->value();
  m_loadedInnerSectorRadius = m_innerSectorRadius->value();
  m_loadedOuterSectorRadius = m_outerSectorRadius->value();
  m_loadedLineLength        = m_lineLength->value();
  m_loadedSectorAngle       = m_workTp.getTaskSectorAngle();

  // Check the validity of inner and outer m_sector.
  slot_outerSectorRadiusChanged( 1.0 );

  m_autoZoom->setChecked( m_workTp.getAutoZoom() );
  m_loadedAutoZoom = m_autoZoom->isChecked();
}

void TaskPointEditor::save()
{
  bool edited = false;

  if( m_pointShortNameEditor->isModified() )
    {
      m_workTp.setWPName( m_pointShortNameEditor->text().trimmed() );
    }

  if( m_pointLongNameEditor->isModified() )
    {
      m_workTp.setName( m_pointLongNameEditor->text().trimmed() );
    }

  if( m_latEditor->isInputChanged() || m_lonEditor->isInputChanged() )
    {
      m_workTp.setWGSPosition( WGSPoint(m_latEditor->KFLogDegree(), m_lonEditor->KFLogDegree()));
    }

  m_workTp.setElevation( static_cast<float> (Altitude::convertToMeters(m_elevetionEditor->text().toDouble())) );

  if( m_loadedCSLScheme != m_selectedCSLScheme )
    {
      edited = true;
      m_workTp.setActiveTaskPointFigureScheme( (GeneralConfig::ActiveTaskFigureScheme) m_selectedCSLScheme );
    }

  // @AP: radius is always saved in meters. Save is done only after a
  // real change to avoid rounding errors.
  Distance cRadius;
  Distance iRadius;
  Distance oRadius;
  Distance lLength;

  Distance::distanceUnit distUnit = Distance::getUnit();

  if( m_loadedCircleRadius != m_circleRadius->value() )
  {
    edited = true;

    switch ( distUnit )
    {
      case Distance::kilometers: // user gets kilometers
        cRadius.setKilometers( m_circleRadius->value() );
        break;
      case Distance::miles: // user gets miles
        cRadius.setMiles( m_circleRadius->value() );
        break;
      default: // ( distUnit == Distance::nautmiles )
        cRadius.setNautMiles( m_circleRadius->value() );
        break;
    }

    m_workTp.setTaskCircleRadius( cRadius );
  }

  if( m_loadedInnerSectorRadius != m_innerSectorRadius->value() )
    {
      edited = true;

      switch (distUnit)
        {
          case Distance::kilometers: // user gets kilometers
            iRadius.setKilometers(m_innerSectorRadius->value());
            break;
          case Distance::miles: // user gets miles
            iRadius.setMiles(m_innerSectorRadius->value());
            break;
          default: // ( distUnit == Distance::nautmiles )
            iRadius.setNautMiles(m_innerSectorRadius->value());
            break;
        }

      m_workTp.setTaskSectorInnerRadius(iRadius);
    }

  if (m_loadedOuterSectorRadius != m_outerSectorRadius->value())
    {
      edited = true;

      switch (distUnit)
        {
          case Distance::kilometers: // user gets kilometers
            oRadius.setKilometers(m_outerSectorRadius->value());
            break;
          case Distance::miles: // user gets miles
            oRadius.setMiles(m_outerSectorRadius->value());
            break;
          default: // ( distUnit == Distance::nautmiles )
            oRadius.setNautMiles(m_outerSectorRadius->value());
            break;
        }

      m_workTp.setTaskSectorOuterRadius(oRadius);
    }

  if (m_loadedLineLength != m_lineLength->value())
    {
      edited = true;

      switch (distUnit)
        {
          case Distance::kilometers: // user gets kilometers
            lLength.setKilometers(m_lineLength->value());
            break;
          case Distance::miles: // user gets miles
            lLength.setMiles(m_lineLength->value());
            break;
          default: // ( distUnit == Distance::nautmiles )
            lLength.setNautMiles(m_lineLength->value());
            break;
        }

      m_workTp.setTaskLineLength(lLength);
    }

  if( m_loadedSectorAngle != m_sectorAngle->value() )
    {
      edited = true;
      m_workTp.setTaskSectorAngle( m_sectorAngle->value() );
    }

  if( m_loadedAutoZoom != m_autoZoom->isChecked() )
    {
      edited = true;
      m_workTp.setAutoZoom( m_autoZoom->isChecked() );
    }

  // Update edited flag only, if edited is true. Otherwise a previous set
  // flag is unwanted reset. The user can press ok but has nothing changed before.
  if( edited )
    {
      m_workTp.setUserEditFlag( edited );
    }

  // Here we store the edited task point data in its original instance.
  *m_taskPoint = m_workTp;

  // If a flight task is set, we must update the m_sector angles in it
  FlightTask *task = _globalMapContents->getCurrentTask();

  if( task != 0 )
   {
     task->updateTask();
   }
}

void TaskPointEditor::slot_schemaConfigurationDefaults()
{
  m_workTp.setConfigurationDefaults();
  loadSchema();
}
