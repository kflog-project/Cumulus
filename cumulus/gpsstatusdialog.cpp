/***********************************************************************
**
**   gpsstatusdialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>

#include "gpsstatusdialog.h"
#include "gpsnmea.h"
#include "mapcalc.h"
#include "resource.h"

GpsStatusDialog::GpsStatusDialog(QWidget * parent) : QDialog(parent)
{
  setWindowTitle(tr("GPS Status - <Esc> to Close"));
  setModal(true);

#ifdef MAEMO
  resize(500,480);
  setSizeGripEnabled(false);
#else
  setSizeGripEnabled(true);
#endif

  elevAziDisplay = new GPSElevationAzimuthDisplay(this);
  elevAziDisplay->setFrameStyle(QFrame::Panel | QFrame::Sunken);

  snrDisplay = new GPSSnrDisplay(this);
  snrDisplay->setFrameStyle(QFrame::Panel | QFrame::Sunken);

  nmeaBox = new QTextEdit(this);
  nmeaBox->setObjectName("nmeabox");
  nmeaBox->setReadOnly(true);
  nmeaBox->document()->setMaximumBlockCount(100);
  nmeaBox->setLineWrapMode(QTextEdit::NoWrap);

  QFont f = font();
  f.setPixelSize(12);
  nmeaBox->setFont(f);

  /* #warning: FIXME
     Something is seriously rotten here. I can add either the 
     snrDisplay _or_ the nmeaBox. Adding both will result in a
     crash. I really, really have no idea what's causing that,
     and I don't want to spend any more frustrating hours finding
     the cause just now. So, I ditched the NMEA box for now.
     The crash most often happens with showMaximized, but on 
     destruction is also a rather popular moment.
     
     A.S. */

  QVBoxLayout* topLayout=new QVBoxLayout(this);
  topLayout->addWidget(elevAziDisplay, 10);
  topLayout->addWidget(snrDisplay, 5);
  topLayout->addWidget(nmeaBox, 5);

  connect(gps, SIGNAL(newSentence(const QString&)),
          this, SLOT(slot_Sentence(const QString&)));
  connect(gps, SIGNAL(newSatInViewInfo()),
          this, SLOT(slot_SIV()));

/*  if( QApplication::desktop()->screenGeometry().width() > 240 &&
      QApplication::desktop()->screenGeometry().height() > 320 )
    { 
//      resize( 600, 400 );
      show();
    }
  else
    {
//      showMaximized();
    }*/
}


GpsStatusDialog::~GpsStatusDialog()
{
  // qDebug("GpsStatusDialog::~GpsStatusDialog()");
  disconnect(gps, SIGNAL(newSentence(const QString&)),
             this, SLOT(slot_Sentence(const QString&)));
  disconnect(gps, SIGNAL(newSatInViewInfo()),
             this, SLOT(slot_SIV()));
}


void GpsStatusDialog::slot_SIV()
{
  //qDebug("received new sivi info signal");
  elevAziDisplay->setSatInfo(gps->getSivInfo());
  snrDisplay->setSatInfo(gps->getSivInfo());
}


void GpsStatusDialog::slot_Sentence(const QString& sentence)
{
  nmeaBox->insertPlainText(sentence.trimmed().append("\n"));
}


void GpsStatusDialog::keyPressEvent(QKeyEvent *e)
{
  // close the dialog on key press
  // qDebug("GpsStatusDialog::keyPressEvent");
  switch(e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
      accept();
      break;
    }
}

void GpsStatusDialog::accept()
{
  QDialog::accept();
}

void GpsStatusDialog::reject()
{
  QDialog::reject();
}


/*************************************************************************************/

#define MARGIN 10

GPSElevationAzimuthDisplay::GPSElevationAzimuthDisplay(QWidget *parent):
    QFrame(parent)
{
  setObjectName("ElevationAzimuthDisplay");
  background = new QPixmap();
}


GPSElevationAzimuthDisplay::~GPSElevationAzimuthDisplay()
{
  delete background;
}


void GPSElevationAzimuthDisplay::resizeEvent(QResizeEvent *)
{
  width = contentsRect().width();
  height = contentsRect().height();

  if (width > height)
    {
      width = height;
    }

  width -= ( MARGIN * 2 ); //keep a 10 pixel margin
  center = QPoint(contentsRect().width()/2, contentsRect().height()/2);

  delete background;
  background = new QPixmap( contentsRect().width(), contentsRect().height() );

  background->fill( palette().color(QPalette::Window) );

  QPainter p(background);
  p.setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
  //outer circle
  p.drawEllipse(center.x() - ( width / 2 ), center.y() - ( width / 2 ), width, width);
  //inner circle. This is the 45 degrees. The diameter is cos(angle)
  p.drawEllipse(center.x() - ( width / 4 ), center.y() - ( width / 4 ),
                width / 2, width / 2 );

  p.drawLine(center.x(), center.y() - ( width / 2 ) - 5, center.x(), center.y() + ( width / 2 ) + 5);
  p.drawLine(center.x() - ( width / 2 ) -5 , center.y(), center.x() + ( width / 2 ) + 5, center.y());
}

void GPSElevationAzimuthDisplay::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  QFont f = font();
  f.setPixelSize(12);
  p.setFont(f);

  //copy background to widget
  p.drawPixmap ( contentsRect().left(), contentsRect().top(), *background,
                 0, 0, background->width(), background->height() );

  //draw satelites
  if (sats.count())
    {
      for (int i=0; i < sats.count(); i++)
        {
          drawSat(&p, sats.at(i));
        }
    }
  else
    {
      p.fillRect( center.x()-23, center.y()-7, 46, 14, palette().color(QPalette::Window) );
      p.drawText(center.x()-23, center.y()-7, 46, 14, Qt::AlignCenter, tr("No Data"));
    }

}

void GPSElevationAzimuthDisplay::setSatInfo(QList<SIVInfo>& list)
{
  sats = list;
  // update();
  repaint();
}

void GPSElevationAzimuthDisplay::drawSat(QPainter * p, const SIVInfo& sivi)
{
  if (sivi.db < 0)
    return;

  double a = (double(( sivi.azimuth - 180 )/ -180.0) * M_PI);
  double r = (90 - sivi.elevation) / 90.0;
  int x = int(center.x() + (width / 2) * r * sin (a));
  int y = int(center.y() + (width / 2) * r * cos (a));
  //qDebug("%f", a);

  int R, G;
  int db=MIN(sivi.db * 2, 99);

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
  p->fillRect(x - 9, y - 7, 18 , 14 , p->brush());
  p->drawRect(x - 9, y - 7, 18 , 14 );
  p->drawText(x - 9, y - 5, 18 , 14 , Qt::AlignCenter, QString::number(sivi.id) );
}


/*************************************************************************************/

GPSSnrDisplay::GPSSnrDisplay(QWidget *parent):
    QFrame(parent)
{
  setObjectName("GPSSnrDisplay");
  background = new QPixmap();
  canvas = new QPixmap();
  mask = new QBitmap();
}

GPSSnrDisplay::~GPSSnrDisplay()
{
  delete background;
  delete canvas;
  delete mask;
}

void GPSSnrDisplay::resizeEvent(QResizeEvent *)
{
  width = contentsRect().width();
  height = contentsRect().height();
  center = QPoint(contentsRect().width()/2, contentsRect().height()/2);

  delete background;
  background = new QPixmap( width, height );
  background->fill();
  delete canvas;
  canvas = new QPixmap( width, height );

  delete mask;
  mask = new QBitmap( width, height );
  mask->fill();

  QPainter p(background);

  int R, G;
  double dbfactor = 100.0 / height;
  double db;

  for (int i=0;i<=height;i++)
    {
      db = i * dbfactor;
      if (db < 50)
        {
          R = 255;
          G = int( 255/50 * db );
        }
      else
        {
          R = int( 255 - (255/50 * (db - 50)) );
          G = 255;
        }

      p.setPen( QPen( QColor(R, G, 0), 1, Qt::SolidLine ) );
      p.drawLine(0, height-i, width, height-i);
    }
}

void GPSSnrDisplay::paintEvent(QPaintEvent *)
{
  QPainter p;

  p.begin(canvas);
  p.drawPixmap( 0, 0, *background, 0, 0, background->width(), background->height() );

//  p.fillRect(0, 0, width, height, palette().color(QPalette::Window) );
  //draw satellites
  if (sats.count())
    {
      QPainter pm(mask);
      pm.fillRect(0, 0, width, height, Qt::color0);
      // int cnt=MIN(sats->count(), 8);
      for (int i=0; i < sats.count(); i++)
        {
          drawSat(&p, &pm, i, sats.count(), sats.at(i));
        }

      p.end();

      //copy canvas to widget, masked by the mask
      canvas->setMask(*mask);

      QPainter pw(this);
      pw.drawPixmap( 0, 0, *canvas, 0, 0, canvas->width(), canvas->height() );
    }
  else
    {
      QPainter pd(this);
      QFont f = font();
      f.setPixelSize(12);
      pd.setFont(f);
      pd.fillRect( center.x()-23, center.y()-7, 46, 14, palette().color(QPalette::Window) );
      pd.drawText(center.x()-23, center.y()-7, 46, 14, Qt::AlignCenter, tr("No Data"));
    }
}

void GPSSnrDisplay::setSatInfo(QList<SIVInfo>& list)
{
  sats = list;
  // update();
  repaint();
}

void GPSSnrDisplay::drawSat(QPainter * p, QPainter * pm, int i, int cnt, const SIVInfo& sivi)
{
  int bwidth = width / cnt;
  int left = bwidth * i + 2;
  int db = sivi.db * 2;
  db = MIN(db, 100);
  int bheight = MAX(int((double(height) / 99.0) * db), 14);
  //qDebug("id: %d, db: %d, bheight: %d (height: %d)", sivi->id, sivi->db, bheight, height);
  pm->fillRect(left, height, bwidth - 2, -bheight, Qt::color1);

  if (sivi.db < 0)
    {
      p->fillRect(left, height, bwidth - 2, -bheight, Qt::white);
    }

  QFont f = font();
  f.setPixelSize(12);
  p->setFont(f);
  p->setPen(Qt::black);
  p->drawRect(left, height, bwidth - 2, -bheight);
  p->drawText(left+1, height-13, bwidth-4, 12, Qt::AlignCenter, QString::number(sivi.id));
}
