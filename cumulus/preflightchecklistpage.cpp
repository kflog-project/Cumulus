/***********************************************************************
**
**   preflightchecklistpage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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

#include "generalconfig.h"
#include "layout.h"
#include "preflightchecklistpage.h"

PreFlightCheckListPage::PreFlightCheckListPage( QWidget* parent ) :
  QWidget( parent ),
  CheckListFileName("cumulus-checklist.txt")
{
  setObjectName("PreFlightCheckListPage");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("PreFlight - Checklist") );

  if( parent )
    {
      resize( parent->size() );
    }

  QVBoxLayout *contentLayout = new QVBoxLayout;
  setLayout(contentLayout);

  m_fileDisplay = new QLabel;
  m_fileDisplay->setWordWrap( true );
  m_fileDisplay->hide();
  contentLayout->addWidget( m_fileDisplay );

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setMargin( 0 );
  contentLayout->addLayout( hbox );

  m_editor = new QTextEdit(this);
  m_editor->setReadOnly( true );
  m_editor->setInputMethodHints(Qt::ImhNoPredictiveText);

#ifdef QSCROLLER
  QScroller::grabGesture(m_editor->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_editor->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  hbox->addWidget( m_editor );

  QPushButton* toggleButton  = new QPushButton(this);
  toggleButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("list32.png")));
  toggleButton->setIconSize( QSize(IconSize, IconSize) );
  toggleButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_editButton = new QPushButton(this);
  m_editButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  m_editButton->setIconSize( QSize(IconSize, IconSize) );
  m_editButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_ok = new QPushButton(this);
  m_ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  m_ok->setIconSize(QSize(IconSize, IconSize));
  m_ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  m_ok->hide();

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png"));

  connect(toggleButton, SIGNAL(pressed()), SLOT(slotToogleFilenameDisplay()));
  connect(m_editButton, SIGNAL(pressed()), SLOT(slotEdit()));
  connect(m_ok, SIGNAL(pressed()), SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(toggleButton, 1);
  buttonBox->addSpacing(15);
  buttonBox->addWidget(m_editButton, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(m_ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  hbox->addLayout(buttonBox);
}

PreFlightCheckListPage::~PreFlightCheckListPage()
{
}

void PreFlightCheckListPage::showEvent(QShowEvent *)
{
  QString style = "QTextEdit { background: rgb(255,255,224); }";
  m_editor->setStyleSheet( style );

  QString checklist;
  loadCheckList( checklist );
  m_editor->setPlainText( checklist );

  QString path = tr("File: ") +
                 GeneralConfig::instance()->getUserDataDirectory() + "/" +
                 CheckListFileName;
  m_fileDisplay->setText( path );
}

void PreFlightCheckListPage::slotToogleFilenameDisplay()
{
  m_fileDisplay->setVisible( ! m_fileDisplay->isVisible() );
}

void PreFlightCheckListPage::slotEdit()
{
  QString style = "QTextEdit { background: white; }";
  m_editor->setStyleSheet( style );
  m_editor->setReadOnly( false );
  m_editButton->setEnabled( false );
  m_ok->show();
}

void PreFlightCheckListPage::slotAccept()
{
  if( m_editor->isReadOnly() == false )
    {
      QString style = "QTextEdit { background: rgb(255,255,224); }";
      m_editor->setStyleSheet( style );
      m_ok->hide();
      m_editor->setReadOnly( true );
      m_editButton->setEnabled( true );

      // save new text
      QString newText = m_editor->toPlainText();
      saveCheckList( newText );
      return;
    }

  emit closingWidget();
  QWidget::close();
}

void PreFlightCheckListPage::slotReject()
{
  if( m_editor->isReadOnly() == false )
    {
      QString style = "QTextEdit { background: rgb(255,255,224); }";
      m_editor->setStyleSheet( style );
      m_ok->hide();
      m_editor->setReadOnly( true );
      m_editButton->setEnabled( true );

      // load old text
      QString checklist;
      loadCheckList( checklist );
      m_editor->setText( checklist );
      return;
    }

  emit closingWidget();
  QWidget::close();
}

bool PreFlightCheckListPage::loadCheckList( QString& checklist )
{
  checklist.clear();

  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/" +
           CheckListFileName );

  if ( ! f.open( QIODevice::ReadOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      return false;
    }

  QTextStream stream( &f );
  checklist = stream.readAll();
  f.close();
  return true;
}

bool PreFlightCheckListPage::saveCheckList( QString& checklist )
{
  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/" +
           CheckListFileName );

  if ( !f.open( QIODevice::WriteOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      return false;
    }

  QTextStream stream( &f );
  stream << checklist;
  f.close();
  return true;
}
