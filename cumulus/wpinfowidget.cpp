/***********************************************************************
 **
 **   wpinfowidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Andre Somers, 2008 Axel Pauli
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

#include "cumulusapp.h"
#include "basemapelement.h"
#include "airport.h"
#include "wpinfowidget.h"
#include "generalconfig.h"
#include "cucalc.h"
#include "mapcontents.h"
#include "mapcalc.h"
#include "wgspoint.h"
#include "altitude.h"
#include "mapcontents.h"
#include "waypointcatalog.h"
#include "sonne.h"

extern MapConfig *_globalMapConfig;
extern MapContents  *_globalMapContents;
extern CuCalc       *calculator;

WPInfoWidget::WPInfoWidget( CumulusApp *parent ) :
  QWidget(parent),
  _wp(0)
{
  setObjectName("WPInfoWidget");
  cuApp = parent;
  _lastView = CumulusApp::mapView;
  arrivalInfo = 0;

  resize(parent->size());

  QFont bfont( "Helvetica", 12, QFont::Bold  );
  QFont font( "Helvetica", 12, QFont::Bold  );

  QBoxLayout *topLayout = new QVBoxLayout( this );
  text = new QTextEdit(this, "WaypointInfo");
  text->setReadOnly(true);
  text->setLineWrapMode(QTextEdit::WidgetWidth);
  topLayout->addWidget(text,10);

  int fontSize = this->font().pointSize();

  // qDebug("fontSize=%d", fontSize);

  if( fontSize <= 10 )
    {
      // Set bigger font in text view for a better readability
      text->setFont(QFont( "Helvetica", 14 ));
    }

  buttonrow2=new QHBoxLayout(topLayout);

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

  buttonrow1=new QHBoxLayout(topLayout);

  cmdClose = new QPushButton(tr("Close"), this);
  cmdClose->setFont(bfont);
  buttonrow1->addWidget(cmdClose);
  connect(cmdClose, SIGNAL(clicked()),
	  this, SLOT(slot_SwitchBack()));

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
  if( _wp ) delete _wp;
}

/** This slot get called on the timer timeout. */
void WPInfoWidget::slot_timeout()
{
  if(--_timerCount==0) {
    timer->stop();
    slot_SwitchBack();
  } else {
    QString txt;
    txt.sprintf(tr("Close (%d)"),_timerCount);
    cmdClose->setText(txt);
  }
}

/** This method is called by CumulusApp to set the view to
 * which there must be returned and the waypoint to view. */
bool WPInfoWidget::showWP(int lastView, const wayPoint *wp)
{
  extern MapContents* _globalMapContents;
  extern MapMatrix*   _globalMapMatrix;

  if( wp == 0 ) {
    return false;
  }

  // Check if new point is in waypoint list, so make sure we can add it.
  if (_globalMapContents->getIsInWaypointList(wp))
    cmdAddWaypoint->hide();
  else
    cmdAddWaypoint->show();

  // check, if current home position is different from waypoint
  QPoint home = _globalMapMatrix->getHomeCoord();

  if( home == wp->origP || inFlight() ) {
    cmdSetHome->hide();
  } else {
    cmdSetHome->show();
  }

  // Check if Waypoint is not selected, so make sure we can select
  // it.

  const wayPoint *calcWp = calculator->getselectedWp();

  if( calcWp ) {
    if( wp->origP == calcWp->origP ) {
      cmdUnselectWaypoint->show();
      cmdSelectWaypoint->hide();
    } else {
      cmdSelectWaypoint->show();
      cmdUnselectWaypoint->hide();
    }
  } else {
    cmdSelectWaypoint->show();
    cmdUnselectWaypoint->hide();
  }

  if( wp->taskPointIndex == 0 ) {
    // take-off task points are not select or unselectable
    cmdSelectWaypoint->hide();
    cmdUnselectWaypoint->hide();
  }

  // save new values. We make a deep copy to prevent problems with
  // constant methods elsewhere.
  if( _wp) delete _wp;
  _wp = new wayPoint(*wp);
  _lastView = lastView;

  writeText();

  // @AP: load the time from user configuration
  GeneralConfig *conf = GeneralConfig::instance();
  _timerCount = conf->getInfoDisplayTime();

  if( _timerCount > 0 ) {
    timer->start(1000);
    QString txt;
    txt.sprintf(tr("Close (%d)"), _timerCount);
    cmdClose->setText(txt);
    cmdKeep->show();
  } else {
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
  // resize to size of parent, could be changed in the meantime as the widget was hided
  resize(cuApp->size());
  // set focus to text widget
  text->setFocus();
}

/** This method actually fills the widget with the info. */
void WPInfoWidget::writeText()
{
  if( _wp == 0 ) {
    text->setText("<qt><big><center><b>" + tr("No waypoint selected") +
		  "</b></center></big></qt>");
  } else {
    //display info on waypoint
    QString itxt;
    QString tmp;
    int iTmp;
    itxt+= "<qt><big><center><b>" + _wp->description + "<br>(" + _wp->name;
    if (!_wp->icao.isEmpty())
      itxt+=",&nbsp;"+ _wp->icao;
    itxt+= ")</b></center></big><br>";
    itxt+="<b>" + BaseMapElement::item2Text(_wp->type, tr("(unknown)")) + "</b>";

    if (_wp->isLandable) {
      iTmp=_wp->surface;
      if( iTmp > 3 )
	iTmp = 0;
      // qDebug("_wp->surface %d", _wp->surface );
      if (iTmp<0)
	iTmp=0;
      QString tmp2;
      if( _wp->runway < 0 || _wp->runway > 360 ) {
	// 2250 is used as default for unknown
	tmp2=tr("Unknown");
      } else {
	// @AP: show runway in both directions, start with the lowest one
	int rw1 =_wp->runway;
	int rw2 = rw1 <= 180 ? rw1+180 : rw1-180;
	tmp2.sprintf("<b>%02d/%02d</b>", rw1 < rw2 ? rw1/10 : rw2/10, rw1 < rw2 ? rw2/10 : rw1/10);
      }

      itxt += tmp.sprintf( "<br><table><tr><td>" + tr("Runway: ") + "</td><td>" + tmp2 + ", %s</td></tr><br>" +
			   "<tr><td>" + tr("Length: ") + "</td><td><b>", Airport::item2Text(iTmp).toLatin1().data() );

      if( _wp->length <= 0 ) {
	itxt += tr("Unknown") + "</b></td></tr><br>";
      } else {
	itxt+=tmp.sprintf( "%d m</b></td></tr><br>", _wp->length );
      }
    } else {
      itxt+="<font color=\"#FF0000\"><b> " + tr("NOT LANDABLE") + "</b></font>" +
	"<table>";
    }

    if (_wp->frequency >= 117.97 && _wp->frequency <= 137.0 ) {
      itxt+=tmp.sprintf("<tr><td>"+tr("Frequency:")+"</td><td><b>%1.3f</b></td></tr><br>",_wp->frequency);
    } else {
      itxt+="<tr><td>"+tr("Frequency:")+"</td><td><b>"+tr("Unknown")+"</b></td></tr>";
    }

    itxt+=tmp.sprintf("<tr><td>"+tr("Elevation:") +
		      "</td><td><b>%s</b></td></tr>",
		      Altitude::getText(_wp->elevation, true, 0).latin1());

    QString sr, ss;
    QDate date = QDate::currentDate();

    // calculate Sunrise and Sunset
    bool res = Sonne::sonneAufUnter( sr, ss, date, _wp->origP , 0 );

    if( res )
      {
	// In some areas no results available. In this cas we skip
	// this output.
	itxt+=tmp.sprintf( "<tr><td>" + tr("Sunrise:") +
			   "</td><td><b>" +
			   "%s UTC</b></td></tr>",
			   sr.latin1() );

	itxt+=tmp.sprintf( "<tr><td>" + tr("Sunset:") +
			   "</td><td><b>" +
			   "%s UTC</b></td></tr>",
			   ss.latin1() );
      }

    itxt += "<tr><td>" + tr("Coord:")+"</td><td>" +
      WGSPoint::printPos(_wp->origP.x(),true) +
      "</td></tr> <tr><td>&nbsp;</td><td>" +
      WGSPoint::printPos(_wp->origP.y(),false) + "</td></tr>" +
      "</font></table>";

    if ((_wp->comment!=QString::null) && (_wp->comment!="")) {
      itxt+="<u>"+tr("Comments")+"</u><br>" + _wp->comment;
    }

    itxt+="</qt>";

    text->setText(itxt);
  }
}


/** Hide widget and return to the calling view in cumulusApp */
void WPInfoWidget::slot_SwitchBack()
{
  if( arrivalInfo ) {
    // destroy the arrival widget, user has pressed space button, that
    // means return from this widget.
    disconnect( arrivalInfo, SIGNAL(close() ));
    arrivalInfo->slot_Close();
    arrivalInfo = 0;       
  }

  timer->stop();
  text->clearFocus();
  hide();

  if( _lastView == CumulusApp::infoView ) {
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

  if( inFlight() ) {
    return slot_SwitchBack();
  }

  cmdUnselectWaypoint->hide();
  cmdSelectWaypoint->show();
  slot_KeepOpen();
}


/** This slot is called if the Select Waypoint button is clicked or an
    accelerator has been pressed. */
void WPInfoWidget::slot_selectWaypoint()
{
  // This slot can be called via an accelerator, e.g. Key_Space. If
  // the select button is not visible we will call the unselect
  // routine. Result is toggling between the two modes.
  if( cmdUnselectWaypoint->isVisible() ) {
    return slot_unselectWaypoint();
  }

  emit waypointSelected(_wp, true);

  if( inFlight() ) {
    return slot_SwitchBack();
  }

  cmdUnselectWaypoint->show();
  cmdSelectWaypoint->hide();
  slot_KeepOpen();
}


/** This slot is called if the Add as Waypoint button is clicked. */
void WPInfoWidget::slot_addAsWaypoint()
{
  _wp->importance=wayPoint::High; //importance is high
  emit waypointAdded(_wp);

  if( inFlight() ) {
    return slot_SwitchBack();
  }

  cmdAddWaypoint->hide();
  slot_KeepOpen();
}


/** This slot is called if the Home button is clicked. */
void WPInfoWidget::slot_setAsHome()
{
  if( ! _wp ) {
    return;
  }

  slot_KeepOpen(); // Stop timer

  int answer= QMessageBox::warning(this,tr("Set home site?"),
				   tr("Do you want to use site\n%1\nas your new home site?").arg(_wp->name),
				   QMessageBox::Ok | QMessageBox::Default,
				   QMessageBox::Cancel | QMessageBox::Escape );

  if( answer == QMessageBox::Ok ) {
    // Save new data as home position
    GeneralConfig *conf = GeneralConfig::instance();

    conf->setHomeWp(_wp);
    conf->save();
    emit newHomePosition( &_wp->origP );
  }

  if( inFlight() ) {
    return slot_SwitchBack();
  }

  slot_KeepOpen();
  cmdSetHome->hide();
}

/**
 * This slot is called if the arrival button is clicked.
 */
void WPInfoWidget::slot_arrival()
{
  if( ! _wp )
    {
      return;
    }

  slot_KeepOpen(); // Stop timer

  // switch off all accelerator keys
  cuApp->accInfoView->setEnabled( false );

  // create arrival info widget
  arrivalInfo = new TPInfoWidget( this );
  arrivalInfo->prepareArrivalInfoText( _wp );
  arrivalInfo->showTP( false );

  connect( arrivalInfo, SIGNAL(close()),
	  this, SLOT(slot_arrivalClose()));
}

// sets focus back to wp text view after closing arrival widget
void WPInfoWidget::slot_arrivalClose()
{
  arrivalInfo = 0;
  
  // switch on all accelerator keys
  cuApp->accInfoView->setEnabled( true );

  // get focus back
  text->setFocus();
}

/** get back the current state of cumulus. In flight true, otherwise false */
bool WPInfoWidget::inFlight()
{
  extern CuCalc* calculator;
  extern GPSNMEA* gps;

  if( calculator->currentFlightMode() == CuCalc::unknown ||
      calculator->currentFlightMode() == CuCalc::standstill ||
      ! gps->getConnected() ) {
    return false;
  }

  return true;
}

  
