/***********************************************************************
**
**   tpinfowidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2007-2021 Axel Pauli, kflog.cumulus@gmail.com
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
************************************************************************/

#include <cmath>
#include <ctime>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "tpinfowidget.h"
#include "flighttask.h"
#include "generalconfig.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "calculator.h"
#include "reachablelist.h"
#include "gpsnmea.h"
#include "sonne.h"
#include "time_cu.h"

extern MapContents*  _globalMapContents;
extern Calculator*   calculator;

TPInfoWidget::TPInfoWidget( QWidget *parent ) :
  QWidget( parent ),
  m_timerCount(0),
  m_parent(parent)
{
  setObjectName("TPInfoWidget");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute( Qt::WA_DeleteOnClose );

  if( parent )
    {
      resize( parent->size() );
    }

  QFont bfont = font();
  bfont.setBold(true);

  QBoxLayout *topLayout = new QVBoxLayout( this );

  m_text = new QTextEdit(this);
  m_text->setReadOnly( true );
  m_text->setAutoFillBackground(true);

  QPalette p = palette();

  if( GeneralConfig::instance()->getBlackBgInfoDisplay() == false )
    {
      p.setColor(QPalette::Base, Qt::white);
      m_text->setPalette(p);
      p.setColor(QPalette::Text, Qt::black);
      m_text->setPalette(p);
    }
  else
    {
      p.setColor(QPalette::Base, Qt::black);
      m_text->setPalette(p);
      p.setColor(QPalette::Text, Qt::white);
      m_text->setPalette(p);
    }

  connect( m_text, SIGNAL(cursorPositionChanged()), SLOT(slotCursorChanged()));

#ifdef QSCROLLER
  QScroller::grabGesture( m_text->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( m_text->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  topLayout->addWidget(m_text, 5 );

  QHBoxLayout *buttonrow = new QHBoxLayout;
  topLayout->addLayout(buttonrow);

  m_cmdClose = new QPushButton(tr("Close"), this);
  m_cmdClose->setFont(bfont);
  buttonrow->addWidget(m_cmdClose);
  connect(m_cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()));

  m_cmdKeep = new QPushButton(tr("Stop"), this);
  m_cmdKeep->setFont(bfont);
  buttonrow->addWidget(m_cmdKeep);
  connect(m_cmdKeep, SIGNAL(clicked()), this, SLOT(slot_KeepOpen()));

  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(slot_Timeout()));

  // activate keyboard shortcuts for closing of widget
  connect( new QShortcut( Qt::Key_Escape, this ), SIGNAL(activated()),
           this, SLOT( slot_Close() ));

  connect( new QShortcut( Qt::Key_Close, this ), SIGNAL(activated()),
           this, SLOT( slot_Close() ));
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
  setVisible(false);
  emit closed();
  QWidget::close();
}

/** This slot get called on the timer timeout. If timer expires the
    widget will be closed automatically. */
void TPInfoWidget::slot_Timeout()
{
  if( --m_timerCount == 0 )
    {
      m_timer->stop();
      slot_Close();
    }
  else
    {
      QString txt = tr("Close (%1)").arg(m_timerCount);
      m_cmdClose->setText(txt);
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
  m_timerCount = conf->getInfoDisplayTime();

  if( m_timerCount > 0 && automaticClose )
    {
      m_timer->start(1000);
      QString txt = tr("Close (%1)").arg(m_timerCount);
      m_cmdClose->setText(txt);
      m_cmdKeep->show();
    }
  else
    {
      // Timer is set to zero, no automatic window close
      m_cmdClose->setText(tr("Close"));
      m_cmdKeep->hide();
    }

  QWidget::show();
}

/**
 * Called, if the widget will be shown.
 */
void TPInfoWidget::showEvent( QShowEvent *event )
{
  QWidget::showEvent( event );
}

/**
 * This method fills the widget with the taskpoint switch info. The
 * info is taken from the current active flight task.
 *
 * currentTpIndex: index of current taskpoint in flight task
 * dist2Next: distance to the next taskpoint in kilometers
 *
 */
void TPInfoWidget::prepareSwitchText( const int currentTpIndex,
                                      const double dist2Next )
{
  if( GeneralConfig::instance()->getBlackBgInfoDisplay() == false )
    {
      m_text->setStyleSheet( QString( "color: black; background-color: white;" ) );
    }
  else
    {
      m_text->setStyleSheet( QString( "color: white; background-color: black;" ) );
    }

  FlightTask *task = _globalMapContents->getCurrentTask();

  if( ! task )
    {
      // no flight task is active
      return;
    }

  QList<TaskPoint>& tpList = task->getTpList();

  if( tpList.count() < 2 || tpList.count() <= currentTpIndex+1 )
    {
      // to less taskpoints in the list
      return;
    }

  TaskPoint& currentTP = tpList[ currentTpIndex ];
  TaskPoint& nextTP    = tpList[ currentTpIndex + 1 ];

  QString display;
  QString no1, no2;
  QString currentTpDes = currentTP.getName();
  QString nextTpDes    = nextTP.getName();

  currentTpDes.replace(  QRegExp(" "), "&nbsp;" );
  nextTpDes.replace(  QRegExp(" "), "&nbsp;" );

  no1 = QString( "%1" ).arg( currentTpIndex, 2, 10, QChar('0') );
  no2 = QString( "%1" ).arg( currentTpIndex + 1, 2, 10, QChar('0') );

  display += "<html><center><b>" +
    tr("Taskpoint switch") + " " + no1 + "->" + no2 +
    "</b></center>";

  display += "<table width=100% cellpadding=4 cellspacing=0 border=1><tr><th colspan=2 align=left>" +
    tr("Reached target") + " " + no1 + "</th>" +
    "<th colspan=2 align=left>" + currentTP.getWPName() + "&nbsp;(" + currentTpDes + ")" +
    "</th></tr>";

  display += "<tr><th colspan=2 align=\"left\">" +
    tr("Next target") + " " + no2 + "</th>" +
    "<th colspan=2 align=left>" + nextTP.getWPName() + "&nbsp;(" + nextTpDes + ")" +
    "</th></tr>";

  // to avoid wrapping in the table we have to code spaces as forced spaces in html
  QString distance = Distance::getText(dist2Next * 1000., true, 1);
  distance.replace(  QRegExp(" "), "&nbsp;" );

  display += "<tr><td>&nbsp;&nbsp;" + tr("Distance") + "</td><td align=\"left\"><b>" +
             distance + "</b></td>";

  Altitude arrivalAlt;
  Speed bestSpeed;
  ReachablePoint::reachable reach = ReachablePoint::no;

  // calculate Bearing
  int bearing= int( rint(MapCalc::getBearingWgs( calculator->getlastPosition(),
                                                 nextTP.getWGSPosition() ) * 180/M_PI) );
  // glide path
  calculator->glidePath( bearing, dist2Next,
                         nextTP.getElevation(),
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

  double gs = calculator->getLastSpeed().getMps(); // get last speed

  if( gs == 0.0 && ! GpsNmea::gps->getConnected() )
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

      QDateTime eta( QDateTime::currentDateTime() );

      eta = eta.addSecs( time2Next );

      QString etaString;

        if( Time::getTimeUnit() == Time::local )
          {
            etaString = eta.toLocalTime().toString("hh:mm:ss");
          }
        else
          {
            etaString = eta.toTimeSpec(Qt::UTC).toString("hh:mm:ss") + " UTC";
          }

      display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
        etaString + "</b></td>";
    }
  else
    {
      display += "<td>&nbsp;&nbsp;" + tr("Duration") + "</td><td align=\"left\"><b>" +
        tr("unknown") + "</b></td></tr>";

      display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
        tr("unknown") + "</b></td>";
    }

  // calculate sunset for the destination
  QString sr, ss, tz;
  QDate date = QDate::currentDate();

  bool res = Sonne::sonneAufUnter( sr, ss, date, nextTP.getWGSPositionRef(), tz );

  if( res )
    {
      // In some areas no results available. In this case we skip
      // this output.
      display += "<td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
        ss + " " + tz +  "</b></td></tr>";
    }
  else
    {
      display += "<td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
        tr("unknown")+ "</b></td></tr>";
    }

  //-----------------------------------------------------------------------
  // Show data of finish target, if it is not already included.
  if( nextTP.getTaskPointType() != TaskPointTypes::Finish )
    {
      TaskPoint& finalTP = tpList[ tpList.count() - 1 ];

      no1 = QString( "%1" ).arg( finalTP.getFlightTaskListIndex(), 2, 10, QChar('0') );

      QString finalTpDes = finalTP.getName();

      finalTpDes.replace(  QRegExp(" "), "&nbsp;" );

      display += "<tr><th colspan=\"2\" align=\"left\">" +
                  tr("Finish target") + " " + no1 + "</th>" +
                  "<th colspan=2 align=left>" + finalTP.getWPName() +
                  "&nbsp;(" + finalTpDes + ")" +
                  "</th></tr>";

      // distance in km to final target must be calculated
      double finalDistance = dist2Next;

      for( int loop = currentTpIndex + 2; loop <= tpList.count() - 1; loop++ )
        {
          // qDebug("distance: %f", tpList.at(loop)->distance);
          finalDistance += tpList[loop].distance;
        }

      // to avoid wraping in the table we have to code spaces as forced
      // spaces in html
      distance = Distance::getText( finalDistance*1000., true, 1);
      distance.replace(  QRegExp(" "), "&nbsp;" );

      display += "<tr><td>&nbsp;&nbsp;" + tr("Distance") +
                 "</td><td align=\"left\"><b>" +
		  distance + "</b></td>";

      // calculation of the final arrival altitude
      reach = (ReachablePoint::reachable) task->calculateFinalGlidePath( currentTpIndex, arrivalAlt, bestSpeed );

      if( arrivalAlt.isValid() )
        {
          switch (reach)
            {
            case ReachablePoint::yes:
              display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b>+" +
          arrivalAlt.getText(true,0) + "</b></td></tr>";
              break;
            case ReachablePoint::no:
              display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><font color=\"#FF0000\"><b>" +
          arrivalAlt.getText(true,0) + "</font></b></td></tr>";
              break;
            case ReachablePoint::belowSafety:
              display += "<td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b><font color=\"#FF00FF\">" +
          arrivalAlt.getText(true,0) + "</font></b></td></tr>";
              break;
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

          QDateTime eta( QDateTime::currentDateTime() );

          eta = eta.addSecs( time2Final );

          QString etaString;

            if( Time::getTimeUnit() == Time::local )
              {
          etaString = eta.toLocalTime().toString("hh:mm:ss");
              }
            else
              {
          etaString = eta.toTimeSpec(Qt::UTC).toString("hh:mm:ss") + " UTC";
              }

            display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
                 etaString + "</b></td>";
        }
      else
        {
          display += "<td>&nbsp;&nbsp;" + tr("Duration") + "</td><td align=\"left\"><b>" +
            tr("unknown") + "</b></td></tr>";

          display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
            tr("unknown") + "</b></td>";
        }

      // calculate sunset for the finish destination
      QString sr, ss, tz;
      QDate date = QDate::currentDate();

      bool res = Sonne::sonneAufUnter( sr, ss, date, finalTP.getWGSPositionRef(), tz );

      if( res )
        {
          // In some areas no results available. In this case we skip
          // this output.
          display += "<td>&nbsp;&nbsp;" + tr("Sunset") +
                     "</td><td align=\"left\"><b>" +
                     ss + " " + tz + "</b></td></tr>";
        }
      else
        {
          display += "<td>&nbsp;&nbsp;" + tr("Sunset") +
                     "</td><td align=\"left\"><b>" +
                     tr("unknown")+ "</b></td></tr>";
        }
    }

  display += "</table><html>";
  m_text->setHtml( display );
}

/**
 * This method fills the widget with the arrival info. The info is
 * taken from the passed waypoint.
 *
 * Waypoint: pointer to waypoint object
 *
 */
void TPInfoWidget::prepareArrivalInfoText( Waypoint *wp )
{
  if( ! wp )
    {
      return;
    }

  if( GeneralConfig::instance()->getBlackBgInfoDisplay() == false )
    {
      m_text->setStyleSheet( QString( "color: black; background-color: white;" ) );
    }
  else
    {
      m_text->setStyleSheet( QString( "color: white; background-color: black;" ) );
    }

  Distance distance2Target;
  QPoint lastPosition = calculator->getlastPosition();
  // fetch minimal arrival altitude
  Altitude minAlt( GeneralConfig::instance()->getSafetyAltitude().getMeters() );

  // calculate distance to passed waypoint
  distance2Target.setKilometers(MapCalc::dist(double(lastPosition.x()), double(lastPosition.y()),
                                              wp->wgsPoint.lat(), wp->wgsPoint.lon()));

  // Prepare output data
  QString display;

  display += "<html><center><b>" +
    tr("Arrival Info") +
    "</b></center><p>";

  display += "<table cellpadding=5><tr><th colspan=\"2\" align=\"left\">" +
    tr("Selected target") + "</th></tr>" +
    "<tr><td colspan=\"2\">&nbsp;&nbsp;" + wp->name + " (" + wp->description + ")" +
    "</td></tr>";

  // to avoid wrapping in the table we have to code spaces as forced spaces in html
  QString distance = Distance::getText(distance2Target.getMeters(), true, 1);
  distance.replace(  QRegExp(" "), "&nbsp;" );

  display += "<tr><td>&nbsp;&nbsp;" + tr("Distance") + "</td><td align=\"left\"><b>" +
             distance + "</b></td></tr>";

  Altitude arrivalAlt;
  Speed bestSpeed;
  ReachablePoint::reachable reach = ReachablePoint::no;

  // calculate Bearing
  int bearing= int( rint(MapCalc::getBearingWgs( lastPosition, wp->wgsPoint ) * 180/M_PI) );

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

  double gs = calculator->getLastSpeed().getMps(); // get last speed

  if( gs == 0.0 && ! GpsNmea::gps->getConnected() )
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

      //qDebug("time2Target=%d", time2Target);

      QTime qtime(0,0);
      qtime = qtime.addSecs(time2Target);

      display += "<tr><td>&nbsp;&nbsp;" + tr("Duration") + "</td><td align=\"left\"><b>" +
        qtime.toString() + "</b></td></tr>";

      QDateTime eta( QDateTime::currentDateTime() );

      eta = eta.addSecs( time2Target );

      QString etaString;

        if( Time::getTimeUnit() == Time::local )
          {
            etaString = eta.toLocalTime().toString("hh:mm:ss");
          }
        else
          {
            etaString = eta.toTimeSpec(Qt::UTC).toString("hh:mm:ss") + " UTC";
          }

        display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
        etaString + "</b></td></tr>";
    }

  // calculate sunset for the target
  QString sr, ss, tz;
  QDate date = QDate::currentDate();

  bool res = Sonne::sonneAufUnter( sr, ss, date, wp->wgsPoint, tz );

  if( res )
    {
      // In some areas no results available. In this case we skip
      // this output.
      display += "<tr><td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
        ss + " " + tz + "</b></td></tr>";
    }

  //----------------------------------------------------------------------------
  // Check, if waypoint is part of a flight task. In this case we will
  // also display the final flight target data.
  int tpIdx = wp->taskPointIndex;
  FlightTask *task = _globalMapContents->getCurrentTask();

  if( tpIdx == -1 || task == 0 )
    {
      // no flight task point resp. flight task active
      display += "</table>";
      m_text->setHtml( display );
      return;
    }

  QList<TaskPoint>& tpList = task->getTpList();

  if( tpList.count() < 4 )
    {
      // to less task points in list
      display += "</table>";
      m_text->setHtml( display );
      return;
    }

  TaskPoint& tp = tpList[wp->taskPointIndex];
  TaskPoint& finalTp = tpList[tpList.count() - 1];

  if( tp.getTaskPointType() == TaskPointTypes::Finish )
    {
      // Waypoint is identical in position to finish point of flight
      // task. So we do display nothing more.
      display += "</table>";
      m_text->setHtml( display );
     return;
    }

  display += "<tr><th colspan=\"2\" align=\"left\">" +
    tr("Finish target") + "</th></tr>" +
    "<tr><td colspan=\"2\">&nbsp;&nbsp;" + finalTp.getWPName() + " (" + finalTp.getComment() + ")" +
    "</td></tr>";

  // distance in km to final target must be calculated
  double finalDistance = distance2Target.getKilometers();

  for( int loop=tpIdx+1; loop <= tpList.count() - 1; loop++ )
    {
      // qDebug("distance: %f", tpList.at(loop)->distance);
      finalDistance += tpList[loop].distance;
    }

  // to avoid wrapping in the table we have to code spaces as forced
  // spaces in html
  distance = Distance::getText( finalDistance*1000., true, 1);
  distance.replace(  QRegExp(" "), "&nbsp;" );

  display += "<tr><td>&nbsp;&nbsp;" + tr("Distance") + "</td><td align=\"left\"><b>" +
              distance + "</b></td></tr>";

    // calculation of the final arrival altitude
  reach = (ReachablePoint::reachable) task->calculateFinalGlidePath( tpIdx, arrivalAlt, bestSpeed );

    if( arrivalAlt.isValid() )
      {
        switch (reach)
          {
          case ReachablePoint::yes:
            display += "<tr><td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b>+" +
              arrivalAlt.getText(true,0) + "</b></td><tr>";
            break;
          case ReachablePoint::no:
            display += "<tr><td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><font color=\"#FF0000\"><b>" +
              arrivalAlt.getText(true,0) + "</font></b></td><tr>";
            break;
          case ReachablePoint::belowSafety:
            display += "<tr><td>&nbsp;&nbsp;" + tr("Arrival Alt") + "</td><td><b><font color=\"#FF00FF\">" +
              arrivalAlt.getText(true,0) + "</font></b></td><tr>";
            break;
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

        QDateTime eta( QDateTime::currentDateTime() );

        QString etaString;

        eta = eta.addSecs( time2Final );

        if( Time::getTimeUnit() == Time::local )
          {
            etaString = eta.toLocalTime().toString( "hh:mm:ss" );
          }
        else
          {
            etaString = eta.toTimeSpec( Qt::UTC ).toString( "hh:mm:ss" ) + " UTC";
          }

        display += "<tr><td>&nbsp;&nbsp;" + tr("ETA") + "</td><td align=\"left\"><b>" +
                    etaString + "</b></td></tr>";
      }

    // calculate sunset for the finish destination
    res = Sonne::sonneAufUnter( sr, ss, date, finalTp.getWGSPositionRef(), tz );

    if( res )
      {
        // In some areas no results available. In this case we skip
        // this output.
        display += "<tr><td>&nbsp;&nbsp;" + tr("Sunset") + "</td><td align=\"left\"><b>" +
                    ss + " " + tz + "</b></td></tr>";
       }

    display += "</table></html>";
    m_text->setHtml( display );
}

/** This slot is called by the Keep Open button to keep the dialog open. :-) */
void TPInfoWidget::slot_KeepOpen()
{
  m_timer->stop();
  m_cmdClose->setText(tr("Close"));
  m_cmdKeep->hide();
  m_text->setFocus();
}

void TPInfoWidget::slotCursorChanged()
{
  // Clear cursor's m_text selection.
  QTextCursor textCursor = m_text->textCursor();
  textCursor.clearSelection();
  m_text->setTextCursor( textCursor );
}
