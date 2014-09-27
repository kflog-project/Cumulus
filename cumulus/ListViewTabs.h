/***********************************************************************
**
**   ListViewTabs.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2014 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class ListViewTabs
 *
 * \author Axel Pauli
 *
 * \brief This class manages different display lists in a QTabWidget.
 *
 * \date 2014
 *
 * \version $Id$
 */

#ifndef ListViewTabs_h
#define ListViewTabs_h

#include <QTabWidget>
#include <QWidget>

#include "airfieldlistview.h"
#include "reachpointlistview.h"
#include "tasklistview.h"
#include "waypointlistview.h"

class ListViewTabs : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( ListViewTabs )

 public:

  ListViewTabs( QWidget* parent = 0 );

  virtual ~ListViewTabs();

  /**
   * Sets the tabulator with the related view as active page. If the
   * requested view is not part of the tab widget, nothing happens.
   */
  void setView( const int view );

  const QTabWidget*  getListViewTabs () const
  {
    return m_listViewTabs;
  }

  const QString& getTextAf () const
  {
    return m_textAF;
  }

  void setTextAf (const QString& text)
  {
    m_textAF = text;

    int idx = m_listViewTabs->indexOf( viewAF );

    if( idx != -1 )
      {
	m_listViewTabs->setTabText( idx, text );
      }
  }

  const QString& getTextOl () const
  {
    return m_textOL;
  }

  void setTextOl (const QString& text)
  {
    m_textOL = text;

    int idx = m_listViewTabs->indexOf( viewOL );

    if( idx != -1 )
      {
	m_listViewTabs->setTabText( idx, text );
      }
  }

  const QString& getTextRp () const
  {
    return m_textRP;
  }

  void setTextRp (const QString& text)
  {
    m_textRP = text;

    int idx = m_listViewTabs->indexOf( viewRP );

    if( idx != -1 )
      {
	m_listViewTabs->setTabText( idx, text );
      }
  }

  const QString& getTextTp () const
  {
    return m_textTP;
  }

  void setTextTp (const QString& text)
  {
    m_textTP = text;

    int idx = m_listViewTabs->indexOf( viewTP );

    if( idx != -1 )
      {
	m_listViewTabs->setTabText( idx, text );
      }
  }

  const QString& getTextWp () const
  {
    return m_textWP;
  }

  void setTextWp (const QString& text)
  {
    m_textWP = text;

    int idx = m_listViewTabs->indexOf( viewWP );

    if( idx != -1 )
      {
	m_listViewTabs->setTabText( idx, text );
      }
  }

  const AirfieldListView* getViewAf () const
  {
    return viewAF;
  }

  void setViewAf( AirfieldListView* viewAf)
  {
    viewAF = viewAf;
  }

  const AirfieldListView* getViewOl () const
  {
    return viewOL;
  }

  void setViewOl (AirfieldListView* viewOl)
  {
    viewOL = viewOl;
  }

  const ReachpointListView* getViewRp () const
  {
    return viewRP;
  }

  void setViewRp (ReachpointListView* viewRp)
  {
    viewRP = viewRp;
  }

  const TaskListView* getViewTp () const
  {
    return viewTP;
  }

  void setViewTp (TaskListView* viewTp)
  {
    viewTP = viewTp;
  }

  const WaypointListView* getViewWp () const
  {
    return viewWP;
  }

  void setViewWp (WaypointListView* viewWp)
  {
    viewWP = viewWp;
  }

  /**
   * Lists managed by the tab widget.
   */
  AirfieldListView*   viewAF;
  AirfieldListView*   viewOL;
  ReachpointListView* viewRP;
  TaskListView*       viewTP;
  WaypointListView*   viewWP;

 protected:

  void showEvent( QShowEvent *event );

 private slots:

  /**
   * Called, if the widget shall be closed. Emits signal closed.
   */
  void slotDone();

 signals:

  void closed();

 private:

  QTabWidget* m_listViewTabs;

  /**
   * Tabulator labels
   */
  QString m_textAF;
  QString m_textOL;
  QString m_textRP;
  QString m_textTP;
  QString m_textWP;

};

#endif
