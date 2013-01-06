/***********************************************************************
**
**   settingspageglider.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "layout.h"
#include "generalconfig.h"
#include "settingspageglider.h"

#ifdef FLICK_CHARM
#include "flickcharm.h"
#endif

#ifdef USE_NUM_PAD
#include  "glidereditornumpad.h"
#else
#include "glidereditor.h"
#endif

SettingsPageGlider::SettingsPageGlider(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageGlider");

  resize(parent->size());
  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->setSpacing(5);
  QBoxLayout *editrow=new QHBoxLayout;
  topLayout->addLayout(editrow);
  editrow->setSpacing(0);
  editrow->addStretch(10);

  QPushButton * cmdNew = new QPushButton(this);
  cmdNew->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("add.png")) );
  cmdNew->setIconSize( QSize(IconSize, IconSize) );
  editrow->addWidget(cmdNew,1);

  editrow->addSpacing(10);
  QPushButton * cmdEdit = new QPushButton(this);
  cmdEdit->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  cmdEdit->setIconSize( QSize(IconSize, IconSize) );
  editrow->addWidget(cmdEdit,1);

  editrow->addSpacing(10);
  QPushButton * cmdDel = new QPushButton(this);
  cmdDel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "delete.png" )) );
  cmdDel->setIconSize( QSize(IconSize, IconSize) );
  editrow->addWidget(cmdDel,1);

  m_list = new GliderListWidget(this);

#ifdef QSCROLLER
  QScroller::grabGesture(m_list, QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(m_list);
#endif

  topLayout->addWidget(m_list, 10);

  connect(cmdNew,  SIGNAL(clicked()), this, SLOT(slot_new()));
  connect(cmdEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(cmdDel,  SIGNAL(clicked()), this, SLOT(slot_delete()));
}

SettingsPageGlider::~SettingsPageGlider()
{
  // qDebug("SettingsPageGlider::~SettingsPageGlider() is called");
}

void SettingsPageGlider::showEvent(QShowEvent *)
{
  m_list->setFocus();
}

/** Called when a new glider needs to be made. */
void SettingsPageGlider::slot_new()
{
#ifdef USE_NUM_PAD
  GliderEditorNumPad *editor = new GliderEditorNumPad(this, 0 );
#else
  GliderEditor *editor = new GliderEditor(this, 0 );
#endif

  connect(editor, SIGNAL(newGlider(Glider*)), m_list, SLOT(slot_Added(Glider *)));
  editor->setVisible( true );
}


/** Called when the selected glider needs must be opened in the editor */
void SettingsPageGlider::slot_edit()
{
  Glider *selectedGlider = m_list->getSelectedGlider();

  if( selectedGlider == 0 )
    {
      // @AP: no glider is selected, ignore the request
      return;
    }

#ifdef USE_NUM_PAD
  GliderEditorNumPad *editor = new GliderEditorNumPad(this, selectedGlider );
#else
  GliderEditor *editor = new GliderEditor(this, selectedGlider );
#endif

  connect(editor, SIGNAL(editedGlider(Glider *)), m_list, SLOT(slot_Edited(Glider *)));
  editor->setVisible( true );
}

/** Called when the selected glider should be deleted from the catalog */
void SettingsPageGlider::slot_delete()
{
  Glider *glider = m_list->getSelectedGlider();

  if( !glider )
    {
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Delete?" ),
                  tr( "Delete selected glider?" ),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::Yes )
    {
      m_list->slot_Deleted( glider );
    }
}

/** Called to fill the tree m_list */
void SettingsPageGlider::slot_load()
{
  m_list->fillList();
}

void SettingsPageGlider::slot_save()
{
  m_list->save();
}

/* Called to ask is confirmation on the close is needed. */
void SettingsPageGlider::slot_query_close(bool& warn, QStringList& warnings)
{
  /* set warn to 'true' if the data has changed. Note that we can NOT just set warn equal to
    _changed, because that way we might erase a warning flag set by another page! */
  if( m_list->hasChanged() )
    {
      warn = true;
      warnings.append( tr( "The Glider list" ) );
    }
}
