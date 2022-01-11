/***********************************************************************
**
**   settingspagepersonal.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**                   2008-2018 by Axel Pauli
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

#include "AirfieldSelectionList.h"
#include "generalconfig.h"
#include "helpbrowser.h"
#include "layout.h"
#include "MainWindow.h"
#include "mapcontents.h"
#include "numberEditor.h"
#include "settingspagepersonal.h"

extern MapContents *_globalMapContents;

#ifdef INTERNET
#ifndef ANDROID
#include "proxydialog.h"
#endif
#endif

SettingsPagePersonal::SettingsPagePersonal(QWidget *parent) :
  QWidget(parent),
  m_initalHomeElevationValue(0),
  m_newHomeSelectedFromList(false)
{
  setObjectName("SettingsPagePersonal");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Personal") );

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
  contentLayout->addLayout( sal, 10 );

  // The parent of the layout is the scroll widget
  QGridLayout* topLayout = new QGridLayout(sw);
  topLayout->setSpacing( topLayout->spacing() * Layout::getIntScaledDensity() );

  // save current altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  m_altUnit = Altitude::getUnit();

  GeneralConfig *conf = GeneralConfig::instance();

  int row=0;

  QLabel * lbl = new QLabel(tr("Pilot name:"), this);

  topLayout->addWidget(lbl, row, 0);

  Qt::InputMethodHints imh;

  edtName = new QLineEdit(this);
  imh = (edtName->inputMethodHints() | Qt::ImhNoPredictiveText);
  edtName->setInputMethodHints(imh);

  connect( edtName, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  topLayout->addWidget(edtName, row, 1, 1, 2);
  row++;

  lbl = new QLabel(tr("Language:"), this);
  topLayout->addWidget(lbl, row, 0);
  langBox = new QComboBox(this);
  topLayout->addWidget(langBox, row, 1);
  row++;

  lbl = new QLabel(tr("Home site country:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtHomeCountry = new QLineEdit(this);
  edtHomeCountry->setInputMethodHints(Qt::ImhUppercaseOnly | Qt::ImhNoPredictiveText);
  edtHomeCountry->setMaxLength(2);
  QRegExp rx("[A-Za-z]{2}");
  edtHomeCountry->setValidator( new QRegExpValidator(rx, this) );

  connect( edtHomeCountry, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

#ifndef ANDROID
  connect( edtHomeCountry, SIGNAL(textEdited( const QString& )),
           this, SLOT(slot_textEditedCountry( const QString& )) );
#endif

  topLayout->addWidget(edtHomeCountry, row, 1);
  row++;

  lbl = new QLabel(tr("Home site name:"), this);
  topLayout->addWidget(lbl, row, 0);

  edtHomeName = new QLineEdit(this);
  edtHomeName->setInputMethodHints(imh);
  edtHomeName->setMaxLength(8);
  topLayout->addWidget(edtHomeName, row, 1);

  connect( edtHomeName, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  QHBoxLayout* hbl = new QHBoxLayout;
  hbl->setSpacing( 0 );

  newHome = new QPushButton( tr("New Home") );
  hbl->addWidget(newHome);
  hbl->addStretch( 10 );
  topLayout->addLayout( hbl, row, 2 );
  row++;

  connect( newHome, SIGNAL(clicked()), SLOT(slot_openAirfieldDialog()) );

  lbl = new QLabel(tr("Home site elevation:"), this);
  topLayout->addWidget(lbl, row, 0);

  edtHomeElevation = new NumberEditor( this );
  edtHomeElevation->setDecimalVisible( false );
  edtHomeElevation->setSuffix( " " + Altitude::getUnitText() );
  edtHomeElevation->setMaxLength(6);
  edtHomeElevation->setAlignment( Qt::AlignLeft );
  edtHomeElevation->setText("0");

  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "(0|-?[1-9][0-9]{0,4})" ), this );
  edtHomeElevation->setValidator( eValidator );
  topLayout->addWidget(edtHomeElevation, row, 1);
  row++;

  lbl = new QLabel(tr("Home site latitude:"), this);
  topLayout->addWidget(lbl, row, 0);

  edtHomeLat = new LatEditNumPad(this, conf->getHomeLat());
  topLayout->addWidget(edtHomeLat, row, 1, 1, 2);
  row++;

  lbl = new QLabel(tr("Home site longitude:"), this);
  topLayout->addWidget(lbl, row, 0);

  edtHomeLong = new LongEditNumPad(this, conf->getHomeLon());
  topLayout->addWidget(edtHomeLong, row, 1, 1, 2);
  row++;

#ifdef ANDROID
  QLabel* userDirSelection = new QLabel( tr("Data Directory") + ":", this );
#else
  QPushButton* userDirSelection = new QPushButton( tr("Data Directory"), this );
  userDirSelection->setToolTip(tr("Select your personal data directory"));

  connect(userDirSelection, SIGNAL( clicked()), this, SLOT(slot_openDirectoryDialog()) );
#endif

  topLayout->addWidget(userDirSelection, row, 0);

  userDataDir = new QLineEdit(this);
  userDataDir->setInputMethodHints(imh);

#ifdef ANDROID
  userDataDir->setReadOnly( true );
#endif

  topLayout->addWidget(userDataDir, row, 1, 1, 2);
  row++;

  connect( userDataDir, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

#ifdef INTERNET
#ifndef ANDROID

  QPushButton* editProxy = new QPushButton( tr("Set Proxy") );
  editProxy->setToolTip(tr("Enter Proxy data if needed"));
  topLayout->addWidget(editProxy, row, 0);

  connect( editProxy, SIGNAL( clicked()), this, SLOT(slot_editProxy()) );

  proxyDisplay = new QLabel;
  topLayout->addWidget(proxyDisplay, row, 1, 1, 2);
  row++;

#endif
#endif

  topLayout->setRowStretch(row, 10);
  topLayout->setColumnStretch( 2, 10 );

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

SettingsPagePersonal::~SettingsPagePersonal()
{}

void SettingsPagePersonal::slotHelp()
{
  QString file = "cumulus-settings-personal.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void SettingsPagePersonal::slotAccept()
{
  save();
  QWidget::close();
}

void SettingsPagePersonal::slotReject()
{
  QWidget::close();
}

void SettingsPagePersonal::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  edtName->setText( conf->getSurname() );
  edtHomeCountry->setText( conf->getHomeCountryCode() );
  edtHomeName->setText( conf->getHomeName() );
  edtHomeLat->setKFLogDegree(conf->getHomeLat());
  edtHomeLong->setKFLogDegree(conf->getHomeLon());

  if( _globalMapContents->getListLength(MapContents::GliderfieldList) == 0 &&
      _globalMapContents->getListLength(MapContents::AirfieldList) == 0 )
    {
      // Hide the button, if no airfields are available.
      newHome->hide();
    }

  if( m_altUnit == Altitude::meters )
    { // user wants meters
      edtHomeElevation->setValue((int) rint(conf->getHomeElevation().getMeters()));
    }
  else
    { // user get feet
      edtHomeElevation->setValue((int) rint(conf->getHomeElevation().getFeet()));
    }

  // save spinbox value for later change check
  m_initalHomeElevationValue = edtHomeElevation->value();

  userDataDir->setText( conf->getUserDataDirectory() );

  // Determine user's desired language
  QString langPath = ":/locale";

  // Check for installed languages
  QDir dir( langPath );
  QStringList langs = dir.entryList( QDir::AllDirs | QDir::NoDot | QDir::NoDotDot );

  // Add default language English to the list.
  langs << "en";
  langs.sort();

  langBox->addItems(langs);

  // search item to be selected
  int idx = langBox->findText( conf->getLanguage() );

  if( idx != -1 )
    {
      langBox->setCurrentIndex(idx);
    }

#ifdef INTERNET
#ifndef ANDROID

  slot_setProxyDisplay();

#endif
#endif
}

void SettingsPagePersonal::save()
{
  bool homeChanged = checkIsHomePositionChanged() | m_newHomeSelectedFromList;

  GeneralConfig *conf = GeneralConfig::instance();

  conf->setSurname( edtName->text().trimmed() );
  conf->setHomeCountryCode( edtHomeCountry->text().trimmed().toUpper() );

  if( edtHomeName->text().trimmed().isEmpty() )
    {
      conf->setHomeName( tr("Home") );
    }
  else
    {
      conf->setHomeName( edtHomeName->text().trimmed().toUpper() );
    }

  conf->setLanguage( langBox->currentText() );

  Altitude homeElevation;

  if( m_altUnit == Altitude::meters )
    {
      homeElevation.setMeters( edtHomeElevation->value() );
    }
  else
    {
      homeElevation.setFeet( edtHomeElevation->value() );
    }

  conf->setHomeElevation(homeElevation);

  conf->setUserDataDirectory( userDataDir->text().trimmed() );

  // Check, if string input values have been changed. If not, no
  // storage is done to avoid rounding errors. They can appear if the
  // position formats will be changed between DMS <-> DDM vice versa.
  if( edtHomeLat->isInputChanged() || m_newHomeSelectedFromList == true )
    {
      conf->setHomeLat( edtHomeLat->KFLogDegree() );
    }

  if( edtHomeLong->isInputChanged()  || m_newHomeSelectedFromList == true )
    {
      conf->setHomeLon( edtHomeLong->KFLogDegree() );
    }

  if( homeChanged )
    {
      if( conf->getMapProjectionType() == ProjectionBase::Cylindric &&
          conf->getMapProjectionFollowsHome() == true )
        {
          // @AP: In case of cylinder projection and an active projection follows home
          // option and a latitude change of the home position, the projection
          // parallel is set to the new home latitude. That shall ensure
          // optimized results during map drawing. Note, that is only
          // supported for the cylinder projection!
          conf->setCylinderParallel( conf->getHomeLat() );

          QMessageBox mb( QMessageBox::Information,
                           "Cumulus",
                           tr( "<html>"
                           "<b>Home position was changed!</b><p>"
                           "System update can take a few seconds and more!"
                           "</html>" ),
                           QMessageBox::Ok,
                           this );

     #ifdef ANDROID

           // Under Android the box must be moved into the center of the desktop screen.
           // Note the box must be set as first to visible otherwise move will not work.
           mb.show();
           QPoint pos = mapToGlobal(QPoint( width()/2 - mb.width()/2, height()/2 - mb.height()/2 ));
           mb.move( pos );

     #endif

           mb.exec();
        }

      emit homePositionChanged();
    }

  conf->save();
  emit settingsChanged();
}

/**
 * Checks if the home position has been changed.
 */
bool SettingsPagePersonal::checkIsHomePositionChanged()
{
  bool changed = false;

  if( edtHomeLat->isInputChanged() || edtHomeLong->isInputChanged() )
    {
      changed = true;
    }

  // qDebug( "SettingsPagePersonal::checkIsHomePositionChanged(): %d", changed );
  return changed;
}

/** Checks if the home latitude has been changed */
bool SettingsPagePersonal::checkIsHomeLatitudeChanged()
{
  bool changed = false;

  if (edtHomeLat->isInputChanged())
    {
      changed = true;
    }

  // qDebug( "SettingsPagePersonal::checkIsHomeLatitudeChanged(): %d", changed );
  return changed;
}

/** Checks if the home longitude has been changed */
bool SettingsPagePersonal::checkIsHomeLongitudeChanged()
{
  bool changed = false;

  if( edtHomeLong->isInputChanged() )
    {
      changed = true;
    }

  // qDebug( "SettingsPagePersonal::checkIsHomeLongitudeChanged(): %d", changed );
  return changed;
}

bool SettingsPagePersonal::checkChanges()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed  = (edtName->text() != conf->getSurname());
  changed |= (langBox->currentText() != conf->getLanguage());
  changed |= (edtHomeCountry->text() != conf->getHomeCountryCode());
  changed |= (edtHomeName->text() != conf->getHomeName());
  changed |= checkIsHomePositionChanged();
  changed |= m_newHomeSelectedFromList;
  changed |= m_initalHomeElevationValue != edtHomeElevation->value();
  changed |= (userDataDir->text() != conf->getUserDataDirectory());

  return changed;
}

/** Called to open the directory selection dialog */
void SettingsPagePersonal::slot_openDirectoryDialog()
{
  QString dataDir =
      QFileDialog::getExistingDirectory( this,
                                         tr("Please select your data directory"),
                                         userDataDir->text(),
                                          QFileDialog::ShowDirsOnly );
  if( dataDir.isEmpty() )
    {
      return; // nothing was selected by the user
    }

  userDataDir->setText( dataDir );
}

/** Called to open the airfield selection dialog */
void SettingsPagePersonal::slot_openAirfieldDialog()
{
  AirfieldSelectionList* asl = new AirfieldSelectionList( this );

  connect( asl, SIGNAL(takeThisPoint(const SinglePoint*)),
           SLOT(slot_newHome(const SinglePoint*)) );

  asl->show();
}

void SettingsPagePersonal::slot_textEditedCountry( const QString& text )
{
  // Change edited text to upper cases
  edtHomeCountry->setText( text.toUpper() );
}

/** Called, if a new home shall be set. */
void SettingsPagePersonal::slot_newHome( const SinglePoint* singlePoint )
{
  if( singlePoint == 0 )
    {
      return;
    }

  m_newHomeSelectedFromList = true;

  edtHomeCountry->setText( singlePoint->getCountry() );
  edtHomeName->setText( singlePoint->getWPName() );
  edtHomeLat->setKFLogDegree( singlePoint->getWGSPosition().lat() );
  edtHomeLong->setKFLogDegree( singlePoint->getWGSPosition().lon() );

  Altitude elevation( singlePoint->getElevation() );

  if( m_altUnit == Altitude::meters )
    { // user wants meters
      edtHomeElevation->setValue((int) rint(elevation.getMeters()));
    }
  else
    { // user gets feet
      edtHomeElevation->setValue((int) rint(elevation.getFeet()));
    }
}

#ifdef INTERNET
#ifndef ANDROID

/**
 * Opens proxy dialog on user request.
 */
void SettingsPagePersonal::slot_editProxy()
{
  ProxyDialog *dialog = new ProxyDialog( this );
  connect( dialog, SIGNAL(proxyDataChanged()), SLOT(slot_setProxyDisplay()) );
  dialog->show();

#ifdef ANDROID

  QPoint pos = mapToGlobal(QPoint( width()/2  - dialog->width()/2, 25));
  dialog->move( pos );

#endif
}

void SettingsPagePersonal::slot_setProxyDisplay()
{
  // update proxy display
  if( GeneralConfig::instance()->getProxy().isEmpty() )
    {
      proxyDisplay->setText( tr("No proxy defined") );
    }
  else
    {
      proxyDisplay->setText( GeneralConfig::instance()->getProxy() );
    }
}

#endif
#endif
