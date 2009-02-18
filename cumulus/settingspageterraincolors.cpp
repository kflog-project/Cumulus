/***********************************************************************
**
**   settingspageterraincolors.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QColorDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "altitude.h"
#include "generalconfig.h"
#include "settingspageterraincolors.h"
#include "elevationimage.h"

/**
 * @author Axel Pauli
 *
 * This configuration widget shows the terrain colors used for drawing of contour areas.
 * The user can modify via a color chooser dialog the assigned color, if he wants that.
 * The display shows the altitudes in the user selected unit (meter/feed).
 */

SettingsPageTerrainColors::SettingsPageTerrainColors(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("SettingsPageTerrainColors");

  colorsChanged = false;

  /**
   * Altitude levels to be displayed in color combo box
   */
  char *altitudes[51] = {
                 "< 0",
                 "0",
                 "10",
                 "25",
                 "50",
                 "75",
                 "100",
                 "150",
                 "200",
                 "250",
                 "300",
                 "350",
                 "400",
                 "450",
                 "500",
                 "600",
                 "700",
                 "800",
                 "900",
                 "1000",
                 "1250",
                 "1500",
                 "1750",
                 "2000",
                 "2250",
                 "2500",
                 "2750",
                 "3000",
                 "3250",
                 "3500",
                 "3750",
                 "4000",
                 "4250",
                 "4500",
                 "4750",
                 "5000",
                 "5250",
                 "5500",
                 "5750",
                 "6000",
                 "6250",
                 "6500",
                 "6750",
                 "7000",
                 "7250",
                 "7500",
                 "7750",
                 "8000",
                 "8250",
                 "8500",
                 "8750"
  };

  // load stored terrain colors into working list
  for( short i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
    {
      QColor color = GeneralConfig::instance()->getTerrainColor(i);
      terrainColor[i] = color; // load color into working list
    }

  QHBoxLayout *topLayout = new QHBoxLayout(this);

  elevationImage = new ElevationImage( &terrainColor[0], this );
  topLayout->addWidget( elevationImage );

  QVBoxLayout *editAll = new QVBoxLayout;

  QGroupBox *editBox = new QGroupBox( tr("Color Editor"), this );

  editAll->addStretch( 10 );
  editAll->addWidget( editBox );
  editAll->addStretch( 10 );

  topLayout->addLayout( editAll );

  QVBoxLayout *editLayout = new QVBoxLayout;

  QLabel *label = new QLabel( "Elevation level" );
  editLayout->addWidget( label );

  //--------------------------------------------------------------------------
  // The users altitude unit (meters/feed) must be considered during
  // altitude display in the combo box.
  QString unit;

  altitudeBox = new QComboBox( this );

  if( Altitude::getUnit() == Altitude::meters )
    {
      // use unit meter
       unit = "m";

       for( int i = SIZEOF_TERRAIN_COLORS-1; i > 1; i-- )
        {
          altitudeBox->addItem( QString(altitudes[i]) + unit );
        }
    }
  else
    {
      // use unit feed
      unit = "ft";

      for( int i = SIZEOF_TERRAIN_COLORS-1; i > 1; i-- )
        {
          int altFeed = static_cast<int>(QString(altitudes[i]).toDouble() * 3.28095);
          altitudeBox->addItem( QString::number(altFeed) + unit );
        }
    }

  altitudeBox->addItem( QString(altitudes[1]) );
  altitudeBox->addItem( QString(altitudes[0]) );

  // set index to level 0
  altitudeBox->setCurrentIndex( SIZEOF_TERRAIN_COLORS-2 );

  editLayout->addWidget( altitudeBox );

  //--------------------------------------------------------------------------
  // add push button for color chooser dialog
  editColorButton = new QPushButton( tr("Edit Color") );

  // on click the color chooser dialog will be opened
  connect( editColorButton, SIGNAL(clicked()), this, SLOT(slot_editColor()) );

  editLayout->addWidget( editColorButton );

  //--------------------------------------------------------------------------
  // add button for assigning of default colors
  defaultColorButton = new QPushButton( tr("Color Defaults") );

  // on click all colors are reset to the defaults
  connect( defaultColorButton, SIGNAL(clicked()), this, SLOT(slot_setColorDefaults()) );

  editLayout->addWidget( defaultColorButton );

  // add stretch items to posit buttons in the center of the widget
  editLayout->insertStretch(0, 10 );
  editLayout->addStretch( 10 );

  editBox->setLayout(editLayout);

  topLayout->insertSpacing(1, 60 );
  topLayout->addStretch( 10 );
}

SettingsPageTerrainColors::~SettingsPageTerrainColors()
{
}

/**
 * Called to edit the color of the combo box selection
 */
void SettingsPageTerrainColors::slot_editColor()
{
  int index = SIZEOF_TERRAIN_COLORS - 1 - altitudeBox->currentIndex();

  // save current selected color
  QColor color = terrainColor[index];

  // Open color chooser dialog to edit selected color
  QColor newColor = QColorDialog::getColor( color, this );

  if( newColor.isValid() && color != newColor )
    {
      colorsChanged = true;
      // save color into working list
      terrainColor[index] = newColor;
      // update color in elevation image
      elevationImage->update();
    }
}

/**
 * Called to set all colors to their default value.
 */
void SettingsPageTerrainColors::slot_setColorDefaults()
{
  colorsChanged = true;

  // reset colors in working list
  terrainColor[0] = COLOR_LEVEL_SUB;
  terrainColor[1] = COLOR_LEVEL_0;
  terrainColor[2] = COLOR_LEVEL_10;
  terrainColor[3] = COLOR_LEVEL_25;
  terrainColor[4] = COLOR_LEVEL_50;
  terrainColor[5] = COLOR_LEVEL_75;
  terrainColor[6] = COLOR_LEVEL_100;
  terrainColor[7] = COLOR_LEVEL_150;
  terrainColor[8] = COLOR_LEVEL_200;
  terrainColor[9] = COLOR_LEVEL_250;
  terrainColor[10] = COLOR_LEVEL_300;
  terrainColor[11] = COLOR_LEVEL_350;
  terrainColor[12] = COLOR_LEVEL_400;
  terrainColor[13] = COLOR_LEVEL_450;
  terrainColor[14] = COLOR_LEVEL_500;
  terrainColor[15] = COLOR_LEVEL_600;
  terrainColor[16] = COLOR_LEVEL_700;
  terrainColor[17] = COLOR_LEVEL_800;
  terrainColor[18] = COLOR_LEVEL_900;
  terrainColor[19] = COLOR_LEVEL_1000;
  terrainColor[20] = COLOR_LEVEL_1250;
  terrainColor[21] = COLOR_LEVEL_1500;
  terrainColor[22] = COLOR_LEVEL_1750;
  terrainColor[23] = COLOR_LEVEL_2000;
  terrainColor[24] = COLOR_LEVEL_2250;
  terrainColor[25] = COLOR_LEVEL_2500;
  terrainColor[26] = COLOR_LEVEL_2750;
  terrainColor[27] = COLOR_LEVEL_3000;
  terrainColor[28] = COLOR_LEVEL_3250;
  terrainColor[29] = COLOR_LEVEL_3500;
  terrainColor[30] = COLOR_LEVEL_3750;
  terrainColor[31] = COLOR_LEVEL_4000;
  terrainColor[32] = COLOR_LEVEL_4250;
  terrainColor[33] = COLOR_LEVEL_4500;
  terrainColor[34] = COLOR_LEVEL_4750;
  terrainColor[35] = COLOR_LEVEL_5000;
  terrainColor[36] = COLOR_LEVEL_5250;
  terrainColor[37] = COLOR_LEVEL_5500;
  terrainColor[38] = COLOR_LEVEL_5750;
  terrainColor[39] = COLOR_LEVEL_6000;
  terrainColor[40] = COLOR_LEVEL_6250;
  terrainColor[41] = COLOR_LEVEL_6500;
  terrainColor[42] = COLOR_LEVEL_6750;
  terrainColor[43] = COLOR_LEVEL_7000;
  terrainColor[44] = COLOR_LEVEL_7250;
  terrainColor[45] = COLOR_LEVEL_7500;
  terrainColor[46] = COLOR_LEVEL_7750;
  terrainColor[47] = COLOR_LEVEL_8000;
  terrainColor[48] = COLOR_LEVEL_8250;
  terrainColor[49] = COLOR_LEVEL_8500;
  terrainColor[50] = COLOR_LEVEL_8750;

  // update colors in elevation image
  elevationImage->update();
}

/**
 * Called to initiate saving to the configuration file.
 */
void SettingsPageTerrainColors::slot_save()
{
  if( ! colorsChanged )
    {
      // nothing to save
      return;
    }

  for( ushort i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
    {
      // save new colors permanently
      GeneralConfig::instance()->setTerrainColor( terrainColor[i], i );
    }
}

/**
 * Called to initiate loading of the configuration file
 */
void SettingsPageTerrainColors::slot_load()
{
  // nothing to do atm
}
/**
 * Called to ask is confirmation on the close is needed.
 */
void SettingsPageTerrainColors::slot_query_close(bool& warn, QStringList& warnings)
{
  if( colorsChanged )
    {
      warn = true;
      warnings.append(tr("The Terrain color settings"));
    }
}
