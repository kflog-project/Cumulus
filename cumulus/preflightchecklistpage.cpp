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
#include "rowdelegate.h"

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

  m_list = new QTableWidget( 0, 1, this );
  m_list->setSelectionBehavior( QAbstractItemView::SelectRows );
  // m_list->setSelectionMode( QAbstractItemView::SingleSelection );
  m_list->setAlternatingRowColors( true );
  m_list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_list->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
  hbox->addWidget( m_list );

  connect( m_list, SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
           SLOT(slotEditItem(QTableWidgetItem*)) );

#ifdef QSCROLLER
  QScroller::grabGesture(m_list->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_list->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  QString style = "QTableView QTableCornerButton::section { background: gray }";
  m_list->setStyleSheet( style );
  QHeaderView *vHeader = m_list->verticalHeader();
  style = "QHeaderView::section { width: 2em }";
  vHeader->setStyleSheet( style );

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  rowDelegate = new RowDelegate( m_list, afMargin );
  m_list->setItemDelegate( rowDelegate );

  QHeaderView* hHeader = m_list->horizontalHeader();
  hHeader->setStretchLastSection( true );
  hHeader->hide();

  QTableWidgetItem *item = new QTableWidgetItem( tr(" Check Point ") );
  m_list->setHorizontalHeaderItem( 0, item );

  QPushButton* toggleButton = new QPushButton(this);
  toggleButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("list32.png")));
  toggleButton->setIconSize( QSize(IconSize, IconSize) );
  toggleButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *addButton = new QPushButton;
  addButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "add.png" ) ) );
  addButton->setIconSize(QSize(IconSize, IconSize));
  addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_editButton = new QPushButton(this);
  m_editButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  m_editButton->setIconSize( QSize(IconSize, IconSize) );
  m_editButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_deleteButton = new QPushButton;
  m_deleteButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "delete.png" ) ) );
  m_deleteButton->setIconSize( QSize(IconSize, IconSize) );
  m_deleteButton->setEnabled(false);
  m_deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_ok = new QPushButton(this);
  m_ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  m_ok->setIconSize(QSize(IconSize, IconSize));
  m_ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png"));

  connect( addButton, SIGNAL(pressed()), SLOT(slotAddRow()) );
  connect( toggleButton, SIGNAL(pressed()), SLOT(slotToogleFilenameDisplay()) );
  connect( m_editButton, SIGNAL(pressed()), SLOT(slotEdit()) );
  connect( m_deleteButton, SIGNAL(pressed()), SLOT(slotDeleteRows()) );
  connect( m_ok, SIGNAL(pressed()), SLOT(slotAccept()) );
  connect( cancel, SIGNAL(pressed()), SLOT(slotReject()) );

  QVBoxLayout *buttonBox = new QVBoxLayout;
  hbox->addLayout(buttonBox);

  buttonBox->setSpacing(0);

#ifndef MAEMO
  buttonBox->addWidget(toggleButton, 1);
  buttonBox->addSpacing(20);
  buttonBox->addWidget(addButton, 1);
  buttonBox->addSpacing(20);
  buttonBox->addWidget(m_editButton, 1);
  buttonBox->addSpacing(20);
  buttonBox->addWidget(m_deleteButton, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(m_ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
#else
  // Under Maemo the space is limited at the right side of the window
  buttonBox->addWidget(toggleButton, 1);
  buttonBox->addSpacing(10);
  buttonBox->addWidget(addButton, 1);
  buttonBox->addSpacing(10);
  buttonBox->addWidget(m_editButton, 1);
  buttonBox->addSpacing(10);
  buttonBox->addWidget(m_deleteButton, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(15);
  buttonBox->addWidget(m_ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
#endif
}

PreFlightCheckListPage::~PreFlightCheckListPage()
{
}

void PreFlightCheckListPage::showEvent(QShowEvent *)
{
  QString path = tr("File: ") +
                 GeneralConfig::instance()->getUserDataDirectory() + "/" +
                 CheckListFileName;
  m_fileDisplay->setText( path );
  loadCheckList();

  if( m_list->rowCount() > 0 )
    {
      m_editButton->setEnabled( true );
      m_deleteButton->setEnabled( true );
      m_list->setCurrentItem( m_list->item( 0, 0 ) );
    }
  else
    {
      m_editButton->setEnabled( false );
      m_deleteButton->setEnabled( false );
    }

  m_ok->hide();
}

void PreFlightCheckListPage::slotToogleFilenameDisplay()
{
  m_fileDisplay->setVisible( ! m_fileDisplay->isVisible() );
}

void PreFlightCheckListPage::slotEditItem( QTableWidgetItem* item )
{
  if( item == static_cast<QTableWidgetItem *>(0) )
    {
      // Item can be a Null pointer!
      return;
    }

  m_editButton->setEnabled( false );

  QString title = tr("Edit Checkpoint");
  QString label = tr("Text:");

  QInputDialog qid( this );
  qid.setWindowTitle( title );
  qid.setLabelText( label );
  qid.setInputMode( QInputDialog::TextInput );

#ifndef MAEMO5
  qid.setInputMethodHints( Qt::ImhNoPredictiveText );
#endif

  qid.setTextValue( item->text() );
  qid.show();
  qid.setMinimumWidth( width() );

  int result = qid.exec();

  if( result == QDialog::Accepted )
    {
      item->setText( qid.textValue() );
      m_ok->show();
    }

  m_editButton->setEnabled( true );
}

void PreFlightCheckListPage::slotEdit()
{
  QTableWidgetItem* item = m_list->currentItem();

  if( item == static_cast<QTableWidgetItem *>(0) )
    {
      // Item can be a Null pointer!
      return;
    }

  QList<QTableWidgetItem *> items = m_list->selectedItems();

  if( items.size() != 1 )
    {
      // Only one item must be selected.
      return;
    }

  slotEditItem( item );
}

void PreFlightCheckListPage::slotItemSelectionChanged()
{
  if( m_list->selectedItems().size() != 1 )
    {
      m_editButton->setEnabled( false );
    }
  else
    {
      m_editButton->setEnabled( true );
    }
}

void PreFlightCheckListPage::slotAddRow( QString text )
{
  m_list->setRowCount( m_list->rowCount() + 1 );

  int lastRow   = m_list->rowCount() - 1;
  int appendRow = lastRow;

  if( lastRow > 0 && m_list->currentRow() >= 0 &&
      m_list->currentRow() < lastRow )
    {
      appendRow = m_list->currentRow() + 1;

      for( int i = lastRow - 1; i >= appendRow; i-- )
	{
	  QTableWidgetItem* item = m_list->takeItem( i, 0 );
	  m_list->setItem( i + 1, 0, item );
	}
    }

  QTableWidgetItem* item;

  item = new QTableWidgetItem( text );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  m_list->setItem( appendRow, 0, item );
  m_list->setCurrentItem( item );
  m_list->resizeColumnToContents( 0 );
  m_list->resizeRowsToContents();
  m_editButton->setEnabled( true );
  m_deleteButton->setEnabled(true);
}

void PreFlightCheckListPage::slotDeleteRows()
{
  if( m_list->rowCount() == 0 || m_list->columnCount() != 1 )
    {
      return;
    }

  QList<QTableWidgetItem *> items = m_list->selectedItems();

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

      int row = m_list->row( item );
      int col = m_list->column( item );

      delete m_list->takeItem( row, col );

      if( rows2Remove.contains( row ) )
        {
          continue;
        }

      rows2Remove.append( row );
    }

  qSort( rows2Remove );

  for( int i = rows2Remove.size()-1; i >= 0; i-- )
    {
      m_list->removeRow( rows2Remove.at(i) );
    }

  m_list->resizeColumnToContents( 0 );

  m_ok->show();

  if( m_list->rowCount() == 0 )
    {
      m_editButton->setEnabled( false );
      m_deleteButton->setEnabled( false );
    }

}

void PreFlightCheckListPage::slotAccept()
{
  saveCheckList();
  emit closingWidget();
  QWidget::close();
}

void PreFlightCheckListPage::slotReject()
{
  emit closingWidget();
  QWidget::close();
}

bool PreFlightCheckListPage::loadCheckList()
{
  m_list->clear();

  QFile f( GeneralConfig::instance()->getUserDataDirectory() + "/" +
           CheckListFileName );

  if ( ! f.open( QIODevice::ReadOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      return false;
    }

  QTextStream stream( &f );
  QString line;

  while( !stream.atEnd() )
    {
      line = stream.readLine().trimmed();

      if( line.isEmpty() || line.startsWith( "# Cumulus") )
	{
	  continue;
	}

      slotAddRow( line );
    }

  f.close();
  return true;
}

bool PreFlightCheckListPage::saveCheckList()
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

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "# Cumulus checklist file created at "
         << dtStr
         << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl;

  for( int i = 0; i < m_list->rowCount(); i++ )
    {
      QTableWidgetItem *item = m_list->item( i, 0 );

      if( ! item || item->text().trimmed().isEmpty() )
	{
	  continue;
	}

      stream << m_list->item( i, 0 )->text().trimmed() << endl;
    }

  f.close();
  return true;
}
