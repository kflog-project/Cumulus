/***********************************************************************
**
**   AirspaceFilters.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2020-2021 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class AirspaceFilters
 *
 * \author Axel Pauli
 *
 * \brief Airspace filters display and editor.
 *
 * This widget can create, modify or remove filters for certain airspaces.
 * The single filters are defined is a table. The content of
 * the table is stored in a text file in the user's data directory.
 *
 * \date 2020-2021
 *
 * \version 1.1
 */

#ifndef AIRSPACE_FILTERS_H
#define AIRSPACE_FILTERS_H

#include <QWidget>
#include <QHash>
#include <QMultiHash>
#include <QMutex>

#include "map.h"

class QCheckBox;
class QPushButton;
class QString;
class QTableWidget;
class RowDelegate;

class AirspaceFilters : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( AirspaceFilters )

public:

  /**
   * Constructor
   */
  AirspaceFilters( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~AirspaceFilters();

  /**
   * @return countryHash copy to the caller.
   */
  static QHash<QString, QMultiHash<QString, QString> > getAirspaceFilters()
  {
    mutex.lock();
    QHash<QString, QMultiHash<QString, QString> > ch = countryHash;
    mutex.unlock();
    return ch;
  };

  /** Loads the filter data from the related file into the alias hash. */
  static bool loadFilterData();

private:

  /** Load data from the file into the table. */
  void loadDataFromFile();

  /** Save table data into the file. */
  void saveData2File();

  /** Returns the airspace filters filename. */
  static QString getFilterFileName();

protected:

  void showEvent( QShowEvent *event );

private slots:

  /** Adds a new row with two columns to the table. */
  void slot_AddRow( bool col0=true, QString col1="" );

  /** Removes all selected rows from the table. */
  void slot_DeleteRows();

  /** Ok button press is handled here. */
  void slot_Ok();

  /** Close button press is handled here. */
  void slot_Close();

  /** Called, when a cell is clicked to open an extra editor. */
  void slot_CellClicked ( int row, int column );

  /**
   * Header click is handled here. It sorts the clicked column in ascending
   * order.
   */
  void slot_HeaderClicked( int section );

  /**
   * Called is the checkbox is toggled.
   */
  void slot_scrollerBoxToggled( int state );

signals:

  /**
   * Emit airspace filters change to the world.
   */
  void airspaceFiltersChanged( Map::mapLayer );

private:

  /** Table widget with two columns for alias entries. */
  QTableWidget* table;

  /** Adds additional space in the list. */
  RowDelegate* rowDelegate;

  /** Delete button. */
  QPushButton *deleteButton;

  /** A checkbox to toggle scroller against a big scrollbar. */
  QCheckBox* m_enableScroller;

  /**
   * Country hash dictionary. The key is the country and the value the
   * assigned filter string.
   */
  static QHash< QString, QMultiHash<QString, QString> > countryHash;

  /** Mutex used for filter data file load and save. */
  static QMutex mutex;
};

#endif /* AIRSPACE_FILTERS_H */
