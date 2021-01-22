/***********************************************************************
**
**   logbook.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012-2016 Axel Pauli
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

#include "generalconfig.h"
#include "igclogger.h"
#include "logbook.h"
#include "layout.h"
#include "mainwindow.h"
#include "rowdelegate.h"

/**
 * Constructor
 */
Logbook::Logbook( QWidget *parent ) :
  QWidget( parent ),
  m_tableModified(false)
{
  setObjectName("Logbook");
  setWindowFlags(Qt::Tool);
  setWindowTitle( tr("Logbook"));
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  // Resize the window to the same size as the main window has. That will
  // completely hide the parent window.
  resize( MainWindow::mainWindow()->size() );

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  m_table = new QTableWidget( 0, 8, this );

  m_table->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_table->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef ANDROID
  QScrollBar* lvsb = m_table->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture( m_table->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( m_table->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  m_table->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_table->setAlternatingRowColors( true );

  QHeaderView* hHeader = m_table->horizontalHeader();

  // that makes trouble on N810
  // hHeader->setStretchLastSection( true );
#if QT_VERSION >= 0x050000
  hHeader->setSectionsClickable( true );
#else
  hHeader->setClickable( true );
#endif

  connect( hHeader, SIGNAL(sectionClicked(int)),
           this, SLOT(slot_HeaderClicked(int)) );

  setTableHeader();
  topLayout->addWidget( m_table, 2 );

  QGroupBox* buttonBox = new QGroupBox( this );

  int buttonSize = Layout::getButtonSize();
  int iconSize   = buttonSize - 5;

  m_deleteAllButton  = new QPushButton;
  m_deleteAllButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "clear.png" ) ) );
  m_deleteAllButton->setIconSize(QSize(iconSize, iconSize));
  m_deleteAllButton->setMinimumSize(buttonSize, buttonSize);
  m_deleteAllButton->setMaximumSize(buttonSize, buttonSize);

  m_deleteButton  = new QPushButton;
  m_deleteButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "delete.png" ) ) );
  m_deleteButton->setIconSize(QSize(iconSize, iconSize));
  m_deleteButton->setMinimumSize(buttonSize, buttonSize);
  m_deleteButton->setMaximumSize(buttonSize, buttonSize);
  m_deleteButton->setEnabled(false);

  m_okButton = new QPushButton;
  m_okButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  m_okButton->setIconSize(QSize(iconSize, iconSize));
  m_okButton->setMinimumSize(buttonSize, buttonSize);
  m_okButton->setMaximumSize(buttonSize, buttonSize);

  QPushButton *closeButton = new QPushButton;
  closeButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  closeButton->setIconSize(QSize(iconSize, iconSize));
  closeButton->setMinimumSize(buttonSize, buttonSize);
  closeButton->setMaximumSize(buttonSize, buttonSize);

  connect( m_deleteAllButton, SIGNAL(clicked() ), this, SLOT(slot_DeleteAllRows()) );
  connect( m_deleteButton, SIGNAL(clicked() ), this, SLOT(slot_DeleteRows()) );
  connect( m_okButton, SIGNAL(clicked() ), this, SLOT(slot_Ok()) );
  connect( closeButton, SIGNAL(clicked() ), this, SLOT(slot_Close()) );

  // vertical box with operator buttons
  QVBoxLayout *vbox = new QVBoxLayout;

  vbox->setSpacing(0);
  vbox->addWidget( m_deleteAllButton );
  vbox->addSpacing(32);
  vbox->addWidget( m_deleteButton );
  vbox->addStretch(2);
  vbox->addWidget( m_okButton );
  vbox->addSpacing(32);
  vbox->addWidget( closeButton );
  buttonBox->setLayout( vbox );

  topLayout->addWidget( buttonBox );

  QString style = "QTableView QTableCornerButton::section { background: gray }";
  m_table->setStyleSheet( style );

  QHeaderView *vHeader = m_table->verticalHeader();
  style = "QHeaderView::section { width: 2em }";
  vHeader->setStyleSheet( style );

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  rowDelegate = new RowDelegate( m_table, afMargin );
  m_table->setItemDelegate( rowDelegate );

  loadLogbookData();
}

Logbook::~Logbook()
{
}

void Logbook::showEvent( QShowEvent *event )
{
  if( m_table->rowCount() == 0 )
    {
      m_okButton->setEnabled( false );
      m_deleteButton->setEnabled( false );
      m_deleteAllButton->setEnabled( false );
    }
  else
    {
      m_okButton->setEnabled( false );
      m_deleteButton->setEnabled( true );
      m_deleteAllButton->setEnabled( true );
    }

  m_table->resizeColumnsToContents();
  m_table->resizeRowsToContents();

  QWidget::showEvent( event );
}

void Logbook::setTableHeader()
{
  QTableWidgetItem *item = new QTableWidgetItem( tr("Date") );
  m_table->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr("To") );
  m_table->setHorizontalHeaderItem( 1, item );

  item = new QTableWidgetItem( tr("Lg") );
  m_table->setHorizontalHeaderItem( 2, item );

  item = new QTableWidgetItem( tr("Ft") );
  m_table->setHorizontalHeaderItem( 3, item );

  item = new QTableWidgetItem( tr("Pilot") );
  m_table->setHorizontalHeaderItem( 4, item );

  item = new QTableWidgetItem( tr("Co") );
  m_table->setHorizontalHeaderItem( 5, item );

  item = new QTableWidgetItem( tr("Type") );
  m_table->setHorizontalHeaderItem( 6, item );

  item = new QTableWidgetItem( tr("Reg") );
  m_table->setHorizontalHeaderItem( 7, item );
}

void Logbook::loadLogbookData()
{
  IgcLogger* logger = IgcLogger::instance();

  m_table->clear();
  setTableHeader();
  m_logbook.clear();

  logger->getLogbook( m_logbook );

  if( m_logbook.size() == 0 )
    {
      // no data in logbook.
      return;
    }

  for( int row = 0; row < m_logbook.size(); row++ )
    {
      m_table->setRowCount( m_table->rowCount() + 1 );

      QStringList line = m_logbook.at(row).split(";");

      for( int col = 0; col < line.size() && col < 8; col++ )
        {
          QTableWidgetItem* item;

          item = new QTableWidgetItem( " " + line.at(col) + " " );
          item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

          if( col >= 0 && col <= 3 )
            {
              item->setTextAlignment( Qt::AlignCenter );
            }

          m_table->setItem( row, col, item );
        }
    }

  m_table->resizeColumnsToContents();
  m_table->resizeRowsToContents();
}

void Logbook::slot_DeleteRows()
{
  if( m_table->rowCount() == 0 || m_table->columnCount() != 8 )
    {
      return;
    }

  QList<QTableWidgetItem *> items = m_table->selectedItems();

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

      int row = m_table->row( item );
      int col = m_table->column( item );

      delete m_table->takeItem( row, col );

      if( rows2Remove.contains( row ) )
        {
          continue;
        }

      rows2Remove.append( row );
    }

  std::sort( rows2Remove.begin(), rows2Remove.end() );

  for( int i = rows2Remove.size()-1; i >= 0; i-- )
    {
      // Remove row from table.
      m_table->removeRow( rows2Remove.at(i) );

      // Remove row from logbook
      m_logbook.removeAt( rows2Remove.at(i) );
    }

  m_table->resizeColumnsToContents();
  m_table->resizeRowsToContents();
  m_tableModified = true;

  m_okButton->setEnabled( true );

  if( m_table->rowCount() == 0 )
    {
      m_deleteButton->setEnabled( false );
      m_deleteAllButton->setEnabled( false );
    }
}

void Logbook::slot_DeleteAllRows()
{
  if( m_table->rowCount() == 0 )
    {
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Delete?" ),
                  tr( "Delete all entries?" ),
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

  m_logbook.clear();

  for( int i = m_table->rowCount() - 1; i >= 0; i-- )
    {
      // Remove row from table.
      m_table->removeRow(i);
    }

  m_table->resizeColumnsToContents();
  m_table->resizeRowsToContents();
  m_okButton->setEnabled( true );
  m_deleteButton->setEnabled( false );
  m_deleteAllButton->setEnabled( false );
  m_tableModified = true;
}

void Logbook::slot_Ok()
{
  if( m_tableModified )
    {
      IgcLogger* logger = IgcLogger::instance();
      logger->writeLogbook( m_logbook );
    }

  slot_Close();
}

void Logbook::slot_Close()
{
  setVisible( false );
  emit closed();
  QWidget::close();
}

void Logbook::slot_HeaderClicked( int section )
{
  m_table->sortByColumn( section, Qt::AscendingOrder );
}
