/***********************************************************************
 **
 **   configwidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by André Somers
 **                   2007-2012 by Axel Pauli
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
#include "layout.h"

#ifdef ANDROID
#include "androidevents.h"
#include "mainwindow.h"
#endif

extern MapContents *_globalMapContents;

ConfigWidget::ConfigWidget(QWidget *parent) :
  QWidget(parent), loadConfig(true)
{
  // qDebug("ConfigWidget: height=%d, width=%d", parent->height(), parent->width());
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowTitle( tr("General Settings") );

#ifdef ANDROID
  fullSize = parent->size();
#endif

  m_tabWidget = new QTabWidget( this );

  QScrollArea* sppArea = new QScrollArea( m_tabWidget );
  sppArea->setWidgetResizable( true );
  sppArea->setFrameStyle( QFrame::NoFrame );
  spp = new SettingsPagePersonal( this );
  sppArea->setWidget( spp );
#ifdef QSCROLLER
  QScroller::grabGesture(sppArea, QScroller::LeftMouseButtonGesture);
#endif
  m_tabWidget->addTab( sppArea, tr( "Personal" ) );

#ifdef ANDROID
  spg = new SettingsPageGPS4A( this );
#else
  spg = new SettingsPageGPS( this );
#endif

  m_tabWidget->addTab( spg, tr( "GPS" ) );

  spgl = new SettingsPageGlider( this );
  m_tabWidget->addTab( spgl, tr( "Gliders" ) );

  QScrollArea* spmsArea = new QScrollArea( m_tabWidget );
  spmsArea->setWidgetResizable( true );
  spmsArea->setFrameStyle( QFrame::NoFrame );
  spms = new SettingsPageMapSettings( this );
  spmsArea->setWidget( spms );
#ifdef QSCROLLER
  QScroller::grabGesture(spmsArea, QScroller::LeftMouseButtonGesture);
#endif
  m_tabWidget->addTab( spmsArea, tr( "Map Settings" ) );

  QScrollArea* spmoArea = new QScrollArea( m_tabWidget );
  spmoArea->setWidgetResizable( true );
  spmoArea->setFrameStyle( QFrame::NoFrame );
  spmo = new SettingsPageMapObjects( this );
  spmoArea->setWidget( spmo );
#ifdef QSCROLLER
  QScroller::grabGesture(spmoArea, QScroller::LeftMouseButtonGesture);
#endif
  m_tabWidget->addTab( spmoArea, tr( "Map Objects" ) );

  QScrollArea* sptcArea = new QScrollArea( m_tabWidget );
  sptcArea->setWidgetResizable( true );
  sptcArea->setFrameStyle( QFrame::NoFrame );
  sptc = new SettingsPageTerrainColors( this );
  sptcArea->setWidget( sptc );
#ifdef QSCROLLER
  QScroller::grabGesture(sptcArea, QScroller::LeftMouseButtonGesture);
#endif
  m_tabWidget->addTab( sptcArea, tr( "Terrain Colors" ) );

  QScrollArea* spsArea = new QScrollArea( m_tabWidget );
  spsArea->setWidgetResizable( true );
  spsArea->setFrameStyle( QFrame::NoFrame );
  spt = new SettingsPageTask( this );
  spsArea->setWidget( spt );
#ifdef QSCROLLER
  QScroller::grabGesture(spsArea, QScroller::LeftMouseButtonGesture);
#endif
  m_tabWidget->addTab( spsArea, tr( "Task" ) );

  QScrollArea* spafArea = new QScrollArea( m_tabWidget );
  spafArea->setWidgetResizable( true );
  spafArea->setFrameStyle( QFrame::NoFrame );
  spaf = new SettingsPageAirfields( this );
  spafArea->setWidget( spaf );
#ifdef QSCROLLER
  QScroller::grabGesture(spafArea, QScroller::LeftMouseButtonGesture);
#endif
  m_tabWidget->addTab( spafArea, tr( "Airfields" ) );

  QScrollArea* asArea = new QScrollArea( m_tabWidget );
  asArea->setWidgetResizable( true );
  asArea->setFrameStyle( QFrame::NoFrame );
  spa = new SettingsPageAirspace( this );
  asArea->setWidget( spa );
#ifdef QSCROLLER
  QScroller::grabGesture(asArea, QScroller::LeftMouseButtonGesture);
#endif
  m_tabWidget->addTab( asArea, tr( "Airspaces" ) );

  spu = new SettingsPageUnits( this );
  m_tabWidget->addTab( spu, tr( "Units" ) );

  QScrollArea* infoArea = new QScrollArea( m_tabWidget );
  infoArea->setWidgetResizable( true );
  infoArea->setFrameStyle( QFrame::NoFrame );
  spi = new SettingsPageInformation( this );
  infoArea->setWidget( spi );
#ifdef QSCROLLER
  QScroller::grabGesture(infoArea, QScroller::LeftMouseButtonGesture);
#endif
  m_tabWidget->addTab( infoArea, tr( "Information" ) );

  QScrollArea* splnfArea = new QScrollArea( m_tabWidget );
  splnfArea->setWidgetResizable( true );
  splnfArea->setFrameStyle( QFrame::NoFrame );
  splnf = new SettingsPageLookNFeel( this );
  splnfArea->setWidget( splnf );
#ifdef QSCROLLER
  QScroller::grabGesture(splnfArea, QScroller::LeftMouseButtonGesture);
#endif
  m_tabWidget->addTab( splnfArea, tr( "Look&&Feel" ) );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")) );
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("ok.png")) );
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png") );

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget( cancel, 2 );
  buttonBox->addSpacing(30);
  buttonBox->addWidget( ok, 2 );
  buttonBox->addStretch(2);
  buttonBox->addWidget( titlePix, 1 );

  connect(ok, SIGNAL(released()), this, SLOT(accept()));
  connect(cancel, SIGNAL(released()), this, SLOT(reject()));

  QShortcut* sc = new QShortcut( this );
  sc->setKey( Qt::Key_Close );
  connect( sc, SIGNAL( activated() ), this, SLOT( reject() ) );
  sc = new QShortcut( this );
  sc->setKey(Qt::Key_Escape);
  connect( sc, SIGNAL( activated() ), this, SLOT( reject() ) );

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

  connect(spg, SIGNAL(startNmeaLog()),
          GpsNmea::gps, SLOT(slot_openNmeaLogFile()));

  connect(spg, SIGNAL(endNmeaLog()),
          GpsNmea::gps, SLOT(slot_closeNmeaLogFile()));

#ifdef INTERNET

  connect(spa, SIGNAL(downloadAirspace( QString& )),
          _globalMapContents, SLOT(slotDownloadAirspace( QString& )));

  connect(spms, SIGNAL(downloadMapArea( const QPoint&, const Distance& )),
          _globalMapContents, SLOT(slotDownloadMapArea( const QPoint&, const Distance&)));

  connect(spaf, SIGNAL(downloadWelt2000( const QString& )),
          _globalMapContents, SLOT(slotDownloadWelt2000( const QString& )));

#endif

  QHBoxLayout *contentLayout = new QHBoxLayout;
  contentLayout->addWidget( m_tabWidget );
  contentLayout->addLayout( buttonBox );

  setLayout( contentLayout );

  slot_LoadCurrent();
  m_tabWidget->setCurrentWidget( spp );

#ifdef ANDROID
  MainWindow::mainWindow()->forceFocus();
#endif
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
      QMessageBox mb( QMessageBox::Information,
                      "Cumulus",
                      tr( "<html>"
                      "<b>Configuration settings have been changed!</b><p>"
                      "Update of system can take a few seconds and more!"
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

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents |
                                   QEventLoop::ExcludeSocketNotifiers );

  QCoreApplication::flush();

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

  // Check, if we have not a valid GPS fix. In this case we do move the map
  // to the new home position.
  if( homePositionChange == true && GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      emit gotoHomePosition();
    }

  QApplication::restoreOverrideCursor();

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

      QMessageBox mb( QMessageBox::Warning,
                      "Close without saving!",
                      tr("<html>You have changed:<b><ul>%1</ul></b>Discard changes?</html>").arg(pagelist),
                      QMessageBox::QMessageBox::Save | QMessageBox::Discard,
                      this );

      mb.setDefaultButton( QMessageBox::Save );

#ifdef ANDROID

      // Under Android the box must be moved into the center of the desktop screen.
      // Note the box must be set as first to visible otherwise move will not work.
      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2 - mb.width()/2, height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      if( mb.exec() == QMessageBox::Save )
        { // the user pressed save
          accept();
          return;
        }
    }

  setVisible(false);
  emit reload();
  emit closeConfig();
  QWidget::close();
}

bool ConfigWidget::eventFilter( QObject *o , QEvent *e )
{
  if( e->type() == QEvent::KeyPress )
    {
      QKeyEvent *k = static_cast<QKeyEvent *>(e);

      if( k->key() == Qt::Key_Escape )
        {
          // Call reject, if ESC was received.
          reject();
          return true;
        }
    }

  return QWidget::eventFilter(o, e);
}
