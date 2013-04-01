/***********************************************************************
**
**   settingspagetask.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2007-2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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
#include "generalconfig.h"
#include "layout.h"
#include "numberEditor.h"
#include "settingspagetask.h"

SettingsPageTask::SettingsPageTask( QWidget *parent) :
  QWidget( parent ),
  m_selectedSwitchScheme(0),
  m_distUnit(Distance::getUnit()),
  m_startLineValue(0),
  m_startRingValue(0),
  m_startSectorInnerRadiusValue(0),
  m_startSectorOuterRadiusValue(0),
  m_startSectorAngleValue(0),
  m_finishLineValue(0),
  m_finishRingValue(0),
  m_finishSectorInnerRadiusValue(0),
  m_finishSectorOuterRadiusValue(0),
  m_finishSectorAngleValue(0),
  m_obsCircleRadiusValue(0),
  m_obsSectorInnerRadiusValue(0),
  m_obsSectorOuterRadiusValue(0),
  m_obsSectorAngleValue(0)
{
  setObjectName("SettingsPageTask");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Task") );

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

  GeneralConfig *conf = GeneralConfig::instance();
  int row = 0;

  QGridLayout *topLayout = new QGridLayout(sw);
  //contentLayout->addLayout(topLayout);

  topLayout->setMargin(10);
  topLayout->setSpacing(20);
  topLayout->setColumnStretch( 3, 5 );

  //---------------------------------------------------------------
  QGroupBox *ssBox = new QGroupBox( tr("Switch Scheme"), this );
  topLayout->addWidget( ssBox, row, 0 );

  ntScheme = new QButtonGroup(this);
  QRadioButton* nearest  = new QRadioButton( tr("Minimum"), this );
  QRadioButton* touched  = new QRadioButton( tr("Touched"), this );

  ntScheme->addButton( nearest, 0 );
  ntScheme->addButton( touched, 1 );

  QVBoxLayout* vbox = new QVBoxLayout;
  vbox->addWidget( nearest );
  vbox->addWidget( touched );
  vbox->addStretch(1);
  ssBox->setLayout(vbox);

  nearest->setEnabled(true);
  touched->setEnabled(true);
  touched->setChecked(true);

  // set active button as selected
  m_selectedSwitchScheme = (int) conf->getActiveTaskSwitchScheme();

  if( ntScheme->button(m_selectedSwitchScheme) )
    {
      ntScheme->button(m_selectedSwitchScheme)->setChecked(true);
    }

  connect( ntScheme, SIGNAL(buttonClicked(int)),
           this, SLOT(slot_buttonPressedNT(int)) );

  //--------------------------------------------------------------
  // as next shape group is added
  QGroupBox* shapeGroup = new QGroupBox( tr("Shape"), this );
  topLayout->addWidget( shapeGroup, row, 1 );

  QGridLayout *gBox = new QGridLayout;
  shapeGroup->setLayout(gBox);

  m_drawShape = new QCheckBox( tr("Draw"), this );
  gBox->addWidget(m_drawShape, 0, 0 );
  m_fillShape = new QCheckBox( tr("Fill"), this );
  gBox->addWidget(m_fillShape, 1, 0 );
  gBox->setColumnStretch( 2, 5 );

  connect( m_fillShape, SIGNAL(stateChanged(int)),
           SLOT(slot_fillShapeStateChanged(int)) );

  m_transShape = new NumberEditor( this );
  m_transShape->setDecimalVisible( false );
  m_transShape->setPmVisible( false );
  m_transShape->setMaxLength(3);
  m_transShape->setSuffix( " %" );
  m_transShape->setRange( 0, 100 );
  m_transShape->setTip(tr("Opacity ") + "0...100");
  m_transShape->setValue( 0 );
  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "(0|[1-9][0-9]{0,2})" ), this );
  m_transShape->setValidator( eValidator );

  // Sets a minimum width for the widget
  int mw1 = QFontMetrics(font()).width("100 %") + 10;
  m_transShape->setMinimumWidth( mw1 );
  gBox->addWidget(m_transShape, 1, 1 );

  m_drawShape->setChecked( conf->getTaskDrawShape() );
  m_fillShape->setChecked( conf->getTaskFillShape() );

  //--------------------------------------------------------------
  // as next auto zoom group is added
  QGroupBox* zoomGroup = new QGroupBox( tr("Zoom"), this );
  topLayout->addWidget( zoomGroup, row, 2 );

  QVBoxLayout *zBox = new QVBoxLayout;
  zoomGroup->setLayout(zBox);

  m_autoZoom = new QCheckBox( tr("Auto"), this );
  zBox->addWidget(m_autoZoom);
  zBox->addStretch(10);

  m_autoZoom->setChecked( conf->getTaskPointAutoZoom() );

#ifdef ANDROID
  topLayout->setRowMinimumHeight( ++row, 30 );
#endif

  row++;

  //--------------------------------------------------------------
  // as next start group is added
  QGroupBox* startGroup1 = new QGroupBox( tr("Start"), this );
  topLayout->addWidget( startGroup1, row, 0 );
  QFormLayout *formLayout = new QFormLayout;
  startGroup1->setLayout(formLayout);

  m_startLine = createDNE( this );
  m_startLine->setTip(tr("Line Length"));
  formLayout->addRow(tr("Line:"), m_startLine);

  m_startRing = createDNE( this );
  m_startRing->setTip(tr("Circle Radius"));
  formLayout->addRow(tr("Circle:"), m_startRing);

  //--------------------------------------------------------------
  // as next inner sector group is added
  QGroupBox* startGroup2 = new QGroupBox( tr("Sector"), this );
  topLayout->addWidget( startGroup2, row, 1 );
  formLayout = new QFormLayout;
  startGroup2->setLayout(formLayout);

  m_startSectorInnerRadius = createDNE( this );
  m_startSectorInnerRadius->setTip(tr("Inner Radius"));
  formLayout->addRow(tr("Radius 1:"), m_startSectorInnerRadius);

  m_startSectorOuterRadius = createDNE( this );
  m_startSectorOuterRadius->setTip(tr("Outer Radius"));
  formLayout->addRow(tr("Radius 2:"), m_startSectorOuterRadius);

  m_startSectorAngle = createNE( this );
  m_startSectorAngle->setTip(tr("Angle"));
  formLayout->addRow(tr("Angle:"), m_startSectorAngle);

  //--------------------------------------------------------------
  // as next inner scheme group is added
  QGroupBox* startGroup3 = new QGroupBox( tr("Scheme"), this );
  topLayout->addWidget( startGroup3, row, 2 );
  formLayout = new QFormLayout;
  startGroup3->setLayout(formLayout);

  QRadioButton* circle = new QRadioButton( tr("Circle"), this );
  QRadioButton* sector = new QRadioButton( tr("Sector"), this );
  QRadioButton* line   = new QRadioButton( tr("Line"), this );

  startScheme = new QButtonGroup(this);
  startScheme->addButton( circle, 0 );
  startScheme->addButton( sector, 1 );
  startScheme->addButton( line, 2 );

  formLayout->addWidget( circle );
  formLayout->addWidget( sector );
  formLayout->addWidget( line );

  circle->setEnabled(true);
  circle->setChecked(false);
  sector->setEnabled(true);
  sector->setChecked(false);
  line->setEnabled(true);
  line->setChecked(false);

  // set active button as selected
  m_selectedStartScheme = (int) conf->getActiveTaskStartScheme();

  if( startScheme->button(m_selectedStartScheme) )
    {
      startScheme->button(m_selectedStartScheme)->setChecked(true);
    }

#ifdef ANDROID
  topLayout->setRowMinimumHeight( ++row, 30 );
#endif

  row++;

  //--------------------------------------------------------------
  // as next finish group is added
  QGroupBox* finishGroup1 = new QGroupBox( tr("Finish"), this );
  topLayout->addWidget( finishGroup1, row, 0 );
  formLayout = new QFormLayout;
  finishGroup1->setLayout(formLayout);

  m_finishLine = createDNE( this );
  m_finishLine->setTip(tr("Line Length"));
  formLayout->addRow(tr("Line:"), m_finishLine);

  m_finishRing = createDNE( this );
  m_finishRing->setTip(tr("Circle Radius"));
  formLayout->addRow(tr("Circle"), m_finishRing);

  //--------------------------------------------------------------
  // as next inner sector group is added
  QGroupBox* finishGroup2 = new QGroupBox( tr("Sector"), this );
  topLayout->addWidget( finishGroup2, row, 1 );
  formLayout = new QFormLayout;
  finishGroup2->setLayout(formLayout);

  m_finishSectorInnerRadius = createDNE( this );
  m_finishSectorInnerRadius->setTip(tr("Inner Radius"));
  formLayout->addRow(tr("Radius 1:"), m_finishSectorInnerRadius);

  m_finishSectorOuterRadius = createDNE( this );
  m_finishSectorOuterRadius->setTip(tr("Outer Radius"));
  formLayout->addRow(tr("Radius 2:"), m_finishSectorOuterRadius);

  m_finishSectorAngle = createNE( this );
  m_finishSectorAngle->setTip(tr("Angle"));
  formLayout->addRow(tr("Angle:"), m_finishSectorAngle);

  //--------------------------------------------------------------
  // as next inner scheme group is added
  QGroupBox* finishGroup3 = new QGroupBox( tr("Scheme"), this );
  topLayout->addWidget( finishGroup3, row, 2 );
  formLayout = new QFormLayout;
  finishGroup3->setLayout(formLayout);

  circle = new QRadioButton( tr("Circle"), this );
  sector = new QRadioButton( tr("Sector"), this );
  line   = new QRadioButton( tr("Line"), this );

  finishScheme = new QButtonGroup(this);
  finishScheme->addButton( circle, 0 );
  finishScheme->addButton( sector, 1 );
  finishScheme->addButton( line, 2 );

  formLayout->addWidget( circle );
  formLayout->addWidget( sector );
  formLayout->addWidget( line );

  circle->setEnabled(true);
  circle->setChecked(false);
  sector->setEnabled(true);
  sector->setChecked(false);
  line->setEnabled(true);
  line->setChecked(false);

  // set active button as selected
  m_selectedFinishScheme = (int) conf->getActiveTaskFinishScheme();

  if( finishScheme->button(m_selectedFinishScheme) )
    {
      finishScheme->button(m_selectedFinishScheme)->setChecked(true);
    }

#ifdef ANDROID
  topLayout->setRowMinimumHeight( ++row, 30 );
#endif

  row++;

  //--------------------------------------------------------------
  // as next observation zone is added
  QGroupBox* obZoneGroup1 = new QGroupBox( tr("Observation"), this );
  topLayout->addWidget( obZoneGroup1, row, 0 );
  formLayout = new QFormLayout;
  obZoneGroup1->setLayout(formLayout);

  m_obsCircleRadius = createDNE( this );
  m_obsCircleRadius->setTip(tr("Circle Radius"));
  formLayout->addRow(tr("Circle:"), m_obsCircleRadius);

  //--------------------------------------------------------------
  // as next inner sector group is added
  QGroupBox* obZoneGroup2 = new QGroupBox( tr("Sector"), this );
  topLayout->addWidget( obZoneGroup2, row, 1 );
  formLayout = new QFormLayout;
  obZoneGroup2->setLayout(formLayout);

  m_obsSectorInnerRadius = createDNE( this );
  m_obsSectorInnerRadius->setTip(tr("Inner Radius"));
  formLayout->addRow(tr("Radius 1:"), m_obsSectorInnerRadius);

  m_obsSectorOuterRadius = createDNE( this );
  m_obsSectorOuterRadius->setTip(tr("Outer Radius"));
  formLayout->addRow(tr("Radius 2:"), m_obsSectorOuterRadius);

  m_obsSectorAngle = createNE( this );
  m_obsSectorAngle->setTip(tr("Sector Angle"));
  formLayout->addRow(tr("Angle:"), m_obsSectorAngle);

  //--------------------------------------------------------------
  // as next inner observer scheme group is added
  QGroupBox* obZoneGroup3 = new QGroupBox( tr("Scheme"), this );
  topLayout->addWidget( obZoneGroup3, row, 2 );
  formLayout = new QFormLayout;
  obZoneGroup3->setLayout(formLayout);

  circle = new QRadioButton( tr("Circle"), this );
  sector = new QRadioButton( tr("Sector"), this );
  // no line for observer area

  obsScheme = new QButtonGroup(this);
  obsScheme->addButton( circle, 0 );
  obsScheme->addButton( sector, 1 );

  formLayout->addWidget( circle );
  formLayout->addWidget( sector );

  circle->setEnabled(true);
  circle->setChecked(false);
  sector->setEnabled(true);
  sector->setChecked(false);

  // set active button as selected
  m_selectedObsScheme = (int) conf->getActiveTaskObsScheme();

  if( obsScheme->button(m_selectedObsScheme) )
    {
      obsScheme->button(m_selectedObsScheme)->setChecked(true);
    }

  //--------------------------------------------------------------
  // Connect sector check controls.
  connect( m_startSectorOuterRadius, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_outerSectorStartChanged(const QString&)) );
  connect( m_finishSectorOuterRadius, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_outerSectorFinishChanged(const QString&)) );
  connect( m_obsSectorOuterRadius, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_outerSectorObsChanged(const QString&)) );
  connect( startScheme, SIGNAL(buttonClicked(int)), this, SLOT(slot_buttonPressedSS(int)) );
  connect( finishScheme, SIGNAL(buttonClicked(int)), this, SLOT(slot_buttonPressedFS(int)) );
  connect( obsScheme, SIGNAL(buttonClicked(int)), this, SLOT(slot_buttonPressedOS(int)) );

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

// Destructor of class
SettingsPageTask::~SettingsPageTask()
{
}

void SettingsPageTask::slotAccept()
{
  save();
  emit settingsChanged();
  QWidget::close();
}

void SettingsPageTask::slotReject()
{
  QWidget::close();
}

DoubleNumberEditor* SettingsPageTask::createDNE( QWidget* parent )
{
  DoubleNumberEditor* dne = new DoubleNumberEditor( parent );
  dne->setDecimalVisible( true );
  dne->setPmVisible( false );
  dne->setMaxLength(10);
  dne->setSuffix( " " + Distance::getUnitText() );
  dne->setDecimals( 3 );
  dne->setValue( 0 );
  dne->setMinimum( 0 );
  return dne;
}

NumberEditor* SettingsPageTask::createNE( QWidget* parent )
{
  NumberEditor* ne = new NumberEditor( parent );
  ne->setDecimalVisible( false );
  ne->setPmVisible( false );
  ne->setMaxLength(3);
  ne->setSuffix( QString(Qt::Key_degree) );
  ne->setRange( 1, 360 );
  ne->setTip("1...360");
  ne->setValue( 90 );
  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "[1-9][0-9]{0,2}" ), this );
  ne->setValidator( eValidator );
  return ne;
}

void SettingsPageTask::slot_fillShapeStateChanged( int state )
{
  if( state == Qt::Unchecked )
    {
      m_transShape->setEnabled(false);
    }
  else
    {
      m_transShape->setEnabled(true);
    }
}

// value of outer sector box changed
void SettingsPageTask::slot_outerSectorStartChanged( const QString& value )
{
  bool ok;
  double d = value.toDouble( &ok );

  if( ! ok )
    {
      return;
    }

  // set max range of inner sector to current value of outer sector
  m_startSectorInnerRadius->setMaximum( d );

  if( m_startSectorInnerRadius->value() > d )
    {
      // inner spin box value must be less equal outer spin box d
      m_startSectorInnerRadius->setValue( d );
    }
}

// value of outer sector box changed
void SettingsPageTask::slot_outerSectorFinishChanged( const QString& value )
{
  bool ok;
  double d = value.toDouble( &ok );

  if( ! ok )
    {
      return;
    }

  // set max range of inner sector to current value of outer sector
  m_finishSectorInnerRadius->setMaximum( d );

  if( m_finishSectorInnerRadius->value() > d )
    {
      // inner spin box value must be less equal outer spin box value
      m_finishSectorInnerRadius->setValue( d );
    }
}

// value of outer sector box changed
void SettingsPageTask::slot_outerSectorObsChanged( const QString& value )
{
  bool ok;
  double d = value.toDouble( &ok );

  if( ! ok )
    {
      return;
    }

  // set max range of inner sector to current value of outer sector
  m_obsSectorInnerRadius->setMaximum( d );

  if( m_obsSectorInnerRadius->value() > d )
    {
      // inner spin box value must be less equal outer spin box value
      m_obsSectorInnerRadius->setValue( d );
    }
}

// Start Scheme changed
void SettingsPageTask::slot_buttonPressedSS(int newScheme)
{
  m_selectedStartScheme = newScheme;
}

// Finish Scheme changed
void SettingsPageTask::slot_buttonPressedFS(int newScheme)
{
  m_selectedFinishScheme = newScheme;
}

// Observer Scheme changed
void SettingsPageTask::slot_buttonPressedOS(int newScheme)
{
  m_selectedObsScheme = newScheme;
}


void SettingsPageTask::slot_buttonPressedNT( int newScheme )
{
  m_selectedSwitchScheme = newScheme;
}

void SettingsPageTask::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  m_selectedSwitchScheme = (int) conf->getActiveTaskSwitchScheme();

  if( ntScheme->button(m_selectedSwitchScheme) )
    {
      ntScheme->button(m_selectedSwitchScheme)->setChecked(true);
    }

  m_drawShape->setChecked( conf->getTaskDrawShape() );
  m_fillShape->setChecked( conf->getTaskFillShape() );
  m_transShape->setValue( conf->getTaskShapeAlpha() );
  m_autoZoom->setChecked( conf->getTaskPointAutoZoom() );

  m_selectedStartScheme = (int) conf->getActiveTaskStartScheme();

  if( startScheme->button(m_selectedStartScheme) )
    {
      startScheme->button(m_selectedStartScheme)->setChecked(true);
    }

  m_startLineValue               = conf->getTaskStartLineLength();
  m_startRingValue               = conf->getTaskStartRingRadius();
  m_startSectorInnerRadiusValue  = conf->getTaskStartSectorIRadius();
  m_startSectorOuterRadiusValue  = conf->getTaskStartSectorORadius();
  m_startSectorAngleValue        = conf->getTaskStartSectorAngel();

  if( finishScheme->button(m_selectedFinishScheme) )
    {
      finishScheme->button(m_selectedFinishScheme)->setChecked(true);
    }

  m_finishLineValue              = conf->getTaskFinishLineLength();
  m_finishRingValue              = conf->getTaskFinishRingRadius();
  m_finishSectorInnerRadiusValue = conf->getTaskFinishSectorIRadius();
  m_finishSectorOuterRadiusValue = conf->getTaskFinishSectorORadius();
  m_finishSectorAngleValue       = conf->getTaskFinishSectorAngel();

  if( obsScheme->button(m_selectedObsScheme) )
    {
      obsScheme->button(m_selectedObsScheme)->setChecked(true);
    }

  m_obsCircleRadiusValue      = conf->getTaskObsCircleRadius();
  m_obsSectorInnerRadiusValue   = conf->getTaskObsSectorInnerRadius();
  m_obsSectorOuterRadiusValue   = conf->getTaskObsSectorOuterRadius();
  m_obsSectorAngleValue         = conf->getTaskObsSectorAngle();

  if( m_distUnit == Distance::kilometers ) // user gets meters
    {
      m_startLine->setValue( m_startLineValue.getKilometers() );
      m_startRing->setValue( m_startRingValue.getKilometers() );
      m_startSectorInnerRadius->setValue( m_startSectorInnerRadiusValue.getKilometers() );
      m_startSectorOuterRadius->setValue( m_startSectorOuterRadiusValue.getKilometers() );

      m_finishLine->setValue( m_finishLineValue.getKilometers() );
      m_finishRing->setValue( m_finishRingValue.getKilometers() );
      m_finishSectorInnerRadius->setValue( m_finishSectorInnerRadiusValue.getKilometers() );
      m_finishSectorOuterRadius->setValue( m_finishSectorOuterRadiusValue.getKilometers() );

      m_obsCircleRadius->setValue( m_obsCircleRadiusValue.getKilometers() );
      m_obsSectorInnerRadius->setValue( m_obsSectorInnerRadiusValue.getKilometers() );
      m_obsSectorOuterRadius->setValue( m_obsSectorOuterRadiusValue.getKilometers() );
    }
  else if( m_distUnit == Distance::miles ) // user gets miles
    {
      m_startLine->setValue( m_startLineValue.getMiles() );
      m_startRing->setValue( m_startRingValue.getMiles() );
      m_startSectorInnerRadius->setValue( m_startSectorInnerRadiusValue.getMiles() );
      m_startSectorOuterRadius->setValue( m_startSectorOuterRadiusValue.getMiles() );

      m_finishLine->setValue( m_finishLineValue.getMiles() );
      m_finishRing->setValue( m_finishRingValue.getMiles() );
      m_finishSectorInnerRadius->setValue( m_finishSectorInnerRadiusValue.getMiles() );
      m_finishSectorOuterRadius->setValue( m_finishSectorOuterRadiusValue.getMiles() );

      m_obsCircleRadius->setValue( m_obsCircleRadiusValue.getMiles() );
      m_obsSectorInnerRadius->setValue( m_obsSectorInnerRadiusValue.getMiles() );
      m_obsSectorOuterRadius->setValue( m_obsSectorOuterRadiusValue.getMiles() );
    }
  else // ( distUnit == Distance::nautmiles )
    {
      m_startLine->setValue( m_startLineValue.getNautMiles() );
      m_startRing->setValue( m_startRingValue.getNautMiles() );
      m_startSectorInnerRadius->setValue( m_startSectorInnerRadiusValue.getNautMiles() );
      m_startSectorOuterRadius->setValue( m_startSectorOuterRadiusValue.getNautMiles() );

      m_finishLine->setValue( m_finishLineValue.getNautMiles() );
      m_finishRing->setValue( m_finishRingValue.getNautMiles() );
      m_finishSectorInnerRadius->setValue( m_finishSectorInnerRadiusValue.getNautMiles() );
      m_finishSectorOuterRadius->setValue( m_finishSectorOuterRadiusValue.getNautMiles() );

      m_obsCircleRadius->setValue( m_obsCircleRadiusValue.getNautMiles() );
      m_obsSectorInnerRadius->setValue( m_obsSectorInnerRadiusValue.getNautMiles() );
      m_obsSectorOuterRadius->setValue( m_obsSectorOuterRadiusValue.getNautMiles() );
    }

  // Check the validity of inner and outer sector.
  m_startSectorInnerRadius->setMaximum( m_startSectorOuterRadius->value() );
  m_finishSectorInnerRadius->setMaximum( m_finishSectorOuterRadius->value() );
  m_obsSectorInnerRadius->setMaximum( m_obsSectorOuterRadius->value() );

  m_startSectorAngle->setValue( m_startSectorAngleValue );
  m_finishSectorAngle->setValue( m_finishSectorAngleValue );
  m_obsSectorAngle->setValue( m_obsSectorAngleValue );
}

void SettingsPageTask::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setActiveTaskSwitchScheme( (GeneralConfig::ActiveTaskSwitchScheme) m_selectedSwitchScheme );
  conf->setTaskDrawShape( m_drawShape->isChecked() );
  conf->setTaskFillShape( m_fillShape->isChecked() );
  conf->setTaskShapeAlpha(m_transShape->value() );
  conf->setTaskPointAutoZoom( m_autoZoom->isChecked() );

  conf->setActiveTaskStartScheme( (GeneralConfig::ActiveTaskFigureScheme) m_selectedStartScheme );
  setDistanceValue( m_startLineValue, m_startLine );
  setDistanceValue( m_startRingValue, m_startRing );
  setDistanceValue( m_startSectorInnerRadiusValue, m_startSectorInnerRadius );
  setDistanceValue( m_startSectorOuterRadiusValue, m_startSectorOuterRadius );

  conf->setActiveTaskFinishScheme( (GeneralConfig::ActiveTaskFigureScheme) m_selectedFinishScheme );
  setDistanceValue( m_finishLineValue, m_finishLine );
  setDistanceValue( m_finishRingValue, m_finishRing );
  setDistanceValue( m_finishSectorInnerRadiusValue, m_finishSectorInnerRadius );
  setDistanceValue( m_finishSectorOuterRadiusValue, m_finishSectorOuterRadius );

  conf->setActiveTaskObsScheme( (GeneralConfig::ActiveTaskFigureScheme) m_selectedObsScheme );
  setDistanceValue( m_obsCircleRadiusValue, m_obsCircleRadius );
  setDistanceValue( m_obsSectorInnerRadiusValue, m_obsSectorInnerRadius );
  setDistanceValue( m_obsSectorOuterRadiusValue, m_obsSectorOuterRadius );

  conf->setTaskStartLineLength( m_startLineValue );
  conf->setTaskStartRingRadius( m_startRingValue  );
  conf->setTaskStartSectorIRadius( m_startSectorInnerRadiusValue );
  conf->setTaskStartSectorORadius( m_startSectorOuterRadiusValue );

  conf->setTaskFinishLineLength( m_finishLineValue );
  conf->setTaskFinishRingRadius( m_finishRingValue );
  conf->setTaskFinishSectorIRadius( m_finishSectorInnerRadiusValue );
  conf->setTaskFinishSectorORadius( m_finishSectorOuterRadiusValue );

  conf->setTaskObsCircleRadius( m_obsCircleRadiusValue );
  conf->setTaskObsSectorInnerRadius( m_obsSectorInnerRadiusValue );
  conf->setTaskObsSectorOuterRadius( m_obsSectorOuterRadiusValue );

  conf->setTaskStartSectorAngel( m_startSectorAngle->value() );
  conf->setTaskFinishSectorAngel( m_finishSectorAngle->value() );
  conf->setTaskObsSectorAngle( m_obsSectorAngle->value() );

  conf->save();
}

void SettingsPageTask::setDistanceValue( Distance& dest, DoubleNumberEditor* src)
{
  if( m_distUnit == Distance::kilometers ) // user gets kilometers
    {
      dest.setKilometers( src->value() );
    }
  else if( m_distUnit == Distance::miles ) // user gets miles
    {
      dest.setMiles( src->value() );
    }
  else // ( distUnit == Distance::nautmiles )
    {
      dest.setNautMiles( src->value() );
    }
}
