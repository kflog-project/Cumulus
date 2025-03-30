
/***********************************************************************
**
**   KRT2Widget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2025 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class KRT2Widget
 *
 * \author Axel Pauli
 *
 * \brief KRT2 user interface to set frequencies
 *
 * This widget can set frequencies at the KRT2 device as active or standby
 * and exchange them.
 *
 * \date 2025
 *
 * \version 1.0
 */

#include <QtWidgets>

#include "KRT2Widget.h"
#include <HelpBrowser.h>
#include "generalconfig.h"
#include "layout.h"
#include "MainWindow.h"
#include "rowdelegate.h"
#include "whatsthat.h"

// Timeout in ms for waiting for a FLARM response
#define RESP_TO 5000

/**
 * Constructor
 */
KRT2Widget::KRT2Widget( QWidget *parent, QString& header, QList<Frequency>& fqList ) :
  QWidget( parent ),
  m_header( header ),
  m_table(0),
  m_fqList( fqList )
{
  setObjectName("KRT2Widget");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("KRT2 Radio Interface") );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  QVBoxLayout *vLayout = new QVBoxLayout;
  topLayout->addLayout( vLayout );

  m_headline = new QLabel( m_header, this );
  vLayout->addWidget( m_headline );

  m_table = new QTableWidget( 0, 4, this );
  // list->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_table->setSelectionMode( QAbstractItemView::SingleSelection );
  m_table->setAlternatingRowColors( true );
  m_table->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_table->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  QScroller::grabGesture( m_table->viewport(), QScroller::LeftMouseButtonGesture );

  QString style = "QTableView QTableCornerButton::section { background: gray }";
  m_table->setStyleSheet( style );
  QHeaderView *vHeader = m_table->verticalHeader();
  style = "QHeaderView::section { width: 2em }";
  vHeader->setStyleSheet( style );

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  m_rowDelegate = new RowDelegate( m_table, afMargin );
  m_table->setItemDelegate( m_rowDelegate );

  // hide vertical headers
  // QHeaderView *vHeader = list->verticalHeader();
  // vHeader->setVisible(false);

  QTableWidgetItem *item;

  item = new QTableWidgetItem( tr("Frequency") );
  m_table->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr("Active") );
  m_table->setHorizontalHeaderItem( 1, item );

  item = new QTableWidgetItem( tr("Standby") );
  m_table->setHorizontalHeaderItem( 2, item );

  item = new QTableWidgetItem( tr("Call Sign") );
  m_table->setHorizontalHeaderItem( 3, item );

  QHeaderView* hHeader = m_table->horizontalHeader();
  hHeader->setStretchLastSection( true );
  hHeader->setSortIndicator( 0, Qt::AscendingOrder );
  hHeader->setSortIndicatorShown( true );
  hHeader->setSectionsClickable( true );

  connect( hHeader, SIGNAL(sectionClicked(int)),
           this, SLOT(slot_HeaderClicked(int)) );

  connect( m_table, SIGNAL(cellClicked( int, int )),
           this, SLOT(slot_CellClicked( int, int )) );

  vLayout->addWidget( m_table, 2 );

  QGroupBox* buttonBox = new QGroupBox( this );

  QPushButton *helpButton = new QPushButton(this);
  helpButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  helpButton->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  helpButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_excangeFrequency  = new QPushButton( this );
  m_excangeFrequency->setToolTip( tr("Exchange Frequency at KRT2") );
  m_excangeFrequency->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "resort.png" ) ) );
  m_excangeFrequency->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  m_excangeFrequency->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_closeButton = new QPushButton( this );
  m_closeButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  m_closeButton->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  m_closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  m_statusLed = new QLabel( this );
  m_statusLed->setToolTip( tr("Shows the KRT2 connection status") );
  m_statusLed->setAlignment( Qt::AlignCenter );
  m_statusLed->setScaledContents( true );

  connect( helpButton, SIGNAL(clicked()), this, SLOT(slot_Help()) );
  connect( m_excangeFrequency, SIGNAL(clicked() ), this, SLOT(slot_exchangeFrequency()) );
  connect( m_closeButton, SIGNAL(clicked() ), this, SLOT(slot_Close()) );

  // vertical box with operator buttons
  QVBoxLayout *vbox = new QVBoxLayout;

  vbox->setSpacing(0);
  vbox->addWidget( helpButton, 1 );
  vbox->addStretch(2);
  vbox->addWidget( m_closeButton, 1 );
  vbox->addStretch(2);
  vbox->addWidget( m_excangeFrequency, 1 );
  vbox->addSpacing( 10 * Layout::getIntScaledDensity() );
  vbox->addWidget(m_statusLed, 0, Qt::AlignCenter );
  buttonBox->setLayout( vbox );
  topLayout->addWidget( buttonBox );
  loadTableItems();
}

KRT2Widget::~KRT2Widget()
{
}

void KRT2Widget::showEvent( QShowEvent *event )
{
  createIcons();

  if( checkKRT2Connection() == false )
    {
      // m_statusLed->setText( "OFF" );
      m_statusLed->setPixmap( *m_red );
    }
  else
    {
      // m_statusLed->setText( "ON" );
      m_statusLed->setPixmap( *m_green );
    }

  m_table->setFocus();
  QWidget::showEvent( event );
}

void KRT2Widget::closeEvent( QCloseEvent *event )
{
  QWidget::closeEvent( event );
}

/** Creates the KRT2 status icons. */
void KRT2Widget::createIcons()
{
  // Get the scaled density
  const int SD = Layout::getIntScaledDensity();
  const int w = 32*SD;
  const int h = 32*SD;

  m_red = new QPixmap( w, h );
  m_red->fill( Qt::transparent);

  m_green = new QPixmap( w, h );
  m_green->fill( Qt::transparent);


  QPainter painterR( m_red );
  QPen penR( Qt::black );
  penR.setWidth(3 * SD);
  painterR.setPen( penR );
  painterR.setBrush(Qt::red);
  painterR.drawEllipse( QPointF( w/2, h/2 ), w/2-3*SD, h/2-3*SD );
  painterR.end();

  m_green = new QPixmap( w, h );
  m_green->fill( Qt::transparent );

  QPainter painterG( m_green );
  QPen penG( Qt::black );
  penG.setWidth(3 * SD);
  painterG.setPen( penG );
  painterG.setBrush(Qt::green);
  painterG.drawEllipse( QPointF( w/2, h/2 ), w/2-3*SD, h/2-3*SD );
  painterG.end();
}

void KRT2Widget::enableButtons( const bool toggle )
{
  m_excangeFrequency->setEnabled( toggle );
  // m_closeButton->setEnabled( toggle );

  // Block all signals from the table.
  m_table->blockSignals( ! toggle );
}

void KRT2Widget::loadTableItems()
{
  m_table->clearContents();

  for( int i = 0; i < m_fqList.size(); i++ )
    {
      const Frequency &f = m_fqList.at(i);

      if( f.getValue() == 0.0 )
        {
          // Frequency is unknown, ignore it.
          continue;
        }

      addRow2List( f );
    }

  m_table->setCurrentCell( 0, 0 );
  m_table->resizeRowsToContents();
  m_table->resizeColumnsToContents();
}

void KRT2Widget::addRow2List( const Frequency& frequency )
{
  QString f = QString( "%1" ).arg( frequency.getValue(), 0, 'f', 3 );

  m_table->setRowCount( m_table->rowCount() + 1 );
  int row = m_table->rowCount() - 1;

  QTableWidgetItem* item;

  // Column 0 is used as GET button
  item = new QTableWidgetItem( f );
  item->setTextAlignment( Qt::AlignCenter );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  item->setData( Qt::UserRole, f );

  m_table->setItem( row, 0, item );

  // Column 1 is used as activate button
  item = new QTableWidgetItem( tr("Active") );
  item->setTextAlignment( Qt::AlignCenter );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  m_table->setItem( row, 1, item );

  // Column 2 is used as standby button
  item = new QTableWidgetItem( tr("Standby") );
  item->setTextAlignment( Qt::AlignCenter );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  m_table->setItem( row, 2, item );

  // column 3 is set to Flarm's configuration item value
  item = new QTableWidgetItem( frequency.getCallSign() );
  item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  m_table->setItem( row, 3, item );
}

void KRT2Widget::slot_Close()
{
  setVisible( false );
  emit closed();
  QWidget::close();
}

void KRT2Widget::slot_HeaderClicked( int section )
{
  static Qt::SortOrder so = Qt::AscendingOrder;

  if( section != 0 )
    {
      // Only the item column can be sorted. All others make no sense.
      // Restore sort indicator at column 0
      m_table->horizontalHeader()->setSortIndicator( 0, so );
      return;
    }

  m_table->sortByColumn( section, so );

  // Change sort order for the next click.
  so = ( so == Qt::AscendingOrder ) ? Qt::DescendingOrder : Qt::AscendingOrder;

  // Restore sort indicator at column 0
  m_table->horizontalHeader()->setSortIndicator( 0, so );
}

void KRT2Widget::slot_CellClicked( int row, int column )
{
  QTableWidgetItem* item = m_table->item( row, column );

  if( item == static_cast<QTableWidgetItem *>(0) || row < 0 ||
      ( column != 1 && column != 2 ) )
    {
      // Item can be a Null pointer, if a row has been removed.
      return;
    }

  if( checkKRT2Connection() == false )
    {
      messageBox( QMessageBox::Warning,
                  tr("KRT2 device not connected" ),
                  tr("No KRT2 connection"),
                  QMessageBox::Close );
      return;
    }

  float ff = m_table->item( row, 0 )->data(Qt::UserRole).toFloat();

  KRT2* krt2 = MainWindow::krt2Driver();

  QTableWidgetItem* item3 = m_table->item( row, 3 );

  QString name;

  if( item3 != static_cast<QTableWidgetItem *>(0) )
    {
      name = item3->text();
    }

  if( column == 1 )
    {
      krt2->setActiveFrequency( ff, name );
    }
  else if( column == 2 )
    {
      krt2->setStandbyFrequency( ff, name );
    }
}

/** Called, to exchange the frequencies active/standby on the KRT2 radio. */
void KRT2Widget::slot_exchangeFrequency()
{
  if( checkKRT2Connection() == false )
    {
      messageBox( QMessageBox::Warning,
                  tr("KRT2 device not connected" ),
                  tr("No KRT2 connection"),
                  QMessageBox::Close );
      return;
    }

  KRT2* krt2 = MainWindow::krt2Driver();
  krt2->exchangeFrequency();
}

void KRT2Widget::slot_Help()
{
  QString file = "cumulus-krt2.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

bool KRT2Widget::checkKRT2Connection()
{
  KRT2* krt2 = MainWindow::krt2Driver();

  if( krt2 == nullptr )
    {
      return false;
    }

  return( krt2->connected() );
}

/** Shows a popup message box to the user. */
int KRT2Widget::messageBox( QMessageBox::Icon icon,
                            QString message,
                            QString title,
                            QMessageBox::StandardButtons buttons )
{
  QMessageBox msgBox( this );
  msgBox.setText( title );
  msgBox.setIcon( icon );
  msgBox.setInformativeText( message );
  msgBox.setStandardButtons( buttons );
  msgBox.setDefaultButton( QMessageBox::Ok );

  return msgBox.exec();
}
