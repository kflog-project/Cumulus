/***********************************************************************
 **
 **   wpinfowidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>
#include <time.h>

#include <QMessageBox>
#include <QFont>
#include <QRegExp>
#include <QShortcut>

#include "cumulusapp.h"
#include "basemapelement.h"
#include "airport.h"
#include "wpinfowidget.h"
#include "generalconfig.h"
#include "calculator.h"
#include "mapcontents.h"
#include "mapcalc.h"
#include "wgspoint.h"
#include "altitude.h"
#include "mapcontents.h"
#include "waypointcatalog.h"
#include "sonne.h"

extern MapConfig    *_globalMapConfig;
extern MapContents  *_globalMapContents;
extern Calculator       *calculator;

WPInfoWidget::WPInfoWidget( CumulusApp *parent ) :
    QWidget(parent),
    _wp(0)
{
  setObjectName("WPInfoWidget");
  cuApp = parent;
  _lastView = CumulusApp::mapView;
  arrivalInfo = 0;

  resize(parent->size());

  QFont bfont = font();
  bfont.setBold(true);

  QBoxLayout *topLayout = new QVBoxLayout(this);

  text = new QLabel(this);

  QPalette p = palette();
  p.setColor(QPalette::Window, Qt::white);
  text->setPalette(p);
  text->setAutoFillBackground(true);
  text->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  text->setLineWidth(2);

  topLayout->addWidget(text, 10);

  buttonrow2 = new QHBoxLayout;
  topLayout->addLayout(buttonrow2);

  cmdAddWaypoint = new QPushButton(tr("Add Waypoint"), this);
  QFont buttonFont = cmdAddWaypoint->font();
  cmdAddWaypoint->setFont(bfont);
  buttonrow2->addWidget(cmdAddWaypoint);
  connect(cmdAddWaypoint, SIGNAL(clicked()),
          this, SLOT(slot_addAsWaypoint()));

  cmdSetHome = new QPushButton(tr("New Home"), this);
  cmdSetHome->setFont(bfont);
  buttonrow2->addWidget(cmdSetHome);
  connect(cmdSetHome, SIGNAL(clicked()),
          this, SLOT(slot_setAsHome()));

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
  scClose->setKey( Qt::Key_Escape );
  connect( scClose, SIGNAL(activated()),
           this, SLOT( slot_SwitchBack() ));

  cmdKeep = new QPushButton(tr("Keep"), this);
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
  if( _wp )
    {
      delete _wp;
    }
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

/** This method is called by CumulusApp to set the view to
 * which there must be returned and the waypoint to view. */
bool WPInfoWidget::showWP(int lastView, const wayPoint *wp)
{
  extern MapContents* _globalMapContents;
  extern MapMatrix*   _globalMapMatrix;

  if( wp == 0 )
    {
      return false;
    }

  // save return view
  _lastView = lastView;

  // Check if new point is in waypoint list, so make sure we can add it.
  if (_globalMapContents->getIsInWaypointList(wp))
    cmdAddWaypoint->hide();
  else
    cmdAddWaypoint->show();

  // check, if current home position is different from waypoint
  QPoint home = _globalMapMatrix->getHomeCoord();

  if( home == wp->origP || inFlight() )
    {
      cmdSetHome->hide();
    }
  else
    {
      cmdSetHome->show();
    }

  // Check if Waypoint is not selected, so make sure we can select
  // it.

  const wayPoint *calcWp = calculator->getselectedWp();

  if( calcWp )
    {
      if( wp->origP == calcWp->origP )
        {
          cmdUnselectWaypoint->show();
          cmdSelectWaypoint->hide();
        }
      else
        {
          cmdSelectWaypoint->show();
          cmdUnselectWaypoint->hide();
        }
    }
  else
    {
      cmdSelectWaypoint->show();
      cmdUnselectWaypoint->hide();
    }

  if( wp->taskPointIndex == 0 )
    {
      // take-off task points are not select or unselectable
      cmdSelectWaypoint->hide();
      cmdUnselectWaypoint->hide();
    }

  // save new values. We make a deep copy to prevent problems with
  // constant methods elsewhere.
  if( _wp)
    {
      delete _wp;
    }

  _wp = new wayPoint(*wp);

  writeText();

  // @AP: load the time from user configuration
  _timerCount = GeneralConfig::instance()->getInfoDisplayTime();

  if( _timerCount > 0 )
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

  show();
  return true;
}

/**
 * Called, if the widget will be shown.
 */
void WPInfoWidget::showEvent(QShowEvent *)
{
  // qDebug("WPInfoWidget::showEvent(): name=%s", name());

  // resize to size of parent, could be changed in the meantime as the widget was hidden
  resize(cuApp->size());

  // set focus to text widget
//  text->setFocus();
}

/** This method actually fills the widget with the info. */
void WPInfoWidget::writeText()
{
  if( _wp == 0 )
    {
      text->setText("<html><big><center><b>" +
                    tr("No waypoint selected") +
                    "</b></center></big></html>");
    }
  else
    {
      // display info from waypoint
      QString itxt;
      QString tmp;
      int iTmp;
      bool start = false;
      QString table = "<p><table cellpadding=5 width=100%>";

      itxt+= "<html><!--big--><center><b>" + _wp->description + " (" + _wp->name;

      if (!_wp->icao.isEmpty())
        {
          itxt+=",&nbsp;"+ _wp->icao;
        }

      itxt+= ")<p>" + BaseMapElement::item2Text(_wp->type, tr("(unknown)"));

      if (_wp->isLandable)
        {
          itxt+= "</b></center>";

          iTmp=_wp->surface;

          if( iTmp > 3 )
            iTmp = 0;
          // qDebug("_wp->surface %d", _wp->surface );
          if (iTmp<0)
            iTmp=0;
          QString tmp2;

          if( _wp->runway < 0 || _wp->runway > 360 )
            {
              tmp2 = tr("Unknown");
            }
          else
            {
              // @AP: show runway in both directions, start with the lowest one
              int rw1 =_wp->runway;
              int rw2 = rw1 <= 180 ? rw1+180 : rw1-180;
              int h1 = rw1 < rw2 ? rw1/10 : rw2/10;
              int h2 = rw1 < rw2 ? rw2/10 : rw1/10;
              tmp2 = QString("<b>%1/%2</b>").arg(h1, 2, 10, QLatin1Char('0')).arg(h2, 2, 10, QLatin1Char('0'));
            }

          itxt += table + "<tr><td>" + tr("Runway: ") + "</td><td>" + tmp2 + " (" +
            Airport::item2Text(iTmp) + ")</td>" +
            "<td>" + tr("Length: ") + "</td><td><b>";

          if( _wp->length <= 0 )
            {
              itxt += tr("Unknown") + "</b></td></tr>";
            }
          else
            {
              tmp = QString("%1 m</b></td></tr>").arg(_wp->length);
              itxt += tmp;
            }
        }
      else
        {
          itxt+=" - <font color=\"#FF0000\">" + tr("NOT LANDABLE") + "</font>" +
                "</b></center>" + table;
          start = true;
        }

      if (_wp->frequency >= 108.0 && _wp->frequency <= 137.0 )
        {
          tmp = QString("<tr><td>" + tr("Frequency:") + "</td><td><b>%1 MHz</b></td>").arg(_wp->frequency,0,'f',3);
          itxt += tmp;
//          itxt+=tmp.sprintf("<tr><td>" + tr("Frequency:") + "</td><td><b>%1.3f MHz</b></td>", _wp->frequency);
        }
      else
        {
          itxt+="<tr><td>" + tr("Frequency:") + "</td><td><b>" + tr("Unknown") + "</b></td>";
        }

      // save current unit
      Altitude::altitude currentUnit = Altitude::getUnit();

      Altitude::setUnit(Altitude::meters);
      QString meters = Altitude::getText(_wp->elevation, true, 0);

      Altitude::setUnit(Altitude::feet);
      QString feet = Altitude::getText(_wp->elevation, true, 0);

      // restore save unit
       Altitude::setUnit(currentUnit);

       if( currentUnit == Altitude::meters )
         {
           itxt += "<td>"+tr("Elevation:") +
             "</td><td><b>" + meters + " / " + feet +
             "</b></td></tr>";
         }
       else
         {
           itxt += "<td>"+tr("Elevation:") +
             "</td><td><b>" + feet + " / " + meters +
             "</b></td></tr>";
         }

      QString sr, ss;
      QDate date = QDate::currentDate();

      // calculate Sunrise and Sunset
      bool res = Sonne::sonneAufUnter( sr, ss, date, _wp->origP , 0 );

      if( res )
        {
          // In some areas no results available. In this case we skip
          // this output.
          itxt += QString( "<tr><td>" + tr("Sunrise:") + "</td><td><b>" + sr +
                               " UTC</b></td>" );

          itxt += QString( "<td>" + tr("Sunset:") + "</td><td><b>" + ss +
                               " UTC</b></td></tr>" );
        }

      itxt += "<tr><td>" + tr("Latitude:") + "</td><td><b>" +
        WGSPoint::printPos(_wp->origP.x(),true) + "</b></td>" +
        "<td>" + tr("Longitude:") + "</td><td><b>" +
        WGSPoint::printPos(_wp->origP.y(),false) +
        "</b></td></tr>" +
        "</table>";

      if ((_wp->comment!=QString::null) && (_wp->comment!=""))
        {
          itxt += "<table cellpadding=5><tr><th align=left>" + tr("Comments") +
            "</th></tr><tr><td>" + _wp->comment + "</td></tr></table>";
        }

      itxt+="<!--/big--></html>";

      text->setText(itxt);
    }
}

/** Hide widget and return to the calling view in cumulusApp */
void WPInfoWidget::slot_SwitchBack()
{
  if( arrivalInfo )
    {
      // destroy the arrival widget, if exists
      disconnect( arrivalInfo, SIGNAL(close() ));
      arrivalInfo->slot_Close();
      arrivalInfo = 0;
    }

  timer->stop();
  text->clearFocus();
  hide();

  if( _lastView == CumulusApp::infoView )
    {
      // make sure last view isn't this view
      _lastView = CumulusApp::mapView;
    }

  cuApp->setView((CumulusApp::appView)_lastView);
}


/** This slot is called by the KeepOpen button to... yes... keep the dialog open. :-) */
void WPInfoWidget::slot_KeepOpen()
{
  timer->stop();
  cmdClose->setText(tr("Close"));
  cmdKeep->hide();
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

  emit waypointSelected(_wp, true);

  return slot_SwitchBack();
}


/** This slot is called if the Add Waypoint button is clicked. */
void WPInfoWidget::slot_addAsWaypoint()
{
  _wp->importance=wayPoint::High; //importance is high
  emit waypointAdded(_wp);

  cmdAddWaypoint->hide();
  slot_KeepOpen();
}


/** This slot is called if the Home button is clicked. */
void WPInfoWidget::slot_setAsHome()
{
  if( ! _wp )
    {
      return;
    }

  slot_KeepOpen(); // Stop timer

  int answer= QMessageBox::warning(this,tr("Set home site?"),
                                   tr("<html><b>Do you want to use site<br>%1<br>as your new home site?</b></html>").arg(_wp->name),
                                   QMessageBox::Ok | QMessageBox::Default,
                                   QMessageBox::Cancel | QMessageBox::Escape );

  if( answer == QMessageBox::Ok )
    {
      // Save new data as home position
      GeneralConfig *conf = GeneralConfig::instance();

      conf->setHomeWp(_wp);
      conf->save();
      emit newHomePosition( &_wp->origP );
    }

  slot_KeepOpen();
  cmdSetHome->hide();
}

/**
 * This slot is called if the arrival button is clicked.
 */
void WPInfoWidget::slot_arrival()
{
  // qDebug("WPInfoWidget::slot_arrival()");

  if( ! _wp )
    {
      return;
    }

  slot_KeepOpen(); // Stop timer

  // switch off all accelerator keys
  scClose->setEnabled(false);

  // create arrival info widget
  arrivalInfo = new TPInfoWidget( this );
  // arrivalInfo->setWindowModality(Qt::WindowModal);
  arrivalInfo->prepareArrivalInfoText( _wp );
  arrivalInfo->showTP( false );

  connect( arrivalInfo, SIGNAL(close()),
           this, SLOT(slot_arrivalClose()));
}

// sets focus back to wp text view after closing arrival widget
void WPInfoWidget::slot_arrivalClose()
{
  disconnect( arrivalInfo, SIGNAL(close() ));
  arrivalInfo = 0;

  // switch on close shortcut keys
  scClose->setEnabled(true);

  // get focus back
  text->setFocus();
  show();
}

/** get back the current state of cumulus. In flight true, otherwise false */
bool WPInfoWidget::inFlight()
{
  extern Calculator* calculator;
  extern GPSNMEA* gps;

  if( calculator->currentFlightMode() == Calculator::unknown ||
      calculator->currentFlightMode() == Calculator::standstill ||
      ! gps->getConnected() )
    {
      return false;
    }

  return true;
}

