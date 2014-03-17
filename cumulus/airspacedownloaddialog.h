/***********************************************************************
**
**   airspacedownloaddialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2014 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class AirspaceDownloadDialog
 *
 * \author Axel Pauli
 *
 * \brief This dialog manages the download of an airspace file via HTTP.
 *
 * \date 2010-2014
 *
 * \version $Id$
 *
 */

#ifndef AIRSPACE_DOWNLOAD_DIALOG_H
#define AIRSPACE_DOWNLOAD_DIALOG_H

#include <QWidget>
#include <QLineEdit>
#include <QStringList>

class AirspaceDownloadDialog : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( AirspaceDownloadDialog )

 public:

  AirspaceDownloadDialog( QWidget *parent = 0 );

  virtual ~AirspaceDownloadDialog();

 private slots:

  /**
  * Called if the Ok button is pressed.
  */
  void slotAccept();

  /**
   * Called if download button is pressed.
   */
  void slotDownload();

 signals:

  /**
   * Called, to trigger a download of openAIP airspace files from the Internet.
   *
   * \param openAipCountryList he list of countries to be downloaded.
   */
  void downloadAirspaces( const QStringList& openAipCountryList );

 private:

  /**
   * Checks the passed countries and returns a list of single countries.
   *
   * \param countries Countries to be checked
   *
   * \param countrylist List containing single countries calculated by this method
   *
   * \return true in case of success otherwise false
   */
  bool checkCountryList( const QString& countries, QStringList& countrylist );

  /** Editor for airspace countries */
  QLineEdit *m_editCountries;
};

#endif /* AIRSPACE_DOWNLOAD_DIALOG_H */
