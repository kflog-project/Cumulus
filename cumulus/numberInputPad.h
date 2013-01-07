/***********************************************************************
**
**   numberInputPad.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012-2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class NumberInputPad
 *
 * \author Axel Pauli
 *
 * \brief Number input and editor pad.
 *
 * This widget can be used to enter a number or to modify it with an own
 * provided keypad.
 *
 * \date 2012-2013
 *
 * \version $Id$
 */

#ifndef NumberInputPad_h
#define NumberInputPad_h

#include <QtGui>

#include <QLineEdit>
#include <QPair>
#include <QPushButton>
#include <QString>
#include <QValidator>

#include <QFrame>

class QSignalMapper;
class QTimer;

class NumberInputPad : public QFrame
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( NumberInputPad )

 public:

  NumberInputPad( const QString number="", QWidget *parent=0 );

  virtual ~NumberInputPad();

  void setNumber( const QString& number )
  {
    m_setNumber = number;
    m_editor->setText(number);
    m_editor->setCursorPosition( 0 );
  };

  QString getNumber() const
  {
    return m_editor->text().trimmed();
  };

  QString number() const
  {
    return m_editor->text().trimmed();
  };

  void setDecimalVisible( const bool flag )
  {
    m_decimal->setVisible( flag );
  };

  void setPmVisible( const bool flag )
  {
    m_pm->setVisible( flag );
  };

  void setValidator( QValidator* validator )
  {
    m_editor->setValidator( validator );
  };

  void setInputMask( const QString mask )
  {
    m_editor->setInputMask( mask );
  };

  void setMaxLength( int max )
  {
    m_editor->setMaxLength( max );
  };

  void home()
  {
    m_editor->setCursorPosition( 0 );
  };

  /**
   * Returns the editor widget to the caller.
   *
   * \return The editor widget.
   */
  QLineEdit* getEditor()
  {
    return m_editor;
  };

  /**
   * Sets the maximum integer input value.
   *
   * \param maximum The new maximum input value
   */
  void setIntMaximum( const int maximum )
  {
    m_intMaximum.first = true;
    m_intMaximum.second = maximum;
  };

  /**
   * Returns the maximum integer input value.
   *
   * \return The new maximum integer input value
   */
  QPair<bool, int> intMaximum() const
  {
    return m_intMaximum;
  };

  /**
   * Sets the minimum integer input value.
   *
   * \param maximum The new minimum input value
   */
  void setIntMinimum( const int minimum )
  {
    m_intMinimum.first = true;
    m_intMinimum.second = minimum;
  };

  /**
   * Returns the minimum integer input value.
   *
   * \return The minimum integer input value
   */
  QPair<bool, int> intMinimum() const
  {
    return m_intMinimum;
  };

  /**
   * Sets the maximum double input value.
   *
   * \param maximum The new maximum input value
   */
  void setDoubleMaximum( const int maximum )
  {
    m_doubleMaximum.first = true;
    m_doubleMaximum.second = maximum;
  };

  /**
   * Returns the maximum double input value.
   *
   * \return The new maximum double input value
   */
  QPair<bool, double> doubleMaximum() const
  {
    return m_doubleMaximum;
  };

  /**
   * Sets the minimum double input value.
   *
   * \param maximum The new minimum input value
   */
  void setDoubleMinimum( const int minimum )
  {
    m_doubleMinimum.first = true;
    m_doubleMinimum.second = minimum;
  };

  /**
   * Returns the minimum double input value.
   *
   * \return The minimum integer input value
   */
  QPair<bool, double> doubleMinimum() const
  {
    return m_doubleMinimum;
  };

  /**
   * Sets a user tip in the text editor pad.
   *
   * \param tip Tip to be set in the editor pad.
   */
  void setTip( QString tip );

 protected:

  /**
   * Catch show events in this class to set the widths of some widgets.
   */
  void showEvent( QShowEvent* event );

 signals:

  /**
   * Emitted, if the widget is closed. The text is the last input.
   */
  void numberEdited( const QString& text );

  /**
   * Emitted, if a new acceptable input is available.
   */
  void valueChanged( int i );

  /**
   * Emitted, if a new acceptable input is available.
   */
  void valueChanged( double d );

 private slots:

  void slot_ButtonPressed( QWidget *widget );

  void slot_Repeat();

  void slot_TextChanged( const QString& text );

  /**
   * Toggles a leading minus sign.
   */
  void slot_Pm();

  void slot_Ok();

  void slot_Close();

 private:

  /** The editor widget */
  QLineEdit* m_editor;

  QPushButton* m_ok;
  QPushButton* m_cancel;
  QPushButton* m_delLeft;
  QPushButton* m_delRight;
  QPushButton* m_left;
  QPushButton* m_right;
  QPushButton* m_decimal;
  QPushButton* m_pm;
  QPushButton* m_home;
  QPushButton* m_num0;
  QPushButton* m_num1;
  QPushButton* m_num2;
  QPushButton* m_num3;
  QPushButton* m_num4;
  QPushButton* m_num5;
  QPushButton* m_num6;
  QPushButton* m_num7;
  QPushButton* m_num8;
  QPushButton* m_num9;

  QSignalMapper* m_signalMapper;

  QTimer* m_timer;

  /** A user tip label, what is expected as input. */
  QLabel* m_tipLabel;

  /** The state of the SIP */
  bool m_autoSip;

  /** The set number to be edited*/
  QString m_setNumber;

  /** The maximum allowed integer input value. */
  QPair<bool, int> m_intMaximum;

  /** The minimum allowed integer input value. */
  QPair<bool, int> m_intMinimum;

  /** The maximum allowed double input value. */
  QPair<bool, double> m_doubleMaximum;

  /** The minimum allowed double input value. */
  QPair<bool, double> m_doubleMinimum;

  /** To remember the last pressed button */
  QPushButton* m_pressedButton;
};

#endif // NumberInputPad_h
