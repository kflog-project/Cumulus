/***********************************************************************
**
**   flarmaliaslist.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2022 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <algorithm>

#include <QtWidgets>

#include "flarmaliaslist.h"
#include "flarmdisplay.h"
#include "layout.h"
#include "generalconfig.h"
#include "helpbrowser.h"
#include "mainwindow.h"
#include "rowdelegate.h"
#include "target.h"

QHash<QString, QPair<QString, bool> > FlarmAliasList::aliasHash;

QHash<QString, QPair<QString, bool> > FlarmAliasList::aliasShowHash;

QMutex FlarmAliasList::mutex;

/**
 * Constructor
 */
FlarmAliasList::FlarmAliasList( QWidget *parent ) :
  QWidget( parent ),
  list(0),
  m_enableScroller(0)
{
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowFlags( Qt::Tool );
  resize( MainWindow::mainWindow()->size() );

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  list = new QTableWidget( 0, 3, this );
  list->setSelectionBehavior( QAbstractItemView::SelectRows );
  // list->setSelectionMode( QAbstractItemView::SingleSelection );
  list->setAlternatingRowColors( true );
  list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  list->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );

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
  hHeader->setSectionsClickable( true );

  connect( hHeader, SIGNAL(sectionClicked(int)),
           this, SLOT(slot_HeaderClicked(int)) );

  QTableWidgetItem *item = new QTableWidgetItem( tr(" Flarm ID ") );
  list->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr(" Alias (15) ") );
  list->setHorizontalHeaderItem( 1, item );

  item = new QTableWidgetItem( tr(" Show ") );
  list->setHorizontalHeaderItem( 2, item );

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

  QPushButton *helpButton  = new QPushButton;
  helpButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "help32.png" ) ) );
  helpButton->setIconSize(QSize(iconSize, iconSize));
  // helpButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  helpButton->setMinimumSize(buttonSize, buttonSize);
  helpButton->setMaximumSize(buttonSize, buttonSize);

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

  m_enableScroller = new QCheckBox("][");
  m_enableScroller->setCheckState( Qt::Checked );
  m_enableScroller->setMinimumHeight( Layout::getButtonSize(12) );

  connect( m_enableScroller, SIGNAL(stateChanged(int)),
	   this, SLOT(slot_scrollerBoxToggled(int)) );

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

  connect( helpButton, SIGNAL(clicked() ), this, SLOT(slot_Help()) );
  connect( addButton, SIGNAL(clicked() ), this, SLOT(slot_AddRow()) );
  connect( deleteButton, SIGNAL(clicked() ), this, SLOT(slot_DeleteRows()) );
  connect( okButton, SIGNAL(clicked() ), this, SLOT(slot_Ok()) );
  connect( closeButton, SIGNAL(clicked() ), this, SLOT(slot_Close()) );

  // vertical box with operator buttons
  QVBoxLayout *vbox = new QVBoxLayout;

  vbox->setSpacing(0);
  vbox->addWidget( helpButton );
  vbox->addSpacing(32);
  vbox->addWidget( addButton );
  vbox->addSpacing(32);
  vbox->addWidget( deleteButton );
  vbox->addStretch(2);
  vbox->addWidget( m_enableScroller, 0, Qt::AlignCenter );
  vbox->addStretch(2);
  vbox->addWidget( okButton );
  vbox->addSpacing(32);
  vbox->addWidget( closeButton );
  buttonBox->setLayout( vbox );

  topLayout->addWidget( buttonBox );

  // load alias data into table
  if( ! aliasHash.isEmpty() )
    {
      QMutableHashIterator<QString, QPair<QString, bool> > it(aliasHash);

      while( it.hasNext() )
        {
          it.next();
          slot_AddRow( it.key(), it.value().first, it.value().second );
        }

      list->sortByColumn( 1, Qt::AscendingOrder );
    }
}

FlarmAliasList::~FlarmAliasList()
{
}

void FlarmAliasList::showEvent( QShowEvent *event )
{
  QHeaderView* hv = list->horizontalHeader();
  int len = hv->length() / 3;
  int len1 = len - len / 5;
  int len2 = len + (2 * len / 5);

  hv->setSectionResizeMode( QHeaderView::Fixed );
  hv->resizeSection( 0, len1 );
  hv->resizeSection( 1, len2 );
  hv->resizeSection( 2, len1 );

  list->resizeRowsToContents();
  QWidget::showEvent( event );
}

void FlarmAliasList::slot_AddRow( QString col0, QString col1, bool col2 )
{
  list->setRowCount( list->rowCount() + 1 );

  int row = list->rowCount() - 1;

  QTableWidgetItem* item;

  item = new QTableWidgetItem( col0 );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  list->setItem( row, 0, item );
  list->setCurrentItem( item );

  item = new QTableWidgetItem( col1 );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  list->setItem( row, 1, item );

  item = new QTableWidgetItem( col2 );
  item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  item->setCheckState( (col2 == false) ? Qt::Unchecked : Qt::Checked );
  list->setItem( row, 2, item );

  if( ! col0.isEmpty() && col0 == FlarmDisplay::getSelectedObject() )
    {
      // Set this row to be selected because Flarm Id is selected in display.
      list->setCurrentCell( row, 0,
                            QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

  list->resizeRowsToContents();

  deleteButton->setEnabled(true);
}

void FlarmAliasList::slot_DeleteRows()
{
  if( list->rowCount() == 0 || list->columnCount() != 3 )
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

  std::sort( rows2Remove.begin(), rows2Remove.end() );

  for( int i = rows2Remove.size()-1; i >= 0; i-- )
    {
      list->removeRow( rows2Remove.at(i) );
    }

  list->resizeColumnsToContents();
}

/** Called if the help button was pressed. */
void FlarmAliasList::slot_Help()
{
  QString file = "cumulus-flarm.html";

  HelpBrowser *hb = new HelpBrowser( this, file, "AliasList" );
  hb->resize( MainWindow::mainWindow()->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
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
                        qMakePair( list->item( i, 1 )->text().trimmed().left(MaxAliasLength),
                                   (list->item( i, 2 )->checkState() == Qt::Checked) ? true : false ) );
    }

  loadAliasShowData(); // update show data

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

  if( column == 2 )
    {
      // Toggle check box state
      if( item->checkState() == Qt::Checked )
        {
          item->setCheckState( Qt::Unchecked );
        }
      else
        {
          item->setCheckState( Qt::Checked );
        }

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
                                        &ok,
                                        Qt::Dialog,
                                        Qt::ImhNoPredictiveText );
  if( ok )
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
      QTableWidgetItem *item2 = list->item( i, 2 );

      if( item0 && item1 && item2 )
        {
          if( item0->isSelected() && item1->isSelected() && item2->isSelected() )
            {
              enabled = true;
              continue;
            }

          if( ! item0->isSelected() && ! item1->isSelected() && ! item2->isSelected() )
            {
              continue;
            }

          enabled = false;
          break;
        }
    }

  deleteButton->setEnabled( enabled );
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
      // is the delimiter between the two parts. (old format)
      QStringList sl = line.split( "=", Qt::KeepEmptyParts );

      if( sl.size() == 1 )
        {
          // The new alias file format has to be used, where the delimeter is ;
          // <id>;<alias>;<draw-flag>
          sl = line.split( ";", Qt::KeepEmptyParts );
        }

      QString key;
      QString val;
      QString checkState;

      if( sl.size() >= 2 )
        {
          key = sl.at(0).trimmed();
          val = sl.at(1).trimmed();
        }

      if( sl.size() == 3 )
        {
          checkState = sl.at(2).trimmed();
        }

      qDebug() << sl;

      if( sl.size() < 2 || key.isEmpty() || val.isEmpty() ||
          (sl.size() == 3 && checkState.isEmpty()) )
        {
          qWarning() << "Wrong entry at line" << lineNo << "of"
                     << GeneralConfig::instance()->getFlarmAliasFileName();
          continue;
        }

      // Alias names are limited to MaxAliasLength characters
      aliasHash.insert( key, qMakePair( val.left(MaxAliasLength),
                                        (checkState == "1") ? true : false) );
    }

  f.close();

  loadAliasShowData();

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
         << QCoreApplication::applicationVersion() << "\n";

  QMutableHashIterator<QString, QPair<QString, bool> > it(aliasHash);

  while( it.hasNext() )
    {
      it.next();
      stream << it.key() << ";"
             << it.value().first << ";"
             << it.value().second << "\n";
    }

  f.close();

  mutex.unlock();
  return true;
}

void FlarmAliasList::loadAliasShowData()
{
  // remove all hash data
  aliasShowHash.clear();

  if( aliasHash.isEmpty() )
    {
      return;
    }

  QMutableHashIterator<QString, QPair<QString, bool> > it(aliasHash);

  while( it.hasNext() )
    {
      it.next();

      if( it.value().second == true )
        {
          aliasShowHash.insert( it.key(), it.value() );
        }
    }
}

void FlarmAliasList::slot_scrollerBoxToggled( int state )
{
  if( m_enableScroller == 0 )
    {
      return;
    }

  if( state == Qt::Checked )
    {
      list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
    }
  else if( state == Qt::Unchecked)
    {
      QScroller::ungrabGesture( list->viewport() );
    }
}
