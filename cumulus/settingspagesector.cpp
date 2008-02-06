/***********************************************************************
**
**   settingspagesector.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**

**   Copyright (c):  2007, 2008 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QRadioButton>
#include <QLabel>
#include <QGridLayout>
#include <Q3Grid>

#include "settingspagesector.h"
#include "generalconfig.h"
#include "flighttask.h"
#include "mapcontents.h"

extern MapContents  *_globalMapContents;

/**
 * @short Configuration settings for start, turn and end points of a
 * task
 *
 * @author Axel Pauli
 */

// Constructor of class
SettingsPageSector::SettingsPageSector( QWidget *parent, const char *name ) :
  QWidget( parent, name ),
  loadedCylinderRadius(0),
  loadedInnerSectorRadius(0),
  loadedOuterSectorRadius(0)
{
  GeneralConfig *conf = GeneralConfig::instance();

  QGridLayout *topLayout = new QGridLayout( this, 3, 1, 3, 0 );
  
  Q3Grid *scheme = new Q3Grid( 2, Qt::Horizontal, this, "Scheme" );
  scheme->setSpacing( 5 );
  topLayout->addWidget( scheme, 0, 0 );

  csScheme = new Q3ButtonGroup( 2, Qt::Vertical, tr("TP Scheme"),
                                scheme, "csScheme" );
  
  QRadioButton* cylinder = new QRadioButton( tr("Cylinder"), csScheme );
  QRadioButton* sector   = new QRadioButton( tr("Sector"), csScheme );
  
  cylinder->setMaximumHeight(11);
  sector->setMaximumHeight(11);
  cylinder->setEnabled(true);
  sector->setEnabled(true);
  sector->setChecked(true);

  // set active button as selected
  selectedCSScheme = (int) conf->getActiveCSTaskScheme();
  csScheme->setButton( selectedCSScheme );

  //---------------------------------------------------------------

  ntScheme = new Q3ButtonGroup( 2, Qt::Vertical, tr("Switch Scheme"),
                                scheme, "ntScheme" );

  QRadioButton* nearst   = new QRadioButton( tr("Nearst"), ntScheme );
  QRadioButton* touched  = new QRadioButton( tr("Touched"), ntScheme );
 
  nearst->setMaximumHeight(11);
  touched->setMaximumHeight(11);
  nearst->setEnabled(true);
  touched->setEnabled(true);
  touched->setChecked(true);

  // set active button as selected
  selectedNTScheme = (int) conf->getActiveNTTaskScheme();
  ntScheme->setButton( selectedNTScheme );

  //--------------------------------------------------------------
  
  // as next cylinder group is added
  cylinderGroup = new Q3GroupBox( tr("Cylinder"), this );
  topLayout->addWidget( cylinderGroup, 1, 0 ); 
  
  QGridLayout *cylinderLayout = new QGridLayout( cylinderGroup, 5, 4, 10, 2 );

  int row = 0;
  cylinderLayout->addRowSpacing( row, 4 );
  row++;
  QLabel *lbl = new QLabel( tr("Radius:"), cylinderGroup );
  cylinderLayout->addWidget( lbl, row, 0 );
  cylinderLayout->addRowSpacing( row, 20 );
  
  cylinderRadius = new QDoubleSpinBox( cylinderGroup );
  cylinderRadius->setRange(0.1, 10.0);
  cylinderRadius->setSingleStep(0.1);
  cylinderRadius->setButtonSymbols(QSpinBox::PlusMinus);
  cylinderLayout->addWidget( cylinderRadius, row, 1 );
  
  // get current distance unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  distUnit = Distance::getUnit();

  const char *unit = "";

  // Input accepts different units 
  if( distUnit == Distance::kilometers )
    {
      unit = "km";
    }
  else if( distUnit == Distance::miles )
    {
      unit = "ml";
    }
  else // if( distUnit == Distance::nautmiles )
    {
      unit = "nm";
    }

  cylinderLayout->addWidget( new QLabel( unit, cylinderGroup ), row, 2 );

  // as next sector group is added
  sectorGroup = new Q3GroupBox( tr("Sector"), this );
  topLayout->addWidget( sectorGroup, 2, 0 ); 
  
  QGridLayout *sectorLayout = new QGridLayout( sectorGroup, 5, 4, 10, 2 );

  row = 0;
  sectorLayout->addRowSpacing( row, 4 );
  row++;

  lbl = new QLabel( tr("Inner Radius:"), sectorGroup );
  sectorLayout->addWidget( lbl, row, 0 );
  sectorLayout->addRowSpacing( row, 15 );
  innerSectorRadius = new QDoubleSpinBox( sectorGroup );
  innerSectorRadius->setRange(0.0, 10.0);
  innerSectorRadius->setSingleStep(0.1);
  innerSectorRadius->setButtonSymbols(QSpinBox::PlusMinus);
  sectorLayout->addWidget( innerSectorRadius, row, 1 );
  sectorLayout->addWidget( new QLabel( unit, sectorGroup), row, 2 );

  row++;
  lbl = new QLabel( tr("Outer Radius:"), sectorGroup );
  sectorLayout->addWidget( lbl, row, 0 );
  outerSectorRadius = new QDoubleSpinBox( sectorGroup );
  outerSectorRadius->setRange(0.1, 10.0);
  outerSectorRadius->setSingleStep(0.1);
  outerSectorRadius->setButtonSymbols(QSpinBox::PlusMinus);
  sectorLayout->addWidget( outerSectorRadius, row, 1 );
  sectorLayout->addWidget( new QLabel( unit, sectorGroup), row, 2 );

  row++;
  lbl = new QLabel( tr("Angle:"), sectorGroup );
  sectorLayout->addWidget( lbl, row, 0 );
  sectorAngle = new QSpinBox( 90, 180, 5, sectorGroup );
  sectorAngle->setButtonSymbols(QSpinBox::PlusMinus);
  sectorAngle->setWrapping( true );
  sectorLayout->addWidget( sectorAngle, row, 1 );
  sectorLayout->addWidget( new QLabel( tr("degree"), sectorGroup), row, 2 );
  sectorAngle->setValue( conf->getTaskSectorAngle() );

  // as next shape group is added
  shapeGroup = new Q3GroupBox( 2, Qt::Horizontal, tr("Shape"), this );
  topLayout->addWidget( shapeGroup, 3, 0 );

  drawShape = new QCheckBox( tr("Draw Shape"), shapeGroup, "Draw Shape" );
  drawShape->setMaximumHeight( 12 );
  fillShape = new QCheckBox( tr("Fill Shape"), shapeGroup, "Fill Shape" );
  fillShape->setMaximumHeight( 12 );

  drawShape->setChecked( conf->getTaskDrawShape() );
  fillShape->setChecked( conf->getTaskFillShape() );

  connect( csScheme, SIGNAL(clicked(int)), this, SLOT(slot_buttonPressedCS(int)) );
  connect( ntScheme, SIGNAL(clicked(int)), this, SLOT(slot_buttonPressedNT(int)) );
  connect( outerSectorRadius, SIGNAL(valueChanged(double)), this, SLOT(slot_outerSBChanged(double)) );
}

// Destructor of class
SettingsPageSector::~SettingsPageSector()
{
}

// value of outer spin box changed
void SettingsPageSector::slot_outerSBChanged( double /* value */ )
{
  // set max range of inner radius spin box to current value of this box
  innerSectorRadius->setMaximum( outerSectorRadius->value() );

  if( innerSectorRadius->value() > outerSectorRadius->value() )
    {
      // inner spin box value must be less equal outer spin box value
      innerSectorRadius->setValue( outerSectorRadius->value() );
    }
}

// Set the passed scheme widget as active and the other one to
// inactive
void SettingsPageSector::slot_buttonPressedCS( int newScheme )
{
  selectedCSScheme = newScheme;

  if( newScheme == GeneralConfig::Sector )
    {
      sectorGroup->setEnabled(true);
      cylinderGroup->setEnabled(false);
    }
  else
    {
      sectorGroup->setEnabled(false);
      cylinderGroup->setEnabled(true);
    }
}

void SettingsPageSector::slot_buttonPressedNT( int newScheme )
{
  selectedNTScheme = newScheme;
}

void SettingsPageSector::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  slot_buttonPressedCS( (int) conf->getActiveCSTaskScheme() );

  // @AP: radius is always fetched in meters.
  Distance cRadius = conf->getTaskCylinderRadius();
  Distance iRadius = conf->getTaskSectorInnerRadius();
  Distance oRadius = conf->getTaskSectorOuterRadius();

  if( distUnit == Distance::kilometers ) // user gets meters
    {
      cylinderRadius->setValue( cRadius.getKilometers() );
      innerSectorRadius->setValue( iRadius.getKilometers() );
      outerSectorRadius->setValue( oRadius.getKilometers() );
    }
  else if( distUnit == Distance::miles ) // user gets miles
    {
      cylinderRadius->setValue( cRadius.getMiles() );
      innerSectorRadius->setValue( iRadius.getMiles() );
      outerSectorRadius->setValue( oRadius.getMiles() );
    }
  else // ( distUnit == Distance::nautmiles )
    {
      cylinderRadius->setValue( cRadius.getNautMiles() );
      innerSectorRadius->setValue( iRadius.getNautMiles() );
      outerSectorRadius->setValue( oRadius.getNautMiles() );
    }

  // Save loaded integer values of spin boxes
  loadedCylinderRadius = cylinderRadius->value();
  loadedInnerSectorRadius = innerSectorRadius->value();
  loadedOuterSectorRadius = outerSectorRadius->value();

  drawShape->setChecked( conf->getTaskDrawShape() );
  fillShape->setChecked( conf->getTaskFillShape() );
}

void SettingsPageSector::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setActiveCSTaskScheme( (GeneralConfig::ActiveCSTaskScheme) selectedCSScheme );
  conf->setActiveNTTaskScheme( (GeneralConfig::ActiveNTTaskScheme) selectedNTScheme );

  // @AP: radius is always saved in meters. Save is done only after a
  // real change to avoid rounding errors.
  Distance cRadius;
  Distance iRadius;
  Distance oRadius;

  if( loadedCylinderRadius != cylinderRadius->value() )
    {
      if( distUnit == Distance::kilometers ) // user gets kilometers
        {
          cRadius.setKilometers( cylinderRadius->value() );
        }
      else if( distUnit == Distance::miles ) // user gets miles
        {
          cRadius.setMiles( cylinderRadius->value() );
        }
      else // ( distUnit == Distance::nautmiles )
        {
          cRadius.setNautMiles( cylinderRadius->value() );
        }

      conf->setTaskCylinderRadius( cRadius );
    }

  if( loadedInnerSectorRadius != innerSectorRadius->value() )
    {
      if( distUnit == Distance::kilometers ) // user gets kilometers
        {
          iRadius.setKilometers( innerSectorRadius->value() );
        }
      else if( distUnit == Distance::miles ) // user gets miles
        {
          iRadius.setMiles( innerSectorRadius->value() );
        }
      else // ( distUnit == Distance::nautmiles )
        {
          iRadius.setNautMiles( innerSectorRadius->value() );
        }
      
      conf->setTaskSectorInnerRadius( iRadius );
    }

  if( loadedOuterSectorRadius != outerSectorRadius->value() )
    {
      if( distUnit == Distance::kilometers ) // user gets kilometers
        {
          oRadius.setKilometers( outerSectorRadius->value() );
        }
      else if( distUnit == Distance::miles ) // user gets miles
        {
          oRadius.setMiles( outerSectorRadius->value() );
        }
      else // ( distUnit == Distance::nautmiles )
        {
          oRadius.setNautMiles( outerSectorRadius->value() ); 
        }

      conf->setTaskSectorOuterRadius( oRadius );
    }

  conf->setTaskSectorAngle( sectorAngle->value() );

  conf->setTaskDrawShape( drawShape->isChecked() );
  conf->setTaskFillShape( fillShape->isChecked() );

  // If a flight task is set, we must update the sector angles in it
   FlightTask *task = _globalMapContents->getCurrentTask();

   if( task != 0 )
     {
       task->updateTask();
     }
}
