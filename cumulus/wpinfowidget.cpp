/***********************************************************************
 **
 **   wpinfowidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by André Somers
 **                   2008-2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>
#include <time.h>

#include <QtGui>

#include "mainwindow.h"
#include "basemapelement.h"
#include "wpinfowidget.h"
#include "generalconfig.h"
#include "calculator.h"
#include "mapcontents.h"
#include "mapcalc.h"
#include "wgspoint.h"
#include "altitude.h"
#include "sonne.h"
#include "gpsnmea.h"

extern Calculator   *calculator;

WPInfoWidget::WPInfoWidget( MainWindow *parent ) :
  QWidget(parent)
{
  setObjectName("WPInfoWidget");
  mainWindow = parent;
  _lastView = MainWindow::mapView;
  _wp.name = "";
  homeChanged = false;

  resize(parent->size());

  QFont bfont = font();
  bfont.setBold(true);

  QBoxLayout *topLayout = new QVBoxLayout(this);

  text = new QTextEdit(this);
  text->setReadOnly( true );

  QPalette p = palette();
  p.setColor(QPalette::Window, Qt::white);
  text->setPalette(p);

  topLayout->addWidget(text, 10);

  buttonrow2 = new QHBoxLayout;
  topLayout->addLayout(buttonrow2);

  cmdAddWaypoint = new QPushButton(tr("Add Waypoint"), this);
  cmdAddWaypoint->setFont(bfont);
  buttonrow2->addWidget(cmdAddWaypoint);
  connect(cmdAddWaypoint, SIGNAL(clicked()),
          this, SLOT(slot_addAsWaypoint()));

  cmdHome = new QPushButton(tr("Home"), this);
  cmdHome->setFont(bfont);
  buttonrow2->addWidget(cmdHome);
  connect(cmdHome, SIGNAL(clicked()),
          this, SLOT(slot_setNewHome()));

  cmdArrival = new QPushButton(tr("Arrival"), this);
  cmdArrival->setFont(bfont);
  buttonrow2->addWidget(cmdArrival);
  connect(cmdArrival, SIGNAL(clicked()),
          this, SLOT(slot_arrival()));

  buttonrow1=new QHBoxLayout;
  topLayout->addLayout(buttonrow1);

  cmdClose = new QPushButton(tr("Close"), this);
  cmdClose->setFont(bfont);
  buttonrow1->addWidget(cmdClose);
  connect(cmdClose, SIGNAL(clicked()),
          this, SLOT(slot_SwitchBack()));

  // Activate keyboard shortcut cancel to close the window too
  scClose = new QShortcut( this );

#ifndef ANDROID
  scClose->setKey( Qt::Key_Escape );
#else
  scClose->setKey( Qt::Key_Close );
#endif

  connect( scClose, SIGNAL(activated()),
           this, SLOT( slot_SwitchBack() ));

  cmdKeep = new QPushButton(tr("Stop"), this);
  cmdKeep->setFont(bfont);
  buttonrow1->addWidget(cmdKeep);
  connect(cmdKeep, SIGNAL(clicked()),
          this, SLOT(slot_KeepOpen()));

  cmdUnselectWaypoint = new QPushButton(tr("Unselect"), this);
  cmdUnselectWaypoint->setFont(bfont);
  buttonrow1->addWidget(cmdUnselectWaypoint);
  connect(cmdUnselectWaypoint, SIGNAL(clicked()),
          this, SLOT(slot_unselectWaypoint()));

  cmdSelectWaypoint = new QPushButton(tr("Select"), this);
  cmdSelectWaypoint->setFont(bfont);
  buttonrow1->addWidget(cmdSelectWaypoint);
  connect(cmdSelectWaypoint, SIGNAL(clicked()),
          this, SLOT(slot_selectWaypoint()));

  timer=new QTimer(this);
  connect(timer, SIGNAL(timeout()),
          this, SLOT(slot_timeout()));
}

WPInfoWidget::~WPInfoWidget()
{
}

/** This slot get called on the timer timeout. */
void WPInfoWidget::slot_timeout()
{
  if(--_timerCount==0)
    {
      timer->stop();
      slot_SwitchBack();
    }
  else
    {
      QString txt = tr("Close (%1)").arg(_timerCount);
      cmdClose->setText(txt);
    }
}

/** This method is called by MainWindow to set the view to
 * which there must be returned and the waypoint to view. */
bool WPInfoWidget::showWP(int lastView, const Waypoint& wp)
{
  extern MapContents* _globalMapContents;

  // save return view
  _lastView = lastView;

  // save passed waypoint
  _wp = wp;

  // Check if new point is in the waypoint list, so make sure we can add it.
  if (_globalMapContents->isInWaypointList(wp.origP))
    {
      cmdAddWaypoint->setVisible( false );
    }
  else
    {
      cmdAddWaypoint->setVisible( true );
    }

  // Reset home changed
  homeChanged = false;

  // check, if current home position is different from waypoint
  GeneralConfig *conf = GeneralConfig::instance();
  QPoint home = conf->getHomeCoord();

  // Show the home button only if we are not to fast in move to avoid
  // wrong usage. The redefinition of the home position can trigger
  // a reload of the airfield list.
  if( home == wp.origP || calculator->moving() )
    {
      cmdHome->setVisible( false );
    }
  else
    {
      cmdHome->setVisible( true );
    }

  // Check if Waypoint is not selected, so make sure we can select it.
  const Waypoint *calcWp = calculator->getselectedWp();

  if( calcWp )
    {
      if( wp.origP == calcWp->origP )
        {
          cmdUnselectWaypoint->setVisible( true );
          cmdSelectWaypoint->setVisible( false );
        }
      else
        {
          cmdSelectWaypoint->setVisible( true );
          cmdUnselectWaypoint->setVisible( false );
        }
    }
  else
    {
      cmdSelectWaypoint->setVisible( true );
      cmdUnselectWaypoint->setVisible( false );
    }

  if( wp.taskPointIndex == 0 )
    {
      // take-off task points are not select or unselectable
      cmdSelectWaypoint->setVisible( false );
      cmdUnselectWaypoint->setVisible( false );
    }

  writeText();

  // @AP: load the time from user configuration
  _timerCount = GeneralConfig::instance()->getInfoDisplayTime();

  if( _timerCount > 0 )
    {
      timer->start(1000);
      QString txt = tr("Close (%1)").arg(_timerCount);
      cmdClose->setText(txt);
      cmdKeep->setVisible( true );
    }
  else
    {
      // Timer is set to zero, no automatic window close
      cmdClose->setText(tr("Close"));
      cmdKeep->setVisible( false );
    }

  setVisible( true );
  return true;
}

/**
 * Called, if the widget will be shown.
 */
void WPInfoWidget::showEvent(QShowEvent *)
{
  // qDebug("WPInfoWidget::showEvent(): name=%s", name());

  // resize to size of parent, could be changed in the meantime as the widget was hidden
  resize(mainWindow->size());
}

/** This method actually fills the widget with the info to be displayed. */
void WPInfoWidget::writeText()
{
  if( _wp.name.isEmpty() )
    {
      text->setHtml("<html><big><center><b>" +
                    tr("No waypoint data available") +
                    "</b></center></big></html>");
    }
  else
    {
      // display info from waypoint
      QString itxt;
      QString tmp;
      QString table = "<p><table cellpadding=5 width=100%>";

      itxt+= "<html><center><b>" + _wp.description + " (" + _wp.name;

      if( !_wp.icao.isEmpty() )
        {
          itxt += ",&nbsp;" + _wp.icao;
        }

      if( !_wp.country.isEmpty() )
        {
          itxt += ",&nbsp;" + _wp.country;
        }

      itxt+= ")<p>" + BaseMapElement::item2Text(_wp.type, tr("(unknown)"));

      if( _wp.isLandable )
        {
          itxt += "</b></center>";

          QString tmp2 = tr("Unknown");

          // @AP: show runway in both directions
          uint rwh1 = _wp.runway / 256;
          uint rwh2 = _wp.runway % 256;

          // qDebug("wp.rw=%d rwh1=%d rwh2=%d", _wp.runway, rwh1, rwh2);

          if( rwh1 > 0 && rwh1 <= 36 && rwh2 > 0 && rwh2 <= 36 )
            {
              // heading lays in the expected range
             if( rwh1 == rwh2 || abs( rwh1-rwh2) == 18 )
                {
                  // a) If both directions are equal, there is only one landing direction.
                  // b) If the difference is 18, there are two landing directions,
                  //    where the first direction is the preferred one.
                  tmp2 = QString("<b>%1/%2</b>").arg(rwh1, 2, 10, QLatin1Char('0')).
                                 arg(rwh2, 2, 10, QLatin1Char('0'));
                }
              else
              {
                int rwh1inv = rwh1 <= 18 ? rwh1+18 : rwh1-18;
                int rwh2inv = rwh2 <= 18 ? rwh2+18 : rwh2-18;

                // There are two different runways available
                tmp2 = QString( "<b>%1/%2, %3/%4</b>").
                                arg(rwh1, 2, 10, QLatin1Char('0')).
                                arg(rwh1inv, 2, 10, QLatin1Char('0')).
                                arg(rwh2, 2, 10, QLatin1Char('0')).
                                arg(rwh2inv, 2, 10, QLatin1Char('0'));
              }
            }

          itxt += table + "<tr><td>" + tr("Runway: ") + "</td><td>" + tmp2 + " (" +
                  Runway::item2Text(_wp.surface, tr("Unknown")) + ")</td>" +
                  "<td>" + tr("Length: ") + "</td><td><b>";

          if( _wp.length <= 0 )
            {
              itxt += tr("Unknown") + "</b></td></tr>";
            }
          else
            {
              tmp = QString("%1 m</b></td></tr>").arg(_wp.length);
              itxt += tmp;
            }
        }
      else
        {
          itxt += table;
        }

      // save current unit
      Altitude::altitudeUnit currentUnit = Altitude::getUnit();

      Altitude::setUnit(Altitude::meters);
      QString meters = Altitude::getText(_wp.elevation, true, 0);

      Altitude::setUnit(Altitude::feet);
      QString feet = Altitude::getText(_wp.elevation, true, 0);

      // restore save unit
       Altitude::setUnit(currentUnit);

       if( currentUnit == Altitude::meters )
         {
           itxt += "<tr><td>" + tr( "Elevation:" ) +
             "</td><td><b>" + meters + "&nbsp;/&nbsp;" + feet +
             "</b></td>";
         }
       else
         {
           itxt += "<tr><td>" + tr( "Elevation:" ) +
             "</td><td><b>" + feet + "&nbsp;/&nbsp;" + meters +
             "</b></td>";
         }

       if( _wp.frequency > 0.0 )
         {
           tmp = QString("<td>" + tr("Frequency:") + "</td><td><b>%1&nbsp;MHz</b></td></tr>").arg(_wp.frequency, 0, 'f', 3);
           itxt += tmp;
         }
       else
         {
           itxt+="<tr><td>&nbsp;</td><td>&nbsp;</td></tr>";
         }

      QString sr, ss, tz;
      QDate date = QDate::currentDate();

      // calculate Sunrise and Sunset
      bool res = Sonne::sonneAufUnter( sr, ss, date, _wp.origP , tz );

      if( res )
        {
        // In some areas are no results available. In this case we skip
        // the times output.
          itxt += QString( "<tr><td>" + tr("Sunrise:") + "</td><td><b>" + sr +
                           " " + tz + "</b></td>" );

          itxt += QString( "<td>" + tr("Sunset:") + "</td><td><b>" + ss +
                           " " + tz + "</b></td></tr>" );
        }

      QString lat = WGSPoint::printPos(_wp.origP.x(),true);
      QString lon = WGSPoint::printPos(_wp.origP.y(),false);

      lat = lat.replace(QRegExp(" "), "&nbsp;");
      lon = lon.replace(QRegExp(" "), "&nbsp;");

      itxt += "<tr><td>" + tr("Latitude:") + "</td><td><b>" +
              lat + "</b></td>" +
              "<td>" + tr("Longitude:") + "</td><td><b>" +
              lon +
              "</b></td></tr>";

      if( _wp.comment.isEmpty() == false )
        {
          itxt += "<tr><td>" + tr("Comment") + ":</td>"
                  "<td colspan=3>" + _wp.comment + "</td></tr>";
        }

      itxt+="</table></html>";

      text->setHtml(itxt);
    }
}

/** Hide widget and return to the calling view in MainWindow */
void WPInfoWidget::slot_SwitchBack()
{
  timer->stop();
  text->clearFocus();
  setVisible( false );

  if( _lastView == MainWindow::infoView )
    {
      // make sure last view isn't this view
      _lastView = MainWindow::mapView;
    }

  mainWindow->setView( (MainWindow::appView) _lastView );

  // Check, if we have not a valid GPS fix. In this case we do move the map
  // to the new home position.
  if( homeChanged == true && GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      emit gotoHomePosition();
      homeChanged = false;
    }
}


/** This slot is called by the KeepOpen button to... yes... keep the dialog open. :-) */
void WPInfoWidget::slot_KeepOpen()
{
  timer->stop();
  cmdClose->setText( tr( "Close" ) );
  cmdKeep->setVisible( false );
}


/** This slot is called if the unselect Waypoint button is clicked. */
void WPInfoWidget::slot_unselectWaypoint()
{
  emit waypointSelected(0, true);

  return slot_SwitchBack();
}


/** This slot is called if the Select Waypoint button is clicked */
void WPInfoWidget::slot_selectWaypoint()
{
  // If the select button is not visible we will call the unselect
  // routine. Result is toggling between the two modes.
  if( cmdUnselectWaypoint->isVisible() )
    {
      return slot_unselectWaypoint();
    }

  emit waypointSelected(&_wp, true);

  return slot_SwitchBack();
}


/** This slot is called if the Add Waypoint button is clicked. */
void WPInfoWidget::slot_addAsWaypoint()
{
  _wp.priority = Waypoint::High; //priority is high
  emit waypointAdded(_wp);

  cmdAddWaypoint->setVisible( false );
  slot_KeepOpen();
}


/**
 *  This slot is called if the Home button is clicked. The change of the home
 *  position can trigger a reload of many map data, if Welt2000 has radius
 *  option set or option projection follows home is active.
 */
void WPInfoWidget::slot_setNewHome()
{
  slot_KeepOpen(); // Stop timer

  QMessageBox mb( QMessageBox::Question,
                  tr( "Set home site" ),
                  tr( "Use point<br><b>%1</b><br>as your home site?").arg(_wp.name) +
                  tr("<br>Change can take<br>a few seconds and more."),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::Yes );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif


  if( mb.exec() == QMessageBox::Yes )
    {
      //QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents |
                                       QEventLoop::ExcludeSocketNotifiers );

      QCoreApplication::flush();

      // save new home position and elevation
      GeneralConfig *conf = GeneralConfig::instance();
      conf->setHomeCountryCode( _wp.country );
      conf->setHomeName( _wp.name );
      conf->setHomeCoord( _wp.origP );
      conf->setHomeElevation( Distance(_wp.elevation) );

      emit newHomePosition( _wp.origP );
      homeChanged = true;
      cmdHome->setVisible( false );

      //QApplication::restoreOverrideCursor();
    }
}

/**
 * This slot is called if the arrival button is clicked.
 */
void WPInfoWidget::slot_arrival()
{
  // qDebug("WPInfoWidget::slot_arrival()");
  slot_KeepOpen(); // Stop timer

  // switch off all accelerator keys
  scClose->setEnabled(false);

  // create arrival info widget
  TPInfoWidget *arrivalInfo = new TPInfoWidget( this );
  arrivalInfo->prepareArrivalInfoText( &_wp );
  arrivalInfo->showTP( false );

  connect( arrivalInfo, SIGNAL(closed()),
           this, SLOT(slot_arrivalClose()));
}

// sets focus back to wp text view after closing arrival widget
void WPInfoWidget::slot_arrivalClose()
{
  // switch on close shortcut keys
  scClose->setEnabled(true);
}
