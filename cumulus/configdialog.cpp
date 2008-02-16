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

#include <QMessageBox>

#include "configdialog.h"


ConfigDialog::ConfigDialog(QWidget * parent, const char * name) :
    Q3TabDialog(parent, name, false, Qt::WStyle_StaysOnTop), loadConfig(true)
{
  // qDebug("height=%d, width=%d", parent->height(), parent->width());

  setWindowTitle(tr("Cumulus settings"));
  setOkButton();
  setCancelButton();

  spp=new SettingsPagePersonal(this);
  addTab(spp, tr("&Personal"));

  spgl=new SettingsPageGliderList(this);
  addTab(spgl, tr("G&liders"));

  sps=new SettingsPageSector(this);
  addTab(sps, tr("&Sector"));

  spg=new SettingsPageGPS(this);
  addTab(spg, tr("&GPS"));

  spm=new SettingsPageMap(this);
  addTab(spm, tr("&Map"));

  spa=new SettingsPageAirspace(this);
  addTab(spa, tr("&Airspace"));

  spu=new SettingsPageUnits(this);
  addTab(spu, tr("&Units"));

  spi=new SettingsPageInformation(this);
  addTab(spi, tr("&Information"));

  connect(this, SIGNAL(aboutToShow()), this, SLOT(slot_LoadCurrent()));
  connect(this, SIGNAL(load()), spp, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spgl, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spg, SLOT(slot_load()));
  connect(this, SIGNAL(load()), sps, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spm, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spa, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spu, SLOT(slot_load()));
  connect(this, SIGNAL(load()), spi, SLOT(slot_load()));

  connect(this, SIGNAL(reload()), spu, SLOT(slot_load()));

  connect(this, SIGNAL(save()), spp, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spgl, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spg, SLOT(slot_save()));
  connect(this, SIGNAL(save()), sps, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spm, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spa, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spi, SLOT(slot_save()));
  connect(this, SIGNAL(save()), spu, SLOT(slot_save()));

  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spgl, SLOT(slot_query_close(bool&, QStringList&)));
  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spm, SLOT(slot_query_close(bool&, QStringList&)));
  connect(this, SIGNAL(query_close(bool&, QStringList& )),
          spa, SLOT(slot_query_close(bool&, QStringList&)));

  showPage(spp);
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

  // qDebug("slot_LoadCurrent() in config.cpp is called");

  if( loadConfig )
    loadConfig = false;
  else
    return;

  emit load();
}


/** Called if OK button is pressed */
void ConfigDialog::accept()
{
  // save change states before restoring of data
  bool projectionChange = spm->advancedPage->checkIsProjectionChanged();
  bool welt2000Change   = spm->advancedPage->checkIsWelt2000Changed();

  emit save();
  Q3TabDialog::accept();
  emit settingsChanged();

  if( projectionChange == false && welt2000Change == true )
    {
      // AP: There was a change in the welt 2000 config data. We must
      // trigger a reload of these data. This is only done, if the
      // projection has not been changed. A projection change includes
      // a reload of welt 2000 data.
      emit welt2000ConfigChanged();
    }

  delete this;
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
                                      tr("<qt>You have made changes to:<ul>%1</ul> If you continue, these changes will <b>not</b> be saved, and your changes will be lost.<br>Do you wish to close without saving anyway?</qt>").arg(pagelist),
                                      tr("Close w/o saving"),
                                      tr("Save all changes"),
                                      QString::null,
                                      0);
      if( answer == QMessageBox::Ok )
        { //the user pressed cancel
          accept();
          return;
        }
    }

  Q3TabDialog::reject();
  emit reload();
  delete this;
}
