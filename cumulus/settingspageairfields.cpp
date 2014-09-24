/***********************************************************************
 **
 **   settingspageairfields.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2008-2014 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief Contains the airfield data settings.
 *
 * \date 2008-2014
 *
 * \version $Id$
 */

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "helpbrowser.h"
#include "layout.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "numberEditor.h"
#include "settingspageairfields.h"
#include "settingspageairfieldloading.h"

#ifdef INTERNET
#include "httpclient.h"
#endif

extern MapContents *_globalMapContents;

SettingsPageAirfields::SettingsPageAirfields(QWidget *parent) :
  QWidget(parent),
  m_homeRadiusInitValue(0.0),
  m_runwayFilterInitValue(0.0)
{
  setObjectName("SettingsPageAirfields");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Airfields") );

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
  QVBoxLayout *topLayout = new QVBoxLayout(sw);

  QGridLayout *sourceLayout = new QGridLayout;
  QLabel* lbl = new QLabel( tr("Source:") );
  sourceLayout->addWidget( lbl, 0, 0 );
  m_sourceBox = new QComboBox;
  m_sourceBox->addItem("OpenAIP");
  m_sourceBox->addItem("Welt2000");
  sourceLayout->addWidget( m_sourceBox, 0, 1 );

  QPushButton *cmdHelp = new QPushButton( tr("Help") );
  sourceLayout->addWidget( cmdHelp, 0, 3 );
  sourceLayout->setColumnStretch( 2, 5 );
  topLayout->addLayout(sourceLayout);

  connect( m_sourceBox, SIGNAL(currentIndexChanged(int)),
           this, SLOT(slot_sourceChanged(int)));

  connect( cmdHelp, SIGNAL(clicked()), this, SLOT(slot_openHelp()) );

  //----------------------------------------------------------------------------
  m_oaipGroup = new QGroupBox( "www.openaip.net", this );
  topLayout->addWidget(m_oaipGroup);

  int grow = 0;
  QGridLayout *oaipLayout = new QGridLayout(m_oaipGroup);

#ifdef INTERNET

  m_downloadOpenAip = new QPushButton( tr("Download") );
  oaipLayout->addWidget( m_downloadOpenAip, grow, 0 );

  connect( m_downloadOpenAip, SIGNAL(clicked()), this, SLOT(slot_downloadOpenAip()));

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setMargin(0);
  oaipLayout->addLayout( hbox, grow, 1, 1, 3 );

  lbl = new QLabel( tr("Countries:") );
  hbox->addWidget(lbl);

  Qt::InputMethodHints imh;

  m_countriesOaip4Download = new QLineEdit(m_oaipGroup);
  imh = Qt::ImhLowercaseOnly | Qt::ImhNoPredictiveText;
  m_countriesOaip4Download->setInputMethodHints(imh);

  connect( m_countriesOaip4Download, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  hbox->addWidget( m_countriesOaip4Download, 5 );
  grow++;

#endif

  QPushButton *cmdLoadFiles = new QPushButton( tr("Load") );
  oaipLayout->addWidget( cmdLoadFiles, grow, 0 );

  connect( cmdLoadFiles, SIGNAL(clicked()), this, SLOT(slot_openLoadDialog()) );

  hbox = new QHBoxLayout;
  hbox->setMargin(0);
  oaipLayout->addLayout( hbox, grow, 1, 1, 3);

  lbl = new QLabel( tr("Radius:") );
  hbox->addWidget(lbl);

  m_homeRadiusOaip = createHomeRadiusWidget( this );
  hbox->addWidget(m_homeRadiusOaip);
  hbox->addSpacing( 10 );

  lbl = new QLabel( tr("Rwy Filter:") );
  hbox->addWidget(lbl);

  m_minRwyLengthOaip = createRwyLenthFilterWidget( this );
  hbox->addWidget(m_minRwyLengthOaip);
  hbox->addStretch(5);
  grow++;

  oaipLayout->setColumnStretch( 3, 5 );

  //----------------------------------------------------------------------------
  m_weltGroup = new QGroupBox("www.segelflug.de/vereine/welt2000", this);
  topLayout->addWidget(m_weltGroup);

  QGridLayout* weltLayout = new QGridLayout(m_weltGroup);

  grow = 0;
  lbl = new QLabel(tr("Country Filter:"), (m_weltGroup));
  weltLayout->addWidget(lbl, grow, 0);

  m_countriesW2000 = new QLineEdit(m_weltGroup);
  imh = Qt::ImhUppercaseOnly | Qt::ImhNoPredictiveText;
  m_countriesW2000->setInputMethodHints(imh);

  connect( m_countriesW2000, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  weltLayout->addWidget(m_countriesW2000, grow, 1, 1, 3);
  grow++;

  lbl = new QLabel(tr("Radius:"), m_weltGroup);
  weltLayout->addWidget(lbl, grow, 0);

  m_homeRadiusW2000 = createHomeRadiusWidget( this );
  weltLayout->addWidget(m_homeRadiusW2000, grow, 1 );

  m_loadOutlandings = new QCheckBox( tr("Load Outlandings"), m_weltGroup );
  weltLayout->addWidget(m_loadOutlandings, grow, 3, Qt::AlignRight );
  grow++;

  lbl = new QLabel(tr("Rwy Filter:"), m_weltGroup);
  weltLayout->addWidget(lbl, grow, 0);

  m_minRwyLengthW2000 = createRwyLenthFilterWidget( this );
  weltLayout->addWidget(m_minRwyLengthW2000, grow, 1 );
  grow++;

#ifdef INTERNET

  weltLayout->setRowMinimumHeight(grow++, 10);

  m_installWelt2000 = new QPushButton( tr("Install"), m_weltGroup );
#ifndef ANDROID
  m_installWelt2000->setToolTip(tr("Install Welt2000 data"));
#endif
  weltLayout->addWidget(m_installWelt2000, grow, 0 );

  connect( m_installWelt2000, SIGNAL( clicked()), this, SLOT(slot_installWelt2000()) );

  m_welt2000FileName = new QLineEdit(m_weltGroup);
  m_welt2000FileName->setInputMethodHints(imh);
#ifndef ANDROID
  m_welt2000FileName->setToolTip(tr("Enter Welt2000 filename as to see on the web page"));
#endif

  connect( m_welt2000FileName, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  weltLayout->addWidget(m_welt2000FileName, grow, 1, 1, 3);

#endif

  weltLayout->setColumnStretch(2, 10);

  //----------------------------------------------------------------------------
  // JD: adding group box for diverse list display settings
  grow = 0;
  QGroupBox* listGroup = new QGroupBox(tr("List Display"), this);
  topLayout->addWidget(listGroup);
  topLayout->addStretch(10);

  QGridLayout* listLayout = new QGridLayout(listGroup);

  lbl = new QLabel(tr( "More space in AF/WP/OL lists:"), listGroup);
  listLayout->addWidget(lbl, grow, 0);

  m_afMargin = new NumberEditor( this );
  m_afMargin->setDecimalVisible( false );
  m_afMargin->setPmVisible( false );
  m_afMargin->setMaxLength(2);
  m_afMargin->setSuffix( tr(" Pixels") );
  m_afMargin->setMaximum( 30 );
  m_afMargin->setTitle("0...30");
  m_afMargin->setRange(0, 30);
  listLayout->addWidget(m_afMargin, grow, 1 );
  grow++;
  lbl = new QLabel(tr( "More space in Emergency list:"), listGroup);
  listLayout->addWidget(lbl, grow, 0);

  m_rpMargin = new NumberEditor( this );
  m_rpMargin->setDecimalVisible( false );
  m_rpMargin->setPmVisible( false );
  m_rpMargin->setMaxLength(2);
  m_rpMargin->setSuffix( tr(" Pixels") );
  m_rpMargin->setMaximum( 30 );
  m_rpMargin->setTitle("0...30");
  m_rpMargin->setRange(0, 30);
  listLayout->addWidget(m_rpMargin, grow, 1 );
  grow++;
  listLayout->setRowStretch(grow, 10);
  listLayout->setColumnStretch(2, 10);

#ifndef ANDROID
  // Android makes trouble, if word detection is enabled and the input is
  // changed by us.
  connect( m_countriesW2000, SIGNAL(textChanged(const QString&)),
           this, SLOT(slot_filterChanged(const QString&)) );
#endif

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

SettingsPageAirfields::~SettingsPageAirfields()
{
}

NumberEditor* SettingsPageAirfields::createHomeRadiusWidget( QWidget* parent )
{
  NumberEditor* ne = new NumberEditor( parent );
  ne->setDecimalVisible( false );
  ne->setPmVisible( false );
  ne->setMaxLength(4);
  ne->setSuffix( " " + Distance::getUnitText() );
  ne->setSpecialValueText(tr("Off"));
  ne->setTip(tr("Home Radius") + " (" + Distance::getUnitText() + ")" );
  ne->setRange( 0, 9999 );
  ne->setValue(0);
  return ne;
}

NumberEditor* SettingsPageAirfields::createRwyLenthFilterWidget( QWidget* parent )
{
  QString lenUnit = "m";

  if( Altitude::getUnit() != Altitude::meters )
    {
      lenUnit = "ft";
    }

  NumberEditor* ne = new NumberEditor( parent );
  ne->setDecimalVisible( false );
  ne->setPmVisible( false );
  ne->setMaxLength(4);
  ne->setSuffix( " " + lenUnit );
  ne->setRange( 0, 9999 );
  ne->setSpecialValueText(tr("Off"));
  ne->setTip(tr("Minimum Runway length") + " (" + lenUnit + ")");
  ne->setValue(0);
  return ne;
}

void SettingsPageAirfields::slot_sourceChanged( int index )
{
  // Toggle source visibility
  if( index == 0 )
    {
      m_oaipGroup->setVisible( true );
      m_weltGroup->setVisible( false );
    }
  else
    {
      m_oaipGroup->setVisible( false );
      m_weltGroup->setVisible( true );
    }
}

void SettingsPageAirfields::slotAccept()
{
  if( checkChanges() )
    {
      if( save() == false )
        {
          return;
        }

      emit settingsChanged();
    }

  QWidget::close();
}

void SettingsPageAirfields::slotReject()
{
  QWidget::close();
}

/**
 * Called to initiate loading of the configuration file
 */
void SettingsPageAirfields::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  m_sourceBox->setCurrentIndex( conf->getAirfieldSource() );
  slot_sourceChanged( conf->getAirfieldSource() );

  m_countriesW2000->setText( conf->getWelt2000CountryFilter() );

  if( Distance::getUnit() == Distance::meters )
    {
      m_homeRadiusInitValue = conf->getAirfieldHomeRadius();
    }
  else
    {
      Distance dist(conf->getAirfieldHomeRadius());
      m_homeRadiusInitValue = rint(dist.getValueOfCurrentUnit());
    }

  m_homeRadiusOaip->setValue( m_homeRadiusInitValue );
  m_homeRadiusW2000->setValue( m_homeRadiusInitValue );

  if( Altitude::getUnit() == Altitude::meters )
    {
      m_runwayFilterInitValue = conf->getAirfieldRunwayLengthFilter();
    }
  else
    {
      m_runwayFilterInitValue = rint(Altitude(conf->getAirfieldRunwayLengthFilter()).getFeet());
    }

  m_minRwyLengthOaip->setValue( m_runwayFilterInitValue );
  m_minRwyLengthW2000->setValue( m_runwayFilterInitValue );

  if( conf->getWelt2000LoadOutlandings() )
    {
      m_loadOutlandings->setCheckState( Qt::Checked );
    }
  else
    {
      m_loadOutlandings->setCheckState( Qt::Unchecked );
    }

#ifdef INTERNET

  m_countriesOaip4Download->setText( conf->getOpenAipAirfieldCountries() );
  m_welt2000FileName->setText( conf->getWelt2000FileName() );

#endif

  m_afMargin->setValue(conf->getListDisplayAFMargin());
  m_rpMargin->setValue(conf->getListDisplayRPMargin());

  // sets home radius enabled/disabled in dependency to filter string
  slot_filterChanged(m_countriesW2000->text());
}

/**
 * Called to initiate saving to the configuration file.
 */
bool SettingsPageAirfields::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // Save change states.
  bool openAipChanged  = checkIsOpenAipChanged();
  bool welt2000Changed = checkIsWelt2000Changed();

  conf->setAirfieldSource( m_sourceBox->currentIndex() );

  // Look which source widget is active to decide what have to be saved.
  if( m_sourceBox->currentIndex() == 0 )
    {
      Distance dist(0);
      dist.setValueInCurrentUnit( m_homeRadiusOaip->value() );

      conf->setAirfieldHomeRadius(dist.getMeters());

      if( Altitude::getUnit() == Altitude::meters )
        {
          conf->setAirfieldRunwayLengthFilter(m_minRwyLengthOaip->value());
        }
      else
        {
          Altitude alt(0);
          alt.setFeet(m_minRwyLengthOaip->value());
          conf->setAirfieldRunwayLengthFilter(alt.getMeters());
        }

#ifdef INTERNET

      QString openAipCountries = m_countriesOaip4Download->text().trimmed().toLower();
      QStringList clist = openAipCountries.split(QRegExp("[, ]"), QString::SkipEmptyParts);

      // Store only valid entries permanently.
      if( checkCountryList(clist) )
        {
          conf->setOpenAipAirfieldCountries(openAipCountries);
        }
#endif

    }
  else
    {
      Distance dist(0);
      dist.setValueInCurrentUnit( m_homeRadiusW2000->value() );
      conf->setAirfieldHomeRadius(dist.getMeters());

      if( Altitude::getUnit() == Altitude::meters )
        {
          conf->setAirfieldRunwayLengthFilter(m_minRwyLengthW2000->value());
        }
      else
        {
          Altitude alt;
          alt.setFeet(m_minRwyLengthW2000->value());
          conf->setAirfieldRunwayLengthFilter(alt.getMeters());
        }

      // We will check, if the country entries of Welt2000 are
      // correct. If not a warning message is displayed and the
      // modifications are discarded.
      QStringList clist = m_countriesW2000->text().split(QRegExp("[, ]"),
                          QString::SkipEmptyParts);

      if( ! checkCountryList(clist) )
        {
          QMessageBox mb( QMessageBox::Warning,
                          tr( "Please check entries" ),
                          tr("Every Welt2000 country sign must consist of two letters!<br>Allowed separators are space and comma!"),
                          QMessageBox::Ok,
                          this );

#ifdef ANDROID

          mb.show();
          QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                           height()/2 - mb.height()/2 ));
          mb.move( pos );

#endif
          mb.exec();
          return false;
        }

      conf->setWelt2000CountryFilter(m_countriesW2000->text().trimmed().toUpper());

      if( m_loadOutlandings->checkState() == Qt::Checked )
        {
          conf->setWelt2000LoadOutlandings( true );
        }
      else
        {
          conf->setWelt2000LoadOutlandings( false );
        }
    }

  conf->setListDisplayAFMargin(m_afMargin->value());
  conf->setListDisplayRPMargin(m_rpMargin->value());
  conf->save();

  if( openAipChanged == true )
    {
      emit reloadOpenAip();
    }

  if( welt2000Changed == true )
    {
      emit reloadWelt2000();
    }

  return true;
}

/**
 * Called if the text of the filter has been changed
 */
void SettingsPageAirfields::slot_filterChanged(const QString& text)
{
  Q_UNUSED( text )
}

void SettingsPageAirfields::slot_openLoadDialog()
{
  SettingsPageAirfieldLoading* dlg = new SettingsPageAirfieldLoading(this);

  connect( dlg, SIGNAL(fileListChanged()),
           _globalMapContents, SLOT(slotReloadOpenAipPoi()) );

  dlg->setVisible( true );
}

void SettingsPageAirfields::slot_openHelp()
{
  // Check, which help is requested.
  QString file = (m_sourceBox->currentIndex() == 0 ? "cumulus-maps-openAIP.html" : "cumulus-maps-welt2000.html");

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

bool SettingsPageAirfields::checkChanges()
{
  bool changed =
      (GeneralConfig::instance()->getAirfieldSource() != m_sourceBox->currentIndex());

  changed |= checkIsOpenAipChanged();
  changed |= checkIsWelt2000Changed();
  changed |= checkIsListDisplayChanged();

  return changed;
}

#ifdef INTERNET

/**
 * Called, if install button of Welt2000 is clicked.
 */
void SettingsPageAirfields::slot_installWelt2000()
{
  QString wfn = m_welt2000FileName->text().trimmed();

  if( wfn.isEmpty() )
    {
      QMessageBox mb( QMessageBox::Information,
                      tr( "Welt2000 settings invalid!" ),
                      tr( "Please add a valid Welt2000 filename!"),
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      mb.exec();
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Download Welt2000?"),
                  tr( "Active Internet connection is needed!") +
                  QString("<p>") + tr("Start download now?"),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif


  if( mb.exec() == QMessageBox::No )
    {
      return;
    }

  emit downloadWelt2000( wfn );
}

void SettingsPageAirfields::slot_downloadOpenAip()
{
  GeneralConfig *conf = GeneralConfig::instance();

  QString openAipCountries = m_countriesOaip4Download->text().trimmed().toLower();

  if( openAipCountries.isEmpty() )
    {
      conf->setOpenAipAirfieldCountries(openAipCountries);
      return;
    }

  // We will check, if the country entries of openAIP are valid. If not a
  // warning message is displayed and the action is aborted.
  QStringList clist = openAipCountries.split(QRegExp("[, ]"), QString::SkipEmptyParts);

  if( ! checkCountryList(clist) )
    {
      QMessageBox mb( QMessageBox::Warning,
                      tr( "Please check entries" ),
                      tr("Every openAIP country sign must consist of two letters!<br>Allowed separators are space and comma!"),
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif
      mb.exec();
      return;
    }

  conf->setOpenAipAirfieldCountries(openAipCountries);

  QMessageBox mb( QMessageBox::Question,
                  tr( "Download openAIP files?"),
                  tr( "Active Internet connection is needed!") +
                  QString("<p>") + tr("Start download now?"),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif


  if( mb.exec() == QMessageBox::No )
    {
      return;
    }

  emit downloadOpenAipPois( clist );
  return;
}

#endif

bool SettingsPageAirfields::checkIsOpenAipChanged()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed |= (conf->getAirfieldSource() == 1 && m_sourceBox->currentIndex() == 0);
  changed |= (conf->getOpenAipAirfieldCountries() != m_countriesOaip4Download->text().trimmed().toLower());
  changed |= (m_homeRadiusInitValue != m_homeRadiusOaip->value());
  changed |= (m_runwayFilterInitValue != m_minRwyLengthOaip->value());

  return changed;
}

/**
 * Checks, if the configuration of the Welt2000 has been changed
 */
bool SettingsPageAirfields::checkIsWelt2000Changed()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed |= (conf->getAirfieldSource() == 0 && m_sourceBox->currentIndex() == 1);
  changed |= (conf->getWelt2000CountryFilter() != m_countriesW2000->text().trimmed().toUpper());
  changed |= (m_homeRadiusInitValue != m_homeRadiusW2000->value());
  changed |= (m_runwayFilterInitValue != m_minRwyLengthW2000->value());

  bool currentState = false;

  if( m_loadOutlandings->checkState() == Qt::Checked )
    {
      currentState = true;
    }

  changed |= (conf->getWelt2000LoadOutlandings() != currentState);

  // qDebug( "SettingsPageAirfields::checkIsWelt2000Changed(): %d", changed );
  return changed;
}

/**
 * Checks if the configuration of list display has been changed
 */
bool SettingsPageAirfields::checkIsListDisplayChanged()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed |= (conf->getListDisplayAFMargin() != m_afMargin->value());
  changed |= (conf->getListDisplayRPMargin() != m_rpMargin->value());

  // qDebug( "SettingsPageAirfields::checkIsListDisplayChanged(): %d", changed );
  return changed;
}

bool SettingsPageAirfields::checkCountryList( QStringList& clist )
{
  if( clist.size() == 0 )
    {
      return true;
    }

  for( int i = 0; i < clist.size(); i++ )
    {
      const QString& s = clist.at(i);

      if( ! (s.length() == 2 && s.contains(QRegExp("[A-Za-z][A-Za-z]")) == true) )
        {
          return false;
        }
    }

  return true;
}
