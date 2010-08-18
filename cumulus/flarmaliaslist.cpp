/***********************************************************************
**
**   flarmaliaslist.cpp
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

#include <QtGui>

#include "generalconfig.h"
#include "flarmaliaslist.h"

/**
 * Constructor
 */
FlarmAliasList::FlarmAliasList( QWidget *parent ) :
  QWidget( parent ),
  list(0)
{
  setAttribute( Qt::WA_DeleteOnClose );

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  list = new QTableWidget( 0, 2, this );
  list->setSelectionBehavior( QAbstractItemView::SelectRows );

  // hide vertical headers
  QHeaderView *vHeader = list->verticalHeader();
  vHeader->setVisible(false);

  QTableWidgetItem *item = new QTableWidgetItem( tr("Flarm ID") );
  list->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr("Alias") );
  list->setHorizontalHeaderItem( 1, item );

  topLayout->addWidget( list, 2 );

  QGroupBox* buttonBox = new QGroupBox( this );
  //buttonBox->setContentsMargins(2,2,2,2);

  int size = 40;

#ifdef MAEMO
  size = 60;
#endif

  QPushButton *addButton  = new QPushButton;
  addButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "add.png" ) ) );
  addButton->setIconSize(QSize(32, 32));
  addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  addButton->setMinimumSize(size, size);
  addButton->setMaximumSize(size, size);

  QPushButton *deleteButton  = new QPushButton;
  deleteButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "delete.png" ) ) );
  deleteButton->setIconSize(QSize(32, 32));
  deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  deleteButton->setMinimumSize(size, size);
  deleteButton->setMaximumSize(size, size);

  QPushButton *okButton = new QPushButton;
  okButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  okButton->setIconSize(QSize(32, 32));
  okButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  okButton->setMinimumSize(size, size);
  okButton->setMaximumSize(size, size);

  QPushButton *closeButton = new QPushButton;
  closeButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  closeButton->setIconSize(QSize(32, 32));
  closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  closeButton->setMinimumSize(size, size);
  closeButton->setMaximumSize(size, size);

  connect( addButton, SIGNAL(clicked() ), this, SLOT(slot_AddRow()) );
  connect( deleteButton, SIGNAL(clicked() ), this, SLOT(slot_DeleteRow()) );
  connect( okButton, SIGNAL(clicked() ), this, SLOT(slot_Ok()) );
  connect( closeButton, SIGNAL(clicked() ), this, SLOT(slot_Close()) );

  // vertical box with operator buttons
  QVBoxLayout *vbox = new QVBoxLayout;

  vbox->setSpacing(0);
  vbox->addWidget( addButton );
  vbox->addSpacing(32);
  vbox->addWidget( deleteButton );
  vbox->addStretch(2);
  vbox->addWidget( okButton );
  vbox->addSpacing(32);
  vbox->addWidget( closeButton );
  buttonBox->setLayout( vbox );

  topLayout->addWidget( buttonBox );

  qDebug() << "FlarmAliasList() created";
}

FlarmAliasList::~FlarmAliasList()
{
  qDebug() << "FlarmAliasList::~FlarmAliasList()";
}

void FlarmAliasList::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  list->resizeColumnsToContents();
}

void FlarmAliasList::slot_AddRow()
{
  list->setRowCount( list->rowCount() + 1 );

  int row = list->rowCount() - 1;

  QString col0 = "new FLARM Id";
  QString col1 = "new alias";

  QTableWidgetItem* item;

  item = new QTableWidgetItem( col0 );
  item->setFlags( Qt::ItemIsSelectable| Qt::ItemIsEditable | Qt::ItemIsEnabled );
  list->setItem( row, 0, item );

  item = new QTableWidgetItem( col1 );
  item->setFlags( Qt::ItemIsSelectable| Qt::ItemIsEditable | Qt::ItemIsEnabled );
  list->setItem( row, 1, item );

  list->resizeColumnsToContents();
}

void FlarmAliasList::slot_DeleteRow()
{
  if( list->rowCount() == 0 || list->columnCount() != 2 )
    {
      return;
    }

  int cr = list->currentRow();

  delete list->takeItem ( cr, 0 );
  delete list->takeItem ( cr, 1 );

  list->removeRow( cr );
}

void FlarmAliasList::slot_Ok()
{
  // Save table content into a file
  slot_Close();
}

void FlarmAliasList::slot_Close()
{
  setVisible( false );
  emit closed();
  QWidget::close();
}
