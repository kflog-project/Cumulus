/***********************************************************************
**
**   settingspagelooknfeel.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2009-2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * This class represents the personal style settings.
 */

#include <QtGui>

#include "generalconfig.h"
#include "mainwindow.h"
#include "settingspagelooknfeel.h"
#include "varspinbox.h"

SettingsPageLookNFeel::SettingsPageLookNFeel(QWidget *parent) :
  QWidget(parent),
  m_loadConfig(true),
  m_currentFont(""),
  m_currentMenuFont(""),
  m_autoSip(true)
{
  setObjectName("SettingsPageLookNFeel");

  // get current used horizontal speed m_unit. This m_unit must be considered
  // during storage.
  m_unit = Speed::getHorizontalUnit();

  QGridLayout* topLayout = new QGridLayout(this);
  int row=0;

  QLabel * lbl = new QLabel(tr("GUI Style:"), this);
  topLayout->addWidget( lbl, row, 0 );
  styleBox = new QComboBox(this);
  topLayout->addWidget(styleBox, row, 1 );
  row++;

  // Put available styles in combo box
  QStringList styles = QStyleFactory::keys();
  QString style;

  foreach( style, styles )
    {
      styleBox->addItem(style);
    }

  lbl = new QLabel(tr("GUI Font:"), this);
  topLayout->addWidget(lbl, row, 0);
  fontDialog = new QPushButton(tr("Select Font"));
  fontDialog->setObjectName("fontDialog");
  topLayout->addWidget( fontDialog, row, 1 );

  connect(fontDialog, SIGNAL(clicked()), this, SLOT(slot_openFontDialog()));
  row++;

  lbl = new QLabel(tr("GUI Menu Font:"), this);
  topLayout->addWidget(lbl, row, 0);
  menuFontDialog = new QPushButton(tr("Select Font"));
  menuFontDialog->setObjectName("menuFontDialog");
  topLayout->addWidget( menuFontDialog, row, 1 );

  connect(menuFontDialog, SIGNAL(clicked()), this, SLOT(slot_openMenuFontDialog()));
  row++;

  lbl = new QLabel(tr("Map sidebar color:"), this);
  topLayout->addWidget(lbl, row, 0);
  editMapFrameColor = new QPushButton(tr("Select Color"));
  topLayout->addWidget( editMapFrameColor, row, 1 );
  connect(editMapFrameColor, SIGNAL(clicked()), this, SLOT(slot_openColorDialog()));
  row++;

  lbl = new QLabel(tr("Screensaver on:"), this);
  topLayout->addWidget(lbl, row, 0);
  screenSaverSpeedLimit = new QDoubleSpinBox( this );
  screenSaverSpeedLimit->setButtonSymbols(QSpinBox::PlusMinus);
  screenSaverSpeedLimit->setRange( 1.0, 99.0);
  screenSaverSpeedLimit->setSingleStep( 1 );
  screenSaverSpeedLimit->setPrefix( "< " );
  screenSaverSpeedLimit->setDecimals( 1 );
  screenSaverSpeedLimit->setSuffix( QString(" ") + Speed::getHorizontalUnitText() );
  VarSpinBox* hspin = new VarSpinBox( screenSaverSpeedLimit );
  topLayout->addWidget( hspin, row, 1 );
  row++;

#if 0
  virtualKeybord = new QCheckBox(tr("Virtual Keyboard"), this);
  virtualKeybord->setObjectName("VirtualKeyboard");
  virtualKeybord->setChecked(false);
  topLayout->addWidget( virtualKeybord, row, 0 );
  row++;
#endif

  topLayout->setRowStretch( row, 10 );
  topLayout->setColumnStretch( 2, 10 );
}

SettingsPageLookNFeel::~SettingsPageLookNFeel()
{
}

void SettingsPageLookNFeel::showEvent( QShowEvent* )
{
  // Switch off automatic software input panel popup
  m_autoSip = qApp->autoSipEnabled();
  qApp->setAutoSipEnabled( false );
}

void SettingsPageLookNFeel::hideEvent( QHideEvent* )
{
  qApp->setAutoSipEnabled( m_autoSip );
}

/** Called to initiate loading of the configuration items. */
void SettingsPageLookNFeel::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();
  m_currentFont         = conf->getGuiFont();
  m_currentMenuFont     = conf->getGuiMenuFont();

  // search item to be selected
  int idx = styleBox->findText( conf->getGuiStyle() );

  if( idx != -1 )
    {
      styleBox->setCurrentIndex(idx);
    }

  m_currentMapFrameColor = conf->getMapFrameColor();

  Speed speed;
  // speed is stored in Km/h
  speed.setKph( GeneralConfig::instance()->getScreenSaverSpeedLimit() );
  screenSaverSpeedLimit->setValue( speed.getValueInUnit( m_unit ) );
  // save loaded value for change control
  m_loadedSpeed = screenSaverSpeedLimit->value();

  // virtualKeybord->setChecked( conf->getVirtualKeyboard() );
}

/** called to initiate saving to the configuration items */
void SettingsPageLookNFeel::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  if( conf->getGuiFont() != m_currentFont )
    {
      conf->setGuiFont( m_currentFont );
    }

  if( conf->getGuiMenuFont() != m_currentMenuFont )
    {
      conf->setGuiMenuFont( m_currentMenuFont );
    }

  if( conf->getGuiStyle() != styleBox->currentText() )
    {
      conf->setGuiStyle( styleBox->currentText() );
      conf->setOurGuiStyle();
    }

  if( conf->getMapFrameColor() != m_currentMapFrameColor )
    {
      conf->setMapFrameColor( m_currentMapFrameColor );
    }

  if( m_loadedSpeed != screenSaverSpeedLimit->value() )
    {
      Speed speed;
      speed.setValueInUnit( screenSaverSpeedLimit->value(), m_unit );

      // store speed in Km/h
      GeneralConfig::instance()->setScreenSaverSpeedLimit( speed.getKph() );
    }

  // Note! enabling/disabling requires GUI restart
  // conf->setVirtualKeyboard( virtualKeybord->isChecked() );
}

/**
 * Called to ask is confirmation on the close is needed.
 */
void SettingsPageLookNFeel::slot_query_close( bool& warn, QStringList& warnings )
{
  /* set warn to 'true' if the data has changed. Note that we can NOT
     just set warn equal to _changed, because that way we might erase
     a warning flag set by another page! */

  GeneralConfig * conf = GeneralConfig::instance();
  bool changed=false;

  changed |= conf->getGuiFont() != m_currentFont;
  changed |= conf->getGuiMenuFont() != m_currentMenuFont;
  changed |= conf->getGuiStyle() != styleBox->currentText();
  // changed |= conf->getVirtualKeyboard() != virtualKeybord->isChecked();
  changed |= conf->getMapFrameColor() != m_currentMapFrameColor;
  changed |= m_loadedSpeed != screenSaverSpeedLimit->value() ;

  if (changed)
    {
      warn=true;
      warnings.append(tr("The Look&Feel settings"));
    }
}

/** Called to open the font dialog */
void SettingsPageLookNFeel::slot_openFontDialog()
{
  bool ok = false;

  QFont currFont;

  if( ! m_currentFont.isEmpty() )
    {
      ok = currFont.fromString( m_currentFont );
    }

  QFontDialog fd( this );
  fd.setWindowTitle(tr("GUI Font"));

  if( ok )
    {
      // preselect current active font
      fd.setCurrentFont( currFont );
    }
  else
    {
      fd.setCurrentFont( font() );
    }

#ifdef ANDROID
  fd.setVisible(true);
  fd.resize( MainWindow::mainWindow()->size() );
#endif

  if( fd.exec() == QDialog::Accepted )
    {
     // the user clicked OK and font is set to the font the user selected
      m_currentFont = fd.selectedFont().toString();

     // Set the new GUI font for all widgets. Note this new font
     // is only set temporary. The user must save it for permanent
     // usage.
     QApplication::setFont( fd.selectedFont() );
    }
  else
    {
      // the user clicked cancel, reset m_currentFont variable
      m_currentFont = GeneralConfig::instance()->getGuiFont();
    }
}

/** Called to open the menu font dialog */
void SettingsPageLookNFeel::slot_openMenuFontDialog()
{
  bool ok = false;

  QFont currFont;

  if( ! m_currentMenuFont.isEmpty() )
    {
      ok = currFont.fromString( m_currentMenuFont );
    }

  QFontDialog fd( this );

  if( ok )
    {
      // preselect current active font
      fd.setCurrentFont( currFont );
      fd.setWindowTitle(tr("GUI Menu Font"));
    }
  else
    {
      fd.setCurrentFont( font() );
    }

#ifdef ANDROID
  fd.setVisible(true);
  fd.resize( MainWindow::mainWindow()->size() );
#endif

  if( fd.exec() == QDialog::Accepted )
    {
     // the user clicked OK and menu font is set to the font the user selected
      m_currentMenuFont = fd.selectedFont().toString();
    }
  else
    {
      // the user clicked cancel, reset m_currentMenuFont variable
      m_currentMenuFont = GeneralConfig::instance()->getGuiMenuFont();
    }
}

/** Called to open the color dialog */
void SettingsPageLookNFeel::slot_openColorDialog()
{
  // get current color
  QColor& color = GeneralConfig::instance()->getMapFrameColor();

  // Open color chooser dialog to edit selected color
  QString title = tr("Map sidebar color");

  QColor newColor = QColorDialog::getColor( color, this, title );

  if( newColor.isValid() && color != newColor )
    {
      // save color into temporary buffer
      m_currentMapFrameColor = newColor;
    }
}
