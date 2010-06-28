/***********************************************************************
 **
 **   configwidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by André Somers
 **                   2007-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * This is the configuration widget of Cumulus. General settings are
 * handled here. These are in general not related to the preflight
 * preparation. For that exists a separate configuration widget.
 */

#include <QtGui>

#include "configwidget.h"
#include "generalconfig.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "gpsnmea.h"

extern MapContents *_globalMapContents;

ConfigWidget::ConfigWidget(QWidget *parent) :
  QWidget(parent), loadConfig(true)
{
  // qDebug("ConfigWidget: height=%d, width=%d", parent->height(), parent->width());
  setObjectName( "ConfigWidget" );
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowTitle( "General Settings" );

  QTabWidget* tabWidget = new QTabWidget( this );

  QScrollArea* sppArea = new QScrollArea( tabWidget );
  sppArea->setWidgetResizable( true );
  sppArea->setFrameStyle( QFrame::NoFrame );
  spp = new SettingsPagePersonal( this );
  sppArea->setWidget( spp );
  tabWidget->addTab( sppArea, tr( "Personal" ) );

  spg = new SettingsPageGPS( this );
  tabWidget->addTab( spg, tr( "GPS" ) );

  spgl = new SettingsPageGlider( this );
  tabWidget->addTab( spgl, tr( "Gliders" ) );

  spms = new SettingsPageMapSettings( this );
  tabWidget->addTab( spms, tr( "Map Settings" ) );

  spmo = new SettingsPageMapObjects( this );
  tabWidget->addTab( spmo, tr( "Map Objects" ) );

  QScrollArea* sptcArea = new QScrollArea( tabWidget );
  sptcArea->setWidgetResizable( true );
  sptcArea->setFrameStyle( QFrame::NoFrame );
  sptc = new SettingsPageTerrainColors( this );
  sptcArea->setWidget( sptc );
  tabWidget->addTab( sptcArea, tr( "Terrain Colors" ) );

  QScrollArea* spsArea = new QScrollArea( tabWidget );
  spsArea->setWidgetResizable( true );
  spsArea->setFrameStyle( QFrame::NoFrame );
  spt = new SettingsPageTask( this );
  spsArea->setWidget( spt );
  tabWidget->addTab( spsArea, tr( "Task" ) );

  QScrollArea* spafArea = new QScrollArea( tabWidget );
  spafArea->setWidgetResizable( true );
  spafArea->setFrameStyle( QFrame::NoFrame );
  spaf = new SettingsPageAirfields( this );
  spafArea->setWidget( spaf );
  tabWidget->addTab( spafArea, tr( "Airfields" ) );

  QScrollArea* asArea = new QScrollArea( tabWidget );
  asArea->setWidgetResizable( true );
  asArea->setFrameStyle( QFrame::NoFrame );
  spa = new SettingsPageAirspace( this );
  asArea->setWidget( spa );
  tabWidget->addTab( asArea, tr( "Airspaces" ) );

  spu = new SettingsPageUnits( this );
  tabWidget->addTab( spu, tr( "Units" ) );

  QScrollArea* infoArea = new QScrollArea( tabWidget );
  infoArea->setWidgetResizable( true );
  infoArea->setFrameStyle( QFrame::NoFrame );
  spi = new SettingsPageInformation( this );
  infoArea->setWidget( spi );
  tabWidget->addTab( infoArea, tr( "Information" ) );

  splnf = new SettingsPageLookNFeel( this );
  tabWidget->addTab( splnf, tr( "Look&&Feel" ) );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")) );
  cancel->setIconSize(QSize(26,26));
  cancel->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("ok.png")) );
  ok->setIconSize(QSize(26,26));
  ok->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png") );

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget( cancel, 2 );
  buttonBox->addSpacing(20);
  buttonBox->addWidget( ok, 2 );
  buttonBox->addStretch(2);
  buttonBox->addWidget( titlePix, 1 );

  connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

  connect(this, SIGNAL(load()), spp, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spgl, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spg, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spt, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spms, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spmo, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spaf, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spa, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spu, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spi, SLOT(slot_load()));
  connect(this, SIGNAL(load()), splnf, SLOT(slot_load()));
  connect(this, SIGNAL(load()), sptc, SLOT(slot_load()));

  connect(this, SIGNAL(reload()), spu, SLOT(slot_load()));

  connect(this, SIGNAL(save()), spp, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spgl, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spg, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spt, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spms, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spmo, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spaf, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spa, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spi, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spu, SLOT(slot_save()));
  connect(this, SIGNAL(save()), splnf, SLOT(slot_save()));
  connect(this, SIGNAL(save()), sptc, SLOT(slot_save()));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spp, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spgl, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spms, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spmo, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spaf, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spa, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          splnf, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          sptc, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spu, SLOT(slot_query_close(bool&, QStringList&)));

  extern MapConfig *_globalMapConfig;

  connect(spa, SIGNAL(airspaceColorsUpdated()),
          _globalMapConfig, SLOT(slotReloadAirspaceColors()));

#ifdef INTERNET

  connect(spa, SIGNAL(downloadAirspace( QString& )),
          _globalMapContents, SLOT(slotDownloadAirspace( QString& )));

  connect(spms, SIGNAL(downloadMapArea( const QPoint&, const Distance& )),
          _globalMapContents, SLOT(slotDownloadMapArea( const QPoint&, const Distance&)));

  connect(spaf, SIGNAL(downloadWelt2000( const QString& )),
          _globalMapContents, SLOT(slotDownloadWelt2000( const QString& )));

#endif

  QHBoxLayout *contentLayout = new QHBoxLayout;
  contentLayout->addWidget( tabWidget );
  contentLayout->addLayout( buttonBox );

  setLayout( contentLayout );

  slot_LoadCurrent();
  tabWidget->setCurrentWidget( spp );
}

ConfigWidget::~ConfigWidget()
{
  // qDebug("~ConfigWidget() is called");
}

/** This slot is called if the window will be shown or resized */
void ConfigWidget::slot_LoadCurrent()
{
  // Block multiple loads to avoid reset of changed values in the configuration
  // tabulators.
  if( loadConfig == false )
    {
      return;
    }

  loadConfig = false;
  emit load();
}

/** Called if OK button is pressed */
void ConfigWidget::accept()
{
  hide();

  // save some change states before restoring of data
  bool homeLatitudeChange = spp->checkIsHomeLatitudeChanged();
  bool homePositionChange = spp->checkIsHomePositionChanged();
  bool projectionChange   = spms->checkIsProjectionChanged();
  bool welt2000Change     = spaf->checkIsWelt2000Changed();

  // All setting pages will save their configuration data
  emit save();

  GeneralConfig *conf = GeneralConfig::instance();

  if( conf->getMapProjectionType() == ProjectionBase::Cylindric &&
      conf->getMapProjectionFollowsHome() == true &&
      homeLatitudeChange == true )
    {
      // @AP: In case of cylinder projection and an active projection follows home
      // option and a latitude change of the home position, the projection
      // parallel is set to the new home latitude. That shall ensure
      // optimized results during map drawing. Note, that is only
      // supported for the cylinder projection!
      projectionChange = true;
      conf->setCylinderParallel( conf->getHomeLat() );
    }

  // save all configuration items permanently into the configuration file
  GeneralConfig::instance()->save();

  if( projectionChange == true || welt2000Change == true )
    {
      QMessageBox::warning( this, "Cumulus",
                            tr( "<html>"
                                "<b>Configuration settings have been changed!</b><p>"
                                "Update of system can take a few seconds!"
                                "</html>" ) );
    }

  emit settingsChanged();

  if( projectionChange == false &&
      ( welt2000Change == true ||
      ( conf->getWelt2000CountryFilter() == "" &&
        homePositionChange == true )))
    {
      // @AP: There was a change in the Welt2000 configuration data. We must
      // trigger a reload of these data. This is only done, if
      // a) The projection has not been changed. A projection change includes
      //    a reload of Welt2000 data.
      // b) The Welt2000 country filter is not set and
      //    the home position has been changed.
      // c) Welt2000 configuration items have been changed
      emit welt2000ConfigChanged();
    }

  if( homePositionChange == true && GpsNmea::gps->getConnected() == false )
    {
      // That moves the map to the new home position. We do that only,
      // if we have no GPS connection.
      emit gotoHomePosition();
    }

  emit closeConfig();
  QWidget::close();
}

/** Called if the Cancel button is pressed */
void ConfigWidget::reject()
{
  bool need_warning=false;
  QStringList changed_pages;
  emit query_close(need_warning, changed_pages);
  QString pagelist;
  QString separator;

  if( need_warning )
    {
      for( int i = 0; i < changed_pages.count(); i++ )
        {
          pagelist += QString("<li>%1</li>").arg(changed_pages[i]);
        }

      int answer=QMessageBox::warning(this,
                                      tr("Close without saving!"),
                                      tr("<html>You have changed:<b><ul>%1</ul></b>Discard changes?</html>").arg(pagelist),
                                      QMessageBox::Discard,
                                      QMessageBox::Save);

      if( answer == QMessageBox::Save )
        { // the user pressed save
          accept();
          return;
        }
    }

  hide();
  emit reload();
  emit closeConfig();
  QWidget::close();
}
