/***********************************************************************
 **
 **   preflightweatherpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2013-2019 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <algorithm>

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
#include "whatsthat.h"

QHash<QString, QHash<QString, QString> > PreFlightWeatherPage::m_metarReports;

QHash<QString, QString> PreFlightWeatherPage::m_tafReports;

const QString PreFlightWeatherPage::MetarUrl = "https://tgftp.nws.noaa.gov/data/observations/metar/decoded/";

const QString PreFlightWeatherPage::TafUrl = "https://tgftp.nws.noaa.gov/data/forecasts/taf/stations/";

PreFlightWeatherPage::PreFlightWeatherPage( QWidget *parent ) :
  QWidget(parent),
  m_downloadManger(0),
  m_updateIsRunning(false),
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

#ifdef ANDROID
      // On Galaxy S3 there are size problems observed
      setMinimumSize( MainWindow::mainWindow()->size() );
      setMaximumSize( MainWindow::mainWindow()->size() );
#endif
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

#ifdef ANDROID
  QScrollBar* lvsb = m_list->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

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

  m_listUpdateButton = new QPushButton(tr("Update"), this);
  hbbox1->addWidget(m_listUpdateButton);
  connect (m_listUpdateButton, SIGNAL(clicked()), SLOT(slotRequestWeatherData()));

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

#ifdef ANDROID
  lvsb = m_display->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture(m_display->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_display->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  displayLayout->addWidget( m_display );

  QHBoxLayout* hbbox = new QHBoxLayout;
  displayLayout->addLayout( hbbox );

  m_detailsUpdateButton = new QPushButton(tr("Update"));
  hbbox->addWidget(m_detailsUpdateButton);
  connect (m_detailsUpdateButton, SIGNAL(clicked()), SLOT(slotRequestWeatherData()));

  hbbox->addSpacing( 10 );

  cmd = new QPushButton(tr("Close"));
  hbbox->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotShowListWidget()));

  //----------------------------------------------------------------------------
  // Editor widget for station adding.
  //----------------------------------------------------------------------------
  QVBoxLayout *editorLayout = new QVBoxLayout( m_editorWidget );

  editorLayout->addWidget( new QLabel(tr("Airport ICAO Code")), 0, Qt::AlignLeft );

  QHBoxLayout *inputLayout = new QHBoxLayout;
  editorLayout->addLayout( inputLayout );

  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "[a-zA-Z0-9]{4}|^$" ), this );

  Qt::InputMethodHints imh;
  m_airportEditor = new QLineEdit;
  m_airportEditor->setInputMethodHints(Qt::ImhUppercaseOnly | Qt::ImhDigitsOnly | Qt::ImhNoPredictiveText);
  m_airportEditor->setValidator( eValidator );

  connect( m_airportEditor, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  inputLayout->addWidget( m_airportEditor, 5 );
  inputLayout->addSpacing( 10 );

  cmd = new QPushButton(tr("Cancel"), this);
  inputLayout->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotShowListWidget()));

  inputLayout->addSpacing( 10 );

  cmd = new QPushButton(tr("Ok"), this);
  inputLayout->addWidget(cmd);
  connect (cmd, SIGNAL(clicked()), SLOT(slotAddAirport()));

  editorLayout->addStretch( 10 );

  //----------------------------------------------------------------------------
  loadAirportData( true );
  show();
}

PreFlightWeatherPage::~PreFlightWeatherPage()
{
}

void PreFlightWeatherPage::switchUpdateButtons( bool enable )
{
  m_listUpdateButton->setEnabled( enable );
  m_detailsUpdateButton->setEnabled( enable );
}

void PreFlightWeatherPage::loadAirportData( bool readFile, const QString& selectIcao )
{
  if( readFile == true )
    {
      readAirportIcaoNames();
    }

  m_list->clear();

  IcaoItem *item2select = 0;

  for( int i = 0; i < m_airportIcaoList.size(); i++ )
    {
      IcaoItem *item = new IcaoItem( m_airportIcaoList.at(i) );
      m_list->addTopLevelItem( item );

      if( m_airportIcaoList.at(i) == selectIcao )
        {
          // Remember the item to be selected.
          item2select = item;
        }

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

  if( item2select != 0 )
    {
      // Select the desired item in the list.
      m_list->setCurrentItem( item2select );
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
  QFile report(file);

  QString station = QFileInfo(file).baseName();

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

      int tempUnit = GeneralConfig::instance()->getUnitTemperature();
      int pressureUnit = GeneralConfig::instance()->getUnitAirPressure();

      while( !stream.atEnd() )
        {
          line = stream.readLine();
          lineNo++;

          // qDebug() << "line=" << line;

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
                  continue;
                }

              int idx = line.indexOf( "(" + station + ")" );

              if( idx > 0 )
                {
                  reportItems.insert( "station", line.left(idx - 1) );
                  continue;
                }

              reportItems.insert( "station", line );
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
                      continue;
                    }
                }

              reportItems.insert( "date", line );
              continue;
            }

          if( line.startsWith( "Wind: ") )
            {
              // Wind: from the NW (310 degrees) at 5 MPH (4 KT):0
              // Wind: from the W (280 degrees) at 5 MPH (4 KT) (direction variable):0
              // Wind: from the ESE (120 degrees) at 20 MPH (17 KT) gusting to 32 MPH (28 KT) (direction variable):0
              line.replace( " degrees", QChar(Qt::Key_degree) );

              if( line.endsWith( ":0") )
                {
                  line.chop(2);
                }

              // Remove the line beginning.
              line = line.mid(6);
              int loop = 6;

              while( loop-- )
                {
                  QRegExp re = QRegExp("[0-9]+ MPH \\([0-9]+ KT\\)");

                  int idx1 = line.indexOf(re, 0);

                  if( idx1 == -1 )
                    {
                      // No speed value found.
                      break;
                    }

                  int idx2 = line.indexOf( "(", idx1 + 1 );
                  int idx3 = line.indexOf( " KT)", idx1 + 1 );

                  if( idx2 > 0 && idx2 < idx3 )
                    {
                      bool ok;
                      double ws = line.mid(idx2 + 1, idx3 - idx2 -1).toDouble(&ok);

                      if( ok )
                        {
                          Speed speed(0);
                          speed.setKnot( ws );

                          QString wsText = speed.getWindText( true, 0 );

                          line = line.left(idx1) + wsText + line.mid(idx3 + 4);
                        }
                    }
                }

              reportItems.insert( "wind", line );
              continue;
            }

          if( line.startsWith( "Visibility: ") )
            {
              // Visibility: greater than 7 mile(s):0
              // Visibility: less than 1 mile:0
              // Visibility: 3/4 mile(s):0
              if( line.contains( " mile") )
                {
                  int idx2  = line.lastIndexOf( " mile" );
                  int idx1  = line.lastIndexOf( " ", idx2 - 1 );

                  if( idx1 > 0 && idx1 < idx2 )
                    {
                      bool ok = false;
                      double visiDouble = 0.0;

                      QString visiText = line.mid(idx1 + 1, idx2 - idx1 -1);

                      if( visiText.contains("/") )
                        {
                          QStringList visiList = visiText.split("/");

                          if( visiList.size() == 2 )
                            {
                              double arg1 = visiList.at(0).toDouble(&ok);

                              if( ok )
                                {
                                  double arg2 = visiList.at(1).toDouble(&ok);

                                  if( ok && arg2 > 0.0 )
                                    {
                                      visiDouble = arg1 / arg2;
                                    }
                                }
                            }
                        }
                      else
                        {
                          visiDouble = visiText.toDouble( &ok );
                        }

                      if( ok )
                        {
                          Distance distance(0);
                          distance.setMiles( visiDouble );

                          QString visibility = line.mid( 12, idx1 - 11 );

                          if( distance.getKilometers() > 5 )
                            {
                              visibility += distance.getText( true, 0 );
                            }
                          else
                            {
                              visibility += distance.getText( true, 1 );
                            }

                          if( line.contains("mile(s)") )
                            {
                              // This must be tested as first to prevent wrong handling!
                              if( ! line.endsWith( "mile(s):0") )
                                {
                                  line.replace( ":0", "" );
                                  visibility += line.mid( line.indexOf( "mile(s)" ) + 7 );
                                }
                            }
                          else if( line.contains("mile") && ! line.endsWith( "mile:0") )
                            {
                              if( ! line.endsWith( "mile:0") )
                                {
                                  line.replace( ":0", "" );
                                  visibility += line.mid( line.indexOf( "mile" ) + 4 );
                                }
                            }

                          reportItems.insert( "visibility", visibility );
                          continue;
                        }
                    }
                }

              reportItems.insert( "visibility", line.mid(12) );
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
              if( tempUnit == GeneralConfig::Fahrenheit )
                {
                  // Temperature in F
                  int idx = line.indexOf( " F (" );

                  if( idx > 0 )
                    {
                      reportItems.insert("temperature", line.mid(13, idx-13 ) + QChar(Qt::Key_degree) + "F");
                      continue;
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
                      continue;
                    }
                }

              reportItems.insert("temperature", line.mid( 13 ) );
              continue;
            }

          if( line.startsWith( "Dew Point: ") )
            {
              // Dew Point: 42 F (6 C)
              if( tempUnit == GeneralConfig::Fahrenheit )
                {
                  // Dew point in F
                  int idx = line.indexOf( " F (" );

                  if( idx > 0 )
                    {
                      reportItems.insert("dewPoint", line.mid(11, idx-11) + QChar(Qt::Key_degree) + "F");
                      continue;
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
                      continue;
                    }
                }

              reportItems.insert("dewPoint", line.mid( 11 ) );
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
              if( pressureUnit == GeneralConfig::inHg )
                {
                  // QNH in inch Hg
                  int idx = line.lastIndexOf( " (" );

                  if( idx > 22 )
                    {
                      reportItems.insert("qnh", line.mid(22, idx - 22 ));
                      continue;
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
                      continue;
                    }
                }

              reportItems.insert("qnh", line.mid( 22 ));
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

      // qDebug() << "ReportItems:" << reportItems;

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

      // qDebug() << "TAFs:" << m_tafReports;
    }

  if( m_displayWidget->isVisible() )
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

      if( item->getIcao() == station )
        {
          // The display widget is visible and must be updated too, if new
          // station data are available.
          slotDetails();
        }
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

void PreFlightWeatherPage::slotAddAirport()
{
  QString icao = m_airportEditor->text().toUpper();

  if( icao.size() != 4 )
    {
      QMessageBox mb( QMessageBox::Critical,
                      tr( "Name?" ),
                      tr( "Station name requires 4 characters!" ),
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

  if( m_airportIcaoList.contains( icao ) )
    {
      // The airport to be added was already added.
      slotShowListWidget();
      return;
    }

  m_airportIcaoList.append( icao );
  std::sort( m_airportIcaoList.begin(), m_airportIcaoList.end() );
  storeAirportIcaoNames();
  loadAirportData( false, icao );

  // Download the weather for the new added station.
  QList<QString> stations;
  stations << icao;

  downloadWeatherData( stations );
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

  // Remove downloaded reports of the airport station.
  QString dataDir = GeneralConfig::instance()->getUserDataDirectory();
  QString fn      = item->getIcao() + ".TXT";

  QFile::remove( dataDir + "/weather/METAR/" + fn );
  QFile::remove( dataDir + "/weather/TAF/" + fn );

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

  QString& icao = item->getIcao();

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
  if( m_updateIsRunning == true )
    {
      // Do not allow multiple calls, if download is already running.
      return;
    }

  // This slot can be called from the list and from the display details widget.
  // So we have to determine at first, who was the caller.
  QList<QString> stations;

  if( m_listWidget->isVisible() )
    {
      stations = m_airportIcaoList;

      // Clear all displayed data to show the download progress.
      m_metarReports.clear();
      m_tafReports.clear();
      loadAirportData();
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
      m_display->clear();
    }

  if( stations.size() )
    {
      downloadWeatherData( stations );
    }
}

void PreFlightWeatherPage::downloadWeatherData( QList<QString>& stations )
{
  if( stations.size() == 0 )
    {
      return;
    }

  if( m_updateIsRunning == true )
    {
      // Do not allow multiple calls, if download is already running.
      return;
    }

  // set update marker
  m_updateIsRunning = true;

  // Disable update buttons.
  switchUpdateButtons( false );

  if( m_downloadManger == 0 )
    {
      m_downloadManger = new DownloadManager(this);

      connect( m_downloadManger, SIGNAL(finished( int, int )),
               this, SLOT(slotDownloadsFinished( int, int )) );

      connect( m_downloadManger, SIGNAL(networkError()),
               this, SLOT(slotNetworkError()) );

      connect( m_downloadManger, SIGNAL(fileDownloaded(QString&)),
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

      m_downloadManger->downloadRequest( urlMetar, destMetar, false );

      QString urlTaf = TafUrl + fn;
      QString destTaf = dir.absolutePath() + "/weather/TAF/" + fn;

      m_downloadManger->downloadRequest( urlTaf, destTaf, false );
    }
}

void PreFlightWeatherPage::slotNetworkError()
{
  // A network error has occurred. We delete the download manager to get faster
  // a new connection.
  m_downloadManger->deleteLater();
  m_downloadManger = 0;

  QString msg = QString(tr("<html>Network error occurred!<br>Is the Internet connection down?</html>"));

  QMessageBox mb( QMessageBox::Warning,
                  tr("Network Error"),
                  msg,
                  QMessageBox::Ok,
                  this );

#ifdef ANDROID

  mb.show();
  QPoint pos = QPoint( width()/2  - mb.width()/2, height()/2 - mb.height()/2 );
  mb.move( pos );

#endif

  mb.exec();

  m_updateIsRunning = false;
  switchUpdateButtons( true );
}

void PreFlightWeatherPage::slotDownloadsFinished( int /* requests */, int errors )
{
  QString msg = "<html><br><br>&nbsp;" +
                 QString(tr("All updates with %1 error(s) done.")).arg(errors) +
                 "&nbsp;<br><br></html>";

  // Show result message for 1s
  WhatsThat *mb = new WhatsThat( this, msg, 1000 );
  mb->show();

  // Move window into center of parent.
  QPoint pos = QPoint( width()/2  - mb->width()/2, height()/2 - mb->height()/2 );
  mb->move( pos );

  m_updateIsRunning = false;
  switchUpdateButtons( true );
}

void PreFlightWeatherPage::slotClose()
{
  // Close the hole widget.
  emit closingWidget();
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

  std::sort( m_airportIcaoList.begin(), m_airportIcaoList.end() );
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
