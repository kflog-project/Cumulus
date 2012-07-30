/***********************************************************************
**
**   flarmlogbook.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "generalconfig.h"
#include "igclogger.h"
#include "flarmlogbook.h"
#include "layout.h"
#include "mainwindow.h"
#include "rowdelegate.h"

// Timeout in ms for waiting for response
#define RESP_TO 10000

/**
 * Constructor
 */
FlarmLogbook::FlarmLogbook( QWidget *parent ) :
  QWidget( parent )
{
  setObjectName("FlarmLogbook");
  setWindowFlags(Qt::Tool);
  setWindowTitle( tr("Flarm Logbook"));
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  // Resize the window to the same size as the main window has. That will
  // completely hide the parent window.
  resize( MainWindow::mainWindow()->size() );

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing(5);

  m_table = new QTableWidget( 0, 8, this );
  m_table->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_table->setAlternatingRowColors( true );

  // hide vertical headers
  // QHeaderView *vHeader = m_table->verticalHeader();
  // vHeader->setVisible(false);

  QHeaderView* hHeader = m_table->horizontalHeader();

  // that makes trouble on N810
  // hHeader->setStretchLastSection( true );
  hHeader->setClickable( true );

  connect( hHeader, SIGNAL(sectionClicked(int)),
           this, SLOT(slot_HeaderClicked(int)) );

  setTableHeader();
  topLayout->addWidget( m_table, 5 );
  topLayout->addSpacing( 10 );

  m_readButton      = new QPushButton( tr("Read"));
  m_downloadButton  = new QPushButton( tr("Download"));
  m_closeButton     = new QPushButton( tr("Close"));

  connect( m_readButton, SIGNAL(clicked() ), this, SLOT(slot_ReadFlights()) );
  connect( m_downloadButton, SIGNAL(clicked() ), this, SLOT(slot_DownloadFlights()) );
  connect( m_closeButton, SIGNAL(clicked() ), this, SLOT(slot_Close()) );

  // horizontal box with operator buttons
  QHBoxLayout *hbBox = new QHBoxLayout;

  hbBox->addWidget( m_readButton );
  hbBox->addSpacing( 10 );
  hbBox->addWidget( m_downloadButton );
  hbBox->addSpacing( 10 );
  hbBox->addWidget( m_closeButton );

  topLayout->addLayout( hbBox );

  QString style = "QTableView QTableCornerButton::section { background: gray }";
  m_table->setStyleSheet( style );

  QHeaderView *vHeader = m_table->verticalHeader();
  style = "QHeaderView::section { width: 2em }";
  vHeader->setStyleSheet( style );

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  rowDelegate = new RowDelegate( m_table, afMargin );
  m_table->setItemDelegate( rowDelegate );

  // Timer for command time supervision
  m_timer = new QTimer( this );
  m_timer->setSingleShot( true );
  m_timer->setInterval( RESP_TO );

  connect( m_timer, SIGNAL(timeout()), SLOT(slot_Timeout()));

  // delivers the Flarm configuration data.
  connect( Flarm::instance(), SIGNAL(flarmConfigurationInfo(QStringList&)),
            SLOT(slot_UpdateConfiguration( QStringList&)) );

  // Delivers the Flarm flight list.
  connect(GpsNmea::gps, SIGNAL(newFlarmFlightList(const QString&)),
           this, SLOT(slot_LogbookData(const QString&)) );

  // Delivers a Flarm download info.
  connect(GpsNmea::gps, SIGNAL(newFlarmFlightDownloadInfo(const QString&)),
           this, SLOT(slot_FlarmFlightDownloadInfo(const QString&)) );

  // Delivers a Flarm download progress.
  connect(GpsNmea::gps, SIGNAL(newFlarmFlightDownloadProgress(const int, const int)),
           this, SLOT(slot_FlarmFlightDownloadProgress(const int, const int)) );
}

FlarmLogbook::~FlarmLogbook()
{
}

void FlarmLogbook::enableButtons( const bool toggle )
{
  m_readButton->setEnabled( toggle );
  m_downloadButton->setEnabled( toggle );
  m_closeButton->setEnabled( toggle );
}

void FlarmLogbook::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  // m_table->resizeColumnsToContents();
  // m_table->resizeRowsToContents();
}

void FlarmLogbook::setTableHeader()
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

/** Called if configuration updates from Flarm device are available. */
void FlarmLogbook::slot_UpdateConfiguration( QStringList& info )
{
  if( info.size() >= 4 && info[1] == "A" && info[2] == "NMEAOUT" && info[3] == "0" )
    {
      // NMEA output of Flarm is swichted off, we can request the download
      // of the Flight overview.

      // Restart timer for connection supervision.
      m_timer->start();

      // As next request the flight list
      if( GpsNmea::gps->getFlightListFromFlarm() == false )
        {
          slot_Timeout();
          QString text0 = tr("Flarm device not reachable!");
          QString text1 = tr("Error");
          messageBox( QMessageBox::Warning, text0, text1 );
        }
    }
  else
    {
      // Problem occurred. Abort further actions.
      slot_Timeout();
      QString text0 = tr("Flarm Problem");
      QString text1 = tr("Cannot disable NMEA output of Flarm!");
      messageBox( QMessageBox::Warning, text1, text0 );
      qWarning() << "FL::SUC: NMEAOUT error!" << info.join(",");
    }
}

void FlarmLogbook::slot_LogbookData( QString& list )
{
  slot_Timeout();
  m_table->clear();
  setTableHeader();
  m_logbook.clear();

  if( list.size() == 0 )
    {
      // no data in logbook.
      return;
    }

  if( list.startsWith("Error") )
    {
      QString text0 = tr("Flarm flight list not readable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
      return;
    }

  if( list.startsWith("Empty") )
    {
      QString text0 = tr("Flarm flight list is empty!");
      QString text1 = tr("Info");
      messageBox( QMessageBox::Information, text0, text1 );
      return;
    }

  // The passed string must be split in its single lines.
  m_logbook = list.split("\n");

  for( int row = 0; row < m_logbook.size(); row++ )
    {
      m_table->setRowCount( m_table->rowCount() + 1 );

      QStringList line = m_logbook.at(row).split("|");

      for( int col = 0; col < line.size() && col < 8; col++ )
        {
          QTableWidgetItem* item;

          item = new QTableWidgetItem( line.at(col) );
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

void FlarmLogbook::slot_ReadFlights()
{
  m_table->clear();
  enableButtons( false );

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  // Activate timer for connection supervision.
  m_timer->start();

  // As first switch off NMEA output of Flarm device.
  if( GpsNmea::gps->sendSentence( "$PFLAC,S,NMEAOUT,0" ) == false )
    {
      slot_Timeout();
      QString text0 = tr("Flarm device not reachable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
      return;
    }

  // The answer is delivered via the slot slot_UpdateConfiguration.
}

void FlarmLogbook::slot_DownloadFlights()
{
  if( m_table->rowCount() == 0 )
    {
      return;
    }
}

/**
 * This slot is called, when a new Flarm flight download info was received.
 */
void FlarmLogbook::slot_FlarmFlightDownloadInfo( const QString& info )
{

}

/**
 * This slot is called, when a new Flarm flight download progress was received.
 */
void FlarmLogbook::slot_FlarmFlightDownloadProgress( const int idx,
                                                            const int progress)
{

}

/** Called if the connection timer has expired. */
void FlarmLogbook::slot_Timeout()
{
  QApplication::restoreOverrideCursor();
  enableButtons( true );

  // Note, this method is also called in case on no timeout to enable the
  // buttons and to restore the cursor. Therefore a running timer must be
  // stopped too.
  if( m_timer->isActive() )
    {
      m_timer->stop();
    }
  else
    {
      QString text0 = tr("No response from Flarm device!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
    }
}

void FlarmLogbook::slot_Close()
{
  QApplication::restoreOverrideCursor();
  m_timer->stop();

  disconnect( Flarm::instance(), SIGNAL(flarmConfigurationInfo(QStringList&)),
               this, SLOT(slot_UpdateConfiguration( QStringList&)) );

  // Enable NMEA output again. Errors are ignored.
  GpsNmea::gps->sendSentence( "$PFLAC,S,NMEAOUT,1" );

  setVisible( false );
  emit closed();
  QWidget::close();
}

/** Shows a popup message box to the user. */
void FlarmLogbook::messageBox( QMessageBox::Icon icon,
                                   QString message,
                                   QString title )
{
  QMessageBox mb( icon,
                  title,
                  message,
                  QMessageBox::Ok,
                  this );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  mb.exec();
}
