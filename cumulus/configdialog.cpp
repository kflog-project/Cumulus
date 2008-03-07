/***********************************************************************
 **
 **   configdialog.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 *
 * This is the configuration dialog of cumulus. Different settings are
 * to do here. There are in general not related to the flight
 * preparation. For that it exists a separate dialog.
 */

#include <QMessageBox>
#include <QDialogButtonBox>

#include "configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
  QDialog(parent, Qt::WStyle_StaysOnTop), loadConfig(true)
{
  // qDebug("height=%d, width=%d", parent->height(), parent->width());

  setObjectName("ConfigDialog");
  setModal(true);

  setWindowTitle(tr("Cumulus settings"));

  QTabWidget* tabWidget = new QTabWidget (this);

  spp=new SettingsPagePersonal(this);
  tabWidget->addTab(spp, tr("Personal"));

  spgl=new SettingsPageGliderList(this);
  tabWidget->addTab(spgl, tr("Gliders"));

  sps=new SettingsPageSector(this);
  tabWidget->addTab(sps, tr("Sector"));

  spg=new SettingsPageGPS(this);
  tabWidget->addTab(spg, tr("GPS"));

  spma=new SettingsPageMapAdv(this);
  tabWidget->addTab(spma, tr("Map Settings"));

  spm=new SettingsPageMap(this);
  tabWidget->addTab(spm, tr("Map Objects"));

  spa=new SettingsPageAirspace(this);
  tabWidget->addTab(spa, tr("Airspace"));

  spu=new SettingsPageUnits(this);
  tabWidget->addTab(spu, tr("Units"));

  spi=new SettingsPageInformation(this);
  tabWidget->addTab(spi, tr("Information"));

  QDialogButtonBox* buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(this, SIGNAL(load()), spp, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spgl, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spg, SLOT(slot_load()));
  connect(this, SIGNAL(load()), sps, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spma, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spm, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spa, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spu, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spi, SLOT(slot_load()));

  connect(this, SIGNAL(reload()), spu, SLOT(slot_load()));

  connect(this, SIGNAL(save()), spp, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spgl, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spg, SLOT(slot_save()));
  connect(this, SIGNAL(save()), sps, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spma, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spm, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spa, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spi, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spu, SLOT(slot_save()));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spgl, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spma, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spm, SLOT(slot_query_close(bool&, QStringList&)));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spa, SLOT(slot_query_close(bool&, QStringList&)));

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabWidget);
  mainLayout->addWidget(buttonBox);
  setLayout(mainLayout);

  slot_LoadCurrent();
  tabWidget->showPage(spp);
}


ConfigDialog::~ConfigDialog()
{
  // qDebug("~ConfigDialog() is called");
}


/** This slot is called if the window will be shown or resized */
void ConfigDialog::slot_LoadCurrent()
{
  // Block multiple loads to avoid reset of changed values in the
  // config tabs. This can appear, if the screen keypad is pop up and
  // pop down. In this case the signal aboutToShow() is fired.

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
void ConfigDialog::accept()
{
  hide();

  // save change states before restoring of data
  bool projectionChange = spma->checkIsProjectionChanged();
  bool welt2000Change   = spma->checkIsWelt2000Changed();

  emit save();
  emit settingsChanged();

  if( projectionChange == false && welt2000Change == true )
    {
      // AP: There was a change in the welt 2000 config data. We must
      // trigger a reload of these data. This is only done, if the
      // projection has not been changed. A projection change includes
      // a reload of welt 2000 data.
      emit welt2000ConfigChanged();
    }
    
  QDialog::accept();
}

/** Called if the Cancel button is pressed */
void ConfigDialog::reject()
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
                                      tr("Close w/o saving?"),
                                      tr("<qt>You have made changes to:<ul>%1</ul> If you continue, these changes will <b>not</b> be saved,<br> and your changes will be lost.<p>Do you wish to close without saving anyway?</qt>").arg(pagelist),
                                      tr("Close w/o saving"),
                                      tr("Save all changes"),
                                      QString::null,
                                      0);
      if( answer == QMessageBox::Ok )
        { // the user pressed cancel
          accept();
          return;
        }
    }

  emit reload();
  QDialog::reject();
}
