/***********************************************************************
**
**   messagewidget.h
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
 * \class MessageWidget
 *
 * \author Axel Pauli
 *
 * \brief A dialog widget to display a question with yes and no buttons.
 *
 * This widget displays in a dialog a question with yes and no buttons.
 *
 * \date 2012-2021
 *
 * \version 1.1
 *
*/

#ifndef MESSAGE_WIDGET_H
#define MESSAGE_WIDGET_H

#include <QWidget>

class QShowEvent;
class QString;
class QTextEdit;

class MessageWidget : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( MessageWidget )

 public:

  MessageWidget( QString text, QWidget *parent = 0 );

  virtual ~MessageWidget();

 protected:

  virtual void showEvent(QShowEvent *event);

 signals:

  void yesClicked();
  void noClicked();

 private:

  QTextEdit *m_text;
};

#endif
