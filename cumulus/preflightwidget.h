/***********************************************************************
 **
 **   preflightwidget.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2003      by André Somers
 **                   2008-2018 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \class PreFlightWidget
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Widget for pre-flight settings.
 *
 * This widget provides an interface to set all the pre-flight settings like
 * glider type, copilot, task, amount of water taken on, etc.
 *
 * \date 2003-2018
 *
 * \version 1.2
 */

#ifndef _PreFlightWidget_h
#define _PreFlightWidget_h

#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QWidget>

class QCheckBox;

class PreFlightWidget : public QWidget
{
Q_OBJECT

private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( PreFlightWidget )

public:

  /**
   * Constructor
   *
   * @param parent Pointer to parent widget
   */
  PreFlightWidget (QWidget *parent);

  /**
   * Destructor
   */
  virtual
  ~PreFlightWidget ();

protected:

  void
  keyReleaseEvent (QKeyEvent* event);

signals:

  /**
   * This signal is emitted before the widget is closed.
   * MainWindow will use it to update the current view setting.
   */
  void
  closeConfig ();

private slots:

  /**
   * Called, if the help button is clicked.
   */
  void slotHelp();

  /**
   * Called if dialog is accepted (OK button is clicked)
   */
  void
  slotAccept ();

  /**
   * Called if dialog is rejected (X button is clicked)
   */
  void
  slotReject ();

  /**
   * Called, if an item is pressed in the tree view.
   */
  void
  slotPageClicked (QTreeWidgetItem * item, int column);

private:

#ifdef FLARM

  /**
   * Called to request Flarm configuration data.
   */
  void requestFlarmConfig();

#endif

  QTreeWidget* m_setupTree;
  QCheckBox* m_menuCb;

  /** List with all header labels. */
  QStringList m_headerLabels;
};

#endif
