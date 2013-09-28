/***********************************************************************
**
**   preflightweatherpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreflightWeatherPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for download and display of TAF/METAR reports.
 *
 * A widget for download and display of TAF/METAR reports.
 *
 * \date 2013
 *
 * \version $Id$
 */

#ifndef PREFLIGHT_WEATHER_PAGE_H_
#define PREFLIGHT_WEATHER_PAGE_H_

#include <QWidget>
#include <QLineEdit>
#include <QHash>
#include <QList>
#include <QString>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "downloadmanager.h"

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

  /** Called to clear the airport editor. */
  void slotClearAirportEditor();

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

  /** Loads the airport data into the list. */
  void loadAirportData( bool readFile=false );

  /** Reads the airport ICAO identifiers from the file. */
  bool readAirportIcaoNames();

  /** Stores the airport ICAO identifiers in a file. */
  bool storeAirportIcaoNames();

  /** Updates the ICAO list item with the METAR observation data. */
  void updateIcaoItem( QString& icao );

  /** Widget which shows/manages the METAR-TAF overview */
  QWidget* m_listWidget;

  /** Widget which shows the METAR-TAF details. */
  QWidget* m_displayWidget;

  /** Widget which provides an input editor. */
  QWidget* m_editorWidget;

  /** METAR-TAF list */
  QTreeWidget* m_list;

  /** METAR-TAF detail display */
  QTextEdit*   m_display;

  /** Airport editor */
  QLineEdit* m_airportEditor;

  /** List which stores the airport icao identifiers */
  QList<QString> m_airportIcaoList;

  /** Manager to handle downloads of METAR-TAF data. */
  DownloadManager* m_downloadManger;

  /** Fixed string label for no METAR data available. */
  const QString NoMetar;

  /** Fixed string label for no TAF data available. */
  const QString NoTaf;

  static QHash<QString, QHash<QString, QString> > m_metarReports;

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
