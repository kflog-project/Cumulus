/***********************************************************************
**
**   airspacedownloaddialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2012 Axel Pauli
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
 * \date 2010-2012
 *
 * \version $Id$
 *
 */

#ifndef AIRSPACE_DOWNLOAD_DIALOG_H
#define AIRSPACE_DOWNLOAD_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

class AirspaceDownloadDialog : public QDialog
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( AirspaceDownloadDialog )

 public:

  AirspaceDownloadDialog( QWidget *parent = 0 );

  virtual ~AirspaceDownloadDialog();

 protected:

  /** standard slots */
  virtual void accept();
  virtual void reject();

 private slots:

  signals:

  void downloadAirspace( QString &url );

 private:

  /** Editor for airspace url input */
  QLineEdit *editAirspaceUrl;

  /** The dialog button box */
  QDialogButtonBox *buttonBox;

  /** The state of the SIP */
  bool m_autoSip;
};

#endif /* AIRSPACE_DOWNLOAD_DIALOG_H */
