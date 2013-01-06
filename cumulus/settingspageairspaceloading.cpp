/***********************************************************************
**
**   settingspageairspaceloading.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2011-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "generalconfig.h"
#include "mainwindow.h"
#include "settingspageairspaceloading.h"

#ifdef FLICK_CHARM
#include "flickcharm.h"
#endif

/*
 * Because Maemo 5 is using a special dialog design this window is declared
 * as a tool window.
 */
SettingsPageAirspaceLoading::SettingsPageAirspaceLoading( QWidget *parent ) :
  QWidget( parent )
{
  setObjectName("SettingsPageAirspaceLoading");
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowFlags( Qt::Tool );
  setWindowTitle(tr("Airspace loading settings"));
  setWindowModality( Qt::WindowModal );

  if( _globalMainWindow )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing(10);

  fileTable = new QTableWidget( 0, 1, this );
  fileTable->setToolTip( tr("Use check boxes to activate or deactivate file loading.") );
  fileTable->setSelectionBehavior( QAbstractItemView::SelectRows );
  fileTable->setShowGrid( true );

#ifdef QSCROLLER
  QScroller::grabGesture(fileTable, QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(fileTable);
#endif

  QString style = "QTableView QTableCornerButton::section { background: gray }";
  fileTable->setStyleSheet( style );

  QHeaderView *vHeader = fileTable->verticalHeader();
  style = "QHeaderView::section { width: 2em }";
  vHeader->setStyleSheet( style );

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  fileTable->setItemDelegate( new RowDelegate( fileTable, afMargin ) );

  connect( fileTable, SIGNAL(cellClicked ( int, int )),
           SLOT(slot_toggleCheckBox( int, int )) );

  QHeaderView* hHeader = fileTable->horizontalHeader();
  hHeader->setStretchLastSection( true );

  QTableWidgetItem *item = new QTableWidgetItem( tr("Airspace Files") );
  fileTable->setHorizontalHeaderItem( 0, item );

  topLayout->addWidget( fileTable, 10 );

  QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok |
                                                      QDialogButtonBox::Cancel );

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(slot_save()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

  topLayout->addWidget( buttonBox );

  //---------------------------------------------------------------------------
  // Load table with openair files
  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();
  QStringList preselect;

  for ( int i = 0; i < mapDirs.size(); ++i )
    {
      MapContents::addDir(preselect, mapDirs.at(i) + "/airspaces", "*.txt");
      MapContents::addDir(preselect, mapDirs.at(i) + "/airspaces", "*.TXT");
    }

  preselect.sort();

  int row = 0;
  fileTable->setRowCount( row + 1 );

  item = new QTableWidgetItem( tr("Select all"), 0 );
  item->setFlags( Qt::ItemIsEnabled );
  item->setCheckState( Qt::Unchecked );
  fileTable->setItem( row, 0, item );
  row++;

  for( int i = 0; i < preselect.size(); i++ )
    {
      if ( preselect.at(i).endsWith( ".TXT" ) )
        {
          // Upper case file names are converted to lower case and renamed.
          QFileInfo fInfo = preselect.at(i);
          QString path    = fInfo.absolutePath();
          QString fn      = fInfo.fileName().toLower();
          QString newFn   = path + "/" + fn;
          QFile::rename( preselect.at(i), newFn );
          preselect[i] = newFn;
        }

      fileTable->setRowCount( row + 1 );

      QString file = QFileInfo( preselect.at(i) ).fileName();
      item = new QTableWidgetItem( file, row );
      item->setFlags( Qt::ItemIsEnabled );
      item->setCheckState( Qt::Unchecked );
      fileTable->setItem( row, 0, item );
      row++;
    }

  QStringList& files = GeneralConfig::instance()->getAirspaceFileList();

  if( files.isEmpty() )
    {
      return;
    }

  if( files.at(0) == "All" )
    {
      // Set all items to checked, if All is contained in the list at the first
      // position.
      for( int i = 0; i < fileTable->rowCount(); i++ )
        {
          fileTable->item( i, 0 )->setCheckState( Qt::Checked );
        }
    }
  else
    {
      // Set the All item to unchecked.
      fileTable->item( 0, 0 )->setCheckState( Qt::Unchecked );

      for( int i = 1; i < fileTable->rowCount(); i++ )
        {
          QTableWidgetItem* item = fileTable->item( i, 0 );

          if( files.contains( item->text()) )
            {
              fileTable->item( i, 0 )->setCheckState( Qt::Checked );
            }
          else
            {
              fileTable->item( i, 0 )->setCheckState( Qt::Unchecked );
            }
        }
    }
}

SettingsPageAirspaceLoading::~SettingsPageAirspaceLoading()
{
}

/* Called to toggle the check box of the clicked table cell. */
void SettingsPageAirspaceLoading::slot_toggleCheckBox( int row, int column )
{
  QTableWidgetItem* item = fileTable->item( row, column );

  if( row > 0 && fileTable->item( 0, 0 )->checkState() == Qt::Checked )
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
          for( int i = fileTable->rowCount() - 1; i > 0; i-- )
            {
              fileTable->item( i, 0 )->setCheckState( Qt::Checked );
            }
        }
    }
}

/* Called to save data to the configuration file. */
void SettingsPageAirspaceLoading::slot_save()
{
  QStringList files;

  if( fileTable->item( 0, 0 )->checkState() == Qt::Checked )
    {
      // All files are selected.
      files << "All";
    }
  else
    {
      // Store only checked file items.
      for( int i = 1; i < fileTable->rowCount(); i++ )
        {
          QTableWidgetItem* item = fileTable->item( i, 0 );

          if( item->checkState() == Qt::Checked )
            {
              files << item->text();
            }
        }
    }

  QStringList oldFiles = GeneralConfig::instance()->getAirspaceFileList();

  // save the new file list
  GeneralConfig::instance()->setAirspaceFileList( files );

  // Check, if file list has been modified
  if( oldFiles.size() != files.size() )
    {
      // List size is different, emit signal.
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
      emit airspaceFileListChanged();
      QApplication::restoreOverrideCursor();
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
              QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
              emit airspaceFileListChanged();
              QApplication::restoreOverrideCursor();
              break;
            }
        }
    }

  close();
}
