/***********************************************************************
**
**   settingspageglider.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2016 by Axel Pauli
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

#include "layout.h"
#include "generalconfig.h"
#include "glidereditornumpad.h"
#include "settingspageglider.h"

SettingsPageGlider::SettingsPageGlider(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageGlider");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Gliders") );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  QBoxLayout *topLayout = new QVBoxLayout;
  contentLayout->addLayout(topLayout);

  topLayout->setSpacing(10);
  QBoxLayout *editrow=new QHBoxLayout;
  topLayout->addLayout(editrow);
  editrow->setSpacing(0);
  editrow->addStretch(10);

  const int iconSize = Layout::iconSize( font() );

  QPushButton * cmdNew = new QPushButton(this);
  cmdNew->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("add.png")) );
  cmdNew->setIconSize( QSize(iconSize, iconSize) );
  editrow->addWidget(cmdNew,1);

  editrow->addSpacing( 20 * Layout::getIntScaledDensity() );
  QPushButton * cmdEdit = new QPushButton(this);
  cmdEdit->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  cmdEdit->setIconSize( QSize(iconSize, iconSize) );
  editrow->addWidget(cmdEdit,1);

  editrow->addSpacing(20 * Layout::getIntScaledDensity() );
  QPushButton * cmdDel = new QPushButton(this);
  cmdDel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "delete.png" )) );
  cmdDel->setIconSize( QSize(iconSize, iconSize) );
  editrow->addWidget(cmdDel,1);

  m_list = new GliderListWidget(this);

#ifdef ANDROID
  QScrollBar* lvsb = m_list->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture(m_list->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( m_list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  topLayout->addWidget(m_list, 10);

  connect(cmdNew,  SIGNAL(clicked()), this, SLOT(slot_new()));
  connect(cmdEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(cmdDel,  SIGNAL(clicked()), this, SLOT(slot_delete()));

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

SettingsPageGlider::~SettingsPageGlider()
{
  // qDebug("SettingsPageGlider::~SettingsPageGlider() is called");
}

void SettingsPageGlider::showEvent(QShowEvent *)
{
  m_list->setFocus();
}

void SettingsPageGlider::slotAccept()
{
  save();
  emit settingsChanged();
  QWidget::close();
}

void SettingsPageGlider::slotReject()
{
  QWidget::close();
}

/** Called when a new glider needs to be made. */
void SettingsPageGlider::slot_new()
{
  GliderEditorNumPad *editor = new GliderEditorNumPad(this, 0 );

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

  GliderEditorNumPad *editor = new GliderEditorNumPad(this, selectedGlider );

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
void SettingsPageGlider::load()
{
  m_list->fillList();
}

void SettingsPageGlider::save()
{
  m_list->save();

  if( m_list->hasChanged() )
    {
      emit settingsChanged();
    }
}
