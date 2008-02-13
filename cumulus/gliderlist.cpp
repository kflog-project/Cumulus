/***********************************************************************
**
**   gliderlist.cpp
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

#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QPixmap>

#include "generalconfig.h"
#include "gliderlist.h"
#include "settingspagepolar.h"


SettingsPageGliderList::SettingsPageGliderList(QWidget *parent, const char *name ) : QWidget(parent,name)
{
    resize(parent->size());
    QBoxLayout *topLayout = new QVBoxLayout( this, 5 );
    QBoxLayout *editrow=new QHBoxLayout(topLayout, 0);

    editrow->addStretch(10);

    QPushButton * cmdNew = new QPushButton(this);
    cmdNew->setPixmap(GeneralConfig::instance()->loadPixmap( "new.png" ));
    cmdNew->setFlat(true);
    editrow->addWidget(cmdNew);

    QPushButton * cmdEdit = new QPushButton(this);
    cmdEdit->setPixmap(GeneralConfig::instance()->loadPixmap( "edit.png" ));
    cmdEdit->setFlat(true);
    editrow->addWidget(cmdEdit);

    QPushButton * cmdDel = new QPushButton(this);
    cmdDel->setPixmap(GeneralConfig::instance()->loadPixmap( "trash.png" ));
    cmdDel->setFlat(true);
    editrow->addWidget(cmdDel);
    //cmdDel->setEnabled(false);

    list = new GliderList(this, "gliderlist");

    topLayout->addWidget(list,10);
    //QBoxLayout *buttonrow=new QHBoxLayout(topLayout);

    connect(cmdNew,  SIGNAL(clicked()), this, SLOT(slot_new()));
    connect(cmdEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
    connect(cmdDel,  SIGNAL(clicked()), this, SLOT(slot_delete()));
}


SettingsPageGliderList::~SettingsPageGliderList()
{
    // qDebug("SettingsPageGliderList::~SettingsPageGliderList() is called");
}


void SettingsPageGliderList::showEvent(QShowEvent *)
{
    list->setFocus();
}


/** Called when a new glider needs to be made. */
void SettingsPageGliderList::slot_new()
{
    SettingsPagePolar * dlg=new SettingsPagePolar(0, "glidereditor", 0);
    connect(dlg, SIGNAL(newGlider(Glider*)), list, SLOT(slot_Added(Glider *)));

    dlg->show();
}


/** Called when the selected glider needs must be opened in the editor */
void SettingsPageGliderList::slot_edit()
{
    SettingsPagePolar * dlg=new SettingsPagePolar(0, "glidereditor", list->getSelectedGlider());
    connect(dlg, SIGNAL(editedGlider(Glider *)), list, SLOT(slot_Edited(Glider *)));

    dlg->show();
}


/** Called when the selected glider should be deleted from the catalog */
void SettingsPageGliderList::slot_delete()
{
    Glider * _tmpGlider=list->getSelectedGlider();
    if (!_tmpGlider)
        return;

    int answer= QMessageBox::warning(this,tr("Delete?"),tr("Delete highlighted\nglider?"),
                                     QMessageBox::Ok,
                                     QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default);

    if( answer == QMessageBox::Ok ) {
        list->slot_Deleted(_tmpGlider);
    }
}


void SettingsPageGliderList::slot_load()
{
    list->fillList();
}


void SettingsPageGliderList::slot_save()
{
    list->save();
}


/* Called to ask is confirmation on the close is needed. */
void SettingsPageGliderList::slot_query_close(bool& warn, QStringList& warnings)
{
    /*set warn to 'true' if the data has changed. Note that we can NOT just set warn equal to
      _changed, because that way we might erase a warning flag set by another page! */
    if (list->has_changed()) {
        warn = true;
        warnings.append(tr("the glider list"));
    }
}

/****************************************************************************
       GliderList
 ****************************************************************************/

GliderList::GliderList(QWidget * parent, const char * name):Q3ListView(parent,name)
{
    addColumn(tr("Type"),95);
    addColumn(tr("Registration"),65);
    addColumn(tr("Callsign"),45);

    setAllColumnsShowFocus(true);
    Gliders.setAutoDelete(true);
    _added=0;
    _changed=false;
    //connect (this, SIGNAL(selectionChanged(QListviewItem*)), this, SLOT(slotSelectionChanged(QListviewItem*)));
}


GliderList::~GliderList()
{
    // qDebug("GliderList::~GliderList() is called");
}


/** Retreives the gliders from the configfile, and fills the list. */
void GliderList::fillList()
{
    clear();
    Gliders.clear();

    QSettings config( QSettings::UserScope, "Cumulus" );
    config.beginGroup("Glider Data");

    QString keyname="Glider%1";
    int i=1;

    while(config.contains(keyname.arg(i))) {
        Glider * glider=new Glider;
        if (glider->load(&config ,i)) {
            Gliders.append(glider);
            new Q3ListViewItem(this, glider->type(), glider->registration(), glider->callsign(), QString::number(glider->lastSafeID()));
            //li->setPixmap(0, _globalMapConfig->getPixmap(wp->type,false,true));  //we will add a pixmap later, so don't delete li!
        } else {
            delete glider; //loading failed!
        }
        i++;
    }

    if (i>1) {
        this->setCurrentItem(this->firstChild());
    }

    config.endGroup();
    // qDebug("GliderList::fillList(): gliders=%d", Gliders.count());
    _changed = false;
}


void GliderList::save()
{
  QSettings config( QSettings::UserScope, "Cumulus" );
  config.beginGroup("Glider Data");

  QString keyname="Glider%1";

  config.remove(""); // remove all old entires

  uint u=1;
  /* check for duplicate registrations
     This method is quite expensive if the list is very long. This is not to be expected however. Most
     people will only have a couple of gliders. The number of iterations is N*(N-1)/2.
     In case of 5 gliders, the number of iterations is 10, and for 10 glider the number is 45 (worst
     case). That seems acceptable to me. For long lists, it would be more efficient to first sort
     the list, and then compare. */
  for(uint i=2;i<=Gliders.count();i++) {
    for(uint j=1;j<i;j++) {
      if(Gliders.at(j-1)->registration()==Gliders.at(i-1)->registration())
        // two gliders have the same registration, append a number to the second one.
        Gliders.at(j-1)->setRegistration(Gliders.at(j-1)->registration() + "(" + QString::number(u++) + ")");
    }
  }

  //Should we let the user know what we have done?
  // if(u>1)
  //     qDebug("changed %d registrations",u-1);

  //store gliderlist
  for(uint i=1;i<=Gliders.count();i++) {
    // qDebug("saving glider %d",i);
    Gliders.at(i-1)->safe(&config,i);
  }

  config.endGroup();
  _changed=false;
}


/** Returns a pointer to the currently highlighted glider. If take is
    true, the glider object is taken from the list too. */
Glider * GliderList::getSelectedGlider(bool take)
{
  int i,n;

  n =  Gliders.count();
  Q3ListViewItem* li = this->selectedItem();

  // @ee may be null
  if (li) {
    Glider * glider;
    for (i=0; i < n; i++) {
      glider=Gliders.at(i);
      if (glider->lastSafeID()==li->text(3).toInt()) {
        if (take) {
          return Gliders.take(i);
        } else {
          return glider;
        }
      }
    }
  }
  return 0;
}


/** Called if a glider has been edited. */
void GliderList::slot_Edited(Glider * glider)
{
  if (glider) {
    Q3ListViewItem * li;
    li=this->selectedItem();
    li->setText(0, glider->type());
    li->setText(1, glider->registration());
    li->setText(2, glider->callsign());
    li->setText(3, QString::number(glider->lastSafeID()));
    //li->setPixmap(0, _globalMapConfig->getPixmap(wp->type,false,true));
    this->sort();
    //save();

    _changed = true;
  }
}


/** Called if a glider has been added. */
void GliderList::slot_Added(Glider * glider)
{
  if (glider) {
    _added++;
    Q3ListViewItem * li;
    li=new Q3ListViewItem(this);
    li->setText(0, glider->type());
    li->setText(1, glider->registration());
    li->setText(2, glider->callsign());
    li->setText(3, QString::number(-_added)); //assign temporary ID
    setSelected(li,true);
    this->sort();
    glider->setID(-_added); //store temp ID
    Gliders.append(glider);
    //save();
    _changed = true;
  }

}


void GliderList::slot_Deleted(Glider * glider)
{
    //remove from listView
    if ( glider ) {
        Q3ListViewItemIterator it(this);

        for(;it.current();++it) {
            if (glider->lastSafeID()==it.current()->text(3).toInt()) {
                delete it.current();
                break;
            }
        }

        //remove from catalog
        Gliders.removeRef(glider);
        //save();
        _changed = true;
    }
}


/* Retrieve last selected glider */
Glider * GliderList::getStoredSelection()
{
  QSettings config( QSettings::UserScope, "Cumulus" );
  config.beginGroup("Glider Data");
  
  QString stored = config.value("lastSelected","").toString();
  
  if (!stored.isEmpty()) {
    QString keyname="Glider%1";
    int i=1;
    
    while(config.contains(keyname.arg(i))) {
      Glider * glider=new Glider;
      if (glider->load(&config ,i)) {
        if (glider->registration()==stored) {
          return glider;
        } else {
          delete glider; //this is not the glider we're looking for...
        }
      } else {
        delete glider; //loading failed!
      }
      i++;
    } //while
  } //!stored.isEmpty()
  
  config.endGroup();
  // if we end up here, there is no default: either loading failed, or
  // it is not set, or the set glider has been deleted.
  return NULL;
}


/* save last selected glider */
void GliderList::setStoredSelection(Glider* g)
{
  QSettings config( QSettings::UserScope, "Cumulus" );
  config.beginGroup("Glider Data");
  config.setValue("lastSelected", g->registration());
  config.endGroup();
}
