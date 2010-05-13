/***********************************************************************
**
**   airspacedownloaddialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief This dialog manages the download of an airspace file via HTTP.
 *
*/

#ifndef AIRSPACE_DOWNLOAD_DIALOG_H
#define AIRSPACE_DOWNLOAD_DIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
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

  /**
   * Opens proxy dialog on user request.
   */
 void slot_editProxy();

  signals:

  void downloadAirspace( QString &url );

 private:

  /** Dialog editor for proxy host and port input. */
  QPushButton *editProxy;

  /** Label to show the current proxy settings. */
  QLabel *proxyDisplay;

  /** Editor for airspace url input */
  QLineEdit *editAirspaceUrl;

  /** The dialog button box */
  QDialogButtonBox *buttonBox;
};

#endif /* AIRSPACE_DOWNLOAD_DIALOG_H */
