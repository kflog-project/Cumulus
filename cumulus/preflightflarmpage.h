/***********************************************************************
**
**   preflightflarmpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreflightFlarmPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for pre-flight Flarm settings.
 *
 * A widget for pre-flight Flarm settings.
 *
 * \date 2012
 *
 * \version $Id$
 */

#ifndef PREFLIGHT_FLARM_PAGE_H_
#define PREFLIGHT_FLARM_PAGE_H_

#include <QWidget>

#include "flarm.h"

class QComboBox;
class QLabel;
class QLineEdit;

class PreFlightFlarmPage : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightFlarmPage )

 public:

  PreFlightFlarmPage(QWidget *parent=0);

  virtual ~PreFlightFlarmPage();

 private slots:

 /** Called to update version info. */
  void slotUpdateVersions( const Flarm::FlarmVersion& info );

  /** Called to update error info. */
  void slotUpdateErrors( const Flarm::FlarmError& info );

 private:

  QLabel*    hwVersion;
  QLabel*    swVersion;
  QLabel*    obstVersion;
  QLabel*    errSeverity;
  QLabel*    errCode;
  QComboBox* logInt;
  QLineEdit* pilot;
  QLineEdit* copil;
  QLineEdit* gliderId;
  QLineEdit* gliderType;
  QLineEdit* compId;
  QLineEdit* compClass;
};

#endif
