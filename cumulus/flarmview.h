/***********************************************************************
**
**   flarmview.h
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
 * \brief Flarm Radar and Operation view.
 *
 * This widget shows the Flarm radar view with the operation buttons.
 *
 */

#ifndef FLARM_VIEW_H
#define FLARM_VIEW_H

#include <QWidget>

class FlarmDisplay;

class FlarmView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmView )

public:

  /**
   * Constructor
   */
  FlarmView( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~FlarmView();

private slots:

  /** Called to report widget closing. */
  void slotClosed();

  /** Called if zoom level shall be changed. */
  void slotZoom();

signals:

  /** Emitted if the close button was pressed. */
  void closed();

private:

  FlarmDisplay* display;
};

#endif /* FLARM_VIEW_H */
