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
 * \version 1.0
 */

#ifndef ListViewTabs_h
#define ListViewTabs_h

#include <QTabWidget>
#include <QTimer>
#include <QWidget>

#include "PointListView.h"
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

  const QString& getTextHs () const
  {
    return m_textHS;
  }

  void setTextHs (const QString& text)
  {
    m_textHS = text;

    int idx = m_listViewTabs->indexOf( viewHS );

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

  const QString& getTextNa() const
  {
    return m_textNA;
  }

  void setTextNa(const QString& text)
  {
    m_textNA = text;

    int idx = m_listViewTabs->indexOf( viewNA );

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

  const PointListView* getViewAf () const
  {
    return viewAF;
  }

  void setViewAf( PointListView* view)
  {
    viewAF = view;
  }

  const PointListView* getViewHs () const
  {
    return viewHS;
  }

  void setViewHs( PointListView* view)
  {
    viewHS = view;
  }

  const PointListView* getViewOl () const
  {
    return viewOL;
  }

  void setViewOl (PointListView* view)
  {
    viewOL = view;
  }

  const PointListView* getViewNa () const
  {
    return viewNA;
  }

  void setViewNa( PointListView* view)
  {
    viewNA = view;
  }

  const ReachpointListView* getViewRp () const
  {
    return viewRP;
  }

  void setViewRp (ReachpointListView* view)
  {
    viewRP = view;
  }

  const TaskListView* getViewTp () const
  {
    return viewTP;
  }

  void setViewTp (TaskListView* view)
  {
    viewTP = view;
  }

  const WaypointListView* getViewWp () const
  {
    return viewWP;
  }

  void setViewWp (WaypointListView* view)
  {
    viewWP = view;
  }

  /**
   * Lists managed by the tab widget.
   */
  PointListView*      viewAF;
  PointListView*      viewHS;
  PointListView*      viewOL;
  PointListView*      viewNA;
  ReachpointListView* viewRP;
  TaskListView*       viewTP;
  WaypointListView*   viewWP;

 protected:

  void showEvent( QShowEvent* event );

  void hideEvent( QHideEvent* event );

 private slots:

  /**
   * Called, if the widget shall be closed. Emits signal closed.
   */
  void slotDone();

  /**
   * Called to clear the loaded tabulators.
   */
  void slotClearTabs();

 signals:

  /**
   * Emitted, if the widget is hidden.
   */
  void hidingWidget();

 private:

  QTabWidget* m_listViewTabs;

  QTimer* m_clearTimer;
  /**
   * Tabulator labels
   */
  QString m_textAF;
  QString m_textHS;
  QString m_textOL;
  QString m_textNA;
  QString m_textRP;
  QString m_textTP;
  QString m_textWP;
};

#endif
