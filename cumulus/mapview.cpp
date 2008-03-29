/***************************************************************************
                          mapview.cpp  -  This file is part of Cumulus.
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002 by Andre Somers, 2008 Axel Pauli, Josua Dietze
    email                : andre@kflog.org

    $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFrame>
#include <QPalette>
#include <QWhatsThis>
#include <QBoxLayout>

#include "mapview.h"
#include "gpsnmea.h"
#include "cumulusapp.h"
#include "waypointcatalog.h"
#include "mapmatrix.h"
#include "wgspoint.h"
#include "mapcalc.h"
#include "waypoint.h"
#include "speed.h"
#include "igclogger.h"
#include "mapinfobox.h"
#include "map.h"
#include "generalconfig.h"
#include "interfaceelements.h"
#include "multilayout.h"
#include "filetools.h"
#include "altimetermodedialog.h"
#include "gliderflightdialog.h"
#include "gpsstatusdialog.h"
#include "variomodedialog.h"

//general KFLOG file token: @KFL
#define KFLOG_FILE_MAGIC    0x404b464c
//file type and version
#define FILE_TYPE 0x4c
#define FILE_VERSION 0x01

#define MAX_LINES 10   //ten lines max for each direction
//#define VLINE_BASE MVM_MAX_ELEMENT;
//#define HLINE_BASE VLINE_BASE+MAX_LINES;

const int VLINE_BASE = MVW_MAX_ELEMENT;
const int HLINE_BASE = VLINE_BASE + MAX_LINES;


MapView::MapView(QWidget *parent) : QWidget(parent)
{
  setObjectName("MapView");

  resize(parent->size());
  setContentsMargins(-9,-9,-9,-9);

  qDebug( "MapView window size is %dx%d, width=%d, height=%d",
          parent->size().width(),
          parent->size().height(),
          parent->size().width(),
          parent->size().height() );

  cuApp = (CumulusApp *)parent;

  // Later on the Pretext can change depending on Mode
  GeneralConfig *conf = GeneralConfig::instance();
  _altimeterMode = conf->getAltimeterMode();

  // load pixmap of arrows for relative bearing
  _arrows = GeneralConfig::instance()->loadPixmap( "arrows60pix-15.png" );

  //make the main box layout
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing(0);
//  topLayout->setSizeConstraint( QLayout::SetFixedSize );

  //@JD: the new "sidebar" layout
  QBoxLayout *centerLayout = new QHBoxLayout( topLayout );
  topLayout->setStretchFactor( centerLayout, 1 );
  centerLayout->setSpacing(0);

  QBoxLayout *sideLayout = new QVBoxLayout( centerLayout );
  sideLayout->setSpacing(0);

  // three 'grouping' widgets with slightly differing
  // background color and fixed width (to avoid frequent resizing)

  //widget to group waypoint functions
  QWidget *wayBar = new QWidget( this );
  wayBar->setFixedWidth(216);
  wayBar->setContentsMargins(-9,-8,-9,-3);

  wayBar->setAutoFillBackground(true);
  wayBar->setBackgroundRole(QPalette::Window);
  wayBar->setPalette( QPalette(QColor(Qt::lightGray)) );

  // vertical layout for waypoint widgets
  QBoxLayout *wayLayout = new QVBoxLayout( wayBar );
  wayLayout->setSpacing(4);

  //add Waypoint widget (whole line)
  _waypoint = new MapInfoBox( this, conf->getFrameCol(), 22 );
  _waypoint->setValue("-");
  _waypoint->setPreText("To");
  wayLayout->addWidget( _waypoint );
  connect(_waypoint, SIGNAL(mousePress()),
          (CumulusApp*)parent, SLOT(slotSwitchToWPListViewExt()));


  //layout for Glide Path and Relative Bearing
  QBoxLayout *GRLayout = new QHBoxLayout(wayLayout);
  GRLayout->setSpacing(4);

  //add Glide Path widget
  _glidepath = new MapInfoBox( this, conf->getFrameCol(), 40 );
  _glidepath->setValue("-");
  _glidepath->setPreText("Arr");
  GRLayout->addWidget( _glidepath );
//  QWhatsThis::add(_glidepath, tr("Glide path"));
  connect(_glidepath, SIGNAL(mousePress()),
          (CumulusApp*)parent, SLOT(slotSwitchToReachListView()));

  // add Relative Bearing widget
  _rel_bearing = new QLabel(this,"");
  QPixmap arrow = _arrows.copy( 24*60, 0, 60, 60 );
  _rel_bearing->setPixmap (arrow);
  GRLayout->addWidget(_rel_bearing);
//  QWhatsThis::add(_rel_bearing, tr("Relative bearing"));


  //layout for Distance/ETA and Bearing
  QBoxLayout *DEBLayout = new QHBoxLayout(wayLayout);
  DEBLayout->setSpacing(2);

  //add Distance widget
  _distance = new MapInfoBox( this, conf->getFrameCol() );
  _distance->setValue("-");
  _distance->setPreText("Dis");
  DEBLayout->addWidget( _distance);
  connect(_distance, SIGNAL(mousePress()), this, SLOT(slot_toggleDistanceEta()));

  //add ETA widget
  _eta = new MapInfoBox( this, conf->getFrameCol() );
  _eta->setValue("-");
  _eta->setPreText( "Eta" );
  DEBLayout->addWidget( _eta );
  _eta->hide();
  connect(_eta, SIGNAL(mousePress()), this, SLOT(slot_toggleDistanceEta()));

  //add Bearing widget
  _bearingMode = 1; // normal bearing display is default
  _lastBearing = -1;
  _bearingTimer = new QTimer(this);
  connect (_bearingTimer, SIGNAL(timeout()),
           this, SLOT(slot_resetInversBearing()));
  _bearing = new MapInfoBox( this, conf->getFrameCol() );
  _bearingBGColor = wayBar->backgroundColor();
  _bearing->setValue("-");
  _bearing->setPreText("Brg");
  DEBLayout->addWidget( _bearing);
  connect(_bearing, SIGNAL(mousePress()), this, SLOT(slot_toggleBearing()));

  sideLayout->addWidget( wayBar, 3 );


  //widget to group common displays
  QWidget *commonBar = new QWidget( this );
  commonBar->setFixedWidth(216);
  commonBar->setContentsMargins( -9, -4, -9, -4);

  // vertical layout for common display widgets
  QBoxLayout *commonLayout = new QVBoxLayout( commonBar );
  commonLayout->setSpacing(4);

  //layout for Speed and Heading
  QBoxLayout *SHLayout = new QHBoxLayout(commonLayout);
  SHLayout->setSpacing(2);

  //add Speed widget
  _speed = new MapInfoBox( this, "#efefef" );
  _speed->setValue("-");
  _speed->setPreText("Gs");
  SHLayout->addWidget( _speed);

  //add Heading widget
  _heading = new MapInfoBox( this, "#efefef" );
  _heading->setValue("-");
  _heading->setPreText("Trk");
  SHLayout->addWidget( _heading);

  //add Altitude widget (whole line)
  _altitude = new MapInfoBox( this, conf->getFrameCol() );
  _altitude->setValue("-");
  _altitude->setPreText(AltimeterModeDialog::mode2String()); // get current mode
  commonLayout->addWidget( _altitude );
  connect(_altitude, SIGNAL(mousePress()),
          this, SLOT(slot_AltimeterDialog()));


  //layout for Vario and Wind/LD
  QBoxLayout *VWLLayout = new QHBoxLayout(commonLayout);
  VWLLayout->setSpacing(2);

  //add Vario widget
  _vario = new MapInfoBox( this, conf->getFrameCol() );
  _vario->setValue("-");
  _vario->setPreText("Var");
  VWLLayout->addWidget(_vario );
  connect(_vario, SIGNAL(mousePress()),
          this, SLOT(slot_VarioDialog()));

  //add Wind widget; this is head/tailwind, no direction given !
  _wind = new MapInfoBox( this, conf->getFrameCol() );
  _wind->setValue("-");
  _wind->setPreText("Wd");
  VWLLayout->addWidget(_wind );
  connect(_wind, SIGNAL(mousePress()), this, SLOT(slot_toggleWindAndLD()));

  //add LD widget
  _ld = new MapInfoBox( this, conf->getFrameCol() );
  _ld->setValue("-/-");
  _ld->setPreText( "LD" );
  VWLLayout->addWidget( _ld );
  _ld->hide();
  connect(_ld, SIGNAL(mousePress()), this, SLOT(slot_toggleWindAndLD()));

  sideLayout->addWidget( commonBar, 3 );


  //widget to group McCready functions
  QWidget *mcBar = new QWidget( this );
  mcBar->setFixedWidth(216);
  mcBar->setContentsMargins(-9,-3,-9,-9);

  mcBar->setAutoFillBackground(true);
  mcBar->setBackgroundRole(QPalette::Window);
  mcBar->setPalette( QPalette(QColor(Qt::lightGray)) );

  //layout for McCready and Best Speed
  QBoxLayout *MSLayout = new QHBoxLayout(mcBar);
  MSLayout->setSpacing(2);

  //add McCready widget
  _mc = new MapInfoBox( this, conf->getFrameCol() );
  _mc->setValue("0.0");
  _mc->setPreText("Mc");
  MSLayout->addWidget( _mc );
  connect(_mc, SIGNAL(mousePress()), this, SLOT(slot_gliderFlightDialog()));

  //add Best Speed widget
  _speed2fly = new MapInfoBox( this, "#c6c3c6" );
  _speed2fly->setValue("-");
  _speed2fly->setPreText("S2f");
  MSLayout->addWidget( _speed2fly );
//  QWhatsThis::add(_speed2fly, tr("Best speed"));

  sideLayout->addWidget( mcBar, 1 );


  /*
  //layout for the Heading and Bearing boxes
  QBoxLayout *FWELayout = new QHBoxLayout(sideLayout);
  FWELayout->setSpacing(2);

  //add FlightTime widget
  _flighttime = new MapInfoBox( this );
  _flighttime->setPreText( "T: " );
  _flighttime->setValue("02:27");
  FWELayout->addWidget( _flighttime,10 );

  
  //add elevation widget

  _elevation = new MapInfoBox( this );
  _elevation->setValue("-");
  _elevation->setPreText("Elv");
  FWELayout->addWidget( _elevation,15 );
  QWhatsThis::add (_elevation, tr("Elevation"));
  */


  //--------------------------------------------------------------------
  //layout for the map
  QBoxLayout *MapLayout = new QHBoxLayout(centerLayout);
  _theMap = new Map(this);
  centerLayout->setStretchFactor( MapLayout, 1 );
//  MapLayout->addSpacing(1);
  MapLayout->addWidget(_theMap, 10);
//  MapLayout->addSpacing(1);
  _theMap->setMode(Map::headUp);
//  topLayout->addSpacing(0);

  //--------------------------------------------------------------------
  // Status bar
  _statusbar = new QStatusBar(this, "status");
  _statusbar->setSizeGripEnabled(false);
  _statusbar->setFixedHeight(19);
  QFont font = _statusbar->font();
  font.setBold(true);
  _statusbar->setFont(font);

  _menuToggle = new CuLabel( tr("Menu"),_statusbar);
  _menuToggle->setFrameStyle(QFrame::Box|QFrame::Plain);
  _menuToggle->setLineWidth(0);
  _menuToggle->setAlignment(Qt::AlignCenter);
  _menuToggle->setMargin(0);
  _statusbar->addWidget(_menuToggle);
  connect(_menuToggle, SIGNAL(mousePress()), (CumulusApp*)parent, SLOT(slotToggleMenu()));

  _statusGps = new CuLabel(tr("Man"),_statusbar);
  _statusGps->setFrameStyle(QFrame::Box|QFrame::Plain);
  _statusGps->setLineWidth(0);
  _statusGps->setAlignment(Qt::AlignCenter);
  _statusGps->setMargin(0);
  _statusGps->setMaximumWidth(30);
  _statusGps->setMinimumWidth(10);
  _statusbar->addWidget(_statusGps);
  connect(_statusGps, SIGNAL(mousePress()), this, SLOT(slot_gpsStatusDialog()));

  _statusFlightstatus = new QLabel("<html>" + tr("?","Unknown") + "</html>",_statusbar);
  _statusFlightstatus->setFrameStyle(QFrame::Box|QFrame::Plain);
  _statusFlightstatus->setLineWidth(0);
  _statusFlightstatus->setAlignment(Qt::AlignCenter);
  _statusFlightstatus->setMargin(0);
  _statusFlightstatus->setMinimumSize(_statusFlightstatus->fontMetrics().boundingRect(" L ? ").width(), 5);
  _statusFlightstatus->setTextFormat(Qt::RichText);
  _statusbar->addWidget(_statusFlightstatus);

  _statusFiller = new QLabel(_statusbar);
  _statusFiller->setFrameStyle(QFrame::Box|QFrame::Plain);
  _statusFiller->setLineWidth(0);
  _statusFiller->setAlignment(Qt::AlignCenter);
  _statusFiller->setMargin(0);
  _statusbar->addWidget(_statusFiller, 1);

  QFrame* filler = new QFrame(_statusbar);
  filler->setFrameStyle(QFrame::NoFrame);
  _statusbar->addWidget(filler);

  //  _statusbar->setFrameStyle(QFrame::Raised);

  loggingTimer = new QTimer(this);
  connect (loggingTimer, SIGNAL(timeout()),
           this, SLOT(slot_setFlightStatus()));
  topLayout->addWidget(_statusbar);

  lastPositionChangeSource = CuCalc::MAN;
}


MapView::~MapView()
{
  // qDebug("MapView::~MapView() destructor is called");
}


/** called if heading has changed */
void MapView::slot_Heading(int head)
{
  //  QPoint curPos;

  //    _heading->setValue(QString("%1°").arg(head));
  _heading->setValue(QString("%1").arg( head, 3, 10, QChar('0') ));
  _theMap->setHeading(head);
}


/** Called if speed has changed */
void MapView::slot_Speed(const Speed& speed)
{
  if (speed.getMph()<0)
    {
      _speed->setValue("-");
    }
  else
    {
      _speed->setValue(speed.getHorizontalText(false,0));
    }
}


/** called if the waypoint is changed in the calculator */
void MapView::slot_Waypoint(const wayPoint *wp)
{
  // qDebug("MapView::slot_Waypoint");
  QString dest;

  if( wp )
    {
      if( wp->taskPointIndex != -1 )
        {
          QString idx;
          idx.sprintf( "%d ", wp->taskPointIndex );
          dest += tr("TP") + idx;
        }

      dest += QString("%1 (%2)").arg(wp->name.left(14)).arg( Altitude::getText(wp->elevation, false, 0));
      _waypoint->setValue(dest);
    }
  else
    {
      // @AP: No waypoint is selected, reset waypoint and glidepath display
      _waypoint->setValue("");
      _glidepath->setValue("-");
      // @JD: reset distance too
      _distance->setValue("-");
      QPixmap arrow = _arrows.copy( 24*60, 0, 60, 60 );
      _rel_bearing->setPixmap (arrow);
      qDebug("Rel. bearing icon reset" );
    }

  _theMap->quickDraw();  // this is not really helpful -> it is: the bearingline won't change otherwise!
}


/** This slot is called by calculator if a new bearing has been calculated */
void MapView::slot_Bearing(int bearing)
{
  _lastBearing = bearing; // save received value

  if( bearing < 0 )
    {
      _bearing->setValue("-");
    }
  else
    {

      int ival = bearing;

      if( _bearingMode == 0 )
        {
          // display the revers value
          ival > 180 ? ival -=180 : ival +=180;
        }

      QString val=QString("%1").arg( ival, 3, 10, QChar('0') );
      _bearing->setValue(val);
    }
}


/** This slot is called if the user presses the bearing display */
void MapView::slot_toggleBearing()
{
  // display invers bearing
  _bearing->setAutoFillBackground(true);
  _bearing->setBackgroundRole(QPalette::Window);
  _bearing->setPalette( QPalette(QColor(Qt::red)) );
  _bearingMode = 0;
  slot_Bearing( _lastBearing );

  _bearingTimer->start( 5000, true );
}

/**
 * Reset revers bearing after the timeout
 */
void MapView::slot_resetInversBearing()
{
  // display bearing in normal mode
  _bearing->setAutoFillBackground(true);
  _bearing->setBackgroundRole(QPalette::Window);
  _bearing->setPalette( QPalette( _bearingBGColor ));
  _bearingMode = 1;
  slot_Bearing( _lastBearing );
}

/** This slot is called by calculator if a new relative bearing has been calculated */
void MapView::slot_RelBearing(int relbearing)
{
  // qDebug("RelBearing: %d", relbearing );
  if (relbearing < -360)
    {
      // we need an icon when no relative bearing is available ?!
      // @JD: here it is
      QPixmap arrow = _arrows.copy( 24*60, 0, 60, 60 );
      _rel_bearing->setPixmap (arrow);
      return;
    }

  while (relbearing < 0)
    relbearing += 360;
  int rot=((relbearing+7)/15) % 24;  //we only want to rotate in steps of 15 degrees. Finer is not useful.
  QPixmap arrow = _arrows.copy( rot*60, 0, 60, 60 );

  _rel_bearing->setPixmap (arrow);
}


/** This slot is called by calculator if a new distance has been calculated. */
void MapView::slot_Distance(const Distance& distance)
{
  if (distance.getMeters()<0)
    {
      _distance->setValue("-");
    }
  else
    {
      _distance->setValue(distance.getText(false,1,uint (2)));
    }
}


/** This slot is called by calculator if a new ETA (Estimated Time to Arrival, or the time that is approximatly needed to arrive at the waypoint) has been calculated. */
void MapView::slot_ETA(const QTime& eta)
{
  if( eta.isNull() )
    {
      _eta->setValue("-");
    }
  else
    {
      QString txt;
      txt.sprintf("%d:%02d", eta.hour(), eta.minute());
      _eta->setValue(txt);
    }
}


/** This slot is called if a new positionfix has been established. */
void MapView::slot_Position(const QPoint& position, const int source)
{
  // this slot is called:
  // a) from GPS data coming in, not in manual mode
  // b) for Manual data if no GPS data coming in
  // c) for Manual data if in manual mode during GPS data coming in too

  // check airspace for newPosition from GPS and not for Manual
  // this covers a) and b)
  if(!calculator->isManualInFlight())
    {
      _theMap->checkAirspace (position);
    }

  // if in manual mode: show position in statusbar for the cross, not for the glider
  // this covers a) and b) or c) with source manual
  if(!calculator->isManualInFlight() ||
      calculator->isManualInFlight() && source == CuCalc::MAN)
    {
      _statusFiller->setText(WGSPoint::printPos(position.x(),true) +
                             " / " + WGSPoint::printPos(position.y(),false));
    }

  // remember for slot_settingschange
  lastPositionChangeSource = source;
}


/** This slot is called if the status of the GPS changes. */
void MapView::slotGPSStatus(GPSNMEA::connectedStatus status)
{
  if(status>GPSNMEA::notConnected)
    {
      _statusGps->setText(tr("GPS"));
    }
  else
    {
      _statusGps->setText(tr("Man"));
      _heading->setValue("-");
      _speed->setValue("-");
      _altitude->setValue("-");
    }

  _theMap->setShowGlider(status==GPSNMEA::validFix); //only show glider symbol if the GPS is connected.

  if(status==GPSNMEA::validFix)
    {
      cuApp->actionToggleManualInFlight->setEnabled(true);
    }
  else
    {
      if(!cuApp->actionToggleManualInFlight->isOn())
        {
          cuApp->actionToggleManualInFlight->setEnabled(false);
        }
    }
}


/** This slot is called if a log entry has been made. */
void MapView::slot_LogEntry()
{
  loggingTimer->start(750, true);
  slot_setFlightStatus();
}


/** This slot is being called if the altitude has changed. */
void MapView::slot_Altitude(const Altitude& /*alt*/)
{
  //    Altitude alti = calculator->getAltimeterAltitude();
  //    _altitude->setValue(alti.getText (false,0));
  QString altiText = calculator->getAltimeterAltitudeText();
  _altitude->setValue(altiText);
}


/** This slot is called when the "above glide path" value has changed */
void MapView::slot_GlidePath (const Altitude& above)
{
  _glidepath->setValue(above.getText(false,0));
}


/** This slot is called when the best speed value has changed */
void MapView::slot_bestSpeed (const Speed& speed)
{
  _speed2fly->setValue(speed.getHorizontalText(false,0));
}


/** This slot is called if a new McCready value has been set */
void MapView::slot_Mc (const Speed& mc)
{
  _mc->setValue (mc.getVerticalText(false,1));
}


/** This slot is called if a new variometer value has been set */
void MapView::slot_vario (const Speed& vario)
{
  _vario->setValue (vario.getVerticalText(false,1));
}


/** This slot is called if a new wind value has been set */
void MapView::slot_wind (Vector& wind)
{
  QString w;
  w.sprintf("%d°/%s",wind.getAngleDeg(),
            wind.getSpeed().getHorizontalText(false,0).latin1() );
  _wind->setValue (w);
  _theMap->slotNewWind();
}

/** This slot is called if a new current LD value has been set */
void MapView::slot_LD( const double& rLD, const double& cLD )
{
  // qDebug( "MapView::slot_LD: %f, %f", rLD, cLD );

  QString cld, rld;

  // format required LD
  if( rLD < 0.0 )
    {
      rld = " - ";
    }
  else if( rLD < 100.0 )
    {
      rld.sprintf("%2.1f", rLD);
    }
  else if( rLD < 1000.0 )
    {
      rld.sprintf("%3.0f", rLD);
    }
  else
    {
      rld.sprintf(">999");
    }

  // format current LD
  if( cLD < 0.0 )
    {
      cld = " - ";
    }
  else if( cLD < 100.0 )
    {
      cld.sprintf("%2.1f", cLD);
    }
  else
    {
      cld.sprintf(">99");
    }

  _ld->setValue( rld + "/" + cld );
}


/** This slot is called if the settings have been changed.
 * It refreshes all displayed data because units might have beeen changed.
 */
void MapView::slot_settingschange()
{
  // qDebug("MapView::slot_settingschange");
  slot_Altitude(calculator->getlastAltitude());
  slot_Position(calculator->getlastPosition(), lastPositionChangeSource);
  slot_Speed(calculator->getlastSpeed());
  slot_Distance(calculator->getlastDistance());
  slot_GlidePath(calculator->getlastGlidePath());
  slot_bestSpeed(calculator->getlastBestSpeed());
  slot_Mc(calculator->getlastMc());
  slot_Waypoint(calculator->getselectedWp());
}


/** This slot is called if the number of satelites changes. */
void MapView::slotSatConstellation()
{
  if (gps)
    {
      SatInfo info = gps->getLastSatInfo();
      QString msg = QString ("G-%1").arg(info.satCount);
      //    QString msg = QString ("%1").arg(info.satCount);
      _statusGps->setText (msg);
    }
}


/** This slot updates the FlightStatus statusbar-widget with the
    current logging and flightmode status */
void MapView::slot_setFlightStatus()
{
  QString status="<html>";
  //logging status

  IgcLogger* logger = IgcLogger::instance();

  if (logger->getisLogging())
    {                    //are we logging right now?
      if (loggingTimer->isActive())
        {                //  we are. Is the internal loggingTimer active (wich means there was an entry recently)?
          status+="<b>" + tr("L","Logging") + " </b>"; //    yes, so insert a bold faced L.
        }
      else
        {
          status+=tr("L","Logging") + " ";             //    no, so just insert an L
        }
    }
  else
    {
      if (logger->getisStandby())
        {
          status+=tr("Ls","LoggingStandby") + " ";
        }
      else
        {
          status+="";                                    //  we are not logging
        }
    }

  //flightmode status
  switch (calculator->currentFlightMode())
    {
    case CuCalc::unknown:
      status+=(tr("?","Unknown"));
      break;
    case CuCalc::standstill:
      status+=(tr("S","Standstill"));
      break;
    case CuCalc::cruising:
      status+=(tr("C","Cruising"));
      break;
    case CuCalc::circlingL:
      status+=(tr("L","Circling Left"));
      break;
    case CuCalc::circlingR:
      status+=(tr("R","Circling Right"));
      break;
    case CuCalc::wave:
      status+=(tr("W","Wave"));
      break;
    }

  //finalize
  status+="</html>";
  _statusFlightstatus->setText(status);

}

/** toggle between distance and eta widget on mouse signal */
void MapView::slot_toggleDistanceEta()
{
  if( _distance->isVisible() )
    {
      _distance->hide();
      _eta->show();
      emit toggleETACalculation( true );
    }
  else
    {
      _distance->show();
      _eta->hide();
      emit toggleETACalculation( false );
    }
}


/** toggle between wind, vario and LD widget on mouse signal */
void MapView::slot_toggleWindAndLD()
{
  if( _wind->isVisible() )
    {
      _wind->hide();
      _vario->hide();
      _ld->show();
      // switch on LD calculation in calculator
      emit toggleLDCalculation( true );
      // switch off vario calculation in calculator
      emit toggleVarioCalculation( false );
    }
  else
    {
      _vario->show();
      _wind->show();
      _ld->hide();
      // switch off LD calculation in calculator
      emit toggleLDCalculation( false );
      // switch on vario calculation in calculator
      emit toggleVarioCalculation( true );
    }
}

/** Opens the Altimeter settings dialog. */
void MapView::slot_AltimeterDialog()
{
  AltimeterModeDialog *amDlg = new AltimeterModeDialog( this );
  // delete widget during close event
  amDlg->setAttribute(Qt::WA_DeleteOnClose);
  
  connect( amDlg, SIGNAL( newAltimeterMode() ),
           this, SLOT( slot_newAltimeterMode() ) );
  connect( amDlg, SIGNAL( settingsChanged() ),
           calculator, SLOT( slot_settingschanged() ) );

  amDlg->work();
  amDlg->show();
}

/** Called, if altimeter mode has been changed */
void MapView::slot_newAltimeterMode()
{
  _altitude->setPreText(AltimeterModeDialog::mode2String());
}

/** Opens the Variometer settings dialog. */
void MapView::slot_VarioDialog()
{
  VarioModeDialog  *vmDlg = new VarioModeDialog( this );
  // delete widget during close event
  vmDlg->setAttribute(Qt::WA_DeleteOnClose);

  connect( vmDlg, SIGNAL( newVarioTime( int ) ),
           calculator->getVario(), SLOT( slotNewVarioTime( int ) ) );
  connect( vmDlg, SIGNAL( newTEKMode( bool ) ),
           calculator->getVario(), SLOT( slotNewTEKMode( bool ) ) );
  connect( vmDlg, SIGNAL( newTEKAdjust( int ) ),
           calculator->getVario(), SLOT( slotNewTEKAdjust( int ) ) );

  vmDlg->show();
}

/** Opens the GPS status dialog */
void MapView::slot_gpsStatusDialog()
{
  GpsStatusDialog *gpsDlg = new GpsStatusDialog( this );
  // delete widget during close event
  gpsDlg->setAttribute(Qt::WA_DeleteOnClose);
  gpsDlg->show();
}

/** Opens the inflight glider settings dialog. */
void MapView::slot_gliderFlightDialog()
{
  GliderFlightDialog *gfDlg = new GliderFlightDialog( this );
  // delete widget during close event
  gfDlg->setAttribute(Qt::WA_DeleteOnClose);

  connect( gfDlg, SIGNAL( settingsChanged() ),
           calculator, SLOT( slot_settingschanged() ) );
           
  gfDlg->load();
  gfDlg->show();
}

/**
 * @writes a message into the status bar for the given time. Default
 * is 5s. If time is zero, the message will never disappear.
 */
void MapView::message( const QString& message, int ms )
{
  _statusbar->showMessage( message, ms );
}
