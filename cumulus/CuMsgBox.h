/***********************************************************************
**
**   CuMsgBox.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2025 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#pragma once

#include <QMessageBox>

/**
 * \class CuMsgBox
 *
 * \author Axel Pauli
 *
 * \brief Slight modification of a QMessagebox.
 *
 * This is a slight modification of a QMessageBox. Because Android draws the
 * windows without a frame, a rectangle is drawn around the window borders.
 *
 * \date 2025
 *
 */
class CuMsgBox : public QMessageBox
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( CuMsgBox )

public:

  CuMsgBox( QWidget * parent = 0 );

protected:

  void paintEvent( QPaintEvent * event );
};
