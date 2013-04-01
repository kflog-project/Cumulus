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
#include <ctime>

#include <QtGui>
#include <QMessageBox>

#include "altitude.h"
#include "basemapelement.h"
#include "gpsnmea.h"
#include "generalconfig.h"
#include "calculator.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "mapcalc.h"
#include "sonne.h"
#include "wgspoint.h"
#include "wpeditdialog.h"
#include "wpinfowidget.h"

extern Calculator *calculator;
extern MapContents* _globalMapContents;

WPInfoWidget::WPInfoWidget( MainWindow *parent ) :
  QWidget(parent)
{
  setObjectName("WPInfoWidget");
  m_mainWindow = parent;
  m_returnView = MainWindow::mapView;
  m_homeChanged = false;
  m_editedWpIsTarget = false;

  resize( parent->size() );

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
  connect(cmdAddWaypoint, SIGNAL(clicked()), SLOT(slot_addAsWaypoint()));

  cmdHome = new QPushButton(tr("Home"), this);
  cmdHome->setFont(bfont);
  buttonrow2->addWidget(cmdHome);
  connect(cmdHome, SIGNAL(clicked()), SLOT(slot_setNewHome()));

  cmdArrival = new QPushButton(tr("Arrival"), this);
  cmdArrival->setFont(bfont);
  buttonrow2->addWidget(cmdArrival);
  connect(cmdArrival, SIGNAL(clicked()), SLOT(slot_arrival()));

  cmdEdit = new QPushButton(tr("Edit"), this);
  cmdEdit->setFont(bfont);
  buttonrow2->addWidget(cmdEdit);
  connect(cmdEdit, SIGNAL(clicked()), SLOT(slot_edit()));

  cmdDelete = new QPushButton(tr("Delete"), this);
  cmdDelete->setFont(bfont);
  buttonrow2->addWidget(cmdDelete);
  connect(cmdDelete, SIGNAL(clicked()), SLOT(slot_delete()));

  buttonrow1=new QHBoxLayout;
  topLayout->addLayout(buttonrow1);

  cmdClose = new QPushButton(tr("Close"), this);
  cmdClose->setFont(bfont);
  buttonrow1->addWidget(cmdClose);
  connect(cmdClose, SIGNAL(clicked()), SLOT(slot_SwitchBack()));

  // Activate keyboard shortcut cancel to close the window too
  scClose = new QShortcut( this );

#ifndef ANDROID
  scClose->setKey( Qt::Key_Escape );
  connect( scClose, SIGNAL(activated()), SLOT( slot_SwitchBack() ));
#endif

  cmdKeep = new QPushButton(tr("Stop"), this);
  cmdKeep->setFont(bfont);
  buttonrow1->addWidget(cmdKeep);
  connect(cmdKeep, SIGNAL(clicked()), SLOT(slot_KeepOpen()));

  cmdUnselectWaypoint = new QPushButton(tr("Unselect"), this);
  cmdUnselectWaypoint->setFont(bfont);
  buttonrow1->addWidget(cmdUnselectWaypoint);
  connect(cmdUnselectWaypoint, SIGNAL(clicked()), SLOT(slot_unselectWaypoint()));

  cmdSelectWaypoint = new QPushButton(tr("Select"), this);
  cmdSelectWaypoint->setFont(bfont);
  buttonrow1->addWidget(cmdSelectWaypoint);
  connect(cmdSelectWaypoint, SIGNAL(clicked()), SLOT(slot_selectWaypoint()));

  m_timer=new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), SLOT(slot_timeout()));
}

WPInfoWidget::~WPInfoWidget()
{
}

/** This slot get called on the timer timeout. */
void WPInfoWidget::slot_timeout()
{
  if( --m_timerCount == 0 )
    {
      m_timer->stop();
      slot_SwitchBack();
    }
  else
    {
      QString txt = tr("Close (%1)").arg(m_timerCount);
      cmdClose->setText(txt);
    }
}

/** This method is called by the MainWindow to set the view to
 * which there must be returned and the waypoint to view. */
bool WPInfoWidget::showWP( int returnView, const Waypoint& wp )
{
  // qDebug() << "WPInfoWidget::showWP(): returnView=" << returnView << "WP=" << wp.name;

  // save return view
  m_returnView = returnView;

  // save passed waypoint
  m_wp = wp;

  // Check if new point is already in the waypoint list, so make sure we can add it.
  if( _globalMapContents->isInWaypointList(wp.origP) )
    {
      cmdAddWaypoint->setVisible( false );
    }
  else
    {
      cmdAddWaypoint->setVisible( true );
    }

  // Reset home changed
  m_homeChanged = false;

  // check, if current home position is different from waypoint
  GeneralConfig *conf = GeneralConfig::instance();
  QPoint home = conf->getHomeCoord();

  bool moving = ( calculator->moving() ||
                  ( calculator->currentFlightMode() != Calculator::unknown &&
                    calculator->currentFlightMode() != Calculator::standstill ) );

  // Show the home button only if we are not to fast in move to avoid
  // wrong usage. The redefinition of the home position can trigger
  // a reload of the airfield list.
  if( home == wp.origP || moving )
    {
      cmdHome->setVisible( false );
    }
  else
    {
      cmdHome->setVisible( true );
    }

  cmdArrival->setVisible( false );
  cmdEdit->setVisible( false );
  cmdDelete->setVisible( false );

  if( moving )
    {
      cmdArrival->setVisible( true );
    }
  else
    {
      // Show only these command buttons, if the point is a real waypoint.
      if( wp.taskPointIndex == -1 && wp.wpListMember == true )
        {
          cmdEdit->setVisible( true );
          cmdDelete->setVisible( true );
        }
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

  // @AP: load the display time from user configuration
  m_timerCount = conf->getInfoDisplayTime();

  if( m_timerCount > 0 )
    {
      m_timer->start(1000);
      QString txt = tr("Close (%1)").arg(m_timerCount);
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
  // Resize to size of parent, could be changed in the meantime as the widget was hidden.
  resize(m_mainWindow->size());
}

/** This method actually fills the widget with the info to be displayed. */
void WPInfoWidget::writeText()
{
  if( m_wp.name.isEmpty() )
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

      itxt+= "<html><center><b>" + m_wp.description + " (" + m_wp.name;

      if( !m_wp.icao.isEmpty() )
        {
          itxt += ",&nbsp;" + m_wp.icao;
        }

      if( !m_wp.country.isEmpty() )
        {
          itxt += ",&nbsp;" + m_wp.country;
        }

      itxt+= ")<p>" + BaseMapElement::item2Text(m_wp.type, tr("(unknown)"));

      if( m_wp.isLandable )
        {
          itxt += "</b></center>";

          QString tmp2 = tr("Unknown");

          // @AP: show runway in both directions
          uint rwh1 = m_wp.runway / 256;
          uint rwh2 = m_wp.runway % 256;

          // qDebug("wp.rw=%d rwh1=%d rwh2=%d", m_wp.runway, rwh1, rwh2);

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
                  Runway::item2Text(m_wp.surface, tr("Unknown")) + ")</td>" +
                  "<td>" + tr("Length: ") + "</td><td><b>";

          if( m_wp.length <= 0 )
            {
              itxt += tr("Unknown") + "</b></td></tr>";
            }
          else
            {
              tmp = QString("%1 m</b></td></tr>").arg(m_wp.length);
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
      QString meters = Altitude::getText(m_wp.elevation, true, 0);

      Altitude::setUnit(Altitude::feet);
      QString feet = Altitude::getText(m_wp.elevation, true, 0);

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

       if( m_wp.frequency > 0.0 )
         {
           tmp = QString("<td>" + tr("Frequency:") + "</td><td><b>%1&nbsp;MHz</b></td></tr>").arg(m_wp.frequency, 0, 'f', 3);
           itxt += tmp;
         }
       else
         {
           itxt+="<tr><td>&nbsp;</td><td>&nbsp;</td></tr>";
         }

      QString sr, ss, tz;
      QDate date = QDate::currentDate();

      // calculate Sunrise and Sunset
      bool res = Sonne::sonneAufUnter( sr, ss, date, m_wp.origP , tz );

      if( res )
        {
        // In some areas are no results available. In this case we skip
        // the times output.
          itxt += QString( "<tr><td>" + tr("Sunrise:") + "</td><td><b>" + sr +
                           " " + tz + "</b></td>" );

          itxt += QString( "<td>" + tr("Sunset:") + "</td><td><b>" + ss +
                           " " + tz + "</b></td></tr>" );
        }

      QString lat = WGSPoint::printPos(m_wp.origP.x(),true);
      QString lon = WGSPoint::printPos(m_wp.origP.y(),false);

      lat = lat.replace(QRegExp(" "), "&nbsp;");
      lon = lon.replace(QRegExp(" "), "&nbsp;");

      itxt += "<tr><td>" + tr("Latitude:") + "</td><td><b>" +
              lat + "</b></td>" +
              "<td>" + tr("Longitude:") + "</td><td><b>" +
              lon +
              "</b></td></tr>";

      if( m_wp.comment.isEmpty() == false )
        {
          itxt += "<tr><td>" + tr("Comment") + ":</td>"
                  "<td colspan=3>" + m_wp.comment + "</td></tr>";
        }

      itxt+="</table></html>";

      text->setHtml(itxt);
    }
}

/** Hide widget and return to the calling view in MainWindow */
void WPInfoWidget::slot_SwitchBack()
{
  m_timer->stop();
  text->clearFocus();
  setVisible( false );

  // Return to the calling view
  m_mainWindow->setView( (MainWindow::appView) m_returnView );

  // Check, if we have no GPS fix. In this case we do move the map
  // to the new home position.
  if( m_homeChanged == true && GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      emit gotoHomePosition();
      m_homeChanged = false;
    }
}

/** This slot is called by the KeepOpen button to... yes... keep the dialog open. :-) */
void WPInfoWidget::slot_KeepOpen()
{
  m_timer->stop();
  cmdClose->setText( tr( "Close" ) );
  cmdKeep->setVisible( false );
}

/** This slot is called if the unselect Waypoint button is clicked. */
void WPInfoWidget::slot_unselectWaypoint()
{
  emit selectWaypoint(0, true);

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

  emit selectWaypoint(&m_wp, true);

  return slot_SwitchBack();
}

/** This slot is called if the Add Waypoint button is clicked. */
void WPInfoWidget::slot_addAsWaypoint()
{
  m_wp.priority = Waypoint::High; //priority is high
  m_wp.wpListMember = true;
  emit addWaypoint(m_wp);

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
                  tr( "Use point<br><b>%1</b><br>as your home site?").arg(m_wp.name) +
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
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents |
                                         QEventLoop::ExcludeSocketNotifiers );

      QCoreApplication::flush();

      // save new home position and elevation
      GeneralConfig *conf = GeneralConfig::instance();
      conf->setHomeCountryCode( m_wp.country );
      conf->setHomeName( m_wp.name );
      conf->setHomeCoord( m_wp.origP );
      conf->setHomeElevation( Distance(m_wp.elevation) );

      emit newHomePosition( m_wp.origP );
      m_homeChanged = true;
      cmdHome->setVisible( false );
   }
}

/**
 * This slot is called if the arrival button is clicked.
 */
void WPInfoWidget::slot_arrival()
{
  slot_KeepOpen(); // Stop timer

  // switch off all accelerator keys
  scClose->setEnabled(false);

  // create arrival info widget
  TPInfoWidget *arrivalInfo = new TPInfoWidget( this );
  arrivalInfo->prepareArrivalInfoText( &m_wp );
  arrivalInfo->showTP( false );

  connect( arrivalInfo, SIGNAL(closed()), SLOT(slot_arrivalClose()));
}

// sets focus back to wp text view after closing arrival widget
void WPInfoWidget::slot_arrivalClose()
{
  // switch on close shortcut key
  scClose->setEnabled(true);
}

void WPInfoWidget::slot_edit()
{
  slot_KeepOpen(); // Stop timer

  // Search the waypoint object in the global waypoint list.
  Waypoint* wplelem = _globalMapContents->getWaypointFromList( &m_wp );

  if( wplelem == 0 )
    {
      // No waypoint object found in the global waypoint list.
      return;
    }

  // Check, if waypoint is set in calculator as target. Then we must
  // update the selection after the editing is finished.
  const Waypoint* calcWp = calculator->getselectedWp();

  if( calcWp != 0 && m_wp == *calcWp )
    {
      m_editedWpIsTarget = true;
    }
  else
    {
      m_editedWpIsTarget = false;
    }

  WpEditDialog *dlg = new WpEditDialog( this, wplelem );

  connect( dlg, SIGNAL(wpListChanged(Waypoint &)), this,
            SLOT(slot_edited(Waypoint &)) );

  dlg->setVisible( true );
}

void WPInfoWidget::slot_edited( Waypoint& wp )
{
  // Update display widget after edition of waypoint.
  showWP( m_returnView, wp );

  slot_KeepOpen(); // Stop timer again

  if( m_editedWpIsTarget == true )
    {
      // Update the target waypoint in the calculator
      emit selectWaypoint(&wp, true);
    }

  if( m_returnView == MainWindow::mapView || m_returnView == MainWindow::infoView )
    {
      // The map has called the info view so we must store only the global
      // waypoint list now after the edit.
      _globalMapContents->saveWaypointList();
    }
  else
    {
      // Propagate the waypoint change to the waypoint list view.
      emit waypointEdited( wp );
    }
}

void WPInfoWidget::slot_delete()
{
  slot_KeepOpen(); // Stop timer

  QMessageBox mb( QMessageBox::Question,
                  tr( "Delete?" ),
                  tr( "Delete waypoint" ) + " " + m_wp.name + "?",
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::Yes )
    {
      emit deleteWaypoint( m_wp );
      return slot_SwitchBack();
    }
}
