/***************************************************************************
                          mapview.cpp  -  This file is part of Cumulus.
                             -------------------
    begin                : Sun Jul 21 2002

    copyright            : (C) 2002      by Andre Somers
                               2008      by Josua Dietze
                               2008-2010 by Axel Pauli

    email                : axel@kflog.org

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

#include <QtGui>

#include "mapview.h"
#include "gpsnmea.h"
#include "mainwindow.h"
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
#include "filetools.h"
#include "altimetermodedialog.h"
#include "gliderflightdialog.h"
#include "gpsstatusdialog.h"
#include "variomodedialog.h"

MapView::MapView(QWidget *parent) : QWidget(parent)
{
  setObjectName("MapView");

  resize(parent->size());
  setContentsMargins(-9,-9,-9,-9);

  qDebug( "MapView window size is width=%d x height=%d",
          parent->size().width(),
          parent->size().height() );

  _mainWindow = (MainWindow *)parent;

  // Later on the Pretext can change depending on Mode
  GeneralConfig *conf = GeneralConfig::instance();
  _altimeterMode = conf->getAltimeterMode();

  // load pixmap of arrows for relative bearing
  _arrows = GeneralConfig::instance()->loadPixmap( "arrows60pix-15.png" );
  //make the main box layout
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing(0);

  //@JD: the new "sidebar" layout
  QBoxLayout *centerLayout = new QHBoxLayout;
  topLayout->addLayout(centerLayout);
  topLayout->setStretchFactor( centerLayout, 1 );
  centerLayout->setSpacing(0);

  QBoxLayout *sideLayout = new QVBoxLayout;
  centerLayout->addLayout(sideLayout);
  sideLayout->setSpacing(0);

  // three 'grouping' widgets with slightly differing
  // background color and fixed width (to avoid frequent resizing)

  //widget to group waypoint functions
  QWidget *wayBar = new QWidget( this );
  wayBar->setFixedWidth(216);
  wayBar->setContentsMargins(-9,-9,-9,-6);

  wayBar->setAutoFillBackground(true);
  wayBar->setBackgroundRole(QPalette::Window);
  wayBar->setPalette( QPalette(QColor(Qt::lightGray)) );

  // vertical layout for waypoint widgets
  QBoxLayout *wayLayout = new QVBoxLayout( wayBar );
  wayLayout->setSpacing(2);

  //add Waypoint widget (whole line)
  _waypoint = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _waypoint->setPreText("To");
  _waypoint->setValue("-");
  wayLayout->addWidget( _waypoint, 1 );
  connect(_waypoint, SIGNAL(mousePress()),
          (MainWindow*)parent, SLOT(slotSwitchToWPListViewExt()));

  //layout for Glide Path and Relative Bearing
  QBoxLayout *GRLayout = new QHBoxLayout;
  wayLayout->addLayout(GRLayout);
  GRLayout->setSpacing(2);

  //add Glide Path widget
  _glidepath = new MapInfoBox( this, conf->getMapFrameColor().name(), true, false, 42 );
  _glidepathBGColor = wayBar->palette().color(QPalette::Window);
  _glidepath->setValue("-");
  _glidepath->setPreText("Arr");
  _glidepath->setPreUnit( Altitude::getUnitText() );
  _glidepath->setFixedHeight(60);
  GRLayout->addWidget( _glidepath );

  connect(_glidepath, SIGNAL(mousePress()),
          (MainWindow*)parent, SLOT(slotSwitchToReachListView()));

  // add Relative Bearing widget
  QPixmap arrow = _arrows.copy( 24*60+3, 3, 54, 54 );
  _rel_bearing = new MapInfoBox( this, conf->getMapFrameColor().name(), arrow);
  _rel_bearing->setFixedSize(60,60);
  _rel_bearing->setToolTip( tr("Click here to save current position as waypoint") );
  GRLayout->addWidget(_rel_bearing);
//  _rel_bearing->setPixmap( arrow );

  connect(_rel_bearing, SIGNAL(mousePress()),
          (MainWindow*)parent, SLOT(slotRememberWaypoint()) );

  //layout for Distance/ETA and Bearing
  QBoxLayout *DEBLayout = new QHBoxLayout;
  wayLayout->addLayout(DEBLayout, 1);
  DEBLayout->setSpacing(2);

  //add Distance widget
  _distance = new MapInfoBox( this, conf->getMapFrameColor().name(), true );
  _distance->setPreText("Dis");
  _distance->setValue("-");
  _distance->setPreUnit( Distance::getUnitText() );
  DEBLayout->addWidget( _distance );
  connect(_distance, SIGNAL(mousePress()), this, SLOT(slot_toggleDistanceEta()));

  //add ETA widget
  _eta = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _eta->setVisible(false);
  _eta->setPreText( "Eta" );
  _eta->setValue("-");
  DEBLayout->addWidget( _eta );
  connect(_eta, SIGNAL(mousePress()), this, SLOT(slot_toggleDistanceEta()));

  //add Bearing widget
  _bearingMode = 1; // normal bearing display is default
  _lastBearing = -1;
  _bearingTimer = new QTimer(this);
  connect (_bearingTimer, SIGNAL(timeout()),
           this, SLOT(slot_resetInversBearing()));
  _bearing = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _bearingBGColor = wayBar->palette().color(QPalette::Window);
  _bearing->setValue("-");
  _bearing->setPreText("Brg");
  DEBLayout->addWidget( _bearing);
  connect(_bearing, SIGNAL(mousePress()), this, SLOT(slot_toggleBearing()));

  sideLayout->addWidget( wayBar, 3 );

  //widget to group common displays
  QWidget *commonBar = new QWidget( this );
  commonBar->setFixedWidth(216);
  commonBar->setContentsMargins( -9, -6, -9, -6);
  commonBar->setAutoFillBackground(true);
  commonBar->setBackgroundRole(QPalette::Window);
  commonBar->setPalette( QPalette(QColor(230, 230, 230)) );

  // vertical layout for common display widgets
  QBoxLayout *commonLayout = new QVBoxLayout( commonBar );
  commonLayout->setSpacing(2);

  //layout for Speed and Heading
  QBoxLayout *SHLayout = new QHBoxLayout;
  commonLayout->addLayout(SHLayout);
  SHLayout->setSpacing(2);

  //add Speed widget
  _speed = new MapInfoBox( this, "#c0c0c0" );
  //_speed = new MapInfoBox( this, QColor(Qt::darkGray).name() );

  _speed->setPreText("Gs");
  _speed->setValue("-");
  SHLayout->addWidget( _speed);

  //add Heading widget
  _heading = new MapInfoBox( this, "#c0c0c0" );
  //_heading = new MapInfoBox( this, QColor(Qt::darkGray).name() );
  _heading->setPreText("Trk");
  _heading->setValue("-");
  SHLayout->addWidget( _heading);

#ifdef FLARM
  connect(_heading, SIGNAL(mousePress()), this, SLOT(slot_OpenFlarmWidget()));
#endif

  //layout for Wind/LD
  QBoxLayout *WLLayout = new QHBoxLayout;
  commonLayout->addLayout(WLLayout, 1);

  //add Wind widget; this is head/tailwind, no direction given !
  _wind = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _wind->setPreText("Wd");
  _wind->setValue("-");
  WLLayout->addWidget(_wind );
  connect(_wind, SIGNAL(mousePress()), this, SLOT(slot_toggleWindAndLD()));

  //add LD widget
  _ld = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _ld->setVisible(false);
  _ld->setPreText( "LD" );
  _ld->setValue("-/-");
  WLLayout->addWidget( _ld );
  connect(_ld, SIGNAL(mousePress()), this, SLOT(slot_toggleWindAndLD()));

  //layout for Vario and Altitude
  QBoxLayout *VALayout = new QHBoxLayout;
  commonLayout->addLayout(VALayout);
  VALayout->setSpacing(2);

  // add altitude widget
  _altitude = new MapInfoBox( this, conf->getMapFrameColor().name(), true );
  _altitude->setPreText(AltimeterModeDialog::mode2String());
  _altitude->setPreUnit( Altitude::getUnitText() );
  _altitude->setValue("-");
  VALayout->addWidget( _altitude, 3 );
  connect(_altitude, SIGNAL(mousePress()), this, SLOT(slot_AltimeterDialog()));

  // add Vario widget
  _vario = new MapInfoBox( this, conf->getMapFrameColor().name(), false, true );
  _vario->setPreText("Var");
  _vario->setValue("-");
  VALayout->addWidget(_vario, 2 );
  connect(_vario, SIGNAL(mousePress()), this, SLOT(slot_VarioDialog()));

  sideLayout->addWidget( commonBar, 3 );

  //widget to group McCready functions
  QWidget *mcBar = new QWidget( this );
  mcBar->setFixedWidth(216);
  mcBar->setContentsMargins(-9,-6,-9,-9);

  mcBar->setAutoFillBackground(true);
  mcBar->setBackgroundRole(QPalette::Window);
  mcBar->setPalette( QPalette(QColor(Qt::lightGray)) );

  //layout for McCready and Best Speed
  QBoxLayout *MSLayout = new QHBoxLayout(mcBar);
  MSLayout->setSpacing(2);

  //add McCready widget
  _mc = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _mc->setPreText("Mc");
  calculator->glider() ? _mc->setValue("0.0") : _mc->setValue("-");
  MSLayout->addWidget( _mc );
  connect(_mc, SIGNAL(mousePress()), this, SLOT(slot_gliderFlightDialog()));

  //add Best Speed widget
  _speed2fly = new MapInfoBox( this, "#a6a6a6", true );
  _speed2fly->setPreText("S2f");
  _speed2fly->setValue("-");
  _speed2fly->setPreUnit( "<>" );
  MSLayout->addWidget( _speed2fly );
  connect(_speed2fly, SIGNAL(mousePress()), this, SLOT(slot_toggleMenu()));

  sideLayout->addWidget( mcBar, 1 );

  //--------------------------------------------------------------------
  //layout for the map
  QBoxLayout *MapLayout = new QHBoxLayout;
  centerLayout->addLayout(MapLayout);
  centerLayout->setStretchFactor( MapLayout, 1 );
  _theMap = new Map(this);
  MapLayout->addWidget(_theMap, 10);
  _theMap->setMode(Map::headUp);

#ifdef FLARM

  // Flarm widget with radar view
  _flarmWidget = new FlarmWidget(this);
  MapLayout->addWidget(_flarmWidget, 10);
  _flarmWidget->setVisible( false );
  connect( _flarmWidget, SIGNAL(closed()), this, SLOT(slot_OpenMapView()) );
#endif

  //--------------------------------------------------------------------
  // Status bar
  _statusbar = new QStatusBar(this);
  _statusbar->setObjectName("status");
  _statusbar->setSizeGripEnabled(false);

#ifndef MAEMO
  _statusbar->setFixedHeight(20);
#else
  _statusbar->setFixedHeight(25);
#endif

  QFont font = _statusbar->font();
  font.setBold(true);

#ifndef MAEMO
  font.setPixelSize(13);
#else
  font.setPixelSize(17);
#endif
  _statusbar->setFont(font);

  _statusGps = new CuLabel(tr("Man"),_statusbar);
  _statusGps->setLineWidth(0);
  _statusGps->setAlignment(Qt::AlignCenter);
  _statusGps->setMargin(0);
  _statusbar->addWidget(_statusGps);
  connect(_statusGps, SIGNAL(mousePress()), this, SLOT(slot_gpsStatusDialog()));

  _statusFlightstatus = new QLabel(tr("?","Unknown"),_statusbar);
  _statusFlightstatus->setLineWidth(0);
  _statusFlightstatus->setAlignment(Qt::AlignCenter);
  _statusFlightstatus->setMargin(0);
  _statusFlightstatus->setMinimumSize(_statusFlightstatus->fontMetrics().boundingRect(" L ? ").width(), 5);
  _statusbar->addWidget(_statusFlightstatus);

#ifdef FLARM

  _statusFlarm = new CuLabel( tr( "F" ), _statusbar );
  _statusFlarm->setLineWidth( 0 );
  _statusFlarm->setAlignment( Qt::AlignCenter );
  _statusFlarm->setMargin( 0 );
  _statusbar->addWidget( _statusFlarm );
  _statusFlarm->setVisible( false );

#endif

  _statusPosition = new QLabel(_statusbar);
  _statusPosition->setLineWidth(0);
  _statusPosition->setAlignment(Qt::AlignCenter);
  _statusPosition->setMargin(0);
  _statusbar->addWidget(_statusPosition);

  _statusGlider = new QLabel(_statusbar);
  _statusGlider->setLineWidth(0);
  _statusGlider->setAlignment(Qt::AlignCenter);
  _statusGlider->setMargin(0);
  _statusbar->addWidget(_statusGlider);

  _statusInfo = new QLabel(_statusbar);
  _statusInfo->setAlignment(Qt::AlignCenter);
  _statusInfo->setMargin(0);
  _statusbar->addWidget(_statusInfo, 1);

  QFrame* filler = new QFrame(_statusbar);
  filler->setLineWidth(0);
  _statusbar->addWidget(filler);
  topLayout->addWidget(_statusbar);

  lastPositionChangeSource = Calculator::MAN;
}

MapView::~MapView()
{
  // qDebug("MapView::~MapView() destructor is called");
}

void MapView::showEvent( QShowEvent* event )
{
  Q_UNUSED( event );

  // Used map info box widgets
  MapInfoBox *boxWidgets[14] = { _heading,
                                 _bearing,
                                 _rel_bearing,
                                 _distance,
                                 _speed,
                                 _speed2fly,
                                 _mc,
                                 _vario,
                                 _wind,
                                 _ld,
                                 _waypoint,
                                 _eta,
                                 _altitude,
                                 _glidepath };

  int gtWidth = 0;
  QFontMetrics fm( font() );

  // Determine the greatest pretext width of the used map info boxes.
  for( int i = 0; i < 14; i++ )
    {
      MapInfoBox *ptr = boxWidgets[i];

      if( ptr->isVisible() )
        {
          int w = fm.width( ptr->getPreText() );

          if( w > gtWidth )
            {
              gtWidth = w;
            }
        }
    }

  // qDebug() << "maxWidth=" << gtWidth;

  for( int i = 0; i < 14; i++ )
    {
      // Set uniform width for pretext of all map info boxes.
      MapInfoBox *ptr = boxWidgets[i];

      if( ptr->getPreText().isEmpty() )
        {
          // No text box, do nothing.
          continue;
        }

      ptr->getPreTextLabelWidget()->setFixedWidth( gtWidth );
    }
}

/** called if heading has changed */
void MapView::slot_Heading(int head)
{
  static QTime lastDisplay;

  // The display is updated every 1 seconds only.
  // That will reduce the X-Server load.
  if( lastDisplay.elapsed() < 750 )
    {
      return;
    }
  else
    {
      lastDisplay = QTime::currentTime();
    }

  _heading->setValue(QString("%1").arg( head, 3, 10, QChar('0') ));
  _theMap->setHeading(head);
}


/** Called if speed has changed */
void MapView::slot_Speed(const Speed& speed)
{
  if (speed.getMph() < 0 )
    {
      _speed->setValue("-");
    }
  else
    {
      static QTime lastDisplay;

      // The display is updated every 1 seconds only.
      // That will reduce the X-Server load.
      if( lastDisplay.elapsed() < 750 )
        {
          return;
        }
      else
        {
          lastDisplay = QTime::currentTime();
        }

      _speed->setValue(speed.getHorizontalText(false,0));
    }
}


/** called if the waypoint is changed in the calculator */
void MapView::slot_Waypoint(const wayPoint *wp)
{
  // qDebug("MapView::slot_Waypoint");
  QString dest = "";

  if (wp)
    {
      if (wp->taskPointIndex != -1)
        {
          QString idx = QString("%1 ").arg( wp->taskPointIndex );
          dest += tr("TP") + idx;
        }

      // @JD: suggestion: removal of spaces in wp display -> bigger font
      for (int i = 0; i < wp->name.size(); i++)
        {
          if (wp->name[i] != QChar(' '))
            {
              dest += wp->name[i];
            }

          if (dest.size() == 8)
            {
              break;
            }
        }

      if( wp->name != tr("Home") )
        {
          dest += QString("(%1)").arg(Altitude::getText(wp->elevation, false, 0));
        }

      _waypoint->setValue(dest);
    }
  else
    {
      // @AP: No waypoint is selected, reset waypoint and glidepath display
      _waypoint->setValue("");
      _glidepath->setValue("-");
      _glidepath->setAutoFillBackground(true);
      _glidepath->setBackgroundRole(QPalette::Window);
      _glidepath->setPalette( QPalette(_glidepathBGColor) );
      // @JD: reset distance too
      _distance->setValue("-");
      QPixmap arrow = _arrows.copy(24 * 60 + 3, 3, 54, 54);
      _rel_bearing->setPixmap(arrow);
      // qDebug("Rel. bearing icon reset" );
    }

  _theMap->scheduleRedraw(Map::informationLayer); // this is not really helpful -> it is: the bearingline won't change otherwise!
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
      static QTime lastDisplay;

      // The display is updated every 1 seconds only.
      // That will reduce the X-Server load.
      if( lastDisplay.elapsed() < 750 )
        {
          return;
        }
      else
        {
          lastDisplay = QTime::currentTime();
        }

      int ival = bearing;

      if( _bearingMode == 0 )
        {
          // display the reverse value
          ival > 180 ? ival -=180 : ival +=180;
        }

      QString val=QString("%1").arg( ival, 3, 10, QChar('0') );
      _bearing->setValue(val);
    }
}


/** This slot is called if the user presses the bearing display */
void MapView::slot_toggleBearing()
{
  // display inverse bearing
  _bearing->setAutoFillBackground(true);
  _bearing->setBackgroundRole(QPalette::Window);
  _bearing->setPalette( QPalette(QColor(Qt::red)) );
  _bearingMode = 0;
  slot_Bearing( _lastBearing );

  _bearingTimer->setSingleShot(true);
  _bearingTimer->start(5000);
}

/** Called to toggle the menu of the main window. */
void MapView::slot_toggleMenu()
{
  if( _theMap->isVisible() == true )
    {
      // Toggle menu only, if map widget is visible
      emit toggleMenu();
    }
}

/**
 * Reset reverse bearing after the timeout
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
      QPixmap arrow = _arrows.copy( 24*60+3, 3, 54, 54 );
      _rel_bearing->setPixmap (arrow);
      return;
    }

  static QTime lastDisplay;

  // The display is updated every 1 seconds only.
  // That will reduce the X-Server load.
  if( lastDisplay.elapsed() < 750 )
    {
      return;
    }
  else
    {
      lastDisplay = QTime::currentTime();
    }

  while (relbearing < 0)
    {
      relbearing += 360;
    }

  //we only want to rotate in steps of 15 degrees. Finer is not useful.
  int rot=((relbearing+7)/15) % 24;
  QPixmap arrow = _arrows.copy( rot*60+3, 3, 54, 54 );
  _rel_bearing->setPixmap (arrow);
}


/** This slot is called by calculator if a new distance has been calculated. */
void MapView::slot_Distance(const Distance& distance)
{
  if (distance.getMeters() < 0 )
    {
      _distance->setValue("-");
    }
  else
    {
      static QTime lastDisplay;

      // The display is updated every 1 seconds only.
      // That will reduce the X-Server load.
      if( lastDisplay.elapsed() < 750 )
        {
          return;
        }
      else
        {
          lastDisplay = QTime::currentTime();
        }

      _distance->setValue(distance.getText(false, 1, (uint) 2 ) );
    }
}


/**
 * This slot is called by the calculator if a new ETA (Estimated Time to Arrival
 * or the time that is approximately needed to arrive at the waypoint) has been
 * calculated.
 */
void MapView::slot_ETA(const QTime& eta)
{
  if( eta.isNull() )
    {
      _eta->setValue("-");
    }
  else
    {
      static QTime lastDisplay;

      // The display is updated every 1 seconds only.
      // That will reduce the X-Server load.
      if( lastDisplay.elapsed() < 750 )
        {
          return;
        }
      else
        {
          lastDisplay = QTime::currentTime();
        }

      QString txt = QString("%1:%2").arg( eta.hour() )
                                    .arg( eta.minute(), 2, 10, QChar('0') );
      _eta->setValue(txt);
    }
}


/** This slot is called if a new position fix has been established. */
void MapView::slot_Position(const QPoint& position, const int source)
{
  static QTime lastDisplay;

  // remember for slot_settingsChange
  lastPositionChangeSource = source;

  // this slot is called:
  // a) from GPS data coming in, not in manual mode
  // b) for Manual data if no GPS data coming in
  // c) for Manual data if in manual mode during GPS data coming in too

  // check airspace for newPosition from GPS and not for Manual
  // this covers a) and b)
  if( ! calculator->isManualInFlight() )
    {
      _theMap->checkAirspace (position);
    }

  if( GpsNmea::gps->getGpsStatus() == GpsNmea::validFix &&
      source != Calculator::MAN )
    {
      // If we have a valid fix the display is updated every 5 seconds only.
      // That will reduce the X-Server load.
      if( lastDisplay.elapsed() < 5000 )
        {
          return;
        }

      lastDisplay = QTime::currentTime();
    }

  // if in manual mode: show position in status bar for the cross, not for the glider
  // this covers a) and b) or c) with source manual
  if( !calculator->isManualInFlight() ||
      (calculator->isManualInFlight() && (source == Calculator::MAN)))
    {
      _statusPosition->setText(" " + WGSPoint::printPos(position.x(),true) +
                             " / " + WGSPoint::printPos(position.y(),false) + " ");
    }
}


/** This slot is called if the status of the GPS changes. */
void MapView::slot_GPSStatus(GpsNmea::GpsStatus status)
{
  switch ( status )
    {
    case GpsNmea::validFix:
      slot_info( tr( "GPS new fix" ) );
      break;
    case GpsNmea::noFix:
      slot_info(tr( "GPS fix lost" ) );
      break;
    default:
      slot_info( tr( "GPS lost" ) );
    }

  if(status>GpsNmea::notConnected)
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

  // Only show glider symbol if the GPS is connected.
  _theMap->setShowGlider( status == GpsNmea::validFix );
}


/** This slot is called if a log entry has been made by the logger. */
void MapView::slot_LogEntry()
{
  slot_setLoggerStatus();
}


/** This slot is being called if the altitude has been changed. */
void MapView::slot_Altitude(const Altitude& altitude )
{
  _altitude->setValue( altitude.getText( false, 0 ) );
}


/** This slot is called when the "above glide path" value has changed */
void MapView::slot_GlidePath (const Altitude& above)
{
  static QColor lastColor = _glidepathBGColor;

  _glidepath->setValue(above.getText(false,0));

  if( above.getMeters() < 0.0 && lastColor != QColor(Qt::red) )
    {
      // display red background if arrival is under zero
      _glidepath->setAutoFillBackground(true);
      _glidepath->setBackgroundRole(QPalette::Window);
      _glidepath->setPalette( QPalette(QColor(Qt::red)) );
      lastColor = QColor(Qt::red);
    }
  else if( above.getMeters() >= 0.0 && lastColor != _glidepathBGColor )
    {
      _glidepath->setAutoFillBackground(true);
      _glidepath->setBackgroundRole(QPalette::Window);
      _glidepath->setPalette( QPalette(_glidepathBGColor) );
      lastColor = _glidepathBGColor;
    }
}

/** This slot is called when the best speed value has changed */
void MapView::slot_bestSpeed (const Speed& speed)
{
  if( speed.isValid() )
    {
      _speed2fly->setValue(speed.getHorizontalText(false, 0));
    }
  else
    {
      _speed2fly->setValue("-");
    }
}


/** This slot is called if a new McCready value has been set */
void MapView::slot_Mc (const Speed& mc)
{
  if( mc.isValid() )
    {
      _mc->setValue(mc.getVerticalText(false, 1));
    }
  else
    {
      _mc->setValue("-");
    }
}


/** This slot is called if a new variometer value has been set */
void MapView::slot_Vario (const Speed& vario)
{
  static QTime lastDisplay;

  // The display is updated every 1 seconds only.
  // That will reduce the X-Server load.
  if( lastDisplay.elapsed() < 750 )
    {
      return;
    }
  else
    {
      lastDisplay = QTime::currentTime();
    }

  QString varValue;

  // if altitude has more than 3 digits, vario is rounded to one
  // digit. Normal vario display is e.g. 1.1 (2 digits plus decimal
  // point)
  if( _altitude->getValue().size() > 4 )
    {
      varValue = vario.getVerticalText(false, 0);
    }
  else
    {
      varValue = vario.getVerticalText(false, 1);
    }

  _vario->setValue( varValue );
}


/** This slot is called if a new wind value has been set */
void MapView::slot_Wind(Vector& wind)
{
  QString w;

  w = QString("%1/" + wind.getSpeed().getWindText(false, 0) ).arg( wind.getAngleDeg() );
  _wind->setValue (w);
  _theMap->slotNewWind();
}

/** This slot is called if a new current LD value has been set */
void MapView::slot_LD( const double& rLD, const double& cLD )
{
  static QTime lastDisplay;

  // The display is updated every 1 seconds only.
  // That will reduce the X-Server load.
  if( lastDisplay.elapsed() < 750 )
    {
      return;
    }
  else
    {
      lastDisplay = QTime::currentTime();
    }

  // qDebug( "MapView::slot_LD: %f, %f", rLD, cLD );

  QString cld, rld;

  // format required LD
  if( rLD < 0.0 )
    {
      rld = " - ";
    }
  else if( rLD < 100.0 )
    {
      rld = QString("%1").arg( rLD, 2, 'f', 1 );
    }
  else if( rLD < 1000.0 )
    {
      rld = QString("%1").arg( rLD, 3, 'f', 0 );
    }
  else
    {
      rld = ">999";
    }

  // format current LD
  if( cLD < 0.0 )
    {
      cld = " - ";
    }
  else if( cLD < 100.0 )
    {
      cld = QString("%1").arg( cLD, 2, 'f', 1 );
    }
  else
    {
      cld = ">99";
    }

  _ld->setValue( rld + "/" + cld );
}


/**
 * This slot is called if the glider selection has been modified
 */
void MapView::slot_glider( const QString& glider )
{
  _statusGlider->setText( glider );

  if( glider.isNull() )
    {
      // reset arrival display because glider has been removed
      _glidepath->setValue("-");
    }
}

/**
 * This slot is called if a info message shall be displayed
 */
void MapView::slot_info( const QString& info )
{
  _statusInfo->setText( info );
}


/** This slot is called if the settings have been changed.
 * It refreshes all displayed data because units might have been changed.
 */
void MapView::slot_settingsChange()
{
  // qDebug("MapView::slot_settingsChange");
  slot_newAltimeterMode();
  slot_Position(calculator->getlastPosition(), lastPositionChangeSource);
  slot_Speed(calculator->getLastSpeed());
  slot_Distance(calculator->getlastDistance());
  slot_GlidePath(calculator->getlastGlidePath());
  slot_bestSpeed(calculator->getlastBestSpeed());
  slot_Mc(calculator->getlastMc());
  slot_Waypoint(calculator->getselectedWp());

  _glidepath->setPreUnit( Altitude::getUnitText() );
  _distance->setPreUnit( Distance::getUnitText() );
}


/** This slot is called if the number of satellites changes. */
void MapView::slot_SatCount( SatInfo& satInfo )
{
  // Display the number of satellites in use.
  QString msg = QString ("G-%1").arg(satInfo.satsInUse);
  _statusGps->setText (msg);
}

#ifdef FLARM

/** This slot is called if the number of received Flarms has been changed. */
void MapView::slot_FlarmCount( int flarmCount )
{
  // Display the number of Flarms received.
  if( flarmCount == -1 )
    {
      // hide Flarm display
      _statusFlarm->setVisible( false );
      return;
    }

  QString msg = QString ("F-%1").arg( flarmCount );
  _statusFlarm->setText (msg);

  if( _statusFlarm->isVisible() == false )
    {
      _statusFlarm->setVisible( true );
    }
}

/** Opens the Flarm widget. */
void MapView::slot_OpenFlarmWidget()
{
  if( _theMap->isVisible() == true )
    {
      extern MainWindow  *_globalMainWindow;
      _globalMainWindow->setView( MainWindow::flarmView );
      _theMap->setVisible( false );
      _flarmWidget->setVisible( true );
    }
}

/** Opens the Map view. */
void MapView::slot_OpenMapView()
{
  if( _theMap->isVisible() == false )
    {
      extern MainWindow  *_globalMainWindow;
      _globalMainWindow->setView( MainWindow::mapView );
      _theMap->setVisible( true );
      _flarmWidget->setVisible( false );
    }
}

#endif

/** This slot updates the FlightStatus status bar-widget with the
    current logging and flight mode status */
void MapView::slot_setFlightStatus()
{
  QString status="";

  // flight mode status
  switch (calculator->currentFlightMode())
    {
    case Calculator::unknown:
      status += (tr("?","Unknown"));
      break;
    case Calculator::standstill:
      status += (tr("S","Standstill"));
      break;
    case Calculator::cruising:
      status += (tr("C","Cruising"));
      break;
    case Calculator::circlingL:
      status += (tr("L","Circling Left"));
      break;
    case Calculator::circlingR:
      status += (tr("R","Circling Right"));
      break;
    case Calculator::wave:
      status += (tr("W","Wave"));
      break;
    }

  _statusFlightstatus->setText(status);
}

/*
 * Sets the logger status in the status bar.
 */
void MapView::slot_setLoggerStatus()
{
  QString status = "";

  IgcLogger* logger = IgcLogger::instance();

  if( ! logger )
    {
      return;
    }

  if( logger->getIsLogging() )
    {
      // are we logging right now?
      _statusFlightstatus->setAutoFillBackground( true );
      _statusFlightstatus->setBackgroundRole( QPalette::Window );
      _statusFlightstatus->setPalette( QPalette( QColor( Qt::green ) ) );

      status += tr( "L", "Logging" ) + " "; // so just insert an L
    }
  else
    {
      if( logger->getIsStandby() )
        {
          _statusFlightstatus->setAutoFillBackground( true );
          _statusFlightstatus->setBackgroundRole( QPalette::Window );
          _statusFlightstatus->setPalette( QPalette( QColor( Qt::yellow ) ) );

          status += tr( "Ls", "LoggingStandby" ) + " ";
        }
      else
        {
          _statusFlightstatus->setAutoFillBackground( true );
          _statusFlightstatus->setBackgroundRole( QPalette::Window );
          _statusFlightstatus->setPalette( QPalette( _bearingBGColor ) );

          status += ""; //  we are not logging
        }
    }
}

/** toggle between distance and eta widget on mouse signal */
void MapView::slot_toggleDistanceEta()
{
  if( _distance->isVisible() )
    {
      _distance->setVisible(false);
      _eta->setVisible(true);
      _eta->setValue( _eta->getValue() );
      emit toggleETACalculation( true );
    }
  else
    {
      _eta->setVisible(false);
      _distance->setVisible(true);
      _distance->setValue( _distance->getValue() );
      emit toggleETACalculation( false );
    }
}

/** toggle between wind, vario and LD widget on mouse signal */
void MapView::slot_toggleWindAndLD()
{
  if( _wind->isVisible() )
    {
      _wind->setVisible(false);
      _ld->setVisible(true);
      _ld->setValue( _ld->getValue() );
      // switch on LD calculation in calculator
      emit toggleLDCalculation( true );
    }
  else
    {
      _ld->setVisible(false);
      _wind->setVisible(true);
      _wind->setValue( _wind->getValue() );
      // switch off LD calculation in calculator
      emit toggleLDCalculation( false );
    }
}

/** Opens the Altimeter settings dialog. */
void MapView::slot_AltimeterDialog()
{
  AltimeterModeDialog *amDlg = new AltimeterModeDialog( this );

  connect( amDlg, SIGNAL( newAltimeterMode() ),
           this, SLOT( slot_newAltimeterMode() ) );
  connect( amDlg, SIGNAL( newAltimeterSettings() ),
           GpsNmea::gps, SLOT( slot_reset() ) );

  amDlg->setVisible(true);
}

/** Called, if altimeter mode has been changed. */
void MapView::slot_newAltimeterMode()
{
  _altitude->setPreText( AltimeterModeDialog::mode2String() );
  _altitude->setPreUnit( Altitude::getUnitText() );
  _glidepath->setPreUnit( Altitude::getUnitText() );

  // Mode change needs always a altitude display update.
  slot_Altitude( calculator->getlastAltitude() );
}

/** Opens the Variometer settings dialog. */
void MapView::slot_VarioDialog()
{
  if( VarioModeDialog::getNrOfInstances() > 0 )
    {
      // Sometimes the mouse event is delayed under Maemo, which triggers this
      // method. In such a case multiple dialogs are opened. This check shall
      // prevent that.
      return;
    }

  VarioModeDialog *vmDlg = new VarioModeDialog( this );

  connect( vmDlg, SIGNAL( newVarioTime( int ) ),
           calculator->getVario(), SLOT( slotNewVarioTime( int ) ) );
  connect( vmDlg, SIGNAL( newTEKMode( bool ) ),
           calculator->getVario(), SLOT( slotNewTEKMode( bool ) ) );
  connect( vmDlg, SIGNAL( newTEKAdjust( int ) ),
           calculator->getVario(), SLOT( slotNewTEKAdjust( int ) ) );

  vmDlg->setVisible(true);
}

/** Opens the GPS status dialog */
void MapView::slot_gpsStatusDialog()
{
  GpsStatusDialog *gpsDlg = new GpsStatusDialog( this );
  // delete widget during close event
  gpsDlg->setVisible(true);

#ifdef MAEMO
  gpsDlg->setWindowState( Qt::WindowFullScreen );
#endif

}

/** Opens the inflight glider settings dialog. */
void MapView::slot_gliderFlightDialog()
{
  if( ! calculator->glider() || GliderFlightDialog::getNrOfInstances() > 0 )
    {
      // Open dialog only, if a glider is selected.
      // Sometimes the mouse event is delayed under Maemo, which triggers this
      // method. In such a case multiple dialogs are opened. This check shall
      // prevent that.
      return;
    }

  GliderFlightDialog *gfDlg = new GliderFlightDialog( this );

  connect( gfDlg, SIGNAL(newMc(const Speed&)),
           calculator, SLOT(slot_Mc(const Speed&)) );

  connect( gfDlg, SIGNAL(newWaterAndBugs(const int, const int)),
           calculator, SLOT(slot_WaterAndBugs(const int, const int)) );

  gfDlg->setVisible(true);
}

/**
 * @writes a message into the status bar for the given time. Default
 * is 5s. If time is zero, the message will never disappear.
 */
void MapView::message( const QString& message, int ms )
{
  _statusbar->showMessage( message, ms );
}
