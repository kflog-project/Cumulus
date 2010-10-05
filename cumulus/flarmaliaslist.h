/***********************************************************************
**
**   flarmaliaslist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief Flarm alias list display and editor.
 *
 * This widget can create, modify or remove alias names for FLARM hexadecimal
 * identifiers. The names are displayed in a two column table. The content of
 * the table is stored in a text file in the user's data directory.
 *
 */

#ifndef FLARM_ALIAS_LIST_H_
#define FLARM_ALIAS_LIST_H_

#include <QWidget>
#include <QHash>
#include <QString>

class QTableWidget;
class QString;

class FlarmAliasList : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmAliasList )

public:

  /**
   * Constructor
   */
  FlarmAliasList( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~FlarmAliasList();

  /**
   * @return the aliasHash to the caller.
   */
  static const QHash<QString, QString>& getAliasHash()
  {
    return aliasHash;
  };

  /** Loads the Flarm alias data from the related file into the alias hash. */
  static bool loadAliasData();

  /** Saves the Flarm alias data from the alias hash into the related file. */
  static bool saveAliasData();

protected:

  void showEvent( QShowEvent *event );

private slots:

  /** Adds a new row with two columns to the table. */
  void slot_AddRow( QString col0="", QString col1="" );

  /** Removes all selected rows from the table. */
  void slot_DeleteRows();

  /** Ok button press is handled here. */
  void slot_Ok();

  /** Close button press is handled here. */
  void slot_Close();

  /** Cell content change is handled here. */
  void slot_CellChanged( int row, int column );

  /**
   * Header click is handled here. It sorts the clicked column in ascending
   * order.
   */
  void slot_HeaderClicked( int section );

signals:

  /**
   * Emit a new Flarm object selection.
   */
  void newObjectSelection( QString newObject );

  /** Emitted if the widget was closed. */
  void closed();

private:

  /** Table widget with two columns for alias entries. */
  QTableWidget* list;

  /**
   * Flarm alias hash dictionary. The key is the Flarm Id and the value the
   * assigned alias name.
   * */
  static QHash<QString, QString> aliasHash;
};

#endif /* FLARM_ALIAS_LIST_H_ */
