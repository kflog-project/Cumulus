/***********************************************************************
**
**   gpsstatusdialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2003      by André Somers
**                  2008-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "gpsstatusdialog.h"
#include "gpsnmea.h"
#include "layout.h"
#include "mainwindow.h"

// initialize static member variable
int GpsStatusDialog::noOfInstances = 0;

GpsStatusDialog::GpsStatusDialog(QWidget * parent) :
  QWidget( parent ),
  showNmeaData( true ),
  nmeaLines( 0 ),
  cntSIVSentence( 1 )
{
  noOfInstances++;

  setWindowTitle(tr("GPS Status"));
  setWindowFlags( Qt::Tool );
  // setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( _globalMainWindow )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  uTimer = new QTimer( this );
  uTimer->setSingleShot( true );

  connect( uTimer, SIGNAL(timeout()), this,
           SLOT(slot_updateGpsMessageDisplay()) );

  elevAziDisplay = new GpsElevationAzimuthDisplay(this);
  snrDisplay     = new GpsSnrDisplay(this);

  nmeaBox = new QLabel;
  nmeaBox->setObjectName("NmeaBox");
  nmeaBox->setTextFormat(Qt::PlainText);
  nmeaBox->setBackgroundRole( QPalette::Light );
  nmeaBox->setAutoFillBackground( true );
  nmeaBox->setMargin(5);
  nmeaBox->setTextInteractionFlags( Qt::TextSelectableByMouse |
                                    Qt::TextSelectableByKeyboard);

  QFont f = font();

#if defined MAEMO
  f.setPixelSize(16);
#elif ANDROID
  f.setPointSize(6);
#else
  f.setPixelSize(14);
#endif

  nmeaBox->setFont(f);

  QScrollArea *nmeaScrollArea = new QScrollArea;
  nmeaScrollArea->setWidgetResizable( true );
  nmeaScrollArea->setWidget(nmeaBox);

#ifdef ANDROID
  // Make the vertical scrollbar bigger for Android
  QScrollBar* vsb = nmeaScrollArea->verticalScrollBar();
  vsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture( nmeaScrollArea->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( nmeaScrollArea->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  QVBoxLayout* nmeaBoxLayout = new QVBoxLayout;
  nmeaBoxLayout->setSpacing( 0 );
  nmeaBoxLayout->addWidget( nmeaScrollArea );

  satSource = new QComboBox;
  satSource->setToolTip( tr("Editable GPS source filter") );
  satSource->addItem( "$GP" );
  satSource->addItem( "$BD" );
  satSource->addItem( "$GA" );
  satSource->addItem( "$GL" );
  satSource->addItem( "$GN" );
  satSource->addItem( "ALL" );
  satSource->setEditable( true );
  satSource->setCurrentIndex( GeneralConfig::instance()->getGpsFilterIndex() );

  startStop = new QPushButton( tr("Stop"), this );
  save      = new QPushButton( tr("Save"), this );

  QPushButton* close = new QPushButton( tr("Close"), this );

  QVBoxLayout* buttonBox = new QVBoxLayout;
  buttonBox->addWidget( satSource );
  buttonBox->addStretch( 5 );
  buttonBox->addWidget( startStop );
  buttonBox->addSpacing( 5 );
  buttonBox->addWidget( save );
  buttonBox->addSpacing( 5 );
  buttonBox->addWidget( close );

  QHBoxLayout* hBox = new QHBoxLayout;
  hBox->addWidget(elevAziDisplay, 1);
  hBox->addWidget(snrDisplay, 2);
  hBox->addLayout( buttonBox );

  QVBoxLayout* topLayout = new QVBoxLayout( this );
  topLayout->addLayout( hBox );
  topLayout->addLayout( nmeaBoxLayout );

  connect( satSource, SIGNAL(currentIndexChanged(int)),
                      SLOT(slot_GpsSourceChanged(int)) );

  connect( GpsNmea::gps, SIGNAL(newSentence(const QString&)),
           this, SLOT(slot_Sentence(const QString&)) );

  connect( startStop, SIGNAL(clicked()), this, SLOT(slot_ToggleStartStop()) );

  connect( save, SIGNAL(clicked()), this, SLOT(slot_SaveNmeaData()) );

  connect( close, SIGNAL(clicked()), this, SLOT(slot_Close()) );
}

GpsStatusDialog::~GpsStatusDialog()
{
  noOfInstances--;
}

void GpsStatusDialog::slot_GpsSourceChanged( int /* index */ )
{
  // We pass an empty list to clear the display.
  QList<SIVInfo> siv;
  slot_SIV( siv );

  // clear NMEA box and list
  nmeaData.clear();
  nmeaLines = 0;
  nmeaBox->clear();
}

void GpsStatusDialog::slot_SIV( QList<SIVInfo>& siv )
{
  //qDebug("received new sivi info signal");
  elevAziDisplay->setSatInfo( siv );
  snrDisplay->setSatInfo( siv );
}

void GpsStatusDialog::slot_Sentence(const QString& sentence)
{
  int maxLines = 100;

  // Selected GPS source
  QString gps = satSource->currentText();

  if( showNmeaData )
    {
      if( gps == "ALL" || sentence.startsWith( gps ) )
        {
          // Show only desired sentences
          nmeaLines++;

          if( nmeaLines > maxLines )
            {
              // Note! To display to many lines can become a performance issue
              // because the window update effort is huge.
              int idx = nmeaData.indexOf( QChar( '\n' ) );

              if( idx != -1 && nmeaData.size() > idx + 1 )
                {
                  nmeaData.remove( 0, idx + 1 );
                  nmeaLines--;
                }
            }

        QString string = sentence.trimmed().append("\n");

        // NMEA data string to be displayed in QLabel
        nmeaData.append( string );

        // NMEA data to be saved into a file on user request.
        nmeaList.append( string );

        if( nmeaList.size() > 250 )
          {
            nmeaList.removeFirst();
          }

        // To reduce the load the messages are displayed only if the timer
        // has expired. The time expires after 750ms. That was the breakdown!!!
        if( ! uTimer->isActive() )
          {
            uTimer->start( 750 );
          }
        }
    }

  if( gps == "ALL" && sentence.contains( "GSV" ) )
    {
      ExtractSatsInView( sentence );
    }
  else if( sentence.startsWith( gps + "GSV") )
    {
      ExtractSatsInView( sentence );
    }
}

/**
  GPGSV - GPS Satellites in view
  GLGSV - GLONASS Satellites in view

          1 2 3 4 5 6 7     n
          | | | | | | |     |
   $GPGSV,x,x,x,x,x,x,x,...*hh<CR><LF>
   $GLGSV,x,x,x,x,x,x,x,...*hh<CR><LF>

   Field Number:
    1) total number of messages
    2) message number
    3) satellites in view
    4) satellite number
    5) elevation in degrees
    6) azimuth in degrees to true
    7) SNR in dB
    more satellite infos like 4)-7)
    n) checksum

  Extract Satellites In View (SIV) info from a NMEA sentence.
*/
void GpsStatusDialog::ExtractSatsInView(const QString& sentence)
{
  QStringList slst = sentence.split( QRegExp("[,*]"), QString::KeepEmptyParts );

  if( slst.size() < 8 )
    {
      qWarning() << slst[0] << "contains too less parameters!";
      return;
    }

  // The GPGSV sentence can be split into multiple sentences.
  // qDebug("expecting: %d, found: %s",cntSIVSentence,sentence[2].toLatin1().data());
  // Check if we were expecting this part of the info.
  if( cntSIVSentence != slst[2].toUInt() )
    {
      return;
    }

  if( cntSIVSentence == 1 ) //this is the first sentence of our series
    {
      sivInfoInternal.clear();
    }

  // extract info on the individual sats
  ExtractSatsInView( slst[4], slst[5], slst[6], slst[7] );

  if( slst.count() > 11 )
    {
      ExtractSatsInView( slst[8], slst[9], slst[10], slst[11] );
    }

  if( slst.count() > 15 )
    {
      ExtractSatsInView( slst[12], slst[13], slst[14], slst[15] );
    }

  if( slst.count() > 19 )
    {
      ExtractSatsInView( slst[16], slst[17], slst[18], slst[19] );
    }

  cntSIVSentence++;

  if( cntSIVSentence > slst[1].toUInt() )
    //this was the last sentence in our series
    {
      cntSIVSentence = 1;
      slot_SIV( sivInfoInternal );
    }
}

/** Extract Satellites In View (SIV) info from a NMEA sentence. */
void GpsStatusDialog::ExtractSatsInView( const QString& id,
                                         const QString& elev,
                                         const QString& azimuth,
                                         const QString& snr )
{
  if( id.isEmpty() || elev.isEmpty() || azimuth.isEmpty() || snr.isEmpty() )
    {
      // ignore empty data
      return;
    }

  SIVInfo sivi;
  sivi.id = id.toInt();
  sivi.elevation = elev.toInt();
  sivi.azimuth = azimuth.toInt();
  sivi.db = snr.toInt();

  sivInfoInternal.append( sivi );
  //qDebug("new sivi info (snr: %d", sivi->db);
}


/**
 * Called to update the GPS message display.
 */
void GpsStatusDialog::slot_updateGpsMessageDisplay()
{
  nmeaBox->setText(nmeaData);
}

/**
 * Called if the start/stop button is pressed to start or stop NMEA display.
 */
void GpsStatusDialog::slot_ToggleStartStop()
{
  showNmeaData = ! showNmeaData;

  if( showNmeaData )
    {
      startStop->setText( tr("Stop") );
    }
  else
    {
      startStop->setText( tr("Start") );
    }
}

/**
 * Called if space button is pressed to toggle the window size.
 */
void GpsStatusDialog::slot_ToggleWindowSize()
{
  //setWindowState(windowState() ^ Qt::WindowFullScreen);
}

/**
 * Called if the save button is pressed to save NMEA display content into a file.
 */
void GpsStatusDialog::slot_SaveNmeaData()
{
  if( nmeaList.isEmpty() )
    {
      // nothing to save
      return;
    }

  bool ok;

#ifndef MAEMO5
  QString fileName = QInputDialog::getText( this, tr("Append to?"),
                                            tr("File name:"),
                                            QLineEdit::Normal,
                                            "nmea-stream.log",
                                            &ok,
                                            0,
                                            Qt::ImhNoPredictiveText );
#else
  QString fileName = QInputDialog::getText( this, tr("Append to?"),
                                            tr("File name:"),
                                            QLineEdit::Normal,
                                            "nmea-stream.log",
                                            &ok,
                                            0 );

#endif

  if( ok && ! fileName.isEmpty() )
    {
      if( ! fileName.startsWith( "/") )
        {
          // We will store the file in the user's home location, when it not
          // starts with a slash. Other cases as . or .. are ignored by us.
          QFileInfo fi( fileName );
          fileName = QDir::homePath() + "/" + fi.fileName();
        }

      QFile file( fileName );

      if( ! file.open( QIODevice::Append | QIODevice::Text ) )
        {
          QMessageBox mb( QMessageBox::Critical,
                          tr( "Save failed" ),
                          tr( "Cannot open file!" ),
                          QMessageBox::Ok,
                          this );
#ifdef ANDROID

          mb.show();
          QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                           height()/2 - mb.height()/2 ));
          mb.move( pos );

#endif
          mb.exec();
          return;
        }

      QString str = nmeaList.join( "" );
      file.write( str.toLatin1() );
      file.close();

      // The stored data is cleared from the list.
      nmeaList.clear();
    }
}

void GpsStatusDialog::slot_Close()
{
  close();
}

void GpsStatusDialog::closeEvent( QCloseEvent *event )
{
  GeneralConfig::instance()->setGpsFilterIndex( satSource->currentIndex() );
  emit closingWidget();
  event->accept();
}

void GpsStatusDialog::keyReleaseEvent(QKeyEvent *event)
{
  // close the dialog on key press
  switch(event->key())
    {
      case Qt::Key_Close:
      case Qt::Key_Escape:
         close();
        break;
      default:
        QWidget::keyReleaseEvent( event );
        break;
    }
}

/*************************************************************************************/

#define MARGIN 10

GpsElevationAzimuthDisplay::GpsElevationAzimuthDisplay(QWidget *parent) :
  QFrame(parent),
  width(0),
  height(0)
{
  setFrameStyle(StyledPanel | QFrame::Plain);
  setLineWidth(2);
  setMidLineWidth(2);
}

GpsElevationAzimuthDisplay::~GpsElevationAzimuthDisplay()
{
}

void GpsElevationAzimuthDisplay::resizeEvent( QResizeEvent *event )
{
  QFrame::resizeEvent( event );

  width  = contentsRect().width();
  height = contentsRect().height();

  if (width > height)
    {
      width = height;
    }

  width  -= ( MARGIN * 2 * Layout::getIntScaledDensity() );
  height -= ( MARGIN * 2 * Layout::getIntScaledDensity() );
  center  = QPoint(contentsRect().width()/2, contentsRect().height()/2);

  background = QPixmap( contentsRect().width(), contentsRect().height() );

  background.fill( palette().color(QPalette::Window) );

  QPainter p( &background );

  p.setPen( QPen( Qt::black, 2 * Layout::getIntScaledDensity(), Qt::SolidLine ) );
  //outer circle
  p.drawEllipse(center.x() - ( width / 2 ), center.y() - ( width / 2 ), width, width);
  //inner circle. This is the 45 degrees. The diameter is cos(angle)
  p.drawEllipse(center.x() - ( width / 4 ), center.y() - ( width / 4 ),
                width / 2, width / 2 );

  p.drawLine(center.x(), center.y() - ( width / 2 ) - 5, center.x(), center.y() + ( width / 2 ) + 5);
  p.drawLine(center.x() - ( width / 2 ) -5 , center.y(), center.x() + ( width / 2 ) + 5, center.y());
}

void GpsElevationAzimuthDisplay::paintEvent( QPaintEvent *event )
{
  // Call paint method from QFrame otherwise the frame is not drawn.
  QFrame::paintEvent( event );

  QPainter p(this);
  QFont f = font();

#ifdef ANDROID
  f.setPointSize(6);
#else
  f.setPixelSize(12);
#endif

  p.setFont(f);

  // copy background to widget
  p.drawPixmap ( contentsRect().left(), contentsRect().top(), background,
                 0, 0, background.width(), background.height() );

  // draw satellites
  if( sats.count() )
    {
      for( int i = 0; i < sats.count(); i++ )
        {
          drawSat( &p, sats.at( i ) );
        }
    }
  else
    {
      QString text( tr("No Data") );
      QRect rect = p.fontMetrics().boundingRect( text );

      int w = rect.width();
      int h = rect.height();

      p.fillRect( center.x() - w / 2,
                  center.y() - h / 2,
                  w,
                  h,
                  palette().color(QPalette::Window) );

      p.drawText( center.x() - w / 2,
                  center.y() - h / 2,
                  w,
                  h,
                  Qt::AlignCenter,
                  text );
    }
}

void GpsElevationAzimuthDisplay::setSatInfo( QList<SIVInfo>& list )
{
  sats = list;
  update();
}

void GpsElevationAzimuthDisplay::drawSat( QPainter *p, const SIVInfo& sivi )
{
  if (sivi.db < 0)
    {
      return;
    }

  double a = (double(( sivi.azimuth - 180 )/ -180.0) * M_PI);
  double r = (90 - sivi.elevation) / 90.0;

  int x = int(center.x() + (width / 2) * r * sin (a));
  int y = int(center.y() + (width / 2) * r * cos (a));

  int R, G;
  int db = qMin(sivi.db * 2, 99);

  if (db < 50)
    {
      R=255;
      G=(255/50 * db);
    }
  else
    {
      R=255 - (255/50 * (db - 50));
      G=255;
    }

  QFont f = font();

#ifdef ANDROID
  f.setPointSize(6);
#else
  f.setPixelSize(12);
#endif

  p->setFont(f);

  int margin = 2 * Layout::getIntScaledDensity();
  int w = QFontMetrics(f).width("MM") + 2 * margin;
  int h = QFontMetrics(f).height() + 2 * margin;

  p->setBrush(QColor(R,G,0));

  p->drawRect(x - w/2, y - h/2, w , h );
  p->drawText(x - w/2, y - h/2, w, h, Qt::AlignCenter, QString::number(sivi.id) );
}

/*************************************************************************************/

GpsSnrDisplay::GpsSnrDisplay(QWidget *parent) :
  QFrame(parent),
  width(0),
  height(0),
  xoff(0),
  yoff(0)
{
  setFrameStyle(StyledPanel | QFrame::Plain);
  setLineWidth(2);
  setMidLineWidth(2);
}

GpsSnrDisplay::~GpsSnrDisplay()
{
}

void GpsSnrDisplay::resizeEvent( QResizeEvent *event )
{
  QFrame::resizeEvent( event );

  width  = contentsRect().width();
  height = contentsRect().height();
  xoff   = (QWidget::width() - width) / 2;
  yoff   = (QWidget::height() - height) / 2;
  center = QPoint(contentsRect().width()/2, contentsRect().height()/2);

  background = QPixmap( width, height );

  QPainter p( &background );

  // Draws the background
  int R, G;
  double dbfactor = 100.0 / height;
  double db;

  for( int i = 0; i <= height; i++ )
    {
      db = i * dbfactor;

      if( db < 50 )
        {
          R = 255;
          G = int( 255 / 50 * db );
        }
      else
        {
          R = int( 255 - (255 / 50 * (db - 50)) );
          G = 255;
        }

      p.setPen( QPen( QColor( R, G, 0 ), 1, Qt::SolidLine ) );
      p.drawLine( 0, height - i, width, height - i );
    }
}

void GpsSnrDisplay::paintEvent( QPaintEvent *event )
{
  // Call paint method from QFrame otherwise the frame is not drawn.
  QFrame::paintEvent( event );

  QPainter pw(this);

  pw.fillRect( xoff, yoff, width, height, palette().color(QPalette::Window) );

  // draw satellites
  if( sats.count() )
    {
      for (int i=0; i < sats.count(); i++)
        {
          drawSat(&pw, i, sats.count(), sats.at(i));
        }
    }
  else
    {
      QFont f = font();

#ifdef ANDROID
  f.setPointSize(6);
#else
  f.setPixelSize(12);
#endif

      pw.setFont(f);
      pw.fillRect( center.x()-23, center.y()-7, 46, 14, palette().color(QPalette::Window) );

      QString text = tr("No Data");

      QFontMetrics fm( font() );

      int w = fm.width( text );
      int h = fm.height();

      pw.drawText( center.x() - w / 2, center.y() + h / 2, text );
    }
}

void GpsSnrDisplay::setSatInfo(QList<SIVInfo>& list)
{
  sats = list;
  update();
}

void GpsSnrDisplay::drawSat( QPainter *p, int i, int cnt, const SIVInfo& sivi )
{
  int bwidth = width / cnt;
  int left = bwidth * i + 2;
  int db = sivi.db * 2;
  db = qMin(db, 100);
  int bheight = qMax(int((double(height-5) / 99.0) * db), 14);

  // Copy the needed part from the background picture.
  p->drawPixmap( left, height-bheight, bwidth - 2, bheight,
                 background,
                 left, height-bheight, bwidth - 2, bheight );

  if( sivi.db < 0 )
    {
      p->fillRect( left, height-bheight, bwidth - 2, bheight, Qt::white );
    }

  QFont f = font();

#ifdef ANDROID
  f.setPointSize(6);
#else
  f.setPixelSize(12);
#endif

  p->setFont( f );
  p->setPen( Qt::black );
  p->setBrush( Qt::NoBrush );
  p->drawRect( left, height, bwidth - 2, -bheight );

  int margin = 2 * Layout::getIntScaledDensity();
  int h = QFontMetrics(f).height() + 2 * margin;

  p->drawText( left + Layout::getIntScaledDensity(),
               height-h,
               bwidth-4,
               h,
               Qt::AlignCenter,
               QString::number(sivi.id));
}
