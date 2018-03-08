/***********************************************************************
**
**   TaskPointSelectionList.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class TaskPointSelectionList
 *
 * \author Axel Pauli
 *
 * \brief A widget with a list and a search function for a point object.
 *
 * A widget with a list and a search function for a point object.
 * With the help of the search function you can navigate to a certain entry in
 * the list. The currently selected entry in the list is emitted as signal,
 * if the OK button is clicked.
 *
 * \date 2018
 *
 * \version 1.0
 */

#ifndef TaskPointSelectionList_h
#define TaskPointSelectionList_h

#include <QDebug>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QString>
#include <QTreeWidget>
#include <QWidget>

#include "singlepoint.h"

class TaskPointSelectionList : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( TaskPointSelectionList )

 public:

  TaskPointSelectionList( QWidget *parent=0, QString title="" );

  virtual ~TaskPointSelectionList();

  /**
   * Gets the task point list pointer.
   *
   * \return Pointer to task point list.
   */
  QTreeWidget* getSelectionList()
  {
    return m_taskpointTreeWidget;
  }

  /**
   * Called to fill the selection list with the desired content.
   */
  void fillSelectionListWithAirfields();

  /**
   * Called to fill the selection list with the desired content.
   */
  void fillSelectionListWithOutlandings();

  /**
   * Called to fill the selection list with the desired content.
   */
  void fillSelectionListWithNavaids();

  /**
   * Called to fill the selection list with the desired content.
   */
  void fillSelectionListWithHotspots();

  /**
   * Called to fill the selection list with the desired content.
   */
  void fillSelectionListWithWaypoints();

  void setGroupBoxTitle( QString title )
  {
    m_groupBox->setTitle( title );
  };

  /**
   * Set the header items of the selection list to the passed texts.
   */
  void setTreeHeader( QString h0, QString h1 )
  {
    QTreeWidgetItem* headerItem = m_taskpointTreeWidget->headerItem();
    headerItem->setText( 0, h0 );
    headerItem->setText( 1, h1 );
  }

  /**
   * Gets the selected search column.
   */
  int getSearchColumn()
  {
    if( m_RBCol0->isChecked() )
      {
        return 0;
      }

    return 1;
  }

 private:

  /**
   * Selects the passed radio button.
   */
  void setRadioButton( const ushort button )
   {
      if( button == 0 )
	{
	  m_RBCol0->setChecked( true );
	  m_RBCol1->setChecked( false );
	}
      else
	{
	  m_RBCol0->setChecked( false );
	  m_RBCol1->setChecked( true );
	}
   };

 protected:

  void showEvent( QShowEvent *event );

 signals:

  /**
   * Emitted, if the OK button is pressed to broadcast the selected point.
   */
  void takeThisPoint( const SinglePoint* singePoint );

 private slots:

 /**
  * Called if the Ok button is pressed.
  */
 void slotAccept();

 /**
  * Called if the Cancel button is pressed.
  */
 void slotReject();

 /**
  * Called, if the clear button is clicked.
  */
  void slotClearSearchEntry();

  /**
   * Called if the text in the search box is modified.
   */
  void slotTextEdited( const QString& text );

  /**
   * Called, if the selection in the m_airfieldTreeWidget is changed.
   */
  void slotItemSelectionChanged();

 private:

  QGroupBox*   m_groupBox;
  QLineEdit*   m_searchInput;
  QPushButton* m_ok;
  QTreeWidget* m_taskpointTreeWidget;

  QRadioButton* m_RBCol0;
  QRadioButton* m_RBCol1;

  /**
   * \class PointItem
   *
   * \author Axel Pauli
   *
   * \brief A user point item element of the selection list.
   *
   * \date 2018
   */

  class PointItem : public QTreeWidgetItem
    {
      public:

      PointItem(QString item0, QString item1, SinglePoint* point);
      PointItem(QString item0, QString item1, Waypoint* point);

      ~PointItem()
      {
        if( deleteSinglePoint == true )
          {
            delete point;
          }
      }

      SinglePoint* getPoint() const
        {
          return point;
        };

      private:

        SinglePoint* point;
        bool deleteSinglePoint;
    };
};

#endif /* TaskPointSelectionList_h */
