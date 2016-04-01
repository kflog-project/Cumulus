/***********************************************************************
**
**   GliderSelectionList.h
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
 * \class GliderSelectionList
 *
 * \author Axel Pauli
 *
 * \brief A widget with a list and a search function for a single glider.
 *
 * A widget with a list and a search function for a single glider.
 * With the help of the search function you can navigate to a certain entry in
 * the list. The currently selected entry in the list is emitted as signal,
 * if the OK button is clicked.
 *
 * \date 2016
 *
 * \version 1.0
 */

#ifndef GliderSelectionList_h
#define GliderSelectionList_h

#include <QGroupBox>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QString>
#include <QTreeWidget>
#include <QWidget>

#include "polar.h"

class GliderSelectionList : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( GliderSelectionList )

 public:

  GliderSelectionList( QWidget *parent=0 );

  virtual ~GliderSelectionList();

  /**
   * Gets the list selection pointer.
   *
   * \return Pointer to the selection list.
   */
  QTreeWidget* getSelectionList()
  {
    return m_ListTreeWidget;
  }

  /**
   * Called to fill the selection list with content.
   */
  void fillSelectionList( QList<Polar>& polarList );

  /**
   * Called to select a certain glider entry in the list.
   */
  void selectGlider( QString gliderName );

  void setGroupBoxTitle( QString title )
  {
    m_groupBox->setTitle( title );
  };

 protected:

  void showEvent( QShowEvent *event );

 signals:

  /**
   * Emitted, if the OK button is pressed to broadcast the selected polar.
   */
  void takeThis( Polar* polar );

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
  QTreeWidget* m_ListTreeWidget;

  /**
   * \class PolarItem
   *
   * \author Axel Pauli
   *
   * \brief A user polar item element.
   *
   * \date 2016
   */

  class PolarItem : public QTreeWidgetItem
    {
      public:

	PolarItem( Polar* polar );

	Polar* getPolar() const
	  {
	    return m_polar;
	  };

      private:

        Polar* m_polar;
    };
};

#endif /* GliderSelectionList_h */
