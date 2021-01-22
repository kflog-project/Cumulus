/***********************************************************************
**
**   settingspageterraincolors.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009-2018 by Axel Pauli
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

#include "altitude.h"
#include "colordialog.h"
#include "elevationcolorimage.h"
#include "generalconfig.h"
#include "helpbrowser.h"
#include "layout.h"
#include "mainwindow.h"
#include "settingspageterraincolors.h"
#include "varspinbox.h"

SettingsPageTerrainColors::SettingsPageTerrainColors(QWidget *parent) :
  QWidget(parent),
  colorsChanged(false),
  m_autoSip( true )
{
  setObjectName("SettingsPageTerrainColors");

  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Terrain Colors") );

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

  /**
   * Altitude levels in meters to be displayed in color combo box.
   */
  const char *altitudes[51] = {
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

  // Determine pixmap size to be used for icons in dependency of the used font
  int size = QFontMetrics(font()).boundingRect("XM").height() - 2;
  pixmapSize = QSize( size, size );
  QPixmap pixmap(pixmapSize);

  // load stored terrain colors into working list
  for( int i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
    {
      QColor color = GeneralConfig::instance()->getTerrainColor(i);
      terrainColor[i] = color;
    }

  // load ground color
  groundColor = GeneralConfig::instance()->getGroundColor();

  // put all widgets in a HBox layout
  QHBoxLayout *topLayout = new QHBoxLayout(sw);

  // create elevation color bar as image
  elevationImage = new ElevationColorImage( &terrainColor[0], this );
  topLayout->addWidget( elevationImage );

  // all editor widgets will be put into a group box to get a better view
  QGroupBox *editBox = new QGroupBox( tr("Color Selection"), this );

  // put group box in an extra VBox layout to center it vertically
  QVBoxLayout *editAll = new QVBoxLayout;
  editAll->addStretch( 10 );
  editAll->addWidget( editBox );
  editAll->addStretch( 10 );

  topLayout->addLayout( editAll );

  // put all edit widgets (combo box and buttons) in a separate VBox layout
  QVBoxLayout *editLayout = new QVBoxLayout;
  editLayout->setSpacing( editLayout->spacing() * Layout::getIntScaledDensity() );

  QLabel *label = new QLabel( tr("Terrain Level") );
  editLayout->addWidget( label );

  //--------------------------------------------------------------------------
  // The users altitude unit (meters/feed) must be considered during
  // elevation display in the combo box.
  QString unit;

  elevationBox = new QComboBox( this );

#ifdef ANDROID
  QAbstractItemView* listView = elevationBox->view();
  QScrollBar* lvsb = listView->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  elevationBox->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  QScroller::grabGesture( elevationBox->view()->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  elevationBox->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  QtScroller::grabGesture( elevationBox->view()->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  if( Altitude::getUnit() == Altitude::meters )
    {
      // use unit meter
       unit = "m";

       for( int i = SIZEOF_TERRAIN_COLORS-1; i > 1; i-- )
        {
          pixmap.fill( terrainColor[i] );
          elevationBox->addItem( QIcon( pixmap ), QString(altitudes[i]) + unit );
        }
    }
  else
    {
      // use unit feed
      unit = "ft";

      for( int i = SIZEOF_TERRAIN_COLORS-1; i > 1; i-- )
        {
          int altFeed = static_cast<int>(QString(altitudes[i]).toDouble() * 3.28095);
          pixmap.fill( terrainColor[i] );
          elevationBox->addItem( QIcon( pixmap ), QString::number(altFeed) + unit );
        }
    }

  pixmap.fill( terrainColor[1] );
  elevationBox->addItem( QIcon( pixmap ), QString(altitudes[1]) );

  pixmap.fill( terrainColor[0] );
  elevationBox->addItem( QIcon( pixmap ), QString(altitudes[0]) );

  // set index to level 0
  elevationBox->setCurrentIndex( SIZEOF_TERRAIN_COLORS-2 );

  editLayout->addWidget( elevationBox );

  //--------------------------------------------------------------------------
  // add push button for elevation color chooser dialog
  editColorButton = new QPushButton( tr("Terrain Color") );

  // on click the color chooser dialog will be opened
  connect( editColorButton, SIGNAL(clicked()), this, SLOT(slot_editColor()) );

  editLayout->addWidget( editColorButton );

  //--------------------------------------------------------------------------
  // add push button for ground color chooser dialog
  groundColorButton = new QPushButton( tr("Ground Color") );

  pixmap.fill( groundColor );
  groundColorButton->setIcon( QIcon(pixmap) );

  // on click the color chooser dialog will be opened
  connect( groundColorButton, SIGNAL(clicked()), this, SLOT(slot_editGroundColor()) );

  editLayout->addSpacing( 10 * Layout::getIntScaledDensity() );
  editLayout->addWidget( groundColorButton );
  editLayout->addSpacing( 20 * Layout::getIntScaledDensity() );

  //--------------------------------------------------------------------------
  // add button for assigning of default colors
  defaultColorButton = new QPushButton( tr("Color Defaults") );

  // on click all colors are reset to the defaults
  connect( defaultColorButton, SIGNAL(clicked()), this, SLOT(slot_setColorDefaults()) );

  editLayout->addWidget( defaultColorButton );

  // add stretch items to posit editor widgets in the center of the VBox layout
  editLayout->insertStretch(0, 10 );
  editLayout->addStretch( 10 );

  editBox->setLayout(editLayout);

  //--------------------------------------------------------------------------
  // add spin box for moving elevation zero line
  QGroupBox *setOffsetBox = new QGroupBox( tr("Elevation Offset"), this );

  // put group box in an extra VBox layout to center it vertically
  QVBoxLayout *offsetLayout = new QVBoxLayout;
  offsetLayout->addStretch( 10 );
  offsetLayout->addWidget( setOffsetBox );
  offsetLayout->addStretch( 10 );

  QVBoxLayout *spinboxLayout = new QVBoxLayout;

  elevationOffset = new QSpinBox;
  elevationOffset->setSingleStep(1);
  elevationOffset->setRange(-50, 50);

  connect( elevationOffset, SIGNAL(editingFinished()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  VarSpinBox* hspin = new VarSpinBox( elevationOffset );
  spinboxLayout->addWidget(hspin);
  setOffsetBox->setLayout(spinboxLayout);

  topLayout->addLayout( offsetLayout );
  topLayout->insertSpacing(1, 60 );
  topLayout->addStretch( 10 );

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
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);

  load();
}

SettingsPageTerrainColors::~SettingsPageTerrainColors()
{
}

void SettingsPageTerrainColors::showEvent( QShowEvent *event )
{
  // Switch off automatic software input panel popup
  m_autoSip = qApp->autoSipEnabled();
  qApp->setAutoSipEnabled( false );

  QWidget::showEvent( event );
}

void SettingsPageTerrainColors::hideEvent( QHideEvent *)
{
  qApp->setAutoSipEnabled( m_autoSip );
}

void SettingsPageTerrainColors::slotHelp()
{
  QString file = "cumulus-settings-terrain.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void SettingsPageTerrainColors::slotAccept()
{
  if( colorsChanged ||
      GeneralConfig::instance()->getElevationColorOffset() != elevationOffset->value() )
    {
      save();
      emit settingsChanged();
    }

  QWidget::close();
}

void SettingsPageTerrainColors::slotReject()
{
  QWidget::close();
}

/**
 * Called to edit the color of the combo box selection
 */
void SettingsPageTerrainColors::slot_editColor()
{
  int index = SIZEOF_TERRAIN_COLORS - 1 - elevationBox->currentIndex();

  // get current selected color
  QColor& color = terrainColor[index];

  // Enable software input panel for color dialog
  qApp->setAutoSipEnabled( true );

  // Open color chooser dialog to edit selected color
  QString title = tr("Terrain Level") + " " + elevationBox->currentText();
  QColor newColor = ColorDialog::getColor( color, this, title );

  if( newColor.isValid() && color != newColor )
    {
      colorsChanged = true;
      // save color into working list
      terrainColor[index] = newColor;
      // update icon in elevation box
      QPixmap pixmap( pixmapSize );
      pixmap.fill( newColor );
      elevationBox->setItemIcon( elevationBox->currentIndex(), QIcon(pixmap) );
      // update color in elevation image
      elevationImage->update();
    }

  qApp->setAutoSipEnabled( m_autoSip );
}

/**
 * Called to edit the ground color
 */
void SettingsPageTerrainColors::slot_editGroundColor()
{
  // Enable software input panel for color dialog
  qApp->setAutoSipEnabled( true );

  // Open color chooser dialog to edit ground color
  QString title = tr("Ground Color");
  QColor newColor = ColorDialog::getColor( groundColor, this, title );

  if( newColor.isValid() && groundColor != newColor )
    {
      colorsChanged = true;
      // save color
      groundColor = newColor;
      // update icon of ground color button
      QPixmap pixmap( pixmapSize );
      pixmap.fill( newColor );
      groundColorButton->setIcon( QIcon(pixmap) );
    }

  qApp->setAutoSipEnabled( m_autoSip );
}

/**
 * Called to set all colors to their default value.
 */
void SettingsPageTerrainColors::slot_setColorDefaults()
{
  colorsChanged = true;

  // reset ground color
  groundColor = COLOR_LEVEL_GROUND;
  QPixmap pixmap( pixmapSize );
  pixmap.fill( groundColor );
  groundColorButton->setIcon( QIcon(pixmap) );

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

  // update icons in elevation box
  for( int i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
   {
     pixmap.fill( terrainColor[i] );
     elevationBox->setItemIcon( SIZEOF_TERRAIN_COLORS-1-i, QIcon(pixmap) );
   }

  // update colors in elevation image
  elevationImage->update();
}

/**
 * Called to initiate saving to the configuration file.
 */
void SettingsPageTerrainColors::save()
{
  if( colorsChanged )
    {
      for( ushort i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
        {
          // save new terrain colors permanently
          GeneralConfig::instance()->setTerrainColor( terrainColor[i], i );
        }

      // save ground color
      GeneralConfig::instance()->setGroundColor( groundColor );
    }

  GeneralConfig::instance()->setElevationColorOffset( elevationOffset->value() );
  GeneralConfig::instance()->save();
}

/**
 * Called to initiate loading of the configuration file
 */
void SettingsPageTerrainColors::load()
{
  elevationOffset->setValue( GeneralConfig::instance()->getElevationColorOffset() );
}
