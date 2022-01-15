/***********************************************************************
 **
 **   SettingsPagePointData.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2008-2022 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <HelpBrowser.h>
#include <QtWidgets>

#include "generalconfig.h"
#include "layout.h"
#include "MainWindow.h"
#include "mapcontents.h"
#include "numberEditor.h"
#include "SettingsPagePointData.h"
#include "SettingsPagePointDataLoading.h"

#ifdef INTERNET
#include "httpclient.h"
#endif

extern MapContents *_globalMapContents;

SettingsPagePointData::SettingsPagePointData(QWidget *parent) :
  QWidget(parent),
  m_homeRadiusInitValue(0.0),
  m_runwayFilterInitValue(0.0)
{
  setObjectName("SettingsPagePointData");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Point Data") );

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

  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );

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
  m_sourceBox->addItem("OpenAIP  ");
  sourceLayout->addWidget( m_sourceBox, 0, 1 );
  sourceLayout->setColumnStretch( 2, 5 );
  topLayout->addLayout(sourceLayout);

  connect( m_sourceBox, SIGNAL(currentIndexChanged(int)),
           this, SLOT(slot_sourceChanged(int)));

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
  // JD: adding group box for diverse list display settings
  grow = 0;
  QGroupBox* listGroup = new QGroupBox(tr("List Display"), this);
  topLayout->addWidget(listGroup);
  topLayout->addStretch(10);

  QGridLayout* listLayout = new QGridLayout(listGroup);

  lbl = new QLabel(tr( "More space in points lists:"), listGroup);
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
  lbl = new QLabel(tr( "More space in reachable list:"), listGroup);
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

SettingsPagePointData::~SettingsPagePointData()
{
}

NumberEditor* SettingsPagePointData::createHomeRadiusWidget( QWidget* parent )
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

NumberEditor* SettingsPagePointData::createRwyLenthFilterWidget( QWidget* parent )
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

void SettingsPagePointData::slot_sourceChanged( int index )
{
  // Toggle source visibility
  if( index == 0 )
    {
      m_oaipGroup->setVisible( true );
    }
  else
    {
      m_oaipGroup->setVisible( false );
    }
}

void SettingsPagePointData::slotAccept()
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

void SettingsPagePointData::slotReject()
{
  QWidget::close();
}

/**
 * Called to initiate loading of the configuration file
 */
void SettingsPagePointData::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // int afSource = conf->getAirfieldSource();
  int afSource = 0;

  m_sourceBox->setCurrentIndex( afSource );
  slot_sourceChanged( afSource );

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

  if( Altitude::getUnit() == Altitude::meters )
    {
      m_runwayFilterInitValue = conf->getAirfieldRunwayLengthFilter();
    }
  else
    {
      m_runwayFilterInitValue = rint(Altitude(conf->getAirfieldRunwayLengthFilter()).getFeet());
    }

  m_minRwyLengthOaip->setValue( m_runwayFilterInitValue );

#ifdef INTERNET

  m_countriesOaip4Download->setText( conf->getOpenAipAirfieldCountries() );

#endif

  m_afMargin->setValue(conf->getListDisplayAFMargin());
  m_rpMargin->setValue(conf->getListDisplayRPMargin());
}

/**
 * Called to initiate saving to the configuration file.
 */
bool SettingsPagePointData::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // Save change states.
  bool openAipChanged  = checkIsOpenAipChanged();

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
      QStringList clist = openAipCountries.split(QRegExp("[, ]"), Qt::SkipEmptyParts);

      // Store only valid entries permanently.
      if( checkCountryList(clist) )
        {
          conf->setOpenAipAirfieldCountries(openAipCountries);
        }
#endif

    }

  conf->setListDisplayAFMargin(m_afMargin->value());
  conf->setListDisplayRPMargin(m_rpMargin->value());
  conf->save();

  if( openAipChanged == true )
    {
      emit reloadOpenAip();
    }

  return true;
}

/**
 * Called if the text of the filter has been changed
 */
void SettingsPagePointData::slot_filterChanged(const QString& text)
{
  Q_UNUSED( text )
}

void SettingsPagePointData::slot_openLoadDialog()
{
  SettingsPagePointDataLoading* dlg = new SettingsPagePointDataLoading(this);

  connect( dlg, SIGNAL(fileListChanged()),
           _globalMapContents, SLOT(slotReloadOpenAipPoi()) );

  dlg->setVisible( true );
}

void SettingsPagePointData::slotHelp()
{
  // Check, which help is requested.
  QString file = "cumulus-maps-openAIP.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

bool SettingsPagePointData::checkChanges()
{
  bool changed =
      (GeneralConfig::instance()->getAirfieldSource() != m_sourceBox->currentIndex());

  changed |= checkIsOpenAipChanged();
  changed |= checkIsListDisplayChanged();

  return changed;
}

#ifdef INTERNET

void SettingsPagePointData::slot_downloadOpenAip()
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
  QStringList clist = openAipCountries.split(QRegExp("[, ]"), Qt::SkipEmptyParts);

  if( ! checkCountryList(clist) )
    {
      QMessageBox mb( QMessageBox::Warning,
                      tr( "Please check entries" ),
                      tr("Every openAIP country sign must consist of two letters!<br>Allowed separators are space and comma!"),
                      QMessageBox::Ok,
                      this );
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

  if( mb.exec() == QMessageBox::No )
    {
      return;
    }

  emit downloadOpenAipPois( clist );
  return;
}

#endif

bool SettingsPagePointData::checkIsOpenAipChanged()
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
 * Checks if the configuration of list display has been changed
 */
bool SettingsPagePointData::checkIsListDisplayChanged()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed |= (conf->getListDisplayAFMargin() != m_afMargin->value());
  changed |= (conf->getListDisplayRPMargin() != m_rpMargin->value());

  // qDebug( "SettingsPagePointData::checkIsListDisplayChanged(): %d", changed );
  return changed;
}

bool SettingsPagePointData::checkCountryList( QStringList& clist )
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
