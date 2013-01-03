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
 * This widget can be used to display a number in a QLabel and to edit it
 * by using an own number keypad.
 *
 * \date 2012-2013
 *
 * \version $Id$
 */

#ifndef NUMBER_EDITOR_H_
#define NUMBER_EDITOR_H_

#include <QLabel>
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

  void setNumber( const QString number )
  {
    m_number = number;
    setText();
  };

  QString getNumber() const
  {
    return m_number;
  };

  QString number() const
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
    setNumber( QString::number( value ) );
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
   * Sets the maximum input value.
   *
   * \param maximum The new maximum input value
   */
  void setMaximum( const int maximum )
  {
    m_intMax = maximum;
  };

  /**
   * Returns the maximum input value.
   *
   * \return The new maximum input value
   */
  int maximum() const
  {
    return m_intMax;
  };


  /**
   * Sets the title of the number editor pad.
   *
   * \param title Title to be set in the editor pad.
   */
  void setTitle( QString title )
  {
    m_title = title;
  };

 protected:

   /**
    * Handles mouse button presses.
    */
   void mousePressEvent( QMouseEvent* event );

 signals:

   void numberEdited( const QString& number );

   void valueChanged( int i );

 private slots:

  void slot_NumberEdited( const QString& number );

 private:

  void setText()
  {
    QLabel::setText( m_prefix + m_number + m_suffix );
  };

 private:

  NumberInputPad* m_nip;

  QString m_prefix;
  QString m_number;
  QString m_suffix;
  QString m_title;

  bool m_decimalFlag;
  bool m_pmFlag;

  QValidator* m_validator;
  QString m_inputMask;
  int m_maxLength;
  int m_intMax;
};

#endif /* NUMBER_EDITOR_H_ */
