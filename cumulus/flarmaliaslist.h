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

protected:

  void showEvent( QShowEvent *event );

private slots:

  void slot_AddRow();
  void slot_DeleteRow();
  void slot_Ok();
  void slot_Close();

signals:

  /** Emitted if the widget was closed. */
  void closed();

private:

  QTableWidget* list;

};

#endif /* FLARM_ALIAS_LIST_H_ */
