/***********************************************************************
**
**   configwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2007-2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class ConfigWidget
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration widget of Cumulus
 *
 * This is the general configuration widget for Cumulus.
 *
 * \date 2002-2018
 *
 * \version 1.2
 */

#ifndef _ConfigWidget_h
#define _ConfigWidget_h

#include <QString>
#include <QTreeWidget>
#include <QWidget>
#include <QStringList>

class ConfigWidget : public QWidget
{
  Q_OBJECT

 private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( ConfigWidget )

public:

  /**
   * Constructor
   *
   * @param parent Pointer to parent widget
   */
  ConfigWidget(QWidget *parent);

  /**
   * Destructor
   */
  virtual ~ConfigWidget();

protected:

  void keyReleaseEvent( QKeyEvent* event );

  virtual void closeEvent(QCloseEvent * event);

 signals:

  /**
   * This signal is emitted before the widget is closed.
   * MainWindow will use it to update the current view setting.
   */
  void closeConfig();

  /**
   * Emitted, if the map should move to the new home position.
   */
  void gotoHomePosition();

 private slots:

  /**
   * Called, if the help button is clicked.
   */
  void slotHelp();

  /**
   * Called if dialog is accepted (OK button is clicked)
   */
  void slotAccept();

  /**
   * Called if dialog is rejected (X button is clicked)
   */
  void slotReject();

  /**
   * Called, if an item is pressed in the tree view.
   */
  void slotPageClicked( QTreeWidgetItem * item, int column );

  /**
   * Called if a new the home position is defined.
   */
  void slotNewHomePosition();

 private:

  QTreeWidget* m_setupTree;

  /** List with all header labels. */
  QStringList m_headerLabels;
};

#endif
