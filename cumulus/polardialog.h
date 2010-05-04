/***********************************************************************
**
**   polardialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef POLAR_DIALOG_H
#define POLAR_DIALOG_H

#include <QDialog>

#include "speed.h"
#include "polar.h"

/**
 * \author Eggert Ehmke
 *
 * \brief Class to handle glider polar changes done by the user.
 */
class PolarDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( PolarDialog )

public:

  PolarDialog( Polar&, QWidget* );

  virtual ~PolarDialog();

public slots:

  void slot_keyup();
  void slot_keydown();
  void slot_shiftkeyup();
  void slot_shiftkeydown();
  void slot_keyleft();
  void slot_keyright();
  void slot_keyhome();

protected:

  virtual void paintEvent (QPaintEvent*);
  virtual void mousePressEvent( QMouseEvent* );

private:

  Polar _polar;
  Speed wind;
  Speed lift;
  Speed mc;
};

#endif
