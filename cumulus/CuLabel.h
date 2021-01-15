/***********************************************************************
**
**   CuLabel.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012-2021 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef CuLabel_h
#define CuLabel_h

#include <QEvent>
#include <QLabel>

/**
 * \class CuLabel
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Slight modification of a QLabel.
 *
 * This is a slight modification of a QLabel. It adds a mousePress event.
 *
 * \date 2002-2021
 *
 */
class CuLabel : public QLabel
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( CuLabel )

public:

  CuLabel ( QWidget *parent, Qt::WindowFlags flags=Qt::Widget );

  CuLabel ( const QString &text,
            QWidget *parent,
            Qt::WindowFlags flags=Qt::Widget );

signals:

   /** Emitted when the mouse is pressed over the label */
  void mousePress();

protected:

  void mousePressEvent( QMouseEvent *event );
};

#endif
