/***********************************************************************
**
**   flarmaliaslist.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>
#include <QGroupBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>

#include "flarmaliaslist.h"
#include "flarmdisplay.h"
#include "layout.h"
#include "generalconfig.h"
#include "rowdelegate.h"
#include "target.h"

QHash<QString, QString> FlarmAliasList::aliasHash;

QMutex FlarmAliasList::mutex;

/**
 * Constructor
 */
FlarmAliasList::FlarmAliasList( QWidget *parent ) :
  QWidget( parent ),
  list(0)
{
  setAttribute( Qt::WA_DeleteOnClose );

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  list = new QTableWidget( 0, 2, this );
  list->setSelectionBehavior( QAbstractItemView::SelectItems );
  list->setAlternatingRowColors( true );

  QString style = "QTableView QTableCornerButton::section { background: gray }";
  list->setStyleSheet( style );
  QHeaderView *vHeader = list->verticalHeader();
  style = "QHeaderView::section { width: 2em }";
  vHeader->setStyleSheet( style );

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  rowDelegate = new RowDelegate( list, afMargin );
  list->setItemDelegate( rowDelegate );

  // hide vertical headers
  // QHeaderView *vHeader = list->verticalHeader();
  // vHeader->setVisible(false);

  QHeaderView* hHeader = list->horizontalHeader();
  hHeader->setStretchLastSection( true );
#if QT_VERSION >= 0x050000
  hHeader->setSectionsClickable( true );
#else
  hHeader->setClickable( true );
#endif

  connect( hHeader, SIGNAL(sectionClicked(int)),
           this, SLOT(slot_HeaderClicked(int)) );

  QTableWidgetItem *item = new QTableWidgetItem( tr(" Flarm ID ") );
  list->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr(" Alias (15) ") );
  list->setHorizontalHeaderItem( 1, item );

  connect( list, SIGNAL(cellChanged( int, int )),
           this, SLOT(slot_CellChanged( int, int )) );

  connect( list, SIGNAL(cellClicked( int, int )),
           this, SLOT(slot_CellClicked( int, int )) );

  connect( list, SIGNAL(itemSelectionChanged()),
           this, SLOT(slot_ItemSelectionChanged()) );

  topLayout->addWidget( list, 2 );

  QGroupBox* buttonBox = new QGroupBox( this );

  int buttonSize = Layout::getButtonSize();
  int iconSize   = buttonSize - 5;

  QPushButton *addButton  = new QPushButton;
  addButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "add.png" ) ) );
  addButton->setIconSize(QSize(iconSize, iconSize));
  // addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  addButton->setMinimumSize(buttonSize, buttonSize);
  addButton->setMaximumSize(buttonSize, buttonSize);

  deleteButton  = new QPushButton;
  deleteButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "delete.png" ) ) );
  deleteButton->setIconSize(QSize(iconSize, iconSize));
  // deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  deleteButton->setMinimumSize(buttonSize, buttonSize);
  deleteButton->setMaximumSize(buttonSize, buttonSize);
  deleteButton->setEnabled(false);

  QPushButton *okButton = new QPushButton;
  okButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  okButton->setIconSize(QSize(iconSize, iconSize));
  // okButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  okButton->setMinimumSize(buttonSize, buttonSize);
  okButton->setMaximumSize(buttonSize, buttonSize);

  QPushButton *closeButton = new QPushButton;
  closeButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  closeButton->setIconSize(QSize(iconSize, iconSize));
  // closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  closeButton->setMinimumSize(buttonSize, buttonSize);
  closeButton->setMaximumSize(buttonSize, buttonSize);

  connect( addButton, SIGNAL(clicked() ), this, SLOT(slot_AddRow()) );
  connect( deleteButton, SIGNAL(clicked() ), this, SLOT(slot_DeleteRows()) );
  connect( okButton, SIGNAL(clicked() ), this, SLOT(slot_Ok()) );
  connect( closeButton, SIGNAL(clicked() ), this, SLOT(slot_Close()) );

  // vertical box with operator buttons
  QVBoxLayout *vbox = new QVBoxLayout;

  vbox->setSpacing(0);
  vbox->addWidget( addButton );
  vbox->addSpacing(32);
  vbox->addWidget( deleteButton );
  vbox->addStretch(2);
  vbox->addWidget( okButton );
  vbox->addSpacing(32);
  vbox->addWidget( closeButton );
  buttonBox->setLayout( vbox );

  topLayout->addWidget( buttonBox );

  // load alias data into table
  if( ! aliasHash.isEmpty() )
    {
      QMutableHashIterator<QString, QString> it(aliasHash);

      while( it.hasNext() )
        {
          it.next();
          slot_AddRow( it.key(), it.value() );
        }

      list->sortByColumn( 1, Qt::AscendingOrder );
    }
}

FlarmAliasList::~FlarmAliasList()
{
}

void FlarmAliasList::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  list->resizeColumnToContents( 0 );
  list->resizeRowsToContents();
}

void FlarmAliasList::slot_AddRow( QString col0, QString col1 )
{
  list->setRowCount( list->rowCount() + 1 );

  int row = list->rowCount() - 1;

  QTableWidgetItem* item;

  item = new QTableWidgetItem( col0 );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  list->setItem( row, 0, item );

  item = new QTableWidgetItem( col1 );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  list->setItem( row, 1, item );

  if( ! col0.isEmpty() && col0 == FlarmDisplay::getSelectedObject() )
    {
      // Set this row to be selected because Flarm Id is selected in display.
      list->setCurrentCell( row, 0,
                            QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

  list->resizeColumnToContents( 0 );
}

void FlarmAliasList::slot_DeleteRows()
{
  if( list->rowCount() == 0 || list->columnCount() != 2 )
    {
      return;
    }

  QList<QTableWidgetItem *> items = list->selectedItems();

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

  QList<int> rows2Remove;

  for( int i = 0; i < items.size(); i++ )
    {
      QTableWidgetItem *item = items.at(i);

      int row = list->row( item );
      int col = list->column( item );

      delete list->takeItem( row, col );

      if( rows2Remove.contains( row ) )
        {
          continue;
        }

      rows2Remove.append( row );
    }

  qSort( rows2Remove );

  for( int i = rows2Remove.size()-1; i >= 0; i-- )
    {
      list->removeRow( rows2Remove.at(i) );
    }

  list->resizeColumnToContents( 0 );
}

void FlarmAliasList::slot_Ok()
{
  if( list->rowCount() != 0 )
    {
      // Check data for consistency. Empty entries are not accepted.
      for( int i = 0; i < list->rowCount(); i++ )
        {
          for( int j = 0; j < 2; j++ )
            {
              if( list->item( i, j )->text().trimmed().isEmpty() )
                {
                  QMessageBox mb( QMessageBox::Warning,
                                  tr( "Missing Entry" ),
                                  tr( "Please fill out all fields!" ),
                                  QMessageBox::Ok,
                                  this );
#ifdef ANDROID
                  mb.show();
                  QPoint pos = mapToGlobal(QPoint( width()/2 - mb.width()/2,
                                                   height()/2 - mb.height()/2 ));
                  mb.move( pos );
#endif
                  mb.exec();
                  return;
                }
            }
        }
    }

  aliasHash.clear(); // Clear alias hash

  // Save all to alias hash
  for( int i = 0; i < list->rowCount(); i++ )
    {
      // Alias names are limited to MaxAliasLength characters
      aliasHash.insert( list->item( i, 0 )->text().trimmed(),
                        list->item( i, 1 )->text().trimmed().left(MaxAliasLength) );
    }

  saveAliasData(); // Save data into file

  // Check, if only one row is selected. In this case this item is set as
  // the selected Flarm identifier. No row selection will reset the current
  // selected Flarm identifier.
  QList<QTableWidgetItem *> items = list->selectedItems();

  if( items.size() >= 0 && items.size() <= 2 )
    {
      QString selectedObject = "";

      if( items.size() > 0 )
        {
          QTableWidgetItem *item = items.at(0);
          selectedObject = item->text().trimmed();
        }

      // Report new selection to FlarmListView and FlarmDisplay
      emit newObjectSelection( selectedObject );
    }

  slot_Close();
}

void FlarmAliasList::slot_Close()
{
  setVisible( false );
  emit closed();
  QWidget::close();
}

void FlarmAliasList::slot_HeaderClicked( int section )
{
  list->sortByColumn( section, Qt::AscendingOrder );
}

void FlarmAliasList::slot_CellChanged( int row, int column )
{
  QTableWidgetItem* item = list->item ( row, column );

  if( item == static_cast<QTableWidgetItem *>(0) || row < 0 || column < 0 )
    {
      // Item can be a Null pointer, if a row has been removed.
      return;
    }

  if( column == 0 )
    {
      // Converts the Flarm identifier to upper case.
      item->setText( item->text().trimmed().toUpper() );
      list->resizeColumnToContents( 0 );
    }
  else
    {
      // Limits alias name to MaxAliasLength characters.
      item->setText( item->text().trimmed().left(MaxAliasLength) );
    }
}

void FlarmAliasList::slot_CellClicked( int row, int column )
{
  QTableWidgetItem* item = list->item ( row, column );

  if( item == static_cast<QTableWidgetItem *>(0) || row < 0 || column < 0 )
    {
      // Item can be a Null pointer, if a row has been removed.
      return;
    }

  QString title, label;

  if( column == 0 )
    {
      title = tr("Enter Flarm ID");
      label = tr("Flarm ID:");
    }
  else
    {
      title = tr("Enter Alias");
      label = tr("Alias (15):");
    }

  bool ok;
  QString text = QInputDialog::getText( this,
                                        title,
                                        label,
                                        QLineEdit::Normal,
                                        item->text(),
                                        &ok );
  if( ok  )
    {
      item->setText( text );
    }
}

void FlarmAliasList::slot_ItemSelectionChanged()
{
  bool enabled = false;

  for( int i = 0; i < list->rowCount(); i++ )
    {
      QTableWidgetItem *item0 = list->item( i, 0 );
      QTableWidgetItem *item1 = list->item( i, 1 );

      if( item0 && item1 )
        {
          if( item0->isSelected() && item1->isSelected() )
            {
              enabled = true;
              continue;
            }

          if( ! item0->isSelected() && ! item1->isSelected() )
            {
              continue;
            }

          enabled = false;
          break;
        }
    }

  deleteButton->setEnabled(enabled);
}

#warning "FLARM alias file 'cumulus-flarm.txt' is stored at User Data Directory"

/** Loads the Flarm alias data from the related file into the alias hash. */
bool FlarmAliasList::loadAliasData()
{
  mutex.lock();

  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/" +
           GeneralConfig::instance()->getFlarmAliasFileName() );

  if ( ! f.open( QIODevice::ReadOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      mutex.unlock();
      return false;
    }

  int lineNo = 0;

  // remove all alias hash data before read
  aliasHash.clear();

  QTextStream stream( &f );
  QString line;

  while ( !stream.atEnd() )
    {
      line = stream.readLine();
      lineNo++;

      if ( line.startsWith("#") || line.startsWith("$") || line.trimmed().isEmpty() )
        {
          // ignore comment and empty lines
          continue;
        }

      // A valid line entry has the format: <id>=<alias> where the equal sign
      // is the delimiter between the two parts.
      QStringList sl = line.split( "=", QString::KeepEmptyParts );

      QString key = "";
      QString val = "";

      if( sl.size() == 2 )
        {
          key = sl.at(0).trimmed();
          val = sl.at(1).trimmed();
        }

      if( sl.size() != 2 || key.isEmpty() || val.isEmpty() )
        {
          qWarning() << "Wrong entry at line" << lineNo << "of"
                     << GeneralConfig::instance()->getFlarmAliasFileName();
          continue;
        }

      // Alias names are limited to MaxAliasLength characters
      aliasHash.insert( key, val.left(MaxAliasLength) );
    }

  f.close();

  qDebug() << aliasHash.size() << "entries read from" << f.fileName();

  mutex.unlock();
  return true;
}

/** Saves the Flarm alias data from the alias hash into the related file. */
bool FlarmAliasList::saveAliasData()
{
  if( aliasHash.isEmpty() )
    {
      return false;
    }

  mutex.lock();

  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/" +
           GeneralConfig::instance()->getFlarmAliasFileName() );

  if ( !f.open( QIODevice::WriteOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      mutex.unlock();
      return false;
    }

  QTextStream stream( &f );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "# Cumulus Flarm alias file created at "
         << dtStr
         << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl;

  QMutableHashIterator<QString, QString> it(aliasHash);

  while( it.hasNext() )
    {
      it.next();
      stream << it.key() << "=" << it.value() << endl;
    }

  f.close();

  mutex.unlock();
  return true;
}
