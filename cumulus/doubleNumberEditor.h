/***********************************************************************
**
**   doubleNumberEditor.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: numberEditor.h 5648 2013-01-03 21:35:48Z axel $
**
***********************************************************************/

/**
 * \class DoubleNumberEditor
 *
 * \author Axel Pauli
 *
 * \brief Double number editor and display.
 *
 * This widget can be used to display a double number in a QLabel and to edit it
 * by using an own number keypad. This class overwrites some method of its base
 * class.
 *
 * \date 2013
 *
 * \version $Id: numberEditor.h 5648 2013-01-03 21:35:48Z axel $
 */

#ifndef DOUBLE_NUMBER_EDITOR_H_
#define DOUBLE_NUMBER_EDITOR_H_

#include "numberEditor.h"

class DoubleNumberEditor : public NumberEditor
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( DoubleNumberEditor )

 public:

  DoubleNumberEditor( QWidget *parent=0 );

  virtual ~DoubleNumberEditor();

  /**
   * Sets the double value in the display field.
   *
   * \param value Value to be displayed
   */
  void setValue( const double value )
  {
    setText( QString("%1").arg( value, 0, 'f', m_precision ) );
  };

  /**
   * Returns the displayed value as double.
   *
   * \return displayed value as double or zero in error case.
   */
  double value() const
  {
    return m_number.toDouble();
  };

  /**
   * Sets the double maximum input value.
   *
   * \param maximum The double maximum input value
   */
  void setMaximum( const double maximum )
  {
    m_doubleMax.first = true;
    m_doubleMax.second = maximum;
  };

  /**
   * Returns the double maximum input value.
   *
   * \return The double maximum input value
   */
  double maximum()
  {
    return m_doubleMax.second;
  };

  /**
   * Returns the validity of the double maximum value.
   *
   * \return The validity of the double maximum value.
   */
  bool maximumValid() const
  {
    return m_doubleMax.first;
  };

  /**
   * Sets the double minimum input value.
   *
   * \param minimum The new double minimum input value
   */
  void setMinimum( const double minimum )
  {
    m_doubleMin.second = minimum;
    m_doubleMin.first = true;
  };

  /**
   * Returns the double minimum input value.
   *
   * \return The double minimum input value
   */
  double minimum() const
  {
    return m_doubleMin.second;
  };

  /**
   * Returns the validity of the double minimum value.
   *
   * \return The validity of the double minimum value.
   */
  bool minimumValid() const
  {
    return m_doubleMin.first;
  };

  /**
   * Sets a minimum maximum range.
   */
  void setRange( double minimum, double maximum )
  {
    m_doubleMin.first = true;
    m_doubleMin.second = minimum;
    m_doubleMax.first = true;
    m_doubleMax.second = maximum;
  };

  void setDecimals( int prec )
  {
    m_precision = prec;
  };

  int decimals() const
  {
    return m_precision;
  };

 signals:

  void numberEdited( const QString& number );

 protected slots:

  /**
   * Overwrite slot of base class to format the edited value.
   */
  virtual void slot_NumberEdited( const QString& number );

 protected:

  int m_precision;
};

#endif /* DOUBLE_NUMBER_EDITOR_H_ */
