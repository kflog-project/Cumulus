/***********************************************************************
**
**   numberInputPad.h
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
 * \class NumberInputPad
 *
 * \author Axel Pauli
 *
 * \brief Number input and editor pad.
 *
 * This widget can be used to enter a number or to modify it with an own
 * provided keypad.
 *
 * \date 2012
 *
 * \version $Id$
 */

#ifndef NumberInputPad_h
#define NumberInputPad_h

#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QValidator>
#include <QWidget>

class QSignalMapper;

class NumberInputPad : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( NumberInputPad )

 public:

  NumberInputPad( const QString text="", QWidget *parent=0 );

  virtual ~NumberInputPad();

  void setNumber( const QString& text )
  {
    m_editor->setText(text);
  };

  QString getNumber()
  {
    return m_editor->text();
  };

  void setDecimalVisible( const bool flag )
  {
    m_decimal->setVisible( flag );
  };

  void setValidator( QValidator* validator )
  {
    m_editor->setValidator( validator );
  };

  void setInputMask( const QString mask )
  {
    m_editor->setInputMask( mask );
  };

 signals:

  void number( const QString& text );

 private slots:

  void buttonPressed( const QString& text );

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

  /** The state of the SIP */
  bool m_autoSip;
};

#endif // NumberInputPad_h
