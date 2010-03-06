/***********************************************************************
**
**   proxydialog.h
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

/*
 * This widget asks for a proxy name and port. On accept the settings are checked
 * for correctness and stored in the GeneralConfig data.
 */

#ifndef PROXY_DIALOG_H

#include <QDialogButtonBox>
#include <QDialog>
#include <QLineEdit>
#include <QString>

class ProxyDialog : public QDialog
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( ProxyDialog )

 public:

  ProxyDialog( QWidget *parent = 0 );

  virtual ~ProxyDialog();

 protected:

  /** standard slots */
  virtual void accept();
  virtual void reject();

  private slots:

  /** User has clicked a button. */
  void clicked( QAbstractButton *button );

 private:

  /** Editor for host input */
  QLineEdit *hostEdit;

  /** Editor for port input */
  QLineEdit *portEdit;

  /** The dialog button box */
  QDialogButtonBox *buttonBox;
};

#endif /* PROXY_DIALOG_H */
