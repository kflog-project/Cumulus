/***********************************************************************
**
**   AirspaceInfo.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#pragma once

#include <QFrame>
#include <QPushButton>
#include <QTimer>
#include <QString>
#include <QTextEdit>

/**
 * \class AirspaceInfo
 *
 * \author Axel Pauli
 *
 * \brief Window to display airspace information to the user.
 *
 * This class is used to display information about airspaces.
 * It is realized as a dialog window and is closed automatically after a
 * certain time, when the user does not stop the timer.
 * It can display plain or html text.
 *
 * \date 2023
 *
 * \version 1.0
 *
 */
class AirspaceInfo : public QFrame
{
    Q_OBJECT

  private:
    /**
     * That macro forbids the copy constructor and the assignment operator.
     */
    Q_DISABLE_COPY( AirspaceInfo )

  public:

  AirspaceInfo( QWidget* parent, QString& txt );

  virtual ~AirspaceInfo();

 public slots:

  void slot_Timeout();

  void slot_Close();

  void slot_Stop();

 protected:

  void mousePressEvent( QMouseEvent* );
  void mouseReleaseEvent( QMouseEvent* );
  void keyPressEvent( QKeyEvent* );
  void paintEvent( QPaintEvent* );

 private:

  /** Timer for automatic window hide */
  QTimer* m_timer;

  /** TimerCount */
  int m_timerCount;

  /** Display widget for document */
  QTextEdit* m_display;

  /** Close Button */
  QPushButton* m_cmdClose;

  /** Stop Button */
  QPushButton* m_cmdStop;

  // document
  QTextDocument* doc;
};
