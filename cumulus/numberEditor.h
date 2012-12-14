/***********************************************************************
**
**   numberEditor.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012 Axel Pauli
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
 * \date 2012
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

  void setNumber(const QString number)
  {
    m_number = number;
    setText();
  };

  QString getNumber()
  {
    return m_number;
  };

  QString number()
  {
    return m_number;
  };

  void setPrefix(const QString prefix)
  {
    m_prefix = prefix;
    setText();
  };

  void setSuffix(const QString suffix)
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

  void setMaxLength( int max )
  {
    m_maxLength = max;
  };

 protected:

   /**
    * Handles mouse button presses.
    */
   void mousePressEvent( QMouseEvent* event );

 signals:

   void numberEdited( const QString& number );

 private slots:

  void slot_Number( const QString& number );

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

  bool m_decimalFlag;
  bool m_pmFlag;

  QValidator* m_validator;
  QString m_inputMask;
  int m_maxLength;
};

#endif /* NUMBER_EDITOR_H_ */
