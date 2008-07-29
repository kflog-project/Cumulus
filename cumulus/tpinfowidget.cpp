/***********************************************************************
**
**   tpinfowidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2007, 2008 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
************************************************************************
**
**   This class is part of Cumulus. It provides a widget to display
**   information like task point switch, distance to next, duration to
**   next, ETA, when a task point has been reached. Widget will be
**   closed automatically after a configureable time period, if user do
**   nothing. The user can stop the automatic close.
**   
************************************************************************/

#include <cmath>
#include <time.h>

#include <QApplication>
#include <QDateTime>
#include <QFont>
#include <QRegExp>
#include <QShortcut>

#include "tpinfowidget.h"
#include "generalconfig.h"
#include "flighttask.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "cucalc.h"
#include "reachablelist.h"
#include "gpsnmea.h"
#include "sonne.h"

extern MapConfig    *_globalMapConfig;
extern MapContents  *_globalMapContents;
extern CuCalc       *calculator;

TPInfoWidget::TPInfoWidget( QWidget *parent ) :
  QWidget( parent )
{
  setObjectName("TPInfoWidget");
  setAttribute( Qt::WA_DeleteOnClose );
  this->parent = parent;
  resize( parent->size() );

  QFont bfont( "Helvetica", 14, QFont::Bold  );
  QFont font( "Helvetica", 14 );

  QBoxLayout *topLayout = new QVBoxLayout( this );
  text = new QTextEdit(this);
  text->setReadOnly(true);
  text->setLineWrapMode(QTextEdit::WidgetWidth);

  int fontSize = this->font().pointSize();

  // qDebug("fontSize=%d", fontSize);

  if( fontSize < 14 )
    {
      text->setFont( font );
    }

  topLayout->addWidget(text, 5 );
  
  buttonrow = new QHBoxLayout;
  topLayout->addLayout(buttonrow);
  
  cmdClose = new QPushButton(tr("Close"), this);
  cmdClose->setFont(bfont);
  buttonrow->addWidget(cmdClose);
  connect(cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()));

  cmdKeep = new QPushButton(tr("Keep"), this);
  cmdKeep->setFont(bfont);
  buttonrow->addWidget(cmdKeep);
  connect(cmdKeep, SIGNAL(clicked()), this, SLOT(slot_KeepOpen()));
  
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(slot_Timeout()));

  // activate keyboard shotcuts ok for close of widget
  QShortcut* scClose = new QShortcut( this );

  scClose->setKey( Qt::Key_Return );

  connect( scClose, SIGNAL(activated()), this, SLOT( slot_Close() ));
}


TPInfoWidget::~TPInfoWidget()
{
  // qDebug( "TPInfoWidget::~TPInfoWidget(): is called" );
}

/**
 * This slot is called, if the user presses the close button or the
 * timeout timer has expired. Widget will be closed and destroyed
 */
void TPInfoWidget::slot_Close()
{
  hide();
  text->clearFocus();
  emit close();
  QWidget::close();
}

/** This slot get called on the timer timeout. If timer expires the
    widget will be closed automatically. */
void TPInfoWidget::slot_Timeout()
{
  if( --_timerCount == 0 )
    {
      timer->stop();
      slot_Close();
    }
  else
    {
      QString txt = tr("Close (%1)").arg(_timerCount);
      cmdClose->setText(txt);
    }
}


/**
 * Shows task point info to the user. Close timer is activated per
 * default. Use false to deactivate it.
 */
void TPInfoWidget::showTP( bool automaticClose )
{
  // @AP: load the time from user configuration
  GeneralConfig *conf = GeneralConfig::instance();
  _timerCount = conf->getInfoDisplayTime();
  
  if( _timerCount > 0 && automaticClose )
    {
      timer->start(1000);
      QString txt = tr("Close (%1)").arg(_timerCount);
      cmdClose->setText(txt);
      cmdKeep->show();
    }
  else
    {
      // Timer is set to zero, no automatic window close
      cmdClose->setText(tr("Close"));
      cmdKeep->hide();
    }

  text->setFocus();
  QWidget::show();
}

/**
 * Called, if the widget will be shown.
 */
void TPInfoWidget::showEvent(QShowEvent *)
{
  // qDebug("TPInfoWidget::showEvent(): name=%s", name());
  // set focus to text widget
  text->setFocus();
}

/**
 * This method fills the widget with the taskpoint switch info. The
 * info is taken from the current active fligh task.
 *
 * currentTpIndex: index of current taskpoint in flight task
 * dist2Next: distance to next taskpoint in kilometers
 *
 */
void TPInfoWidget::prepareSwitchText( const int currentTpIndex,
				      const double dist2Next )
{
  FlightTask *task = _globalMapContents->getCurrentTask();

  if( ! task )
    {
      // no flight task is active
      return;
    }

  QList<wayPoint*> wpList = task->getWPList();

  if( wpList.count() < 4 || wpList.count() <= currentTpIndex+1 )
    {
      // to less waypoints in list
      return;
    }

  wayPoint *currentTP = wpList.at( currentTpIndex );
  wayPoint *nextTP    = wpList.at( currentTpIndex+1 );

  QString display;
  QString no1, no2;
  QString currentTpDes = currentTP->description;
  QString nextTpDes    = nextTP->description;

  currentTpDes.replace(  QRegExp(" "), "&nbsp;" );
  nextTpDes.replace(  QRegExp(" "), "&nbsp;" );

  no1.sprintf( "%02d", currentTP->taskPointIndex );
  no2.sprintf( "%02d", nextTP->taskPointIndex );

  display += "<html><big><center><b>" +
    tr("Taskpoint switch") + " " + no1 + "->" + no2 +
    "</b></center><p>";

  display += "<table cellpadding=5 align=center border=1><tr><th colspan=2 align=left>" +
    tr("Reached target") + " " + no1 + "</th>" +
    "<th colspan=2 align=left>" + currentTP->name + "&nbsp;(" + currentTpDes + ")" +
    "</th></tr>";
  
  display += "<tr><th colspan=2 align=\"left\">" +
    tr("Next target") + " " + no2 + "</th>" +
    "<th colspan=2 align=left>" + nextTP->name + "&nbsp;(" + nextTpDes + ")" +
    "</th></tr>";
  
  // to avoid wraping in the table we have to code spaces as forced spaces in html
  QString distance = Distance::getText(dist2Next * 1000., true, 1);
  distance.replace(  QRegExp(" "), "&nbsp;" );

  display += "<tr><td>&nbsp;&nbsp;" + tr("Distance") + "</td><td align=\"left\"><b>" +
    distance + "</b></td>";

  Altitude arrivalAlt;
  Speed bestSpeed;
  reachable reach = no;

  // calculate Bearing
  int bearing= int( rint(getBearingWgs( calculator->getlastPosition(),
                                        nextTP->origP ) * 180/M_PI) );
  // glide path
  calculator->glidePath( bearing, dist2Next,
                         nextTP->elevation,
                         arrivalAlt, bestSpeed );

  // fetch minimal arrival altitude
  Altitude minAlt( GeneralConfig::instance()->getSafetyAltitude().getMeters() );

  if( arrivalAlt.isValid() )
    {
      if( arrivalAlt >= minAlt )
        {
          display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b>+" +
            arrivalAlt.getText(true,0) + "</b></td></tr>";
        }
      else if( arrivalAlt.getMeters() > 0.0 ) 
       {
         display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><font color=\"#FF0000\"><b>" +
           arrivalAlt.getText(true,0) + "</font></b></td></tr>";
       }
     else
       {
         display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b><font color=\"#FF00FF\">" +
           arrivalAlt.getText(true,0) + "</font></b></td></tr>";
        }
    }
  else
    {
      display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b>" +
        tr("unknown") + "</b></td></tr>";
    }
  
  double gs = calculator->getlastSpeed().getMps(); // get last speed

  extern GPSNMEA *gps;

  if( gs == 0.0 && ! gps->getConnected() )
    {
      // set a pseudo speed of 100km/h in manual mode for testing
      gs = 100000.0/3600.0; // m/s
    }

  QString speed = Speed(gs).getHorizontalText( true, 1 );
  speed.replace(  QRegExp(" "), "&nbsp;" );

  display += "<tr><td>&nbsp;&nbsp;" + tr("Vg") + "</td><td align=\"left\"><b>" +
    speed + "</b></td>";

  // If speed is to less we do not display any time values
  if( gs > 0.3 )
    {
      int time2Next = (int) rint(dist2Next * 1000. / gs);

      QTime qtime(0,0);
      qtime = qtime.addSecs(time2Next);
      
      display += "<td>&nbsp;&nbsp;" + tr("Duration") + "</td><td align=\"left\"><b>" +
        qtime.toString() + "</b></td></tr>";
      
      // ETA as UTC must be done with c-system functions. This QT
      // release does not provide this :((
      time_t seconds = time(0);
      seconds += time2Next;
      
      struct tm *gmt = gmtime( &seconds );
      
      QString eta;
      eta.sprintf( "%02d:%02d:%02d UTC", gmt->tm_hour, gmt->tm_min, gmt->tm_sec );
      
      display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
        eta + "</b></td>";
    }
  else
    {
      display += "<td>&nbsp;&nbsp;" + tr("Duration") + "</td><td align=\"left\"><b>" +
        tr("unknown") + "</b></td></tr>";

      display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
        tr("unknown") + "</b></td>";
    }

  // calculate sunset for the destination
  QString sr, ss;
  QDate date = QDate::currentDate();
  
  bool res = Sonne::sonneAufUnter( sr, ss, date, nextTP->origP, 0 );

  if( res )
    {
      // In some areas no results available. In this case we skip
      // this output.
      display += "<td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
        ss + " UTC </b></td></tr>";
    }
  else
    {
      display += "<td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
        tr("unknown")+ "</b></td></tr>";      
    }

  //-----------------------------------------------------------------------
  // Show data of landing (final) target, if it is not already
  // included
  if( ( nextTP->taskPointType != wayPoint::Landing &&
        nextTP->taskPointType != wayPoint::End ) ||
      ( nextTP->taskPointType == wayPoint::End &&
        nextTP->origP != wpList.at( wpList.count() - 1 )->origP ) )
  {
    wayPoint *finalTP = wpList.at( wpList.count() - 1 );
    no1.sprintf( "%02d", finalTP->taskPointIndex );

    QString finalTpDes = finalTP->description;

    finalTpDes.replace(  QRegExp(" "), "&nbsp;" );

    display += "<tr><th colspan=\"2\" align=\"left\">" +
      tr("Landing target") + " " + no1 + "</th>" +
      "<th colspan=2 align=left>" + finalTP->name + "&nbsp;(" + finalTpDes + ")" +
      "</th></tr>";

    // distance in km to final target must be calculated
    double finalDistance = dist2Next;

    for( int loop=currentTpIndex+2; loop <= wpList.count() - 1; loop++ )
        {
          // qDebug("distance: %f", wpList.at(loop)->distance);
          finalDistance += wpList.at(loop)->distance;
        }
 
    // to avoid wraping in the table we have to code spaces as forced
    // spaces in html
    distance = Distance::getText( finalDistance*1000., true, 1);
    distance.replace(  QRegExp(" "), "&nbsp;" );

    display += "<tr><td>&nbsp;&nbsp;" + tr("Distance") + "</td><td align=\"left\"><b>" +
      distance + "</b></td>";

    // calculation of the final arrival altitude
    reach = (reachable) task->calculateFinalGlidePath( currentTpIndex, arrivalAlt, bestSpeed );

    if( arrivalAlt.isValid() )
      {
        switch (reach)
          {
          case yes:
            display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b>+" +
              arrivalAlt.getText(true,0) + "</b></td></tr>";
            break;
          case no:
            display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><font color=\"#FF0000\"><b>" +
              arrivalAlt.getText(true,0) + "</font></b></td></tr>";
            break;
          case belowSafety:
            display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b><font color=\"#FF00FF\">" +
              arrivalAlt.getText(true,0) + "</font></b></td></tr>";
          }
      }
    else
      {
        display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b>" +
          tr("unknown") + "</b></td></tr>";
      }

    display += "<tr><td>&nbsp;&nbsp;" + tr("Vg") + "</td><td align=\"left\"><b>" +
      speed + "</b></td>";

    // If speed is to less we do not display any time values
    if( gs > 0.3 )
      {
	int time2Final = (int) rint( finalDistance*1000. / gs );
	
	QTime qtime(0,0);
	qtime = qtime.addSecs(time2Final);
	
	display += "<td>&nbsp;&nbsp;" + tr("Duration") + "</td><td align=\"left\"><b>" +
	  qtime.toString() + "</b></td></tr>";
	
	// ETA as UTC must be done with c-system functions. This QT
	// release does not provide this :((
	time_t seconds = time(0);
	seconds += time2Final;
	
	struct tm *gmt = gmtime( &seconds );
	
	QString eta;
	eta.sprintf( "%02d:%02d:%02d UTC", gmt->tm_hour, gmt->tm_min, gmt->tm_sec );
      
	display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
	  eta + "</b></td>";
      }
    else
      {
        display += "<td>&nbsp;&nbsp;" + tr("Duration") + "</td><td align=\"left\"><b>" +
          tr("unknown") + "</b></td></tr>";
        
        display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
          tr("unknown") + "</b></td>";
      }

    // calculate sunset for the landing destination
    QString sr, ss;
    QDate date = QDate::currentDate();
  
    bool res = Sonne::sonneAufUnter( sr, ss, date, finalTP->origP, 0 );

    if( res )
      {
	// In some areas no results available. In this case we skip
	// this output.
	display += "<td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
	  ss + " UTC </b></td></tr>";
      }
    else
      {
        display += "<td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
          tr("unknown")+ "</b></td></tr>";      
      }
  }

  display += "</table></big><html>";  
  text->setText( display );
}

/**
 * This method fills the widget with the arrival info. The info is
 * taken from the passed waypoint.
 *
 * wayPoint: pointer to waypoint object
 *
 */
void TPInfoWidget::prepareArrivalInfoText( wayPoint *wp )
{
  if( ! wp )
    {
      return;
    }

  Distance distance2Target;
  QPoint lastPosition = calculator->getlastPosition();
  // fetch minimal arrival altitude
  Altitude minAlt( GeneralConfig::instance()->getSafetyAltitude().getMeters() );

  // calculate distance to passed waypoint
  distance2Target.setKilometers(dist(double(lastPosition.x()), double(lastPosition.y()),
				     wp->origP.lat(), wp->origP.lon()));

  // Prepare output data
  QString display;

  display += "<html><big><center><b>" +
    tr("Arrival Info") +
    "</b></center><p>";

  display += "<table cellpadding=5><tr><th colspan=\"2\" align=\"left\">" +
    tr("Selected target") + "</th></tr>" +
    "<tr><td colspan=\"2\">&nbsp;&nbsp;" + wp->name + " (" + wp->description + ")" +
    "</td></tr>";
  
  // to avoid wraping in the table we have to code spaces as forced spaces in html
  QString distance = Distance::getText(distance2Target.getMeters(), true, 1);
  distance.replace(  QRegExp(" "), "&nbsp;" );
    
  display += "<tr><td>&nbsp;&nbsp;" + tr("Distance") + "</td><td align=\"left\"><b>" +
    distance + "</b></td></tr>";
    
  Altitude arrivalAlt;
  Speed bestSpeed;
  reachable reach = no;
  
  // calculate Bearing
  int bearing= int( rint(getBearingWgs( lastPosition, wp->origP ) * 180/M_PI) );
      
  // glide path
  calculator->glidePath( bearing, distance2Target,
                         wp->elevation,
                         arrivalAlt, bestSpeed );

  if( arrivalAlt.isValid() )
    {
      if( arrivalAlt >= minAlt )
        {
          display += "<tr><td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b>+" +
            arrivalAlt.getText(true,0) + "</b></td><tr>";
        }
      else if( arrivalAlt.getMeters() > 0.0 ) 
       {
         display += "<tr><td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><font color=\"#FF0000\"><b>" +
           arrivalAlt.getText(true,0) + "</font></b></td><tr>";
       }
     else
       {
         display += "<tr><td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b><font color=\"#FF00FF\">" +
           arrivalAlt.getText(true,0) + "</font></b></td><tr>";
        }
    }

  double gs = calculator->getlastSpeed().getMps(); // get last speed

  extern GPSNMEA *gps;

  if( gs == 0.0 && ! gps->getConnected() )
    {
      // set a pseudo speed of 100km/h in manual mode for testing
      gs = 100000. / 3600.;
    }

  QString speed = Speed(gs).getHorizontalText( true, 1 );
  speed.replace(  QRegExp(" "), "&nbsp;" );

  display += "<tr><td>&nbsp;&nbsp;" + tr("Vg") + "</td><td align=\"left\"><b>" +
    speed + "</b></td></tr>";

  // If speed is to less we do not display any time values
  if( gs > 0.3 )
    {
      int time2Target = (int) rint(distance2Target.getMeters() / gs);

      QTime qtime(0,0);
      qtime = qtime.addSecs(time2Target);
      
      display += "<tr><td>&nbsp;&nbsp;" + tr("Duration") + "</td><td align=\"left\"><b>" +
        qtime.toString() + "</b></td></tr>";
      
      // ETA as UTC must be done with c-system functions. This QT
      // release does not provide this :((
      time_t seconds = time(0);
      seconds += time2Target;
      
      struct tm *gmt = gmtime( &seconds );
      
      QString eta;
      eta.sprintf( "%02d:%02d:%02d UTC", gmt->tm_hour, gmt->tm_min, gmt->tm_sec );
      
      display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
        eta + "</b></td></tr>";
    }

  // calculate sunset for the target
  QString sr, ss;
  QDate date = QDate::currentDate();
  
  bool res = Sonne::sonneAufUnter( sr, ss, date, wp->origP, 0 );

  if( res )
    {
      // In some areas no results available. In this case we skip
      // this output.
      display += "<tr><td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
        ss + " UTC </b></td></tr>";
    }

  //----------------------------------------------------------------------------
  // Check, if waypoint is part of a flight task.In this case we will
  // also display the final flight target data.

  int tpIdx = wp->taskPointIndex;
  FlightTask *task = _globalMapContents->getCurrentTask();

  if( tpIdx == -1 || task == 0 )
    {
      // no flight task point resp. flight task active
      display += "</table>";  
      text->setText( display );
      return;
    }

  QList<wayPoint*> wpList = task->getWPList();

  if( wpList.count() < 4 )
    {
      // to less waypoints in list
      display += "</table>";  
      text->setText( display );
      return;
    }

  wayPoint *finalTP = wpList.at( wpList.count() - 1 );

  if( ( wp->taskPointType == wayPoint::End &&
        wp->origP == finalTP->origP ) || 
        wp->taskPointType == wayPoint::Landing )
    {
      // Waypoint is identical in position to landing point of flight
      // task. So we do display nothing more.
      display += "</table>";  
      text->setText( display );
     return;
    }

  display += "<tr><th colspan=\"2\" align=\"left\">" +
    tr("Landing target") + "</th></tr>" +
    "<tr><td colspan=\"2\">&nbsp;&nbsp;" + finalTP->name + " (" + finalTP->description + ")" +
    "</td></tr>";

  // distance in km to final target must be calculated
  double finalDistance = distance2Target.getKilometers();

  for( int loop=tpIdx+1; loop <= wpList.count() - 1; loop++ )
    {
      // qDebug("distance: %f", wpList.at(loop)->distance);
      finalDistance += wpList.at(loop)->distance;
    }
 
    // to avoid wraping in the table we have to code spaces as forced
    // spaces in html
    distance = Distance::getText( finalDistance*1000., true, 1);
    distance.replace(  QRegExp(" "), "&nbsp;" );

    display += "<tr><td>&nbsp;&nbsp;" + tr("Distance") + "</td><td align=\"left\"><b>" +
      distance + "</b></td></tr>";

    // calculation of the final arrival altitude
    reach = (reachable) task->calculateFinalGlidePath( tpIdx, arrivalAlt, bestSpeed );

    if( arrivalAlt.isValid() )
      {
        switch (reach)
          {
          case yes:
            display += "<tr><td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b>+" +
              arrivalAlt.getText(true,0) + "</b></td><tr>";
            break;
          case no:
            display += "<tr><td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><font color=\"#FF0000\"><b>" +
              arrivalAlt.getText(true,0) + "</font></b></td><tr>";
            break;
          case belowSafety:
            display += "<tr><td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b><font color=\"#FF00FF\">" +
              arrivalAlt.getText(true,0) + "</font></b></td><tr>";
          }
      }

    display += "<tr><td>&nbsp;&nbsp;" + tr("Vg") + "</td><td align=\"left\"><b>" +
      speed + "</b></td></tr>";

    // If speed is to less we do not display any time values
    if( gs > 0.3 )
      {
	int time2Final = (int) rint( finalDistance*1000. / gs );
	
	QTime qtime(0,0);
	qtime = qtime.addSecs(time2Final);
	
	display += "<tr><td>&nbsp;&nbsp;" + tr("Duration") + "</td><td align=\"left\"><b>" +
	  qtime.toString() + "</b></td></tr>";
	
	// ETA as UTC must be done with c-system functions. This QT
	// release does not provide this :((
	time_t seconds = time(0);
	seconds += time2Final;
	
	struct tm *gmt = gmtime( &seconds );
	
	QString eta;
	eta.sprintf( "%02d:%02d:%02d UTC", gmt->tm_hour, gmt->tm_min, gmt->tm_sec );
      
	display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
	  eta + "</b></td></tr>";
      }

    // calculate sunset for the landing destination
    res = Sonne::sonneAufUnter( sr, ss, date, finalTP->origP, 0 );

    if( res )
      {
	// In some areas no results available. In this case we skip
	// this output.
	display += "<tr><td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
	  ss + " UTC </b></td></tr>";
      }

    display += "</table></big></html>";
    text->setText( display );
}

/** This slot is called by the Keep Open button to keep the dialog open. :-) */
void TPInfoWidget::slot_KeepOpen()
{
  timer->stop();
  cmdClose->setText(tr("Close"));
  cmdKeep->hide();
  text->setFocus();
}
