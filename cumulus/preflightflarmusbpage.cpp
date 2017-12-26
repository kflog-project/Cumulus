/***********************************************************************
 **
 **   preflightflarmusbpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2017 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "flarm.h"
#include "generalconfig.h"
#include "gpsnmea.h"
#include "layout.h"
#include "mainwindow.h"
#include "preflightflarmusbpage.h"

// Timeout in ms for waiting for response
#define RESP_TO 5000

PreFlightFlarmUsbPage::PreFlightFlarmUsbPage(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("PreFlightFlarmUsbPage");
  setWindowTitle(tr("Copy Flarm flights to USB stick"));
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( _globalMainWindow )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  QVBoxLayout *vbox = new QVBoxLayout( this );
  vbox->setSpacing( 10 * Layout::getIntScaledDensity() );

  // new widget as container for the dialog layout.
  m_info = new QLabel( tr("Copy Flarm flights to USB stick") );
  vbox->addWidget( m_info );
  vbox->addSpacing( 25 * Layout::getIntScaledDensity() );

  m_pb = new QProgressBar;
  m_pb->setMinimum( 0 );
  m_pb->setMaximum( 100 );
  vbox->addWidget( m_pb );
  vbox->addStretch( 10 );

  QHBoxLayout *bbox = new QHBoxLayout;
  //QPushButton* cmd = new QPushButton(tr("Close"), this);

  QPushButton *close = new QPushButton(this);
  close->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png", true)));
  close->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  close->setSizePolicy( QSizePolicy::QSizePolicy::Preferred, QSizePolicy::Fixed );

  bbox->addStretch( 2 );
  bbox->addWidget( close, 1 );
  bbox->addStretch( 2 );
  vbox->addLayout( bbox );
  connect( close, SIGNAL(clicked()), SLOT(slotClose()) );

  //----------------------------------------------------------------------------

  // Add Flarm signals to our slots to get Flarm data.
  connect( Flarm::instance(), SIGNAL(flarmErrorInfo( const Flarm::FlarmError&)),
            this, SLOT(slotUpdateErrors(const Flarm::FlarmError&)) );

  connect( Flarm::instance(), SIGNAL(flarmIgcResponse( QStringList&)),
            this, SLOT(slotIgcResponse( QStringList&)) );

  connect( Flarm::instance(), SIGNAL(flarmError( QStringList&)),
            this, SLOT(slotReportError( QStringList&)) );

  connect( Flarm::instance(), SIGNAL(flarmProgressInfo( QStringList&)),
            this, SLOT(slotProgressInfo( QStringList&)) );

  // Timer for command time supervision
  m_timer = new QTimer( this );
  m_timer->setSingleShot( true );
  m_timer->setInterval( RESP_TO );

  connect( m_timer, SIGNAL(timeout()), SLOT(slotTimeout()));

  QTimer::singleShot(0, this, SLOT(slotRequestIgcReadout()));
}

PreFlightFlarmUsbPage::~PreFlightFlarmUsbPage()
{
}

void PreFlightFlarmUsbPage::slotRequestIgcReadout()
{
  // It is not clear, if a Flarm device is connected. We send out
  // some requests to check that. If no answer is reported in a certain
  // time the supervision timer will reset this request.
  // It seems there are some new Flarm commands available, which are not listed
  // in the current DataPortSpec paper 6.00.
  // http://www.flarm.com/support/manual/developer/dataport_spec_6.00_addendum.txt
  //
  // Check, if a Gps is connected, we try to consider it as a Flarm device.
  // But that must not be always true!
  if( GpsNmea::gps->getConnected() != true )
    {
      m_info->setText( tr("Flarm device not reachable!") );
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  QString cmd = "$PFLAI,IGCREADOUT";

  QByteArray ba = FlarmBase::replaceUmlauts(cmd.toLatin1());

  qDebug() << "Flarm $Command:" << ba;

  bool res = GpsNmea::gps->sendSentence( ba );

  if( res == false )
    {
      QApplication::restoreOverrideCursor();
      m_info->setText( tr("Flarm device not reachable!") );
      return;
    }

  m_timer->start();
}

/**
 * The Flarm firmware update 6.x generates this new error. It seems, that the
 * Classic Flarm device has problems with the UART receiver performance, so that
 * UART checksum errors occur.
 */
void PreFlightFlarmUsbPage::slotReportError( QStringList& info )
{
  if( info[0] != "ERROR" && info.size() < 2 )
    {
      return;
    }

  qWarning() << "Flarm $ERROR: " << info.join(",");

  QApplication::restoreOverrideCursor();

  m_info->setText( info.join(",") );
  m_pb->setValue( 100 );
  m_timer->stop();
  return;
}

void PreFlightFlarmUsbPage::slotUpdateErrors( const Flarm::FlarmError& info )
{
  QString error = QString(tr("Severity: %1, ErrorCode: %2\n\n%3"))
                          .arg(info.severity)
                          .arg(info.errorCode)
                          .arg(info.errorText);

  m_info->setText( error );
  m_pb->setValue( 100 );
  m_timer->stop();
  return;
}

void PreFlightFlarmUsbPage::slotIgcResponse( QStringList& info )
{
  qDebug() << "Flarm $Response:" << info;

  /**
   * The complete received $PFLAC sentence is the input here.
   *
   * $PFLAI,IGCREADOUT,ERROR,<error>, ("$PFLAI", "IGCREADOUT", "ERROR", "IO", "31")
   * $PFLAI,IGCREADOUT,OK
   */
  if( info.size() < 3 )
    {
      qWarning() << "$PFLAI too less parameters!" << info.join(",");
      return;
    }

  QApplication::restoreOverrideCursor();

  if( info[2] == "OK" )
    {
      // Download to USB ready
      m_info->setText( tr("All Flarm flights transfered to the USB stick.") );
      m_pb->setValue( 100 );
      m_timer->stop();
      return;
    }

  if( info[2] == "ERROR" && info.size() >= 4 )
    {
      m_info->setText( info[2] + ": " + info[3] + tr("\n\nUSB stick really connected?") );
      m_pb->setValue( 100 );
      qWarning() << "$PFLAI error!" << info.join(",");
      m_timer->stop();
      return;
    }
}

void PreFlightFlarmUsbPage::slotProgressInfo( QStringList& info )
{
  if( info.size() < 4 )
    {
      qWarning() << "$PFLAQ too less parameters!" << info.join(",");
      return;
    }

  m_info->setText( tr("Transferring file ") + info[2] );
  m_pb->setValue( info[3].toInt() );
  m_timer->start();
}

void PreFlightFlarmUsbPage::slotTimeout()
{
  QApplication::restoreOverrideCursor();
  m_timer->stop();
  m_info->setText( tr("Timer expired, stopping transfer.") );
}

void PreFlightFlarmUsbPage::slotClose()
{
  QApplication::restoreOverrideCursor();
  m_timer->stop();
  emit closingWidget();
  close();
}
