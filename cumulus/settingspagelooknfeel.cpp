/***********************************************************************
**
**   settingspagelooknfeel.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2009-2014 Axel Pauli
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

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "colordialog.h"
#include "doubleNumberEditor.h"
#include "fontdialog.h"
#include "generalconfig.h"
#include "layout.h"
#include "mainwindow.h"
#include "mapdefaults.h"
#include "settingspagelooknfeel.h"

SettingsPageLookNFeel::SettingsPageLookNFeel(QWidget *parent) :
  QWidget(parent),
  m_loadConfig(true),
  m_currentFont(""),
  m_currentMenuFont("")
{
  setObjectName("SettingsPageLookNFeel");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Look&Feel") );

  if( parent )
    {
      resize( parent->size() );
    }

  // Determine pixmap size to be used for icons in dependency of the used font
  int size = QFontMetrics(font()).boundingRect("XM").height() - 2;
  m_pixmapSize = QSize( size, size );

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

  // get current used horizontal speed m_unit. This m_unit must be considered
  // during storage.
  m_unit = Speed::getHorizontalUnit();

  int row=0;

  QLabel * lbl = new QLabel(tr("GUI Style:"), this);
  topLayout->addWidget( lbl, row, 0 );
  m_styleBox = new QComboBox(this);
  topLayout->addWidget(m_styleBox, row, 1 );
  row++;

  // Put available styles in combo box
  QStringList styles = QStyleFactory::keys();
  QString style;

  foreach( style, styles )
    {
      m_styleBox->addItem(style);
    }

  lbl = new QLabel(tr("GUI Font:"), this);
  topLayout->addWidget(lbl, row, 0);
  m_fontDialog = new QPushButton(tr("Select Font"));
  m_fontDialog->setObjectName("fontDialog");
  topLayout->addWidget( m_fontDialog, row, 1 );

  connect(m_fontDialog, SIGNAL(clicked()), this, SLOT(slot_openFontDialog()));
  row++;

  lbl = new QLabel(tr("GUI Menu Font:"), this);
  topLayout->addWidget(lbl, row, 0);
  m_menuFontDialog = new QPushButton(tr("Select Font"));
  m_menuFontDialog->setObjectName("menuFontDialog");
  topLayout->addWidget( m_menuFontDialog, row, 1 );

  connect(m_menuFontDialog, SIGNAL(clicked()), SLOT(slot_openMenuFontDialog()));
  row++;

  lbl = new QLabel(tr("Infobox frame color:"), this);
  topLayout->addWidget(lbl, row, 0);

  m_editMapFrameColor = new QPushButton(tr("Select Color"));
  topLayout->addWidget( m_editMapFrameColor, row, 1 );
  connect(m_editMapFrameColor, SIGNAL(clicked()), SLOT(slot_openColorDialog()));

  QPushButton* defaultButton = new QPushButton(tr("Default"));
  topLayout->addWidget( defaultButton, row, 2 );
  connect(defaultButton, SIGNAL(clicked()), SLOT(slot_defaultColor()));

  topLayout->setColumnStretch ( 3, 20 );
  row++;

  lbl = new QLabel(tr("Screensaver on:"), this);
  topLayout->addWidget(lbl, row, 0);

  m_screenSaverSpeedLimit = new DoubleNumberEditor( this );
  m_screenSaverSpeedLimit->setDecimalVisible( true );
  m_screenSaverSpeedLimit->setPmVisible( false );
  m_screenSaverSpeedLimit->setMaxLength(4);
  m_screenSaverSpeedLimit->setRange( 0.0, 99.9);
  m_screenSaverSpeedLimit->setSpecialValueText(tr("Off"));
  m_screenSaverSpeedLimit->setPrefix( "< " );
  m_screenSaverSpeedLimit->setSuffix( QString(" ") + Speed::getHorizontalUnitText() );
  m_screenSaverSpeedLimit->setDecimals( 1 );

  int mlw = QFontMetrics(font()).width("99.9" + Speed::getHorizontalUnitText()) + 10;
  m_screenSaverSpeedLimit->setMinimumWidth( mlw );
  topLayout->addWidget( m_screenSaverSpeedLimit, row, 1 );
  row++;

  topLayout->setRowStretch( row, 10 );
  topLayout->setColumnStretch( 2, 10 );

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

  connect(ok, SIGNAL(pressed()), SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), SLOT(slotReject()));

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

SettingsPageLookNFeel::~SettingsPageLookNFeel()
{
}

void SettingsPageLookNFeel::slotAccept()
{
  save();
  emit settingsChanged();
  QWidget::close();
}

void SettingsPageLookNFeel::slotReject()
{
  QWidget::close();
}

void SettingsPageLookNFeel::load()
{
  GeneralConfig *conf = GeneralConfig::instance();
  m_currentFont         = conf->getGuiFont();
  m_currentMenuFont     = conf->getGuiMenuFont();

  // search item to be selected
  int idx = m_styleBox->findText( conf->getGuiStyle() );

  if( idx != -1 )
    {
      m_styleBox->setCurrentIndex(idx);
    }

  m_currentMapFrameColor = conf->getMapFrameColor();

  // Set color icon at the push button
  QPixmap pixmap( m_pixmapSize );
  pixmap.fill(m_currentMapFrameColor);
  m_editMapFrameColor->setIcon( QIcon(pixmap) );

  Speed speed;
  // speed is stored in Km/h
  speed.setKph( GeneralConfig::instance()->getScreenSaverSpeedLimit() );
  m_screenSaverSpeedLimit->setValue( speed.getValueInUnit( m_unit ) );
  // save loaded value for change control
  m_loadedSpeed = m_screenSaverSpeedLimit->value();
}

void SettingsPageLookNFeel::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  short changes = 0;

  if( conf->getGuiFont() != m_currentFont )
    {
      conf->setGuiFont( m_currentFont );
      changes++;
    }

  if( conf->getGuiMenuFont() != m_currentMenuFont )
    {
      conf->setGuiMenuFont( m_currentMenuFont );
      changes++;
    }

  if( conf->getGuiStyle() != m_styleBox->currentText() )
    {
      conf->setGuiStyle( m_styleBox->currentText() );
      conf->setOurGuiStyle();
      changes++;
    }

  if( conf->getMapFrameColor() != m_currentMapFrameColor )
    {
      conf->setMapFrameColor( m_currentMapFrameColor );
      changes++;
    }

  if( m_loadedSpeed != m_screenSaverSpeedLimit->value() )
    {
      Speed speed;
      speed.setValueInUnit( m_screenSaverSpeedLimit->value(), m_unit );

      // store speed in Km/h
      GeneralConfig::instance()->setScreenSaverSpeedLimit( speed.getKph() );
      changes++;
   }

  if( changes )
    {
      conf->save();
      emit settingsChanged();
    }
}

/**
 * Called to ask is confirmation on the close is needed.
 */
bool SettingsPageLookNFeel::checkChanges()
{
  /* set warn to 'true' if the data has changed. Note that we can NOT
     just set warn equal to _changed, because that way we might erase
     a warning flag set by another page! */

  GeneralConfig * conf = GeneralConfig::instance();
  bool changed=false;

  changed |= conf->getGuiFont() != m_currentFont;
  changed |= conf->getGuiMenuFont() != m_currentMenuFont;
  changed |= conf->getGuiStyle() != m_styleBox->currentText();
  changed |= conf->getMapFrameColor() != m_currentMapFrameColor;
  changed |= m_loadedSpeed != m_screenSaverSpeedLimit->value() ;

  return changed;
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

  if( ! ok )
    {
      // fall back uses current default font
      currFont = font();
    }

  QFont newFt = FontDialog::getFont( ok, currFont, this, tr("GUI Font"));

  if( ok )
    {
     // the user clicked OK and font is set to the font the user selected
      m_currentFont = newFt.toString();

     // Set the new GUI font for all widgets. Note this new font
     // is only set temporary. The user must save it for permanent usage.
     QApplication::setFont( newFt );
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

  // If a none default font is defined, we get here an error. But that is not
  // really an error, therefore we ignore that here.
  if( ! ok )
    {
      // fall back uses current default font adapted to the menu font.
      currFont = font();
      Layout::fitGuiMenuFont( currFont );
    }

  QFont newFt = FontDialog::getFont( ok, currFont, this, tr("GUI Menu Font"));

  if( ok )
    {
      // the user clicked OK and the menu font is set to the font the user selected
      m_currentMenuFont = newFt.toString();
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
  // Enable software input panel for color dialog
  bool ase = qApp->autoSipEnabled();
  qApp->setAutoSipEnabled( true );

  // get current color
  QColor& color = GeneralConfig::instance()->getMapFrameColor();

  // Open color chooser dialog to edit selected color
  QString title = tr("Map sidebar color");

  QColor newColor = ColorDialog::getColor( color, this, title );

  if( newColor.isValid() && color != newColor )
    {
      // save color into temporary buffer
      m_currentMapFrameColor = newColor;

      // Set the pixmap icon at the push button
      QPixmap pixmap( m_pixmapSize );
      pixmap.fill(m_currentMapFrameColor);
      m_editMapFrameColor->setIcon( QIcon(pixmap) );
    }

  qApp->setAutoSipEnabled( ase );
}

void SettingsPageLookNFeel::slot_defaultColor()
{
  // save color into temporary buffer
  m_currentMapFrameColor = QColor( INFOBOX_FRAME_COLOR );

  // Set the pixmap icon at the push button
  QPixmap pixmap( m_pixmapSize );
  pixmap.fill(m_currentMapFrameColor);
  m_editMapFrameColor->setIcon( QIcon(pixmap) );
}
