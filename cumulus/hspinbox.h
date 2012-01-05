/***********************************************************************
**
**   hspinbox.h
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
 * \class HSpinBox
 *
 * \author Axel Pauli
 *
 * \brief A spinbox layout with buttons at the left and right side of the display.
 *
 * A spinbox layout with buttons at the left and right side of the display. The
 * included spinbox can be retrieved by using the \ref spinBox method.
 *
 * \date 2012
 *
 * \version $Id$
 *
 */

#ifndef H_SPINBOX_H_
#define H_SPINBOX_H_

#include <QAbstractSpinBox>
#include <QWidget>

class QPushButton;

class HSpinBox : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( HSpinBox )

 public:

  /**
   * \param spinBox The instance of a spinbox, derived from QAbstractSpinBox.
   *
   * \param parent The parent widget.
   */
  HSpinBox( QAbstractSpinBox* spinBox, QWidget* parent=0 );

  virtual ~HSpinBox();

  /** @return The spinbox instance of this widget. */
  QAbstractSpinBox* spinBox()
  {
    return _spinBox;
  }

 protected:

  void showEvent( QShowEvent *event );

 private slots:

  /** Called if the increase button is pressed. */
  void slotPlusPressed();

  /** Called if the decrease button is pressed. */
  void slotMinusPressed();

 private:

  /** The included spinbox. */
  QAbstractSpinBox* _spinBox;

  /** Increase button. */
  QPushButton* plus;

  /** Decrease button. */
  QPushButton* minus;
};

#endif
