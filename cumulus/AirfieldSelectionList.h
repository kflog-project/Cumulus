/***********************************************************************
**
**   AirfieldSelectionList.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2016 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class AirfieldSelectionList
 *
 * \author Axel Pauli
 *
 * \brief A widget with a list and a search function for a single airfield.
 *
 * A widget with a list and a search function for a single airfield.
 * With the help of the search function you can navigate to a certain entry in
 * the list. The currently selected entry in the list is emitted as signal,
 * if the ok button is clicked.
 *
 * \date 2016
 *
 * \version 1.0
 */

#ifndef AirfieldSelectionList_h
#define AirfieldSelectionList_h

#include <QGroupBox>
#include <QLineEdit>
#include <QString>
#include <QTreeWidget>
#include <QWidget>

#include "singlepoint.h"

class AirfieldSelectionList : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( AirfieldSelectionList )

 public:

  AirfieldSelectionList( QWidget *parent=0 );

  virtual ~AirfieldSelectionList();

  /**
   * Gets the airfield selection pointer.
   *
   * \return Pointer to airfield selection list.
   */
  QTreeWidget* getSelectionList()
  {
    return m_airfieldTreeWidget;
  }

  /**
   * Called to fill the selection box with content.
   */
  void fillSelectionList();

  void setGroupBoxTitle( QString title )
  {
    m_groupBox->setTitle( title );
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

 private:

  QGroupBox*   m_groupBox;
  QLineEdit*   m_searchInput;
  QTreeWidget* m_airfieldTreeWidget;

  /**
   * \class PointItem
   *
   * \author Axel Pauli
   *
   * \brief A user point item element.
   *
   * \date 2016
   */

  class PointItem : public QTreeWidgetItem
    {
      public:

        PointItem(SinglePoint* point);

        SinglePoint* getPoint() const
	  {
	    return point;
	  };

      private:

        SinglePoint* point;
    };
};

#endif /* AirfieldSelectionList_h */
