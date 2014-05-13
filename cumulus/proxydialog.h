/***********************************************************************
**
**   proxydialog.h
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
 * \class ProxyDialog
 *
 * \author Axel Pauli
 *
 * \brief Dialog for entering proxy name and port.
 *
 * This widget asks for a proxy name and port. On accept the settings are checked
 * for correctness and stored in the GeneralConfig data. The widget is realized
 * as a modal tool window to prevent the close of the widget, if return is
 * entered by the user.
 *
 * \date 2010-2014
 */

#ifndef PROXY_DIALOG_H

#include <QDialogButtonBox>
#include <QFrame>
#include <QLineEdit>
#include <QString>

class ProxyDialog : public QFrame
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( ProxyDialog )

 public:

  ProxyDialog( QWidget *parent = 0 );

  virtual ~ProxyDialog();

 private slots:

  /**
   * Called if the Ok button is pressed.
   */
  void accept();

  /**
   * Called if the Cancel button is pressed.
   */
  void reject();

  /** User has clicked a button. */
  void clicked( QAbstractButton *button );

  /**
   * Called to request the close of the Software Input Panel (SIP).
   */
  void slotCloseSip();

 signals:

  /**
   * Emitted, if the user has pressed the ok button.
   */
  void proxyDataChanged();

 private:

  /** Editor for host input */
  QLineEdit *hostEdit;

  /** Editor for port input */
  QLineEdit *portEdit;

  /** The dialog button box */
  QDialogButtonBox *buttonBox;
};

#endif /* PROXY_DIALOG_H */
