/***********************************************************************
**
**   preflightweatherpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class PreflightWeatherPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for download and display of TAF/METAR reports.
 *
 * A widget for download and display of TAF/METAR reports. Three different
 * widgets (overview, adding and display details) are combined in this class
 * to avoid popup windows. The weather data are downloaded from the NOAA
 * server in the USA.
 *
 * \date 2013-2014
 *
 * \version 1.1
 */

#ifndef PREFLIGHT_WEATHER_PAGE_H_
#define PREFLIGHT_WEATHER_PAGE_H_

#include <QWidget>
#include <QLineEdit>
#include <QHash>
#include <QList>
#include <QPushButton>
#include <QString>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "DownloadManager.h"


class PreFlightWeatherPage : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightWeatherPage )

 public:

  PreFlightWeatherPage( QWidget *parent=0 );

  virtual ~PreFlightWeatherPage();

 public slots:

  /** Called, if a new weather report is available. */
  void slotNewWeaterReport( QString& file );

 private slots:

  /** Called to show the airport editor. */
  void slotShowAirportEditor();

  /** Called to add a new airport to the list. */
  void slotAddAirport();

  /** Called to remove the selected airport from the list. */
  void slotDeleteAirport();

  /** Called to show the details of the selected airport from the list. */
  void slotDetails();

  /** Called to requests the weather data from the NOAA server. */
  void slotRequestWeatherData();

  /** Called if the list widget is closed. */
  void slotClose();

  /** Called to show the list widget. */
  void slotShowListWidget();

  /** Called, if a download error has occurred. */
  void slotNetworkError();

  /** Called, if all downloads are finished. */
  void slotDownloadsFinished( int requests, int errors );

 signals:

  /**
   * Emitted, if the widget is closed.
   */
  void closingWidget();

 private:

  /** Loads the airport data into the list.
   *
   * \param readFile If set to true, the airport file with the stored ICAO
   *        identifiers is read. Otherwise the data is taken only from the
   *        m_airportIcaoList.
   *
   * \param select If the argument is not an empty string, the ICAO identifier
   *        in the list is selected.
   */
  void loadAirportData( bool readFile=false, const QString& selectIcao="" );

  /** Reads the airport ICAO identifiers from the file. */
  bool readAirportIcaoNames();

  /** Stores the airport ICAO identifiers in a file. */
  bool storeAirportIcaoNames();

  /**
   * Download the weather data from the stations in the list.
   *
   * \param stations ICAO identifiers of airport stations to be requested
   */
  void downloadWeatherData( QList<QString>& stations );

  /** Updates the ICAO list item with the METAR observation data. */
  void updateIcaoItem( QString& icao );

  /** Enable/disable updates buttons */
  void switchUpdateButtons( bool enable );

  /** Widget which shows/manages the METAR-TAF overview */
  QWidget* m_listWidget;

  /** Widget which shows the METAR-TAF details. */
  QWidget* m_displayWidget;

  /** Widget which provides an input editor. */
  QWidget* m_editorWidget;

  /** METAR-TAF list */
  QTreeWidget* m_list;

  /** METAR-TAF detail display */
  QTextEdit* m_display;

  /** Airport editor */
  QLineEdit* m_airportEditor;

  /** Push button to request list updates.*/
  QPushButton* m_listUpdateButton;

  /** Push button to request details update.*/
  QPushButton* m_detailsUpdateButton;

  /** List which stores the airport ICAO identifiers */
  QList<QString> m_airportIcaoList;

  /** Manager to handle downloads of METAR-TAF data. */
  DownloadManager* m_downloadManger;

  /** Flag to mark a running update action. */
  bool m_updateIsRunning;

  /** Fixed string label for no METAR data available. */
  const QString NoMetar;

  /** Fixed string label for no TAF data available. */
  const QString NoTaf;

  /**
   * A hash table containing METAR reports. The key is the airport ICAO code,
   * the value is a hash table with the report attributes. The following keys
   * are in use:
   *
   * station, date, wind, visibility, sky, weather, temperature, dewPoint,
   * humidity, qnh, observation
   */
  static QHash<QString, QHash<QString, QString> > m_metarReports;

  /**
   * A hash table containing TAF reports. The key is the airport ICAO code,
   * the value is the whole TAF report as string with new lines.
   */
  static QHash<QString, QString> m_tafReports;

  /** URL for METAR request */
  static const QString MetarUrl;

  /** URL for TAF request */
  static const QString TafUrl;

  /**
   * \class IcaoItem
   *
   * \author Axel Pauli
   *
   * \brief A user ICAO item element used by the QTreeWidget m_list
   *
   * \date 2013
   *
   * \version $Id$
   */

  class IcaoItem : public QTreeWidgetItem
    {
      public:

        IcaoItem( const QString& icaoIn ) :
          QTreeWidgetItem(QTreeWidgetItem::UserType),
          icao(icaoIn)
          {
          }

        virtual ~IcaoItem() {};

        QString& getIcao()
          {
            return icao;
          }

      private:

        /** The airport's ICAO name. */
        QString icao;
    };
};

#endif
