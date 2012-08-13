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

#include "flickcharm.h"
#include "generalconfig.h"
#include "hwinfo.h"
#include "igclogger.h"
#include "flarmlogbook.h"
#include "layout.h"
#include "mainwindow.h"
#include "rowdelegate.h"

// Timeout in ms for waiting for response of Flarm device.
#define RESP_TO 30000

/**
 * Constructor
 */
FlarmLogbook::FlarmLogbook( QWidget *parent ) :
  QWidget( parent ),
  m_resetFlarm( false ),
  m_ignoreClose( false )
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

  m_table = new QTableWidget( 0, 6, this );

#ifdef QSCROLLER
  QScroller::grabGesture(m_table, QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(m_table);
#endif

  m_table->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_table->setAlternatingRowColors( true );

  setTableHeader();
  topLayout->addWidget( m_table, 5 );
  topLayout->addSpacing( 10 );

  m_readButton      = new QPushButton( tr("Read"));
  m_downloadButton  = new QPushButton( tr("Download"));
  m_closeButton     = new QPushButton( tr("Close"));

  m_downloadButton->setEnabled( false );

  connect( m_readButton, SIGNAL(clicked() ), this, SLOT(slot_ReadFlights()) );
  connect( m_downloadButton, SIGNAL(clicked() ), this, SLOT(slot_DownloadFlights()) );
  connect( m_closeButton, SIGNAL(clicked() ), this, SLOT(close()) );

  // horizontal box with operator buttons
  QHBoxLayout *hbBox = new QHBoxLayout;
  hbBox->setContentsMargins( 0, 0, 0, 0 );
  hbBox->addWidget( m_readButton );
  hbBox->addSpacing( 10 );
  hbBox->addWidget( m_downloadButton );
  hbBox->addSpacing( 10 );
  hbBox->addWidget( m_closeButton );

  m_buttonWidget = new QWidget( this );
  m_buttonWidget->setLayout( hbBox );
  topLayout->addWidget( m_buttonWidget );

  // horizontal box with progressbar
  QHBoxLayout *hpBox = new QHBoxLayout;
  hpBox->setContentsMargins( 0, 0, 0, 0 );
  hpBox->addWidget( new QLabel(tr("Flight:")) );
  hpBox->addSpacing( 10 );
  m_progressLabel = new QLabel;
  hpBox->addWidget( m_progressLabel );
  hpBox->addSpacing( 10 );
  m_progressBar = new QProgressBar;
  hpBox->addWidget( m_progressBar );

  m_progressWidget = new QWidget( this );
  m_progressWidget->setLayout( hpBox );
  m_progressWidget->setVisible(false);
  topLayout->addWidget( m_progressWidget );

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
           this, SLOT(slot_FlarmLogbookData(const QString&)) );

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
  m_closeButton->setEnabled( toggle );

  if( toggle && m_table->rowCount() > 0 )
    {
      m_downloadButton->setEnabled( true );
    }
  else
    {
      m_downloadButton->setEnabled( false );
    }
}

void FlarmLogbook::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  // m_table->resizeColumnsToContents();
  // m_table->resizeRowsToContents();
}

void FlarmLogbook::closeEvent( QCloseEvent* event )
{
  if( m_ignoreClose )
    {
      event->ignore();
      return;
    }

  QApplication::restoreOverrideCursor();
  m_timer->stop();

  disconnect( Flarm::instance(), SIGNAL(flarmConfigurationInfo(QStringList&)),
               this, SLOT(slot_UpdateConfiguration( QStringList&)) );

  if( m_resetFlarm )
    {
      // Flarm device must be reset to normal mode.
      if( ! GpsNmea::gps->flarmReset() )
        {
          qWarning() << "FL::closeEvent(): Reset Flarm failed!";
        }
    }

  // Set protocol mode of Flarm back to text.
  Flarm::setProtocolMode( Flarm::text );
  event->accept();
}

void FlarmLogbook::setTableHeader()
{
  QTableWidgetItem *item = new QTableWidgetItem( tr("Date") );
  m_table->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr("To") );
  m_table->setHorizontalHeaderItem( 1, item );

  item = new QTableWidgetItem( tr("Ft") );
  m_table->setHorizontalHeaderItem( 2, item );

  item = new QTableWidgetItem( tr("Pilot") );
  m_table->setHorizontalHeaderItem( 3, item );

  item = new QTableWidgetItem( tr("Type") );
  m_table->setHorizontalHeaderItem( 4, item );

  item = new QTableWidgetItem( tr("Idx") );
  m_table->setHorizontalHeaderItem( 5, item );

  m_table->resizeColumnsToContents();
}

void FlarmLogbook::slot_UpdateConfiguration( QStringList& info )
{
  if( info.size() >= 4 &&
      info[0] == "$PFLAC" && info[1] == "A" && info[2] == "NMEAOUT" )
    {
      // Flarm has answered to our NMEAOUT request. So we know,
      // that a Flarm is connected to us and we can request the download
      // of the Flight overview now.

      // Remembered us that Flarm must be reset if window is closed.
      m_resetFlarm = true;

      // Restart timer for connection supervision.
      m_timer->start();

      // As next request the flight list from the Flarm device.
      // The Flarm answer is delivered via the slot:
      //
      // slot_FlarmLogbookData()
      //
      // with flight list info.
      if( GpsNmea::gps->getFlarmFlightList() == false )
        {
          slot_Timeout();
          QString text0 = tr("Flarm device not reachable!");
          QString text1 = tr("Error");
          messageBox( QMessageBox::Warning, text0, text1 );
        }
    }
}

void FlarmLogbook::slot_FlarmLogbookData( const QString& data )
{
  m_logbook.clear(); // remove old content

  // Clear table content.
  for( int i = m_table->rowCount() - 1; i >= 0; i-- )
    {
      // Remove row from table.
      m_table->removeRow(i);
    }

  // Stop timer
  slot_Timeout();

  if( data.size() == 0 )
    {
      // No data delivered, should normally not happen.
      return;
    }

  if( data.startsWith("Error") )
    {
      QString text0 = tr("Flarm flight list read error!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
      return;
    }

  if( data.startsWith("Empty") )
    {
      QString text0 = tr("Flarm flight list is empty!");
      QString text1 = tr("Info");
      messageBox( QMessageBox::Information, text0, text1 );
      return;
    }

  /*
    Returned flight entries by Flarm V5.06
    27LG7QX1.IGC|2012-07-21|11:48:35|03:09:36|Hans-Georg Grund||105
    27AG7QX2.IGC|2012-07-10|09:50:50|05:12:10|Axel Pauli||105
    27AG7QX1.IGC|2012-07-10|09:33:42|00:06:20|Axel Pauli||105
  */
  // The passed data string must be split in its single lines.
  m_logbook = data.split("\n");

  for( int row = 0; row < m_logbook.size(); row++ )
    {
      // A Flarm flight info has several entries, which are separated by a pipe
      // character.
      QStringList line = m_logbook.at(row).split("|");

      if( line.size() < 7 )
        {
          qWarning() << "slot_FlarmLogbookData: 7 entries expected:"
                      << line.size()
                      << line;
          break;
        }

      m_table->setRowCount( m_table->rowCount() + 1 );

      for( int col = 0; col < line.size() && col < 6; col++ )
        {
          QTableWidgetItem* item;

          item = new QTableWidgetItem( line.at(col + 1) );
          item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

          if( col >= 0 && col <= 2 )
            {
              item->setTextAlignment( Qt::AlignCenter );
            }

          m_table->setItem( row, col, item );
        }
    }

  m_table->resizeColumnsToContents();
  m_table->resizeRowsToContents();

  if( m_table->rowCount() )
    {
      m_downloadButton->setEnabled( true );
    }
}

void FlarmLogbook::slot_ReadFlights()
{
  // Read button was pressed to get the flight list from the Flarm device.

  // Clear table content.
  for( int i = m_table->rowCount() - 1; i >= 0; i-- )
    {
      // Remove row from table.
      m_table->removeRow(i);
    }

  enableButtons( false );
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  // Activate timer for connection supervision.
  m_timer->start();

  m_ignoreClose = true;

  if( m_resetFlarm == true )
    {
      // Flarm was already switched to the binary mode. We simulate a callback
      // to the update slot.
      QStringList info;
      info << "$PFLAC" << "A" << "NMEAOUT" << "0";
      slot_UpdateConfiguration(  info );
      return;
    }

  // As first test with requesting setting of NMEAOUT, if a Flarm device is connected.
  if( GpsNmea::gps->sendSentence( "$PFLAC,R,NMEAOUT" ) == false )
    {
      slot_Timeout();
      QString text0 = tr("Flarm device not reachable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
    }

  // The Flarm answer is delivered via the slot slot_UpdateConfiguration.
}

void FlarmLogbook::slot_DownloadFlights()
{
  // The download button was pressed to download the selected flights from the
  // table.
  if( m_table->rowCount() == 0 )
    {
      return;
    }

  QString selectedEnties;
  ushort fc = 0;

  for( int i = 0; i < m_table->rowCount() ; i++ )
    {
      // Check, if item is selected.
      QTableWidgetItem *item = m_table->item( i, 0 );

      if( item && item->isSelected() )
        {
          selectedEnties += "\v" + QString::number(i);
          fc++;
        }
    }

  if( selectedEnties.isEmpty() )
    {
      // nothing is selected
      return;
    }

  QString destination = GeneralConfig::instance()->getUserDataDirectory();

  // Check the free space at the user's file system.
  ulong space = HwInfo::getFreeUserSpace( destination );

  destination += "/flarmIgc";

  qDebug() << "FlarmDownload: Free space:" << ((float) space/ (1024*1024)) << "MB at" << destination;

  const ulong mb = 1024 * 1024;

  if( space < ( fc * mb) )
    {
      // Per file a free space of 1MB is calculated.
      QString text0 = "<html>" +
                      tr("Too less free space on:") +
                      "<br><br>" +
                      destination +
                      "<br><br>" +
                      tr("Download not possible!") +
                      "</html>";

      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
      return;
    }

  // Prepare the download request. First argument is the destination directory
  // for the download, as next follows the flight numbers to be requested for
  // the download. As field separator vertical tab '\v' is used.
  QString args = destination + selectedEnties;

  enableButtons( false );
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  // Activate timer for connection supervision.
  m_timer->start();

  m_ignoreClose = true;

  if( GpsNmea::gps->getFlarmIgcFiles( args ) == false )
    {
      slot_Timeout();
      QString text0 = tr("Flarm device not reachable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
    }

  // Hide buttons and show a progressbar there
  m_buttonWidget->hide();
  m_progressWidget->show();
  m_progressLabel->clear();
  m_progressBar->reset();
}

/**
 * This slot is called, when a new Flarm flight download info was received.
 */
void FlarmLogbook::slot_FlarmFlightDownloadInfo( const QString& info )
{
  // The following messages can be reported:
  // 1. Error ..., the was an error, downlaod aborted
  // 2. Finished, all downloads are finished.
  if( info.startsWith( "Error" ) )
    {
      slot_Timeout();
      QString text0 = tr("Flarm download error!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
      return;
    }

  if( info.startsWith( "Finished" ) )
    {
      slot_Timeout();
      QString text0 = tr("Flights downloaded!");
      QString text1 = tr("Finished");
      messageBox( QMessageBox::Information, text0, text1 );
      return;
    }

  qWarning() << "FlarmLogbook::slot_FlarmFlightDownloadInfo(): unknown info"
              << info;
}

/**
 * This slot is called, when a new Flarm flight download progress was received.
 */
void FlarmLogbook::slot_FlarmFlightDownloadProgress( const int idx,
                                                            const int progress)
{
  // Restart timer supervision.
  m_timer->start();
  m_progressLabel->setText( QString::number( idx + 1 ) );
  m_progressBar->setValue( progress );
}

/** Called if the connection timer has expired. */
void FlarmLogbook::slot_Timeout()
{
  QApplication::restoreOverrideCursor();
  enableButtons( true );

  m_ignoreClose = false;
  m_buttonWidget->show();
  m_progressWidget->hide();

  // Note, this method is also called in case on no timeout to enable the
  // buttons and to restore the cursor. Therefore a running timer must be
  // stopped too.
  if( m_timer->isActive() )
    {
      m_timer->stop();
    }
  else
    {
      QString text0 = tr("Flarm device not reachable!");
      QString text1 = tr("Error");
      messageBox( QMessageBox::Warning, text0, text1 );
    }
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
