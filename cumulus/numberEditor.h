/***********************************************************************
**
**   numberEditor.h
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
 * \class NumberEditor
 *
 * \author Axel Pauli
 *
 * \brief Number editor and display.
 *
 * This widget can be used to display a text in a QLabel and to edit it
 * by using an own text keypad.
 *
 * \date 2012-2013
 *
 * \version $Id$
 */

#ifndef NUMBER_EDITOR_H_
#define NUMBER_EDITOR_H_

#include <QLabel>
#include <QPair>
#include <QString>
#include <QValidator>

class QMouseEvent;
class NumberInputPad;

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

  QString getText() const
  {
    return m_number;
  };

  QString text() const
  {
    return m_number;
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

  void setPrefix( const QString prefix )
  {
    m_prefix = prefix;
    setText();
  };

  void setSuffix( const QString suffix )
  {
    m_suffix = suffix;
    setText();
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

  signals:

   void numberEdited( const QString& number );

   void valueChanged( int i );

   void valueChanged( double d );

  protected slots:

   virtual void slot_NumberEdited( const QString& number );

 protected:

 /**
  * Handles mouse button presses.
  */
  void mousePressEvent( QMouseEvent* event );

  void showEvent( QShowEvent *event );

  void setText()
  {
    QLabel::setText( m_prefix + m_number + m_suffix );
  };

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
};

#endif /* NUMBER_EDITOR_H_ */
