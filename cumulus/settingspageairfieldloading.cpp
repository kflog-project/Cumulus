/***********************************************************************
**
**   settingspageairfieldloading.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 by Axel Pauli <kflog.cumulus@gmail.com>
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
#include "mainwindow.h"
#include "settingspageairfieldloading.h"

/*
 * Because Maemo 5 is using a special dialog design this window is declared
 * as a tool window.
 */
SettingsPageAirfieldLoading::SettingsPageAirfieldLoading( QWidget *parent ) :
  QWidget( parent )
{
  setObjectName("SettingsPageAirfieldLoading");
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowFlags( Qt::Tool );
  setWindowTitle(tr("Airfield loading settings"));
  setWindowModality( Qt::WindowModal );

  if( _globalMainWindow )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing(10);

  m_fileTable = new QTableWidget( 0, 1, this );
  m_fileTable->setToolTip( tr("Use check boxes to activate or deactivate file loading.") );
  m_fileTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_fileTable->setAlternatingRowColors( true );
  m_fileTable->setShowGrid( true );

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  m_fileTable->setItemDelegate( new RowDelegate( m_fileTable, afMargin ) );

  m_fileTable->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_fileTable->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture( m_fileTable->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( m_fileTable->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  QString style = "QTableView QTableCornerButton::section { background: gray }";
  m_fileTable->setStyleSheet( style );

  QHeaderView *vHeader = m_fileTable->verticalHeader();
  style = "QHeaderView::section { width: 2em }";
  vHeader->setStyleSheet( style );

  connect( m_fileTable, SIGNAL(cellClicked ( int, int )),
           SLOT(slot_toggleCheckBox( int, int )) );

  connect( m_fileTable, SIGNAL(itemSelectionChanged()),
           this, SLOT(slot_itemSelectionChanged()) );

  QHeaderView* hHeader = m_fileTable->horizontalHeader();
  hHeader->setStretchLastSection( true );

  QString loadDirs;

  for( int i = 0; i < mapDirs.size(); i++ )
    {
      if( i > 0 )
        {
          loadDirs.append("\n");
        }

      loadDirs += mapDirs.at(i) + "/airfields";
    }

  QTableWidgetItem *item = new QTableWidgetItem( loadDirs );
  item->setTextAlignment( Qt::AlignLeft );
  m_fileTable->setHorizontalHeaderItem( 0, item );

  topLayout->addWidget( m_fileTable, 10 );

  int buttonSize = Layout::getButtonSize();
  int iconSize   = buttonSize - 5;

  m_delButton = new QPushButton;
  m_delButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "delete.png" ) ) );
  m_delButton->setIconSize(QSize(iconSize, iconSize));
  m_delButton->setMinimumSize(buttonSize, buttonSize);
  m_delButton->setMaximumSize(buttonSize, buttonSize);
  m_delButton->setEnabled( false );

  QPushButton *okButton = new QPushButton;
  okButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  okButton->setIconSize(QSize(iconSize, iconSize));
  okButton->setMinimumSize(buttonSize, buttonSize);
  okButton->setMaximumSize(buttonSize, buttonSize);

  QPushButton *cancelButton = new QPushButton;
  cancelButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancelButton->setIconSize(QSize(iconSize, iconSize));
  cancelButton->setMinimumSize(buttonSize, buttonSize);
  cancelButton->setMaximumSize(buttonSize, buttonSize);

  connect( m_delButton, SIGNAL(clicked() ), this, SLOT(slot_DeleteRows()) );
  connect( okButton, SIGNAL(clicked() ), this, SLOT(slot_save()) );
  connect( cancelButton, SIGNAL(clicked() ), this, SLOT(close()) );

  // box with operator buttons
  QHBoxLayout *buttonBox = new QHBoxLayout;

  buttonBox->setSpacing(0);
  buttonBox->addWidget( m_delButton );
  buttonBox->addStretch(2);
  buttonBox->addWidget( cancelButton );
  buttonBox->addSpacing(32);
  buttonBox->addWidget( okButton );
  topLayout->addLayout( buttonBox );

  //---------------------------------------------------------------------------
  // Load table with airfield files
  QStringList preselect;

  for ( int i = 0; i < mapDirs.size(); ++i )
    {
      MapContents::addDir(preselect, mapDirs.at(i) + "/airfields", "*.aip");
      MapContents::addDir(preselect, mapDirs.at(i) + "/airfields", "*.AIP");
    }

  preselect.sort();

  int row = 0;
  m_fileTable->setRowCount( row + 1 );

  item = new QTableWidgetItem( tr("Select all"), 0 );
  item->setFlags( Qt::ItemIsEnabled );
  item->setCheckState( Qt::Unchecked );
  m_fileTable->setItem( row, 0, item );
  row++;

  for( int i = 0; i < preselect.size(); i++ )
    {
      if ( preselect.at(i).endsWith( ".AIP" ) )
        {
          // Upper case file names are converted to lower case and renamed.
          QFileInfo fInfo = preselect.at(i);
          QString path    = fInfo.absolutePath();
          QString fn      = fInfo.fileName().toLower();
          QString newFn   = path + "/" + fn;
          QFile::rename( preselect.at(i), newFn );
          preselect[i] = newFn;
        }

      m_fileTable->setRowCount( row + 1 );

      QString file = QFileInfo( preselect.at(i) ).fileName();
      item = new QTableWidgetItem( file, row );
      item->setFlags( Qt::ItemIsEnabled|Qt::ItemIsSelectable );
      item->setData( Qt::UserRole, preselect.at(i) );
      item->setCheckState( Qt::Unchecked );
      m_fileTable->setItem( row, 0, item );
      row++;
    }

  QStringList& files = GeneralConfig::instance()->getOpenAipAirfieldFileList();

  if( files.isEmpty() )
    {
      return;
    }

  if( files.at(0) == "All" )
    {
      // Set all items to checked, if All is contained in the list at the first
      // position.
      for( int i = 0; i < m_fileTable->rowCount(); i++ )
        {
          m_fileTable->item( i, 0 )->setCheckState( Qt::Checked );
        }
    }
  else
    {
      // Set the All item to unchecked.
      m_fileTable->item( 0, 0 )->setCheckState( Qt::Unchecked );

      for( int i = 1; i < m_fileTable->rowCount(); i++ )
        {
          QTableWidgetItem* item = m_fileTable->item( i, 0 );

          if( files.contains( item->text()) )
            {
              m_fileTable->item( i, 0 )->setCheckState( Qt::Checked );
            }
          else
            {
              m_fileTable->item( i, 0 )->setCheckState( Qt::Unchecked );
            }
        }
    }

  m_fileTable->resizeColumnsToContents();
  m_fileTable->resizeRowsToContents();
}

SettingsPageAirfieldLoading::~SettingsPageAirfieldLoading()
{
}

/* Called to toggle the check box of the clicked table cell. */
void SettingsPageAirfieldLoading::slot_toggleCheckBox( int row, int column )
{
  QTableWidgetItem* item = m_fileTable->item( row, column );

  if( row > 0 && m_fileTable->item( 0, 0 )->checkState() == Qt::Checked )
    {
      // All is checked, do not changed other items
      return;
    }

  item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );

  if( row == 0 && column == 0 )
    {
      // First entry was clicked. Change related check items.
      if( item->checkState() == Qt::Checked )
        {
          // All other items are checked too
          for( int i = m_fileTable->rowCount() - 1; i > 0; i-- )
            {
              m_fileTable->item( i, 0 )->setCheckState( Qt::Checked );
            }
        }
    }
}

/* Called to save data to the configuration file. */
void SettingsPageAirfieldLoading::slot_save()
{
  QStringList files;

  if( m_fileTable->item( 0, 0 )->checkState() == Qt::Checked )
    {
      // All files are selected.
      files << "All";
    }
  else
    {
      // Store only checked file items.
      for( int i = 1; i < m_fileTable->rowCount(); i++ )
        {
          QTableWidgetItem* item = m_fileTable->item( i, 0 );

          if( item->checkState() == Qt::Checked )
            {
              files << item->text();
            }
        }
    }

  QStringList oldFiles = GeneralConfig::instance()->getOpenAipAirfieldFileList();

  // save the new file list
  GeneralConfig::instance()->setOpenAipAirfieldFileList( files );

  // Check, if file list has been modified
  if( oldFiles.size() != files.size() )
    {
      // List size is different, emit signal.
      emit fileListChanged();
    }
  else
    {
      // The list size is equal, we have to check every single list element.
      // Note that the lists are always sorted.
      for( int i = 0; i < files.size(); i++ )
        {
          if( files.at(i) != oldFiles.at(i) )
            {
              // File names are different, emit signal.
              emit fileListChanged();
              break;
            }
        }
    }

  close();
}

void SettingsPageAirfieldLoading::slot_DeleteRows()
{
  if( m_fileTable->rowCount() == 0 || m_fileTable->columnCount() != 1 )
    {
      return;
    }

  QList<QTableWidgetItem *> items = m_fileTable->selectedItems();

  if( items.size() == 0 )
    {
      // no selection is active
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Delete?" ),
                  tr( "Delete selected entries?" ),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2 - mb.width()/2, height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::No )
    {
      return;
    }

  for( int i = m_fileTable->rowCount() - 1; i >= 0; i-- )
    {
      QTableWidgetItem *item = m_fileTable->item( i, 0 );

      if( item->isSelected() )
        {
          QString fn = item->data( Qt::UserRole ).toString();

          // Remove source file from disk
          QFile::remove( fn );

          // Remove compiled file from disk
          fn = fn.replace( fn.size() - 1, 1 , QChar('c') );
          QFile::remove( fn );

          // Remove row from table
          m_fileTable->removeRow( i );
        }
    }

  m_fileTable->resizeColumnsToContents();
  m_fileTable->resizeRowsToContents();
}

void SettingsPageAirfieldLoading::slot_itemSelectionChanged()
{
  if( m_fileTable->QTableWidget::selectedItems().size() > 0 )
    {
      m_delButton->setEnabled( true );
    }
  else
    {
      m_delButton->setEnabled( false );
    }
}
