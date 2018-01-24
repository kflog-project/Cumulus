/***********************************************************************
**
**   SettingsPageFlarm.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2018 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <algorithm>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "SettingsPageFlarm.h"
#include "flarmdisplay.h"
#include "layout.h"
#include "generalconfig.h"
#include "rowdelegate.h"
#include "target.h"


/**
 * Constructor
 */
SettingsPageFlarm::SettingsPageFlarm( QWidget *parent ) :
  QWidget( parent ),
  list(0),
  m_enableScroller(0)
{
  setAttribute( Qt::WA_DeleteOnClose );

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  list = new QTableWidget( 0, 4, this );
  // list->setSelectionBehavior( QAbstractItemView::SelectRows );
  list->setSelectionMode( QAbstractItemView::SingleSelection );
  list->setAlternatingRowColors( true );
  list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  list->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef ANDROID
  QScrollBar* lvsb = list->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

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

  QTableWidgetItem *item = new QTableWidgetItem( tr(" Item ") );
  list->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr(" Value ") );
  list->setHorizontalHeaderItem( 1, item );

  item = new QTableWidgetItem( tr(" Get ") );
  list->setHorizontalHeaderItem( 2, item );

  item = new QTableWidgetItem( tr(" Set ") );
  list->setHorizontalHeaderItem( 3, item );

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

#if defined(QSCROLLER) || defined(QTSCROLLER)

  m_enableScroller = new QCheckBox("][");
  m_enableScroller->setCheckState( Qt::Checked );
  m_enableScroller->setMinimumHeight( Layout::getButtonSize(12) );

  connect( m_enableScroller, SIGNAL(stateChanged(int)),
	   this, SLOT(slot_scrollerBoxToggled(int)) );

#endif

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

#if defined(QSCROLLER) || defined(QTSCROLLER)

  vbox->addWidget( m_enableScroller, 0, Qt::AlignCenter );
  vbox->addStretch(2);

#endif

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

SettingsPageFlarm::~SettingsPageFlarm()
{
}

void SettingsPageFlarm::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  list->resizeColumnToContents( 0 );
  list->resizeRowsToContents();
}

void SettingsPageFlarm::slot_AddRow( QString col0, QString col1 )
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

  if( ! col0.isEmpty() && col0 == FlarmDisplay::getSelectedObject() )
    {
      // Set this row to be selected because Flarm Id is selected in display.
      list->setCurrentCell( row, 0,
                            QItemSelectionModel::Select|QItemSelectionModel::Rows );
    }

  list->resizeColumnToContents( 0 );
  list->resizeRowsToContents();

  deleteButton->setEnabled(true);
}


void SettingsPageFlarm::slot_Ok()
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

void SettingsPageFlarm::slot_Close()
{
  setVisible( false );
  emit closed();
  QWidget::close();
}

void SettingsPageFlarm::slot_HeaderClicked( int section )
{
  list->sortByColumn( section, Qt::AscendingOrder );
}

void SettingsPageFlarm::slot_CellChanged( int row, int column )
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

void SettingsPageFlarm::slot_CellClicked( int row, int column )
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

#ifndef MAEMO5
  QString text = QInputDialog::getText( this,
                                        title,
                                        label,
                                        QLineEdit::Normal,
                                        item->text(),
                                        &ok,
                                        0,
                                        Qt::ImhNoPredictiveText );
#else
  QString text = QInputDialog::getText( this,
                                        title,
                                        label,
                                        QLineEdit::Normal,
                                        item->text(),
                                        &ok,
                                        0 );
#endif

  if( ok )
    {
      item->setText( text );
    }
}

void SettingsPageFlarm::slot_ItemSelectionChanged()
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


void SettingsPageFlarm::slot_scrollerBoxToggled( int state )
{
  if( m_enableScroller == 0 )
    {
      return;
    }

  if( state == Qt::Checked )
    {

#ifdef QSCROLLER
      list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
      list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

#ifdef ANDROID
       // Reset scrollbar style sheet to default.
       QScrollBar* lvsb = list->verticalScrollBar();
       lvsb->setStyleSheet( "" );
#endif

    }
  else if( state == Qt::Unchecked)
    {

#ifdef QSCROLLER
      QScroller::ungrabGesture( list->viewport() );
 #endif

#ifdef QTSCROLLER
       QtScroller::ungrabGesture( list->viewport() );
#endif

#ifdef ANDROID
       // Make the vertical scrollbar bigger for Android
       QScrollBar* lvsb = list->verticalScrollBar();
       lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

    }
}

void SettingsPageFlarm::loadItems2List()
{
  // Clear all old data in the list
  items.clear();

  items << "DEVTYPE;RO;ALL"
        << "SWVER;RO;ALL"
        << "SWEXP;RO;ALL"
        << "FLARMVER;RO;ALL"
        << "BUID;RO;ALL"
        << "SER;RO;ALL"
        << "REGION;RO;ALL"
        << "RADIOID;RO;ALL"
        << "CAP;RO;ALL"
        << "OBSTDB;RO;ALL"
        << "OBSTEXP;RO;ALL"
        << "IGCSER;RO;ALL";
}
