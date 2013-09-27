/***********************************************************************
 **
 **   preflightweatherpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "mainwindow.h"
#include "preflightweatherpage.h"
#include "speed.h"

// Timeout in ms for waiting for response
#define RESP_TO 30000

QHash<QString, QHash<QString, QString> > PreFlightWeatherPage::m_metarReports;

QHash<QString, QString> PreFlightWeatherPage::m_tafReports;

const QString PreFlightWeatherPage::MetarUrl = "http://weather.noaa.gov/pub/data/observations/metar/decoded/";

const QString PreFlightWeatherPage::TafUrl = "http://weather.noaa.gov/pub/data/forecasts/taf/stations/";

PreFlightWeatherPage::PreFlightWeatherPage( QWidget *parent ) :
  QWidget(parent),
  m_downloadManger(0),
  NoMetar(tr("No METAR available")),
  NoTaf(tr("No TAF available"))
{
  setObjectName("PreFlightWeatherPage");
  setWindowTitle(tr("METAR and TAF"));
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( MainWindow::mainWindow() )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( MainWindow::mainWindow()->size() );
    }

  QVBoxLayout *mainLayout  = new QVBoxLayout( this );
  m_listWidget             = new QWidget( this );
  m_displayWidget          = new QWidget( this );
  m_editorWidget           = new QWidget( this );

  mainLayout->addWidget( m_listWidget );
  mainLayout->addWidget( m_displayWidget );
  mainLayout->addWidget( m_editorWidget );

  m_displayWidget->hide();
  m_editorWidget->hide();

  //----------------------------------------------------------------------------
  // List widget
  //----------------------------------------------------------------------------
  QVBoxLayout *listLayout = new QVBoxLayout( m_listWidget );

  m_list = new QTreeWidget;
  m_list->setRootIsDecorated( false );
  m_list->setItemsExpandable( false );
  m_list->setSortingEnabled( true );
  m_list->setSelectionMode( QAbstractItemView::SingleSelection );
  m_list->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_list->setAlternatingRowColors(true);
  m_list->setColumnCount( 1 );
  m_list->setFocusPolicy( Qt::StrongFocus );
  m_list->setUniformRowHeights(true);
  m_list->setHeaderLabel( tr( "METAR and TAF" ) );

  m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture(m_list->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_list->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  listLayout->addWidget( m_list );

  QHBoxLayout* hbbox1 = new QHBoxLayout;
  listLayout->addLayout( hbbox1 );

  QPushButton* cmd = new QPushButton(tr("Add"), this);
  hbbox1->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotShowAirportEditor()));

  hbbox1->addSpacing( 10 );

  cmd = new QPushButton(tr("Update"), this);
  hbbox1->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotRequestWeatherData()));

  hbbox1->addSpacing( 10 );

  cmd = new QPushButton(tr("Details"), this);
  hbbox1->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotDetails()));

  QHBoxLayout* hbbox2 = new QHBoxLayout;
  listLayout->addLayout( hbbox2 );

  cmd = new QPushButton(tr("Delete"), this);
  hbbox2->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotDeleteAirport()));

  hbbox2->addSpacing( 10 );

  cmd = new QPushButton(tr("Close"), this);
  hbbox2->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotClose()));

  //----------------------------------------------------------------------------
  // Display widget for report details
  //----------------------------------------------------------------------------
  QVBoxLayout *displayLayout = new QVBoxLayout( m_displayWidget );
  m_display = new QTextEdit;
  m_display->setReadOnly( true );

#ifdef QSCROLLER
  QScroller::grabGesture(m_display->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_display->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  displayLayout->addWidget( m_display );

  QHBoxLayout* hbbox = new QHBoxLayout;
  displayLayout->addLayout( hbbox );

  cmd = new QPushButton(tr("Update"), this);
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotRequestWeatherData()));

  hbbox->addSpacing( 10 );

  cmd = new QPushButton(tr("Close"), this);
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotShowListWidget()));

  //----------------------------------------------------------------------------
  // Editor widget for station management.
  //----------------------------------------------------------------------------
  QVBoxLayout *editorLayout = new QVBoxLayout( m_editorWidget );

  editorLayout->addWidget( new QLabel(tr("Airport ICAO Code")), 0, Qt::AlignLeft );

  QHBoxLayout *inputLayout = new QHBoxLayout;
  editorLayout->addLayout( inputLayout );

  Qt::InputMethodHints imh;
  m_airportEditor = new QLineEdit;
  imh = (m_airportEditor->inputMethodHints() | Qt::ImhNoPredictiveText);
  m_airportEditor->setInputMethodHints(imh);
  m_airportEditor->setInputMask("NNNN");
  inputLayout->addWidget( m_airportEditor );
  inputLayout->addSpacing( 10 );

  cmd = new QPushButton( "<-]" , this);
  inputLayout->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotClearAirportEditor()));

  editorLayout->addStretch( 10 );

  hbbox = new QHBoxLayout;
  editorLayout->addLayout( hbbox );

  cmd = new QPushButton(tr("Ok"), this);
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotAddAirport()));

  hbbox->addSpacing( 10 );

  cmd = new QPushButton(tr("Cancel"), this);
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotShowListWidget()));

  //----------------------------------------------------------------------------
  loadAirportData( true );
  show();
}

PreFlightWeatherPage::~PreFlightWeatherPage()
{
}

void PreFlightWeatherPage::loadAirportData( bool readFile )
{
  if( readFile == true )
    {
      readAirportIcaoNames();
    }

  m_list->clear();

  for( int i = 0; i < m_airportIcaoList.size(); i++ )
    {
      IcaoItem *item = new IcaoItem( m_airportIcaoList.at(i) );
      m_list->addTopLevelItem( item );

      QString station     = "";
      QString observation = "";

      if( m_metarReports.contains(m_airportIcaoList.at(i)) )
        {
          station     = m_metarReports.value( m_airportIcaoList.at(i) ).value( "station" );
          observation = m_metarReports.value( m_airportIcaoList.at(i) ).value( "observation" );
        }

      QString text = "<html>" + m_airportIcaoList.at(i);

      if( ! station.isEmpty() )
        {
          text += ": " + station;
        }

      text += "<br><font size=\"1\"><i>";

      if( observation.isEmpty() )
        {
          text += NoMetar;
        }
      else
        {
          text += observation;
        }

      text += "</i></font></html>";

      QLabel *label = new QLabel( text );

      // Don't set this property, it hides the widget's grid.
      // label->setAutoFillBackground( true );

      m_list->setItemWidget( item, 0, label );
    }

  slotShowListWidget();
}

/* METAR examples
Berlin-Schoenefeld, Germany (EDDB) 52-23N 013-31E 50M
Sep 26, 2013 - 03:50 AM EDT / 2013.09.26 0750 UTC
Wind: from the NNE (020 degrees) at 13 MPH (11 KT):0
Visibility: 3 mile(s):0
Sky conditions: mostly cloudy
Weather: light drizzle
Temperature: 46 F (8 C)
Dew Point: 46 F (8 C)
Relative Humidity: 100%
Pressure (altimeter): 29.83 in. Hg (1010 hPa)
ob: EDDB 260750Z 02011KT 6000 -DZ SCT003 BKN006 08/08 Q1010 TEMPO BKN010
cycle: 8

Station name not available
Aug 18, 2013 - 09:30 PM EDT / 2013.08.19 0130 UTC
Sky conditions: partly cloudy
Temperature: 59 F (15 C)
Dew Point: 57 F (14 C)
Relative Humidity: 93%
Pressure (altimeter): 30.39 in. Hg (1029 hPa)
ob: OPSD 190130Z CALM 09KM P/CLOUDY FEW015ST SCT035 Q1029/30.41 15/14C RH 95 PERCENT A/V OH CLEAR
cycle: 1
*/

void PreFlightWeatherPage::slotNewWeaterReport( QString& file )
{
  qDebug() << "slotNewWeaterReport():" << file;

  QFile report(file);

  QString station = QFileInfo(file).baseName();

  qDebug() << "station=" << station;

  if( file.contains("/weather/METAR/") )
    {
      if ( ! report.open( QIODevice::ReadOnly ) )
        {
          // could not open file ...
          qWarning() << "Cannot open file: " << report.fileName();
          return;
        }

      // METAR report received
      QTextStream stream( &report );
      QString line;

      QHash<QString, QString> reportItems;
      reportItems.insert( "icao", station );

      int lineNo = 0;

      while( !stream.atEnd() )
        {
          line = stream.readLine();
          lineNo++;

          qDebug() << "line=" << line;

          if( line.trimmed().isEmpty() )
            {
              // ignore comment and empty lines
              continue;
            }

          if( lineNo == 1 )
            {
              // Line 1: station name
              // Berlin-Schoenefeld, Germany (EDDB) 52-23N 013-31E 50M
              // Station name not available
              if( line.startsWith( "Station name not available" ) )
                {
                  reportItems.insert( "station", tr("Station name not available") );
                }
              else
                {
                  int idx = line.indexOf( "(" + station + ")" );

                  if( idx > 0 )
                    {
                      reportItems.insert( "station", line.left(idx - 1) );
                    }
                }

              continue;
            }

          if( lineNo == 2 )
            {
              // Line 2: Date and time
              // Sep 26, 2013 - 11:50 AM EDT / 2013.09.26 1550 UTC
              if( line.endsWith(" UTC") )
                {
                  int idx = line.indexOf( " / " );

                  if( idx > 0 && line.size() > (idx + 3) )
                    {
                      QString date = line.mid(idx + 3, 13) + ":" +
                                     line.mid(idx + 3 + 13);

                      reportItems.insert( "date", date );
                    }
                  else
                    {
                      reportItems.insert( "date", line );
                    }
                }
              else
                {
                  reportItems.insert( "date", line );
                }

              continue;
            }

          if( line.startsWith( "Wind: ") )
            {
              // Wind: from the NW (310 degrees) at 5 MPH (4 KT):0
              // Wind: from the W (280 degrees) at 5 MPH (4 KT) (direction variable):0
              line.replace( " degrees", QChar(Qt::Key_degree) );

              int idx2  = line.lastIndexOf( " KT)" );
              int idx1  = line.lastIndexOf( "(", idx2 - 1 );
              int idxAt = line.lastIndexOf( " at " );

              if( idxAt > 0 && idx1 > 0 && idx1 < idx2 )
                {
                  QString wind = line.mid(6, idxAt + 4 - 6 );

                  bool ok;
                  double ws = line.mid(idx1 + 1, idx2 - idx1 -1).toDouble(&ok);

                  if( ! ok )
                    {
                      reportItems.insert( "wind", line.mid(6) );
                      continue;
                    }

                  Speed speed(0);
                  speed.setKnot( ws );

                  wind += speed.getWindText( true, 0 );

                  if( ! line.endsWith( "KT):0"))
                    {
                      line.replace( ":0", "" );
                      wind += line.mid( line.indexOf( " KT)" ) + 4 );
                    }

                  reportItems.insert( "wind", wind );
                }
              else
                {
                  reportItems.insert( "wind", line.mid(6) );
                }

              continue;
            }

          if( line.startsWith( "Visibility: ") )
            {
              // Visibility: greater than 7 mile(s):0
              if( line.contains( " mile(s)") )
                {
                  int idx2  = line.lastIndexOf( " mile(s)" );
                  int idx1  = line.lastIndexOf( " ", idx2 - 1 );

                  if( idx1 > 0 && idx1 < idx2 )
                    {
                      bool ok;
                      double visi = line.mid(idx1 + 1, idx2 - idx1 -1).toDouble(&ok);

                      if( ok )
                        {
                          Distance distance(0);
                          distance.setMiles( visi );

                          QString visibility = line.mid( 12, idx1 - 11 );
                          visibility += distance.getText( true, 0 );

                          reportItems.insert( "visibility", visibility );
                        }
                    }
                  else
                    {
                      reportItems.insert( "visibility", line.mid(12) );
                    }
                }
              else
                {
                  reportItems.insert( "visibility", line.mid(12) );
                }

              continue;
            }

          if( line.startsWith( "Sky conditions: ") )
            {
              // Sky conditions: partly cloudy or mostly cloudy
              reportItems.insert( "sky", line.mid(16) );
              continue;
            }

          if( line.startsWith( "Weather: ") )
            {
              reportItems.insert( "weather", line.mid(9) );
              continue;
            }

          if( line.startsWith( "Temperature: ") )
            {
              // Temperature: 51 F (11 C)
              if( qgetenv("LANG").startsWith("en_US") )
                {
                  // Temperature in F
                  int idx = line.indexOf( " F (" );

                  if( idx > 0 )
                    {
                      reportItems.insert("temperature", line.mid(13, idx-13 ) + QChar(Qt::Key_degree) + "F");
                    }
                }
              else
                {
                  // Temperature in C
                  int idx2 = line.lastIndexOf( " C)" );
                  int idx1 = line.lastIndexOf( "(", idx2 -1 );

                  if( idx1 > 0 && idx1+1 < idx2 )
                    {
                      reportItems.insert("temperature", line.mid( idx1+1, idx2-idx1-1 ) + QChar(Qt::Key_degree) + "C");
                    }
                }

              continue;
            }

          if( line.startsWith( "Dew Point: ") )
            {
              // Dew Point: 42 F (6 C)
              if( qgetenv("LANG").startsWith("en_US") )
                {
                  // Dew point in F
                  int idx = line.indexOf( " F (" );

                  if( idx > 0 )
                    {
                      reportItems.insert("dewPoint", line.mid(11, idx-11) + QChar(Qt::Key_degree) + "F");
                    }
                }
              else
                {
                  // Dew point in C
                  int idx2 = line.lastIndexOf( " C)" );
                  int idx1 = line.lastIndexOf( "(", idx2 - 1 );

                  if( idx1 > 0 && idx1+1 < idx2 )
                    {
                      reportItems.insert("dewPoint", line.mid( idx1+1, idx2-idx1-1 ) + QChar(Qt::Key_degree) + "C");
                    }
                }

              continue;
            }

          if( line.startsWith( "Relative Humidity: ") )
            {
              // Relative Humidity: 71%
              reportItems.insert( "humidity", line.mid(19) );
              continue;
            }

          if( line.startsWith( "Pressure (altimeter): ") )
            {
              // Pressure (altimeter): 30.00 in. Hg (1016 hPa)
              if( qgetenv("LANG").startsWith("en_US") )
                {
                  // QNH in inch Hg
                  int idx = line.lastIndexOf( " (" );

                  if( idx > 22 )
                    {
                      reportItems.insert("qnh", line.mid(22, idx - 22 ));
                    }
                }
              else
                {
                  // QHN in hPa
                  int idx2 = line.lastIndexOf( " hPa)" );
                  int idx1 = line.lastIndexOf( "(", idx2 - 1 );

                  if( idx1 > 0 && idx1+2 < idx2 )
                    {
                      reportItems.insert("qnh", line.mid( idx1 + 1, idx2 - idx1 + 3 ));
                    }
                }

              continue;
            }

          if( line.startsWith( "ob: ") )
            {
              // Extract the observation line from the report.
              // ob: EDDB 261550Z 31004KT 9999 FEW030 SCT056 11/06 Q1016 NOSIG
              reportItems.insert( "observation", line.mid(4) );
              continue;
            }
        }

      report.close();

      qDebug() << "ReportItems:" << reportItems;

      m_metarReports.insert( station, reportItems );
      updateIcaoItem( station );
    }
  else if( file.contains("/weather/TAF/") )
    {
      /* TAF Example
      2013/09/26 12:29
      TAF EDDF 261100Z 2612/2718 32008KT 9999 SCT035
            BECMG 2617/2619 04005KT
      */

      // TAF report received. The whole report is stored in the hash as one string.
      if ( ! report.open( QIODevice::ReadOnly ) )
        {
          // could not open file ...
          qWarning() << "Cannot open file: " << report.fileName();
          return;
        }

      // TAF report received
      QTextStream stream( &report );
      QString line;
      QString tafReport;

      int lineNo = 0;

      while( !stream.atEnd() )
        {
          line = stream.readLine();
          lineNo++;

          if( line.trimmed().isEmpty() || lineNo == 1 )
            {
              // ignore comment and empty lines
              // ignore line 1, it contains the date and time
              continue;
            }

          if( tafReport.isEmpty() )
            {
              tafReport = line;
            }
          else
            {
              tafReport += "\n" + line;
            }
        }

      report.close();

      m_tafReports.insert( station, tafReport );

      qDebug() << "TAFs:" << m_tafReports;
    }

  if( m_displayWidget->isVisible() )
    {
      // The display widget is visible and must be updated too.
      slotDetails();
    }
}

void PreFlightWeatherPage::updateIcaoItem( QString& icao )
{
  for( int i = 0; i < m_list->topLevelItemCount(); i++ )
    {
      QTreeWidgetItem *item = m_list->topLevelItem( i );

      if( item == 0 )
        {
          continue;
        }

      IcaoItem *ii = dynamic_cast<IcaoItem *>( item );

      if( ii == 0 )
        {
          continue;
        }

      if( ii->getIcao() == icao )
        {
          // We found the station in the list.
          QLabel* label = dynamic_cast<QLabel *>(m_list->itemWidget( item, 0 ));

          if( label == 0 )
            {
              break;
            }

          // Sets the station's name and its observation in the overview list.
          QString station     = "";
          QString observation = "";

          if( m_metarReports.contains( icao ) )
            {
              station     = m_metarReports.value( icao ).value( "station" );
              observation = m_metarReports.value( icao ).value( "observation" );
            }

          QString text = "<html>" + m_airportIcaoList.at(i);

          if( ! station.isEmpty() )
            {
              text += ": " + station;
            }

          text += "<br><font size=\"1\"><i>";

          if( observation.isEmpty() )
            {
              text += NoMetar;
            }
          else
            {
              text += observation;
            }

          text += "</i></font></html>";

          label->setText( text );
          break;
        }
    }
}

void PreFlightWeatherPage::slotShowAirportEditor()
{
  m_listWidget->hide();
  m_displayWidget->hide();

  m_editorWidget->show();
  m_airportEditor->clear();
  m_airportEditor->setFocus();
}

void PreFlightWeatherPage::slotClearAirportEditor()
{
  m_airportEditor->clear();
  m_airportEditor->setFocus();
}

void PreFlightWeatherPage::slotAddAirport()
{
  QString icao = m_airportEditor->text().toUpper();

  if( icao.size() != 4 )
    {
      QMessageBox mb( QMessageBox::Critical,
                      tr( "Name?" ),
                      tr( "Station name requires\n4 characters!" ),
                      QMessageBox::Ok,
                      this );

    #ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

    #endif

      mb.exec();
      return;
    }

  if( m_airportIcaoList.contains( m_airportEditor->text().toUpper()) )
    {
      // The airport to be added was already added.
      slotShowListWidget();
      return;
    }

  m_airportIcaoList.append( icao );
  qSort( m_airportIcaoList );
  storeAirportIcaoNames();
  loadAirportData();
}

void PreFlightWeatherPage::slotDeleteAirport()
{
  if( m_list->topLevelItemCount() == 0 )
    {
      return;
    }

  IcaoItem *item = dynamic_cast<IcaoItem *>( m_list->currentItem() );

  if( item == 0 )
    {
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Delete?" ),
                  QString(tr( "Confirm delete of station %1" ).arg(item->getIcao())),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2 - mb.width()/2, height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::No )
    {
      return;
    }

  m_airportIcaoList.removeOne( item->getIcao() );
  storeAirportIcaoNames();
  loadAirportData( false );
}

void PreFlightWeatherPage::slotDetails()
{
  if( m_list->topLevelItemCount() == 0 )
    {
      return;
    }

  IcaoItem *item = dynamic_cast<IcaoItem *>( m_list->currentItem() );

  if( item == 0 )
    {
      return;
    }

  QString &icao = item->getIcao();

  // Hide the list widget
  m_listWidget->hide();

  // Hide the editor widget
  m_editorWidget->hide();

  // Show the display widget
  m_displayWidget->show();

  m_display->clear();

  QString text = "<html><b>" +
                 tr("METAR and TAF") +
                 ": " + icao
                 + "</b><br><br>\n";

  if( ! m_metarReports.contains( icao ) )
    {
      m_display->setHtml( text +
                          NoMetar + "<br><br>" + NoTaf +
                          "</html>" );
      return;
    }

  QHash<QString, QString> metar = m_metarReports.value( icao );

  if( metar.contains("date") )
    {
      text += tr("Date: ") + metar.value("date") + "<br>\n";
    }

  if( metar.contains("wind") )
    {
      text += tr("Wind: ") + metar.value("wind") + "<br>\n";
    }

  if( metar.contains("visibility") )
    {
      text += tr("Visibility: ") + metar.value("visibility") + "<br>\n";
    }

  if( metar.contains("sky") )
    {
      text += tr("Sky: ") + metar.value("sky") + "<br>\n";
    }

  if( metar.contains("weather") )
    {
      text += tr("Weather: ") + metar.value("weather") + "<br>\n";
    }

  if( metar.contains("temperature") )
    {
      text += tr("Temperature: ") + metar.value("temperature") + "<br>\n";
    }

  if( metar.contains("dewPoint") )
    {
      text += tr("Dew Point: ") + metar.value("dewPoint") + "<br>\n";
    }

  if( metar.contains("humidity") )
    {
      text += tr("Relative Humidity: ") + metar.value("humidity") + "<br>\n";
    }

  if( metar.contains("qnh") )
    {
      text += tr("QNH: ") + metar.value("qnh") + "<br><br>\n";
    }

  if( metar.contains("observation") )
    {
      text += metar.value("observation") + "<br><br>\n";
    }

  if( m_tafReports.contains(icao) )
    {
      QString taf = m_tafReports.value(icao);
      taf.replace("\n", "<br>");
      text += taf + "<br>\n";
    }
  else
    {
      text += NoTaf + "<br>\n";
    }

  text += "</html>\n";

  m_display->setHtml( text );
}

void PreFlightWeatherPage::slotRequestWeatherData()
{
  // This slot can be called from the list and from the display details widget.
  // So we have to determine at first, who was the caller.
  QList<QString> stations;

  if( m_listWidget->isVisible() )
    {
      stations = m_airportIcaoList;
    }
  else if( m_displayWidget->isVisible() )
    {
      if( m_list->topLevelItemCount() == 0 )
        {
          return;
        }

      IcaoItem *item = dynamic_cast<IcaoItem *>( m_list->currentItem() );

      if( item == 0 )
        {
          return;
        }

      stations.append( item->getIcao() );
    }

  if( stations.size() == 0 )
    {
      return;
    }

  if( m_downloadManger == static_cast<DownloadManager *> (0) )
    {
      m_downloadManger = new DownloadManager(this);

      connect( m_downloadManger, SIGNAL(finished( int, int )),
               this, SLOT(slotDownloadsFinished( int, int )) );

      connect( m_downloadManger, SIGNAL(networkError()),
               this, SLOT(slotNetworkError()) );

      connect( m_downloadManger, SIGNAL(weatherDownloaded(QString&)),
               this, SLOT(slotNewWeaterReport(QString&)) );

      // connect( m_downloadManger, SIGNAL(status( const QString& )),
      //         _globalMapView, SLOT(slot_info( const QString& )) );
    }

  // Create download destination directories
  QDir dir( GeneralConfig::instance()->getUserDataDirectory() );
  dir.mkdir( "weather");
  dir.mkdir( "weather/METAR");
  dir.mkdir( "weather/TAF");

  for( int i = 0; i < stations.size(); i++ )
    {
      QString fn = stations.at(i) + ".TXT";
      QString urlMetar = MetarUrl + fn;
      QString destMetar = dir.absolutePath() + "/weather/METAR/" + fn;

      m_downloadManger->downloadRequest( urlMetar, destMetar );

      QString urlTaf = TafUrl + fn;
      QString destTaf = dir.absolutePath() + "/weather/TAF/" + fn;

      m_downloadManger->downloadRequest( urlTaf, destTaf );
    }
}

void PreFlightWeatherPage::slotNetworkError()
{
  QString msg = QString(tr("Network error occurred!\nIs the Internet connection down?"));

  QMessageBox mb( QMessageBox::Warning,
                  tr("Network Error"),
                  msg,
                  QMessageBox::Ok,
                  this );

#ifdef ANDROID

  mb.show();
  QPoint pos = MainWindow::mainWindow()->mapToGlobal(QPoint( MainWindow::mainWindow()->width()/2  - mb.width()/2,
                                                             MainWindow::mainWindow()->height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  mb.exec();
}

void PreFlightWeatherPage::slotDownloadsFinished( int /* requests */, int errors )
{
  QString msg = QString(tr("All update(s) with %1 error(s) done.")).arg(errors);

  QMessageBox mb( QMessageBox::Information,
                  tr("All updates finished"),
                  msg,
                  QMessageBox::Ok,
                  this );

#ifdef ANDROID

  mb.show();
  QPoint pos = MainWindow::mainWindow()->mapToGlobal(QPoint( MainWindow::mainWindow()->width()/2  - mb.width()/2,
                                                             MainWindow::mainWindow()->height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  mb.exec();
}

void PreFlightWeatherPage::slotClose()
{
  // Close the hole widget.
  close();
}

void PreFlightWeatherPage::slotShowListWidget()
{
  // Show the list widget
  m_listWidget->show();

  // Hide the display widget
  m_displayWidget->hide();

  // Hide the editor widget
  m_editorWidget->hide();
}

bool PreFlightWeatherPage::readAirportIcaoNames()
{
  QFile f( GeneralConfig::instance()->getUserDataDirectory() +
           "/weather/airport_icao_names.txt" );

  if ( ! f.open( QIODevice::ReadOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      return false;
    }

  // remove all old data before read
  m_airportIcaoList.clear();

  QTextStream stream( &f );
  QString line;

  while ( !stream.atEnd() )
    {
      line = stream.readLine();

      if ( line.startsWith("#") || line.startsWith("$") || line.trimmed().isEmpty() )
        {
          // ignore comment and empty lines
          continue;
        }

      // Alias names are limited to MaxAliasLength characters
      m_airportIcaoList.append( line );
    }

  f.close();

  qDebug() << m_airportIcaoList.size() << "entries read from" << f.fileName();

  qSort( m_airportIcaoList );
  return true;
}

bool PreFlightWeatherPage::storeAirportIcaoNames()
{
  QDir dir( GeneralConfig::instance()->getUserDataDirectory() );
  dir.mkdir( "weather");

  QFile f( GeneralConfig::instance()->getUserDataDirectory() +
           "/weather/airport_icao_names.txt" );

  if ( ! f.open( QIODevice::WriteOnly ) )
    {
      // could not open file ...
      qWarning() << "Cannot open file: " << f.fileName();
      return false;
    }

  QTextStream stream( &f );

  // writing file-header
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString("yyyy-MM-dd hh:mm:ss");

  stream << "# Cumulus airport ICAO names file created at "
         << dtStr
         << " by Cumulus "
         << QCoreApplication::applicationVersion() << endl;

  QListIterator<QString> it(m_airportIcaoList);

  while( it.hasNext() )
    {
      stream << it.next() << endl;
    }

  f.close();
  return true;
}
