/***********************************************************************
**
**   gpsstatusdialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2003      by André Somers
**                  2008-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QtGui>

#include "gpsstatusdialog.h"
#include "gpsnmea.h"
#include "mainwindow.h"

// initialize static member variable
int GpsStatusDialog::noOfInstances = 0;

GpsStatusDialog::GpsStatusDialog(QWidget * parent) :
  QWidget( parent ),
  showNmeaData( true ),
  nmeaLines( 0 )
{
  noOfInstances++;

  setWindowTitle(tr("GPS Status"));
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
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

  QFont f = font();

#if defined MAEMO || defined ANDROID
  f.setPixelSize(16);
#else
  f.setPixelSize(14);
#endif

  nmeaBox->setFont(f);

  QScrollArea *nmeaScrollArea = new QScrollArea;
  nmeaScrollArea->setWidgetResizable( true );
  nmeaScrollArea->setWidget(nmeaBox);

#ifdef QSCROLLER
  QScroller::grabGesture(nmeaScrollArea, QScroller::LeftMouseButtonGesture);
#endif

  QVBoxLayout* nmeaBoxLayout = new QVBoxLayout;
  nmeaBoxLayout->setSpacing( 0 );
  nmeaBoxLayout->addWidget( nmeaScrollArea );

  startStop = new QPushButton( tr("Stop"), this );
  save      = new QPushButton( tr("Save"), this );

  QPushButton* close = new QPushButton( tr("Close"), this );

  QVBoxLayout* buttonBox = new QVBoxLayout;
  buttonBox->addStretch( 5 );
  buttonBox->addWidget( startStop );
  buttonBox->addSpacing( 10 );
  buttonBox->addWidget( save );
  buttonBox->addSpacing( 10 );
  buttonBox->addWidget( close );
  buttonBox->addStretch( 5 );

  QHBoxLayout* hBox = new QHBoxLayout;
  hBox->addWidget(elevAziDisplay, 1);
  hBox->addWidget(snrDisplay, 2);
  hBox->addLayout( buttonBox );

  QVBoxLayout* topLayout = new QVBoxLayout( this );
  topLayout->addLayout( hBox );
  topLayout->addLayout( nmeaBoxLayout );

  QShortcut* keySpace = new QShortcut( QKeySequence(Qt::Key_Space), this);

  connect( keySpace, SIGNAL(activated()),
           this, SLOT(slot_ToggleWindowSize()));

  connect( GpsNmea::gps, SIGNAL(newSentence(const QString&)),
           this, SLOT(slot_Sentence(const QString&)) );
  connect( GpsNmea::gps, SIGNAL(newSatInViewInfo(QList<SIVInfo>&)),
           this, SLOT(slot_SIV(QList<SIVInfo>&)) );

  connect( startStop, SIGNAL(clicked()), this, SLOT(slot_ToggleStartStop()) );

  connect( save, SIGNAL(clicked()), this, SLOT(slot_SaveNmeaData()) );

  connect( close, SIGNAL(clicked()), this, SLOT(slot_Close()) );
}

GpsStatusDialog::~GpsStatusDialog()
{
  noOfInstances--;
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

  if( showNmeaData )
    {
      nmeaLines++;

      if( nmeaLines > maxLines )
        {
          // Note! To display to many lines can become a performance issue
          // because the window update effort is huge.
          int idx = nmeaData.indexOf( QChar('\n') );

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

  QString fileName = QInputDialog::getText( this, tr("Append to?"),
                                            tr("File name:"), QLineEdit::Normal,
                                            "nmea-stream.log", &ok);
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
      file.write( str.toAscii() );
      file.close();

      // The stored data is cleared from the list.
      nmeaList.clear();
    }
}

void GpsStatusDialog::slot_Close()
{
  emit closingWidget();
  close();
}

void GpsStatusDialog::keyReleaseEvent(QKeyEvent *event)
{
  // close the dialog on key press
  switch(event->key())
    {
      case Qt::Key_Close:
      case Qt::Key_Escape:
        emit closingWidget();
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
  QFrame(parent)
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

  width  -= ( MARGIN * 2 ); //keep a 10 pixel margin
  height -= ( MARGIN * 2 );
  center  = QPoint(contentsRect().width()/2, contentsRect().height()/2);

  background = QPixmap( contentsRect().width(), contentsRect().height() );

  background.fill( palette().color(QPalette::Window) );

  QPainter p( &background );

  p.setPen( QPen( Qt::black, 2, Qt::SolidLine ) );
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
  f.setPixelSize(12);
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
  static uint counter = 1;

  if( ++counter % 2 )
    {
      sats = list;
      update();
      // repaint();
    }
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
  f.setPixelSize(12);
  p->setFont(f);

  p->setBrush(QColor(R,G,0));
  p->drawRect(x - 9, y - 7, 18 , 14 );
  p->drawText(x - 9, y - 5, 18 , 14 , Qt::AlignCenter, QString::number(sivi.id) );
}

/*************************************************************************************/

GpsSnrDisplay::GpsSnrDisplay(QWidget *parent) : QFrame(parent)
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
      f.setPixelSize(12);
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
  static uint counter = 0;

  if( ++counter % 2 )
    {
      sats = list;
      update();
      // repaint();
    }
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
  f.setPixelSize( 12 );
  p->setFont( f );
  p->setPen( Qt::black );
  p->setBrush( Qt::NoBrush );
  p->drawRect( left, height, bwidth - 2, -bheight );
  p->drawText(left+1, height-13, bwidth-4, 12, Qt::AlignCenter, QString::number(sivi.id));
}
