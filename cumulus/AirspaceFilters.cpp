/***********************************************************************
**
**   AirspaceFilters.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2020-2022 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <HelpBrowser.h>
#include <algorithm>

#include <QtWidgets>

#include "AirspaceFilters.h"
#include "layout.h"
#include "generalconfig.h"
#include "map.h"
#include "rowdelegate.h"

QHash<QString, QMultiHash<QString, QString> > AirspaceFilters::countryHash;

QMutex AirspaceFilters::mutex;

/**
 * Constructor
 */
AirspaceFilters::AirspaceFilters( QWidget *parent ) :
  QWidget( parent ),
  table(0),
  m_enableScroller(0)
{
  setObjectName("AirspaceFilters");
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowFlags( Qt::Tool );
  setWindowTitle(tr("Airspace filters"));
  setWindowModality( Qt::WindowModal );

  if( parent )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( parent->size() );
    }

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  table = new QTableWidget( 0, 3, this );
  table->setSelectionBehavior( QAbstractItemView::SelectRows );
  // list->setSelectionMode( QAbstractItemView::SingleSelection );
  table->setAlternatingRowColors( true );
  table->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  table->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

  QScroller::grabGesture( table->viewport(), QScroller::LeftMouseButtonGesture );

  QString style = "QTableView QTableCornerButton::section { background: gray }";
  table->setStyleSheet( style );
  QHeaderView *vHeader = table->verticalHeader();
  style = "QHeaderView::section { width: 2em }";
  vHeader->setStyleSheet( style );

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  rowDelegate = new RowDelegate( table, afMargin );
  table->setItemDelegate( rowDelegate );

  // hide vertical headers
  // QHeaderView *vHeader = list->verticalHeader();
  // vHeader->setVisible(false);

  QHeaderView* hHeader = table->horizontalHeader();
  hHeader->setStretchLastSection( true );
  hHeader->setSectionsClickable( true );

  connect( hHeader, SIGNAL(sectionClicked(int)),
           this, SLOT(slot_HeaderClicked(int)) );

  QTableWidgetItem *item = new QTableWidgetItem( tr(" State ") );
  table->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr(" Country, AS-Type, AS-Name ") );
  table->setHorizontalHeaderItem( 1, item );

  item = new QTableWidgetItem( tr(" Command ") );
  table->setHorizontalHeaderItem( 2, item );

  connect( table, SIGNAL(cellClicked( int, int )),
           this, SLOT(slot_CellClicked( int, int )) );

  topLayout->addWidget( table, 2 );

  QGroupBox* buttonBox = new QGroupBox( this );

  int buttonSize = Layout::getButtonSize();
  int iconSize   = buttonSize - 5;

  QPushButton *helpButton = new QPushButton(this);
  helpButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
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

  connect( this, SIGNAL(airspaceFiltersChanged( Map::mapLayer ) ),
           Map::getInstance(), SLOT(slotRedraw( Map::mapLayer )) );

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

  // load country data from file into table
  loadDataFromFile();

  show();
}

AirspaceFilters::~AirspaceFilters()
{
}

void AirspaceFilters::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  table->resizeColumnsToContents();
  table->resizeRowsToContents();
  QWidget::showEvent( event );
}

QString AirspaceFilters::getFilterFileName()
{
  QString fn = GeneralConfig::instance()->getUserDataDirectory() + "/" +
               GeneralConfig::instance()->getAirspaceFlitersFileName();

  return fn;
}

void AirspaceFilters::loadDataFromFile()
{
  mutex.lock();

  QFile f( getFilterFileName() );

  if ( ! f.open( QIODevice::ReadOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      mutex.unlock();
      return;
    }

  int lineNo = 0;

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

      // A valid line entry has the format:
      // <activated (True/False)>,<country>,<AS-Type>,<Name>
      QStringList sl = line.split( ",", Qt::KeepEmptyParts );

      if( sl.size() != 4 )
        {
          qWarning() << "Wrong element count at line" << lineNo << "of"
                     << GeneralConfig::instance()->getAirspaceFlitersFileName();
          continue;
        }

      slot_AddRow( sl[0] == "False" ? false : true,
                   sl[1] + "," + sl[2] + "," + sl[3] );
    }

  f.close();

  if( table->rowCount() > 0 )
    {
      deleteButton->setEnabled(true);
      table->sortByColumn( 1, Qt::AscendingOrder );
    }


  mutex.unlock();
  return;
}

/** Save table data into the file. */
void AirspaceFilters::saveData2File()
{
  mutex.lock();

  QFile f( getFilterFileName() );

  if ( !f.open( QIODevice::WriteOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      mutex.unlock();
      return;
    }

  QTextStream stream( &f );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "# Cumulus Airspace filters file created at "
         << dtStr
         << " by Cumulus "
         << QCoreApplication::applicationVersion() << Qt::endl;

    for( int i = 0; i < table->rowCount(); i++ )
      {
        QTableWidgetItem *item0 = table->item( i, 0 );
        QTableWidgetItem *item1 = table->item( i, 1 );

        if( item1->text().trimmed().isEmpty() )
          {
            // ignore empty filter definitions
            continue;
          }

        QStringList sl = item1->text().split( ",", Qt::KeepEmptyParts );

        // A valid line entry has the format:
        // <activated (True/False)>,<country>,<AS-Type>,<Name>
        QString activated;
        activated = (item0->checkState() == Qt::Checked) ? "True," : "False,";

        stream << activated
               << sl[0].trimmed() << ","
               << sl[1].trimmed() << ","
               << sl[2].trimmed()
               << Qt::endl;
      }

  f.close();

  mutex.unlock();
  return;
}

void AirspaceFilters::slot_AddRow( bool col0, QString col1 )
{
  table->setRowCount( table->rowCount() + 1 );

  int row = table->rowCount() - 1;

  QTableWidgetItem *checkBoxItem = new QTableWidgetItem();
  checkBoxItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
  checkBoxItem->setCheckState( Qt::Checked );
  checkBoxItem->setTextAlignment( Qt::AlignCenter );

  if( col0 == false )
    {
      checkBoxItem->setCheckState( Qt::Unchecked );
      checkBoxItem->setText( tr("off") );
    }
  else
    {
      checkBoxItem->setText( tr("on") );
    }

  table->setItem( row, 0, checkBoxItem );

  QTableWidgetItem* item;
  item = new QTableWidgetItem( col1 );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

  table->setItem( row, 1, item );
  table->setCurrentItem( item );

  item = new QTableWidgetItem( tr("letters up") );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  item->setTextAlignment( Qt::AlignCenter );

  table->setItem( row, 2, item );
  table->setCurrentItem( item );

  table->resizeColumnsToContents();
  table->resizeRowsToContents();

  deleteButton->setEnabled(true);
}

void AirspaceFilters::slot_DeleteRows()
{
  if( table->rowCount() == 0 || table->columnCount() != 3 )
    {
      return;
    }

  QList<QTableWidgetItem *> items = table->selectedItems();

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

      int row = table->row( item );
      int col = table->column( item );

      delete table->takeItem( row, col );

      if( rows2Remove.contains( row ) )
        {
          continue;
        }

      rows2Remove.append( row );
    }

  std::sort( rows2Remove.begin(), rows2Remove.end() );

  for( int i = rows2Remove.size()-1; i >= 0; i-- )
    {
      table->removeRow( rows2Remove.at(i) );
    }

  if( table->rowCount() == 0 )
    {
      deleteButton->setEnabled( false );
    }

  table->resizeColumnsToContents();
}

void AirspaceFilters::slot_Help()
{
  QString file = "cumulus-settings-airspace.html";

  HelpBrowser *hb = new HelpBrowser( this, file, "LR_Filter" );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void AirspaceFilters::slot_Ok()
{
  if( table->rowCount() != 0 )
    {
      // Check data for consistency. Empty entries are not accepted.
      for( int i = 0; i < table->rowCount(); i++ )
        {
          QString filter = table->item( i, 1 )->text().trimmed();

          if( filter.isEmpty() )
            {
              // ignore an empty filter definition.
              continue;
            }

            // <activated (True/False)>,<country>,<AS-Type>,<AS-Name>
            QStringList sl = filter.split( ",", Qt::KeepEmptyParts );

            if( sl.size() != 3 )
              {
                QMessageBox mb( QMessageBox::Warning,
                                tr( "Filter definition incomplete" ),
                                tr( "Expecting filter elements: <country>,<AS-Type>,<AS-Name>" ),
                                QMessageBox::Ok,
                                this );
                mb.exec();
                return;
              }
        }
    }

  countryHash.clear(); // Clear country hash

  // Save all activated filters to country hash
  for( int i = 0; i < table->rowCount(); i++ )
    {
      if( table->item( i, 0 )->checkState() == Qt::Unchecked ||
          table->item( i, 1 )->text().trimmed().isEmpty() == true )
        {
          // ignore this filter, it is deactivated or undefined
          continue;
        }

      QString text1 = table->item( i, 1 )->text().trimmed();

      // <activated (True/False)>,<country>,<AS-Type>,<Name>
      QStringList sl = text1.split( ",", Qt::KeepEmptyParts );

      if( countryHash.contains( sl[0].trimmed() ) )
        {
          // Key is available, add AS entry to asHash
          QMultiHash<QString, QString>& asHash = countryHash[ sl[0].trimmed() ];
          asHash.insert( sl[1].trimmed(), sl[2].trimmed() );
        }
      else
        {
          // Key not available, create an asHash
          QMultiHash<QString, QString> asHash;
          asHash.insert( sl[1].trimmed(), sl[2].trimmed() );
          countryHash.insert( sl[0].trimmed(), asHash );
        }
    }

  saveData2File(); // Save all data from the table into the file
  emit airspaceFiltersChanged( Map::airspaces );
  slot_Close();
}

void AirspaceFilters::slot_Close()
{
  setVisible( false );
  QWidget::close();
}

void AirspaceFilters::slot_HeaderClicked( int section )
{
  table->sortByColumn( section, Qt::AscendingOrder );
}

void AirspaceFilters::slot_CellClicked( int row, int column )
{
  QTableWidgetItem* item = table->item ( row, column );

  if( item == static_cast<QTableWidgetItem *>(0) || row < 0 || column < 0 )
    {
      // Item can be a Null pointer, if a row has been removed.
      return;
    }

  if( column == 0 )
    {
      if( item->checkState() == Qt::Checked )
        {
          item->setCheckState( Qt::Unchecked );
          item->setText( tr("off") );
        }
      else
        {
          item->setCheckState( Qt::Checked );
          item->setText( tr("on") );
        }

      return;
    }

  if( column == 1 )
    {
      QString title = tr("Enter Country, AS-Type, AS-Name");
      QString label = tr("Country, AS-Type, AS-Name:");

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

      return;
    }

  if( column == 2 )
    {
      QTableWidgetItem* item1 = table->item ( row, 1 );

      if( item1 == static_cast<QTableWidgetItem *>(0) )
        {
          // Item can be a Null pointer, if a row has been removed.
          return;
        }

      if( item1->text().size() == 0 )
        {
          return;
        }

      if( item->text() == tr("letters up") )
        {
          item1->setText( item1->text().toUpper() );
          item->setText( tr("letters down") );
        }
      else
        {
          item1->setText(item1->text().toLower() );
          item->setText( tr("letters up") );
        }

      return;
    }
}

/** Loads the airspace filters data from the related file into the country hash. */
bool AirspaceFilters::loadFilterData()
{
  mutex.lock();

  QFile f( getFilterFileName() );

  if ( ! f.open( QIODevice::ReadOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      mutex.unlock();
      return false;
    }

  int lineNo = 0;

  // remove all hash data before read
  countryHash.clear();

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

      // A valid line entry has the format:
      // <activated (True/False)>,<country>,<AS-Type>,<Name>
      QStringList sl = line.split( ",", Qt::KeepEmptyParts );

      if( sl.size() != 4 )
        {
          qWarning() << "Wrong element count at line" << lineNo << "of"
                     << GeneralConfig::instance()->getAirspaceFlitersFileName();
          continue;
        }

      if( sl[0] == "False" )
        {
          // Filter is deactivated, ignore it.
          continue;
        }

      if( countryHash.contains( sl[1] ) == true )
        {
          // Key is available, add AS entry to asHash
          QMultiHash<QString, QString>& asHash = countryHash[ sl[1] ];
          asHash.insert( sl[2], sl[3] );
        }
      else
        {
          // Key not available, create an asHash
          QMultiHash<QString, QString> asHash;
          asHash.insert( sl[2], sl[3] );
          countryHash.insert( sl[1], asHash );
        }
    }

  f.close();

  qDebug() << countryHash.size() << "entries read from" << f.fileName();

  QList<QString> keys = countryHash.keys();

  for( int i = 0; i < keys.size(); i++ )
    {
      qDebug() << "Key=" << keys.at(i) << " Value=" << countryHash[keys.at(i)];
    }

  mutex.unlock();
  return true;
}

void AirspaceFilters::slot_scrollerBoxToggled( int state )
{
  if( m_enableScroller == 0 )
    {
      return;
    }

  if( state == Qt::Checked )
    {
      table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      QScroller::grabGesture( table->viewport(), QScroller::LeftMouseButtonGesture );
    }
  else if( state == Qt::Unchecked)
    {
      QScroller::ungrabGesture( table->viewport() );
    }
}
