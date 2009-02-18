/***********************************************************************
**
**   settingspagelooknfeel.cpp
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

/**
 * This class represents the personal style settings.
 */

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QToolTip>
#include <QStyleFactory>
#include <QStringList>
#include <QApplication>
#include <QFontDialog>

#include "generalconfig.h"
#include "settingspagelooknfeel.h"

SettingsPageLookNFeel::SettingsPageLookNFeel(QWidget *parent) :
  QWidget(parent), loadConfig(true), currentFont("")
{
  setObjectName("SettingsPageLookNFeel");

  QGridLayout* topLayout = new QGridLayout(this);
  int row=0;

  QLabel * lbl = new QLabel(tr("GUI Style:"), this);
  topLayout->addWidget( lbl, row, 0 );
  styleBox = new QComboBox(this);
  topLayout->addWidget(styleBox, row, 1 );
  row++;

  // Put available syles in combobox
  QStringList styles = QStyleFactory::keys();
  QString style;

  foreach( style, styles )
  {
    styleBox->addItem(style);
  }

  lbl = new QLabel(tr("GUI Fonts:"), this);
  topLayout->addWidget(lbl, row, 0);
  fontDialog = new QPushButton(tr("Show Fonts"));
  fontDialog->setObjectName("fontDialog");
  topLayout->addWidget( fontDialog, row, 1 );

  connect(fontDialog, SIGNAL(clicked()), this, SLOT(slot_openFontDialog()));
  row++;

  lbl = new QLabel(tr("Sidebar frame color:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtFrameCol = new QLineEdit(this);
  topLayout->addWidget( edtFrameCol, row, 1 );
  row++;

  virtualKeybord = new QCheckBox(tr("Virtual Keyboard"), this);
  virtualKeybord->setObjectName("VirtualKeyboard");
  virtualKeybord->setChecked(false);
  topLayout->addWidget( virtualKeybord, row, 0 );
  row++;

  topLayout->setRowStretch( row, 10 );
  topLayout->setColumnStretch( 2, 10 );
}

SettingsPageLookNFeel::~SettingsPageLookNFeel()
{}

/** Called to initiate loading of the configuration items. */
void SettingsPageLookNFeel::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  currentFont = conf->getGuiFont();

  edtFrameCol->setText( conf->getFrameCol() );

  // search item to be selected
  int idx = styleBox->findText( conf->getGuiStyle() );

  if( idx != -1 )
    {
      styleBox->setCurrentIndex(idx);
    }

  virtualKeybord->setChecked( conf->getVirtualKeyboard() );
}

/** called to initiate saving to the configuration items */
void SettingsPageLookNFeel::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  if( conf->getGuiFont() != currentFont )
    {
      conf->setGuiFont( currentFont );
    }

  conf->setFrameCol( edtFrameCol->text() );

  if( conf->getGuiStyle() != styleBox->currentText() )
    {
      conf->setGuiStyle( styleBox->currentText() );
      conf->setOurGuiStyle();
    }

  // Note! enabling/disabling requires GUI restart
  conf->setVirtualKeyboard( virtualKeybord->isChecked() );
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

  changed |= conf->getGuiFont() != currentFont;
  changed |= conf->getGuiStyle() != styleBox->currentText();
  changed |= conf->getVirtualKeyboard() != virtualKeybord->isChecked();

  if (changed)
    {
      warn=true;
      warnings.append(tr("Look&Feel settings"));
    }
}

/** Called to open the font dialog */
void SettingsPageLookNFeel::slot_openFontDialog()
{
  bool ok;

  QFont font = QFontDialog::getFont( &ok, this);

  if (ok)
    {
     // the user clicked OK and font is set to the font the user selected
      currentFont = font.toString();

     // Set the new GUI font for all widgets. Note this new font
     // is only set temporary. The user must save it for permanent
     // usage.
     QApplication::setFont( font );
    }
  else
    {
      // the user clicked cancel, reset currentFont variable
      currentFont = GeneralConfig::instance()->getGuiFont();
    }
}
