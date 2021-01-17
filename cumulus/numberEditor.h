/***********************************************************************
**
**   numberEditor.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012-2021 Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class NumberEditor
 *
 * \author Axel Pauli
 *
 * \brief Number editor and display.
 *
 * This widget can be used to display a text in a QLabel and to edit it
 * by using an own text keypad.
 *
 * \date 2012-2021
 *
 * \version 1.2
 */

#ifndef NUMBER_EDITOR_H_
#define NUMBER_EDITOR_H_

#include <QLabel>
#include <QPair>
#include <QString>
#include <QValidator>

class QMouseEvent;

#include "numberInputPad.h"

class NumberEditor : public QLabel
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( NumberEditor )

 public:

  NumberEditor( QWidget *parent=0,
                const QString number="",
                const QString prefix="",
                const QString suffix="" );

  virtual ~NumberEditor();

  void setText( const QString number )
  {
    m_number = number;
    setText();
  };

  /**
   * Returns only the number text without prefix and suffix.
   */
  QString getText() const
  {
    return m_number;
  };

  /**
   * Returns only the number text without prefix and suffix.
   */
  QString text() const
  {
    return m_number;
  };

  /**
   * Returns only the number text without prefix and suffix.
   */
  QString cleanText() const
  {
    return m_number;
  };

  /**
   * Gets the special text to be displayed as minimum value.
   *
   * \return Special minimum value text.
   */
  QString specialValueText() const
  {
    return m_specialValueText;
  };

  /**
   * Sets a special text to be displayed as minimum value.
   *
   * \param text Minimum value text
   */
  void setSpecialValueText( const QString& text )
  {
    m_specialValueText = text;
    setText();
  };

  /**
   * Sets the integer value in the display field.
   *
   * \param value Value to be displayed
   */
  void setValue( const int value )
  {
    setText( QString::number( value ) );
  };

  /**
   * Returns the displayed value as integer.
   *
   * \return displayed value as integer or zero in error case.
   */
  int value() const
  {
    return m_number.toInt();
  };

  /**
   * Returns the displayed value as double.
   *
   * \return displayed value as double or zero in error case.
   */
  double doubleValue() const
  {
    return m_number.toDouble();
  };

  /**
   * The prefix is prepended to the start of the displayed value. Typical use
   * is to display a unit of measurement or a currency symbol.
   *
   * @param prefix
   */
  void setPrefix( const QString prefix )
  {
    m_prefix = prefix;
    setText();
  };

  /**
   * @return The set prefix for the display.
   */
  QString prefix() const
  {
    return m_prefix;
  };

  /**
   * The suffix is appended to the end of the displayed value. Typical use is
   * to display a unit of measurement or a currency symbol.
   *
   * @param suffix The suffix for the display.
   */
  void setSuffix( const QString suffix )
  {
    m_suffix = suffix;
    setText();
  };

  /**
   * @return The set suffix for the display.
   */
  QString suffix () const
  {
    return m_suffix;
  };

  void setDecimalVisible( const bool flag )
  {
    m_decimalFlag = flag;
  };

  void setPmVisible( const bool flag )
  {
    m_pmFlag = flag;
  };

  void setValidator( QValidator* validator )
  {
    if( m_validator )
      {
        delete m_validator;
      }

    m_validator = validator;

  };

  void setInputMask( const QString inputMask )
  {
    m_inputMask = inputMask;
  };

  /**
   * Sets the maximum input field length.
   *
   * \param max Maximum characters accepted by the input field.
   */
  void setMaxLength( int max )
  {
    m_maxLength = max;
  };

  /**
   * Gets the maximum input field length.
   *
   * \return Maximum characters accepted by the input field.
   */
  int maxLength()
  {
    return m_maxLength;
  };

  /**
   * Sets the integer maximum input value.
   *
   * \param maximum The integer maximum input value
   */
  void setMaximum( const int maximum )
  {
    m_intMax.first = true;
    m_intMax.second = maximum;
  };

  /**
   * Returns the integer maximum input value.
   *
   * \return The integer maximum input value
   */
  int maximum()
  {
    return m_intMax.second;
  };

  /**
   * Returns the validity of the integer maximum value.
   *
   * \return The validity of the integer maximum value.
   */
  virtual bool maximumValid() const
  {
    return m_intMax.first;
  };

  /**
   * Sets the integer minimum input value.
   *
   * \param minimum The new integer minimum input value
   */
  void setMinimum( const int minimum )
  {
    m_intMin.first = true;
    m_intMin.second = minimum;
  };

  /**
   * Returns the integer minimum input value.
   *
   * \return The integer minimum input value
   */
  int minimum() const
  {
    return m_intMin.second;
  };

  /**
   * Returns the validity of the integer minimum value.
   *
   * \return The validity of the integer minimum value.
   */
  virtual bool minimumValid() const
  {
    return m_intMin.first;
  };

  /**
   * Sets a minimum maximum range.
   */
  void setRange( int minimum, int maximum )
  {
    m_intMin.first = true;
    m_intMin.second = minimum;
    m_intMax.first = true;
    m_intMax.second = maximum;
  };

  /**
   * Sets the title of the text editor pad.
   *
   * \param title Title to be set in the editor pad.
   */
  void setTitle( QString title )
  {
    m_title = title;
  };

  /**
   * Sets a user tip in the text editor pad.
   *
   * \param tip Tip to be set in the editor pad.
   */
  void setTip( QString tip )
  {
    m_tip = tip;
  };

  /**
   * The status of the fixed label height flag.
   *
   * @return true if activated otherwise false
   */
  bool getFixHeight() const
  {
    return m_fixHeight;
  };

  /**
   * The new state of the fixed label height flag.
   *
   * @param newValue new state of the related flag
   */
  void setFixHeight( bool newValue )
  {
    m_fixHeight = newValue;
  };

  /**
   * If true, the input number is not checked for a valid number.
   */
  void disableNumberCheck( const bool flag )
  {
    m_disableNumberCheck = flag;
  }

 signals:

  /**
   * Emitted, if the number editor is closed by the user to give back the
   * edited value. If the value was not edited, the initial value is returned.
   */
  void numberEdited( const QString& number );

  /**
   * Emitted after every user input, if the edited number is valid.
   *
   * @param i The new integer value.
   */
  void valueChanged( int i );

  /**
   * Emitted after every user input, if the edited number is valid.
   *
   * @param d The new double value.
   */
  void valueChanged( double d );

 public slots:

 /**
  * Clears the display filed.
  */
 void slot_Clear()
 {
   setText("");
 };

 protected slots:

  /**
   * Called if the number editor was closed. Can be overwritten in a derived
   * class.
   *
   * \param The edited number
   */
  virtual void slot_NumberEdited( const QString& number );

 protected:

 /**
  * Handles mouse button presses.
  */
  virtual void mousePressEvent( QMouseEvent* event );

  virtual void showEvent( QShowEvent *event );

  virtual void closeEvent(QCloseEvent * event);

  /**
   * Sets the number with prefix and suffix in the display label.
   */
  void setText();

  NumberInputPad* m_nip;

  QString m_prefix;
  QString m_number;
  QString m_suffix;
  QString m_title;
  QString m_tip;

  bool m_decimalFlag;
  bool m_pmFlag;

  QValidator* m_validator;
  QString m_inputMask;
  int m_maxLength;

  QPair<bool, int> m_intMax;
  QPair<bool, int> m_intMin;

  /** These pairs can be used in a derived class to manipulate them. */
  QPair<bool, double> m_doubleMax;
  QPair<bool, double> m_doubleMin;

  /** Special-value text for display if minimum condition is true. */
  QString m_specialValueText;

  /** Fix height flag of the label. */
  bool m_fixHeight;

  bool m_disableNumberCheck;
};

#endif /* NUMBER_EDITOR_H_ */
