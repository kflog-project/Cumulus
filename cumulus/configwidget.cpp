/***********************************************************************
 **
 **   configwidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by André Somers, 2009 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * This is the configuration widget of cumulus. Different settings are
 * to do here. There are in general not related to the flight
 * preparation. For that it exists a separate widget.
 */

#include <QMessageBox>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QLabel>

#include "configwidget.h"
#include "generalconfig.h"
#include "mapconfig.h"

ConfigWidget::ConfigWidget(QWidget *parent) :
  QWidget(parent), loadConfig(true)
{
  // qDebug("ConfigWidget: height=%d, width=%d", parent->height(), parent->width());
  setObjectName("ConfigWidget");
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowTitle("General Settings");

  QTabWidget* tabWidget = new QTabWidget(this);

  spp=new SettingsPagePersonal(this);
  tabWidget->addTab(spp, tr("Personal"));

  spgl=new SettingsPageGlider(this);
  tabWidget->addTab(spgl, tr("Gliders"));

  QScrollArea* sectorArea = new QScrollArea(tabWidget);
  sectorArea->setWidgetResizable(true);
  sectorArea->setFrameStyle(QFrame::NoFrame);

  sps=new SettingsPageSector(this);
  sectorArea->setWidget(sps);
  tabWidget->addTab(sectorArea, tr("Sector"));

  spg=new SettingsPageGPS(this);
  tabWidget->addTab(spg, tr("GPS"));

  spms=new SettingsPageMapSettings(this);
  tabWidget->addTab(spms, tr("Map Settings"));

  spmo=new SettingsPageMapObjects(this);
  tabWidget->addTab(spmo, tr("Map Objects"));

  sptc=new SettingsPageTerrainColors(this);
  tabWidget->addTab(sptc, tr("Terrain Colors"));

  QScrollArea* afArea = new QScrollArea(tabWidget);
  afArea->setWidgetResizable(true);
  afArea->setFrameStyle(QFrame::NoFrame);

  spaf=new SettingsPageAirfields(this);
  afArea->setWidget(spaf);
  tabWidget->addTab(afArea, tr("Airfields"));

  spa=new SettingsPageAirspace(this);
  tabWidget->addTab(spa, tr("Airspaces"));

  spu=new SettingsPageUnits(this);
  tabWidget->addTab(spu, tr("Units"));

  QScrollArea* infoArea = new QScrollArea(tabWidget);
  infoArea->setWidgetResizable(true);
  infoArea->setFrameStyle(QFrame::NoFrame);

  spi=new SettingsPageInformation(this);
  infoArea->setWidget(spi);
  tabWidget->addTab(infoArea, tr("Information"));

  splnf=new SettingsPageLookNFeel(this);
  tabWidget->addTab(splnf, tr("Look&&Feel"));

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
  connect(this, SIGNAL(load()), sps, SLOT(slot_load()));
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
  connect(this, SIGNAL(save()), sps, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spms, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spmo, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spaf, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spa, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spi, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spu, SLOT(slot_save()));
  connect(this, SIGNAL(save()), splnf, SLOT(slot_save()));
  connect(this, SIGNAL(save()), sptc, SLOT(slot_save()));

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

  extern MapConfig *_globalMapConfig;

  connect(spa, SIGNAL(airspaceColorsUpdated()),
          _globalMapConfig, SLOT(slotReloadAirspaceColors()));

  QHBoxLayout *contentLayout = new QHBoxLayout;
  contentLayout->addWidget(tabWidget);
  contentLayout->addLayout(buttonBox);

  setLayout(contentLayout);

  slot_LoadCurrent();
  tabWidget->setCurrentWidget(spp);
}

ConfigWidget::~ConfigWidget()
{
  // qDebug("~ConfigWidget() is called");
}

/** This slot is called if the window will be shown or resized */
void ConfigWidget::slot_LoadCurrent()
{
  // Block multiple loads to avoid reset of changed values in the
  // config tabs.

  // qDebug("slot_LoadCurrent() in configdialog.cpp is called");

  if( loadConfig )
    {
      loadConfig = false;
    }
  else
    {
      return;
    }

  emit load();
}

/** Called if OK button is pressed */
void ConfigWidget::accept()
{
  hide();

  // save change states before restoring of data
  bool projectionChange = spms->checkIsProjectionChanged();
  bool welt2000Change   = spaf->checkIsWelt2000Changed();

  emit save();

  // save modifications into file
  GeneralConfig::instance()->save();

  emit settingsChanged();

  if( projectionChange == false && welt2000Change == true )
    {
      // AP: There was a change in the welt 2000 config data. We must
      // trigger a reload of these data. This is only done, if the
      // projection has not been changed. A projection change includes
      // a reload of welt 2000 data.
      emit welt2000ConfigChanged();
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

  if (need_warning)
    {
      for (int i=0; i<changed_pages.count(); i++)
        {
          if (i==changed_pages.count()-1)
            {
              separator=".";
            }
          else
            {
              separator=",";
            }
          pagelist+=QString("<li>%1%2</li>").arg(changed_pages[i]).arg(separator);
        }

      int answer=QMessageBox::warning(this,
                                      tr("Close without saving"),
                                      tr("<html>You have changed:<b>%1</b>Discard changes?</html>").arg(pagelist),
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
