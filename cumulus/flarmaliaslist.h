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
 * This widget can provide alias names for FLARM hexadecimal identifiers.
 * The names are displayed in a table and can be edited.
 *
 */

#ifndef FLARM_ALIAS_LIST_H_
#define FLARM_ALIAS_LIST_H_

#include <QWidget>
#include <QHash>

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

  void slot_AddRow( QString col0="", QString col1="" );
  void slot_DeleteRows();
  void slot_Ok();
  void slot_Close();
  void slot_CellChanged( int row, int column );
  void slot_HeaderClicked( int section );

signals:

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
