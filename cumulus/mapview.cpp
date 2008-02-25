/***************************************************************************
                          mapview.cpp  -  This file is part of Cumulus.
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002 by Andre Somers, 2008 Axel Pauli
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


MapView::MapView(QWidget *parent, const char *name ) : QWidget(parent,name)
{
  resize(parent->size());

  qDebug( "MapView window size is %dx%d, width=%d, height=%d",
          parent->size().width(),
          parent->size().height(),
          parent->size().width(),
          parent->size().height() );

  cuApp = (CumulusApp *)parent;
  elements = new Q3PtrVector<QWidget>;
  elements->setAutoDelete(true);
  elements->fill(0, MVW_MAX_ELEMENT + (2 * MAX_LINES) + 1);  //we need some extra space for lines

#warning FIXME: Cumulus user layout is switched off at the moment
  // loadLayout("/home/andre/Applications/cumulus/default_cruising.cul");

  // Later on the Pretext can change depending on Mode
  GeneralConfig *conf = GeneralConfig::instance();
  _altimeterMode = conf->getAltimeterMode();

  // load pixmap of arrows for relative bearing
  _arrows = GeneralConfig::instance()->loadPixmap( "arrows20pix-15.png" );

  //make the main box layout
  QBoxLayout *topLayout = new QVBoxLayout( this );
  //layout for the Heading, Bearing, Distance and Speed boxes (top line in the main layout)
  QBoxLayout *HBDSLayout = new QHBoxLayout(topLayout);

  //add Heading widget
  _heading = new MapInfoBox( this );
  _heading->setValue("-");
  _heading->setPreText("Trk");
  HBDSLayout->addWidget( _heading, 18);
  QWhatsThis::add(_heading, tr("Heading"));

  //add Bearing widget
  _bearingMode = 1; // normal bearing display is default
  _lastBearing = -1;
  _bearingTimer = new QTimer(this);
  connect (_bearingTimer, SIGNAL(timeout()),
           this, SLOT(slot_resetInversBearing()));
  _bearing = new MapInfoBox( this  );
  _bearingBGColor = _bearing->backgroundColor();
  _bearing->setValue("-");
  _bearing->setPreText("Brg");
  HBDSLayout->addWidget( _bearing, 18);
  QWhatsThis::add(_bearing, tr("Bearing"));
  connect(_bearing, SIGNAL(mousePress()), this, SLOT(slot_toggleBearing()));

  // add relative bearing widget
  _rel_bearing = new QLabel(this,"");
  HBDSLayout->addWidget(_rel_bearing, 7);
  QWhatsThis::add(_rel_bearing, tr("Relative bearing"));

  //add Distance widget
  _distance = new MapInfoBox( this );
  _distance->setValue("-");
  _distance->setPreText("Dis");
  HBDSLayout->addWidget( _distance, 20);
  QWhatsThis::add(_distance, tr("Distance"));
  connect(_distance, SIGNAL(mousePress()), this, SLOT(slot_toggleDistanceEta()));

  //add ETA widget
  _eta = new MapInfoBox( this );
  _eta->setValue("-");
  _eta->setPreText( "Eta" );
  HBDSLayout->addWidget( _eta, 20 );
  QWhatsThis::add(_eta, tr("Estimated time of arrival"));
  _eta->hide();
  connect(_eta, SIGNAL(mousePress()), this, SLOT(slot_toggleDistanceEta()));

  //add Speed widget
  _speed = new MapInfoBox( this );
  _speed->setValue("-");
  _speed->setPreText("Gs");
  HBDSLayout->addWidget( _speed, 15);
  QWhatsThis::add(_speed, tr("Speed"));

  //separator
  QFrame * sep=new QFrame(this);
  sep->setFrameStyle(QFrame::Sunken);
  sep->setFrameShape(QFrame::HLine);
  topLayout->addWidget(sep);

  //layout for the Flighttime, Waypoint and Elevation boxes (second line in the main layout)
  QBoxLayout *FWELayout = new QHBoxLayout(topLayout);

  /*
    //add FlightTime widget
    _flighttime = new MapInfoBox( this );
    _flighttime->setPreText( "T: " );
    _flighttime->setValue("02:27");
    FWELayout->addWidget( _flighttime,10 );
  */
  
  //add Altitude widget
  _altitude = new MapInfoBox( this );
  _altitude->setValue("-");
  _altitude->setPreText(AltimeterModeDialog::mode2String()); // get current mode
  FWELayout->addWidget( _altitude, 20);
  QWhatsThis::add(_altitude, tr("Altitude"));
  connect(_altitude, SIGNAL(mousePress()),
          this, SLOT(slot_AltimeterDialog()));

  //add glide path widget
  _glidepath = new MapInfoBox( this );
  _glidepath->setValue("-");
  _glidepath->setPreText("Arv");
  FWELayout->addWidget( _glidepath,23);
  QWhatsThis::add(_glidepath, tr("Glide path"));
  connect(_glidepath, SIGNAL(mousePress()),
          (CumulusApp*)parent, SLOT(slotSwitchToReachListView()));

  //add Waypoint widget
  _waypoint = new MapInfoBox( this,10 );
  _waypoint->setValue("-");
  _waypoint->setPreText("To");
  FWELayout->addWidget( _waypoint, 50);
  QWhatsThis::add(_waypoint, tr("Waypoint"));
  connect(_waypoint, SIGNAL(mousePress()),
          (CumulusApp*)parent, SLOT(slotSwitchToWPListViewExt()));

  //add elevation widget
  /*
  _elevation = new MapInfoBox( this );
  _elevation->setValue("-");
  _elevation->setPreText("Elv");
  FWELayout->addWidget( _elevation,15 );
  QWhatsThis::add (_elevation, tr("Elevation"));
  */

  //separator
  sep=new QFrame(this);
  sep->setFrameStyle(QFrame::Sunken);
  sep->setFrameShape(QFrame::HLine);
  topLayout->addWidget(sep);

  //layout for the Vario, McCready, Speed2fly and Glide Path boxes (third line in the main layout)
  QBoxLayout *VMFGLayout = new QHBoxLayout(topLayout);

  //add vario widget
  _vario = new MapInfoBox(this);
  _vario->setValue("-");
  _vario->setPreText("Var");
  VMFGLayout->addWidget(_vario,22);
  QWhatsThis::add(_vario, tr("Variometer"));
  connect(_vario, SIGNAL(mousePress()),
          this, SLOT(slot_VarioDialog()));

  //add wind widget; this is head/tailwind, no direction given !
  _wind = new MapInfoBox(this);
  _wind->setValue("-");
  _wind->setPreText("Wd");
  VMFGLayout->addWidget(_wind,30);
  QWhatsThis::add(_wind, tr("Wind"));
  connect(_wind, SIGNAL(mousePress()), this, SLOT(slot_toggleWindAndLD()));

  //add LD widget
  _ld = new MapInfoBox( this );
  _ld->setValue("-/-");
  _ld->setPreText( "LD" );
  VMFGLayout->addWidget( _ld, 30 );
  QWhatsThis::add(_ld, tr("required and current LD"));
  _ld->hide();
  connect(_ld, SIGNAL(mousePress()), this, SLOT(slot_toggleWindAndLD()));

  //add McCready widget
  _mc = new MapInfoBox( this );
  _mc->setValue("0.0");
  _mc->setPreText("Mc");
  VMFGLayout->addWidget(_mc,20);
  QWhatsThis::add(_mc, tr("McCready"));
  connect(_mc, SIGNAL(mousePress()), this, SLOT(slot_gliderFlightDialog()));

  //add best speed widget
  _speed2fly = new MapInfoBox(this);
  _speed2fly->setValue("-");
  _speed2fly->setPreText("S2f");
  VMFGLayout->addWidget(_speed2fly,20);
  QWhatsThis::add(_speed2fly, tr("Best speed"));

  //separator
  sep=new QFrame(this);
  sep->setFrameStyle(QFrame::Sunken);
  sep->setFrameShape(QFrame::HLine);
  topLayout->addWidget(sep);

  QBoxLayout *MapLayout = new QHBoxLayout(topLayout);
  _theMap = new Map(this, "map");
  MapLayout->addSpacing(1);
  MapLayout->addWidget(_theMap, 10);
  MapLayout->addSpacing(1);
  _theMap->setMode(Map::headUp);
  topLayout->addSpacing(0);

  //Add statusbar widget
  _statusbar = new QStatusBar(this, "status");
  _statusbar->setSizeGripEnabled(false);
  _statusbar->setMaximumHeight(25);

  _statusGps = new CuLabel(tr("Man"),_statusbar);
  _statusGps->setFrameStyle(QFrame::Box|QFrame::Plain);
  _statusGps->setLineWidth(0);
  _statusGps->setAlignment(Qt::AlignCenter);
  _statusGps->setMargin(0);
  _statusGps->setMaximumSize(30,15);
  _statusGps->setMinimumSize(10,15);
  _statusbar->addWidget(_statusGps);
  connect(_statusGps, SIGNAL(mousePress()), this, SLOT(slot_gpsStatusDialog()));

  _statusFlightstatus = new QLabel("<qt>" + tr("?","Unknown") + "</qt>",_statusbar);
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
  _statusFiller->setMaximumHeight(15);
  _statusbar->addWidget(_statusFiller,10);

  _menuToggle = new CuLabel( tr("Menu"),_statusbar);
  _menuToggle->setFrameStyle(QFrame::Box|QFrame::Plain);
  _menuToggle->setLineWidth(0);
  _menuToggle->setAlignment(Qt::AlignCenter);
  _menuToggle->setMargin(0);
  _menuToggle->setMaximumSize(30,15);
  _menuToggle->setMaximumHeight(15);
  _statusbar->addWidget(_menuToggle);
  connect(_menuToggle, SIGNAL(mousePress()), (CumulusApp*)parent, SLOT(slotToggleMenu()));

  QFrame* filler = new QFrame(_statusbar);
  filler->setFrameStyle(QFrame::NoFrame);
  _statusbar->addWidget(filler);

  _statusbar->setMaximumHeight(20);
  //  _statusbar->setFrameStyle(QFrame::Raised);

  loggingTimer = new QTimer(this);
  connect (loggingTimer, SIGNAL(timeout()),
           this, SLOT(slot_setFlightStatus()));
  topLayout->addWidget(_statusbar);
  //Activate the layout
  topLayout->activate();

  lastPositionChangeSource = CuCalc::MAN;
}


MapView::~MapView()
{
  // qDebug("MapView::~MapView() destructor is called");
  delete elements;
}


/** called if heading has changed */
void MapView::slot_Heading(int head)
{
  //  QPoint curPos;

  //    _heading->setValue(QString("%1°").arg(head));
  _heading->setValue(QString("%1").arg(head));
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

      QString val=QString("%1").arg( ival );
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
  // qDebug("relbearing: %d", relbearing );
  if (relbearing < -360)
    {
      // we need an icon when no relative bearing is available ?!
      return;
    }

  while (relbearing < 0)
    relbearing += 360;
  int rot=((relbearing+7)/15) % 24;  //we only want to rotate in steps of 15 degrees. Finer is not usefull.
  QPixmap arrow = _arrows.copy( rot*20, 0, 20, 20 );

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
  QString status="<qt>";
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
  status+="</qt>";
  _statusFlightstatus->setText(status);

}


bool MapView::loadLayout(QString pathName)
{
  qint8 typeID, versionID;
  quint32 magic;
  QString name;

  // qDebug ("reading file %s", pathName.latin1() );

  //check if we have a filename
  if(pathName.isEmpty())
    // File does not exist
    return false;

  //check if we can open the file
  QFile f(pathName);
  if(!f.open(IO_ReadOnly))
    {
      // Can't read input data:
      // We need to output a warning...
      qWarning("Cumulus: Can't open layout file %s for reading!"
               "Arborting ...",
               pathName.latin1() );
      return false;
    }

  QDataStream in(&f);
  in.setVersion(2);

  //check magic key
  in >> magic;
  if( magic != KFLOG_FILE_MAGIC )
    { // wrong source file
      f.close();

      qWarning("Cumulus: wrong magic key %x read! Arborting ...", magic);
      return false;
    }

  //check filetype
  in >> typeID;
  if( typeID != FILE_TYPE )
    { // wrong source file
      f.close();

      qWarning("Cumulus: wrong file type key %x read! Arborting ...", typeID);
      return false;
    }

  //check fileversion
  in >> versionID;
  if( versionID != FILE_VERSION )
    { // wrong source file
      f.close();

      qWarning("Cumulus: wrong file version %x read! Arborting ...", versionID);
      return false;
    }

  //ok, it seems we have a valid layout file.

  //get the layout name
  ShortLoad(in, name);
  // qDebug("Loading layout '%s'", name.latin1());

  QBoxLayout * l = new QVBoxLayout(); //this is the "master layout"
  getElement(0, true); //reset linecounter
  loadLayoutElement(in, l); //start loading elements

  f.close();
  return true;
}


bool MapView::loadLayoutElement(QDataStream& in, QBoxLayout * l)
{
  Q_INT8 typeID, flags, eCnt;
  bool result=true;
  QBoxLayout * childLayout;

  in >> typeID;
  qDebug("   loading element type %d", typeID);

  switch (typeID)
    {
    case 0:
      return true;

    case MVW_HOR_LAYOUT:
      childLayout = new QHBoxLayout(l);
      in >> eCnt;
      for (int i=0; i < eCnt && result; i++)
        {
          result=result && loadLayoutElement(in, childLayout);
        }
      return result;

    case MVW_VER_LAYOUT:
      childLayout = new QVBoxLayout(l);
      in >> eCnt;
      for (int i=0; i < eCnt && result; i++)
        {
          result=result && loadLayoutElement(in, childLayout);
        }
      return result;

    case MVW_LINE:
      in >> flags;
      if (l->direction()==QBoxLayout::LeftToRight || l->direction()==QBoxLayout::RightToLeft)
        {
          //horizontal parent layout, so make a vertical line
        }
      else
        {
          //vertical parent layout, so make a horizontal line
        }
      return true;
    default:
      qDebug ("Element type not handled!");
      return false;

    }  //switch

}


QWidget * MapView::getElement(int typeID, bool resetLineCounter=false)
{
  static int hLineCnt;
  static int vLineCnt;

  QFrame * sep;
  Map * m;

  if (resetLineCounter)
    {
      hLineCnt=0;
      vLineCnt=0;
    }

  switch (typeID)
    {
    case 0:
      return 0;

    case 1: //special case, used for lines in horizontal layouts (vertical lines)
      vLineCnt++;
      if (!elements->at(VLINE_BASE + vLineCnt))
        {
          sep=new QFrame(this);
          sep->setFrameStyle(QFrame::Sunken);
          sep->setFrameShape(QFrame::VLine);
          elements->insert(VLINE_BASE + vLineCnt, sep);
        }
      return elements->at(VLINE_BASE + vLineCnt);

    case 2: //special case, used for lines in vertical layouts (horizontal lines)
      hLineCnt++;
      if (!elements->at(HLINE_BASE + hLineCnt))
        {
          sep=new QFrame(this);
          sep->setFrameStyle(QFrame::Sunken);
          sep->setFrameShape(QFrame::HLine);
          elements->insert(HLINE_BASE + hLineCnt,sep);
        }
      return elements->at(HLINE_BASE + hLineCnt);

    case MVW_MAP:
      if (!elements->at(MVW_MAP))
        {
          m=new Map(this, "mapwidget");
          elements->insert(MVW_MAP,m);
          //_theMap=m;
        }
      return elements->at(MVW_MAP);

    default:
      qDebug("Element type not handled!");
    } //switch

  return (QWidget *) 0;
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
