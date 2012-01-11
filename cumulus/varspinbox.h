/***********************************************************************
**
**   varspinbox.h
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
 * \class VarSpinBox
 *
 * \author Axel Pauli
 *
 * \brief A variable spinbox layout with buttons at the left and right side or on
 *        bottom and top of the display.
 *
 * A variable spinbox layout with buttons at the left-right side or bottom-top
 *  of the display. The included spinbox can be retrieved by using the \ref spinBox
 * method. The text of the spinbox is aligned as AlignHCenter.
 *
 * \date 2012
 *
 * \version $Id$
 *
 */

#ifndef VAR_SPINBOX_H_
#define VAR_SPINBOX_H_

#include <QAbstractSpinBox>
#include <QWidget>

class QPushButton;

class VarSpinBox : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( VarSpinBox )

 public:

  /**
   * The button order for the spinbox. Can be left and right or above and below.
   */
  enum ButtonOrder { Horizontal, Vertical };

  /**
   * \param spinBox The instance of a spinbox, derived from QAbstractSpinBox.
   *
   * \param parent The parent widget.
   *
   * \param buttonOrder The arrangements of the spinbox buttons. The default
   *                    value is horizontal.
   */
  VarSpinBox( QAbstractSpinBox* spinBox, QWidget* parent=0, enum ButtonOrder buttonOrder=Horizontal);

  virtual ~VarSpinBox();

  /** @return The spinbox instance of this widget. */
  QAbstractSpinBox* spinBox()
  {
    return m_spinBox;
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
  QAbstractSpinBox* m_spinBox;

  /** Increase button. */
  QPushButton* m_plus;

  /** Decrease button. */
  QPushButton* m_minus;

  /** The button order. */
  enum ButtonOrder m_buttonOrder;
};

#endif
