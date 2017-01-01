/***********************************************************************
**
**   preflightchecklistpage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2014-2016 by Axel Pauli
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
#include "layout.h"
#include "mapconfig.h"
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

  m_list = new QTableWidget( 0, 2, this );
  m_list->setSelectionBehavior( QAbstractItemView::SelectItems );
  m_list->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  m_list->setAlternatingRowColors( true );
  m_list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_list->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

  // calculates the needed icon size
  QFontMetrics qfm( font() );
  int iconSize = qfm.height();

  // Sets the icon size of a list entry
  m_list->setIconSize( QSize(iconSize, iconSize) );

  hbox->addWidget( m_list );

  connect( m_list, SIGNAL(cellDoubleClicked(int, int)),
           SLOT(slotEditCell(int, int)) );

  connect( m_list, SIGNAL(cellClicked(int, int)),
           SLOT(slotCellClicked(int, int)) );

#ifdef ANDROID
  QScrollBar* lvsb = m_list->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

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

#ifndef ANDROID
  int buttonSize = Layout::getButtonSize();
#else
  int buttonSize = Layout::getButtonSize(16);
#endif

  iconSize   = buttonSize - 5;

  QPushButton* toggleButton = new QPushButton(this);
  toggleButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("file-32.png")));
  toggleButton->setIconSize( QSize(iconSize, iconSize) );
  toggleButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  toggleButton->setMinimumSize(buttonSize, buttonSize);
  toggleButton->setMaximumSize(buttonSize, buttonSize);

  QPushButton *addButton = new QPushButton;
  addButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "add.png" ) ) );
  addButton->setIconSize(QSize(iconSize, iconSize));
  addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  addButton->setMinimumSize(buttonSize, buttonSize);
  addButton->setMaximumSize(buttonSize, buttonSize);

  m_editButton = new QPushButton(this);
  m_editButton->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  m_editButton->setIconSize( QSize(iconSize, iconSize) );
  m_editButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  m_editButton->setMinimumSize(buttonSize, buttonSize);
  m_editButton->setMaximumSize(buttonSize, buttonSize);

  m_deleteButton = new QPushButton;
  m_deleteButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "delete.png" ) ) );
  m_deleteButton->setIconSize( QSize(iconSize, iconSize) );
  m_deleteButton->setEnabled(false);
  m_deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  m_deleteButton->setMinimumSize(buttonSize, buttonSize);
  m_deleteButton->setMaximumSize(buttonSize, buttonSize);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(iconSize, iconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  cancel->setMinimumSize(buttonSize, buttonSize);
  cancel->setMaximumSize(buttonSize, buttonSize);

  m_ok = new QPushButton(this);
  m_ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  m_ok->setIconSize(QSize(iconSize, iconSize));
  m_ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  m_ok->setMinimumSize(buttonSize, buttonSize);
  m_ok->setMaximumSize(buttonSize, buttonSize);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

  connect( addButton, SIGNAL(pressed()), SLOT(slotAddRow()) );
  connect( toggleButton, SIGNAL(pressed()), SLOT(slotToogleFilenameDisplay()) );
  connect( m_editButton, SIGNAL(pressed()), SLOT(slotEdit()) );
  connect( m_deleteButton, SIGNAL(pressed()), SLOT(slotDeleteRows()) );
  connect( m_ok, SIGNAL(pressed()), SLOT(slotAccept()) );
  connect( cancel, SIGNAL(pressed()), SLOT(slotReject()) );

  QVBoxLayout *buttonBox = new QVBoxLayout;
  hbox->addLayout(buttonBox);

  buttonBox->setSpacing(0);
  buttonBox->addWidget(toggleButton);
  buttonBox->addSpacing(10 * Layout::getIntScaledDensity());
  buttonBox->addWidget(addButton);
  buttonBox->addSpacing(10 * Layout::getIntScaledDensity());
  buttonBox->addWidget(m_editButton);
  buttonBox->addSpacing(10 * Layout::getIntScaledDensity());
  buttonBox->addWidget(m_deleteButton);
  buttonBox->addSpacing(10 * Layout::getIntScaledDensity());
  buttonBox->addWidget(cancel);
  buttonBox->addSpacing(10 * Layout::getIntScaledDensity());
  buttonBox->addWidget(m_ok);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
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

void PreFlightCheckListPage::slotEditCell( int row, int column )
{
  if( column == 0 )
    {
      // Cell of column 0 can only change the icon.
      return;
    }

  QTableWidgetItem *item = m_list->item( row, column );

  if( item == 0 )
    {
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

void PreFlightCheckListPage::slotCellClicked(int row, int column)
{
  if( column == 1 )
    {
      // Cell of column 1 can only change the text.
      return;
    }

  QTableWidgetItem *item = m_list->item( row, column );

  if( item == 0 )
    {
      return;
    }

  // Change the icon to ok
  item->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "ok.png" ) ) );
}

void PreFlightCheckListPage::slotEdit()
{
  QTableWidgetItem* item = m_list->currentItem();

  if( item == static_cast<QTableWidgetItem *>(0) )
    {
      // Item can be a Null pointer!
      return;
    }

  QList<QTableWidgetSelectionRange> selRangeList = m_list->selectedRanges();

  if( selRangeList.size() == 0 )
    {
      // Nothing seems to be selected
      return;
    }

  QTableWidgetSelectionRange sr = selRangeList.at(0);

  if( sr.rowCount() > 1 )
    {
      // More than one row are selected
      return;
    }

  if( sr.rightColumn() == 1 )
    {
	  slotEditCell( m_list->currentRow(), 1 );
    }
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

	  item = m_list->takeItem( i, 1 );
	  m_list->setItem( i + 1, 1, item );
	}
    }

  QTableWidgetItem* item;

  item = new QTableWidgetItem( "" );
  item->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "help32.png" ) ) );
  item->setToolTip( tr("Click icon to mark line as checked.") );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  m_list->setItem( appendRow, 0, item );

  item = new QTableWidgetItem( text );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  m_list->setItem( appendRow, 1, item );
  m_list->setCurrentItem( item );
  m_list->resizeColumnToContents( 0 );
  m_list->resizeColumnToContents( 1 );
  m_list->resizeRowsToContents();
  m_editButton->setEnabled( true );
  m_deleteButton->setEnabled(true);
}

void PreFlightCheckListPage::slotDeleteRows()
{
  if( m_list->rowCount() == 0 || m_list->columnCount() != 2 )
    {
      return;
    }

  QList<QTableWidgetItem *> items = m_list->selectedItems();

  if( items.size() == 0 )
    {
      // no selection is active
      return;
    }

  QList<QTableWidgetSelectionRange> selRangeList = m_list->selectedRanges();

  if( selRangeList.size() == 0 )
    {
      // Nothing seems to be selected
      return;
    }

  QTableWidgetSelectionRange sr = selRangeList.at(0);

  if( sr.rightColumn() != 1 )
    {
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

  std::sort( rows2Remove.begin(), rows2Remove.end() );

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

  stream << "# Cumulus checklist, created at "
         << dtStr
         << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl;

  for( int i = 0; i < m_list->rowCount(); i++ )
    {
      QTableWidgetItem *item = m_list->item( i, 1 );

      if( ! item || item->text().trimmed().isEmpty() )
	{
	  continue;
	}

      stream << m_list->item( i, 1 )->text().trimmed() << endl;
    }

  f.close();
  return true;
}
