/***********************************************************************
**
**   gliderlistwidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "generalconfig.h"
#include "gliderlistwidget.h"
#include "rowdelegate.h"

GliderListWidget::GliderListWidget(QWidget *parent) :
  QTreeWidget(parent),
  _added(0),
  _changed(false)
{
  setObjectName("GliderListWidget");

  setRootIsDecorated(false);
  setAlternatingRowColors ( true );
  //setItemsExpandable(false);
  setUniformRowHeights(true);
  setSortingEnabled(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setColumnCount(4);
  hideColumn(3);

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  setItemDelegate( new RowDelegate( this, afMargin ) );

  QStringList sl;
  sl << tr("Type") << tr("Registration") << tr("Callsign");
  setHeaderLabels(sl);

  setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
}

GliderListWidget::~GliderListWidget()
{
  qDeleteAll(Gliders);
}

void GliderListWidget::showEvent( QShowEvent* event )
{
  Q_UNUSED( event )

  resizeListColumns();
}

/** Retrieves the gliders from the configuration file and fills the list. */
void GliderListWidget::fillList()
{
  qDeleteAll( Gliders );
  Gliders.clear();

  QSettings config( QSettings::UserScope, "Cumulus" );
  config.beginGroup( "Glider Data" );

  QString keyname = "Glider%1";
  int i = 1;

  while( config.contains( keyname.arg( i ) ) )
    {
      Glider *glider = new Glider();

      if( glider->load( &config, i ) )
        {
          Gliders.append( glider );

          QStringList rowList;
          rowList << glider->type()
                  << glider->registration()
                  << glider->callSign()
                  << QString::number( glider->lastSafeID() );

          addTopLevelItem( new QTreeWidgetItem( rowList, 0 ) );
        }
      else
        {
          delete glider; //loading failed!
        }

      i++;
    }

  if( i > 1 )
    {
      sortItems( 0, Qt::AscendingOrder );
    }

  config.endGroup();

  resizeListColumns();
  _changed = false;
}

void GliderListWidget::save()
{
  QSettings config( QSettings::UserScope, "Cumulus" );
  config.beginGroup("Glider Data");

  QString keyname="Glider%1";

  config.remove(""); // remove all old entries

  uint u=1;

  /* check for duplicate registrations
     This method is quite expensive if the list is very long. This is not to be expected however. Most
     people will only have a couple of gliders. The number of iterations is N*(N-1)/2.
     In case of 5 gliders, the number of iterations is 10, and for 10 glider the number is 45 (worst
     case). That seems acceptable to me. For long lists, it would be more efficient to first sort
     the list, and then compare. */
  for( int i = 2; i <= Gliders.count(); i++ )
    {
      for( int j = 1; j < i; j++ )
        {
          if( Gliders.at( j - 1 )->registration() == Gliders.at( i - 1 )->registration() )

            // two gliders have the same registration, append a number to the second one.
            Gliders.at( j - 1 )->setRegistration(
                Gliders.at( j - 1 )->registration() + "(" + QString::number( u++ )
                    + ")" );
        }
    }

  //Should we let the user know what we have done?
  // if(u>1)
  //     qDebug("changed %d registrations",u-1);

  // store glider list in configuration file
  for( int i = 1; i <= Gliders.count(); i++ )
    {
      // qDebug("saving glider %d",i);
      Gliders.at( i - 1 )->safe( &config, i );
    }

  config.endGroup();
  config.sync();

  _changed = false;
}

/**
 * Returns a pointer to the currently high lighted glider. If take is
 * true, the glider object is taken from the list too.
 */
Glider *GliderListWidget::getSelectedGlider(bool take)
{
  Glider* glider = static_cast<Glider *> (0);

  if( selectedItems().size() > 0 )
    {
      QTreeWidgetItem* selectedItem = selectedItems().at( 0 );

      int n = Gliders.count();

      for( int i = 0; i < n; i++ )
        {
          glider = Gliders.at( i );

          if( glider->lastSafeID() == selectedItem->text( 3 ).toInt() )
            {
              if( take )
                {
                  return Gliders.takeAt( i );
                }
              else
                {
                  return glider;
                }
            }
        }
    }

  return glider;
}


/** Called if a glider has been edited. */
void GliderListWidget::slot_Edited(Glider *glider)
{
  if( glider )
    {
      if( selectedItems().size() > 0 )
        {
          QTreeWidgetItem* selectedItem = selectedItems().at( 0 );

          selectedItem->setText( 0, glider->type() );
          selectedItem->setText( 1, glider->registration() );
          selectedItem->setText( 2, glider->callSign() );
          selectedItem->setText( 3, QString::number( glider->lastSafeID() ) );
          resizeListColumns();
          _changed = true;
        }
    }
}


/** Called if a glider has been added. */
void GliderListWidget::slot_Added(Glider *glider)
{
  if( glider )
    {
      _added++;
      QStringList rowList;
      rowList << glider->type()
              << glider->registration()
              << glider->callSign()
              << QString::number( -_added );

      addTopLevelItem( new QTreeWidgetItem( this, rowList, 0 ) );
      setCurrentItem( itemAt( 0, topLevelItemCount() - 1 ) );
      sortItems( 0, Qt::AscendingOrder );
      resizeListColumns();

      glider->setID( -_added ); //store temp ID
      Gliders.append( glider );
      _changed = true;
    }
}


void GliderListWidget::slot_Deleted(Glider *glider)
{
  if( glider && currentItem() )
    {
      // remove from listView
      delete takeTopLevelItem( currentIndex().row() );

      sortItems( 0, Qt::AscendingOrder );
      setCurrentItem( 0 );
      resizeListColumns();

      // remove glider from glider list
      int index = Gliders.indexOf( glider );
      delete Gliders.takeAt( index );
      _changed = true;
    }
}


/* Retrieve last selected glider */
Glider* GliderListWidget::getStoredSelection()
{
  QSettings config( QSettings::UserScope, "Cumulus" );

  config.beginGroup( "Glider Selection" );
  QString stored = config.value( "lastSelected", "" ).toString();
  config.endGroup();

  config.beginGroup( "Glider Data" );

  if( !stored.isEmpty() )
    {
      QString keyname = "Glider%1";
      int i = 1;

      while( config.contains( keyname.arg( i ) ) )
        {
          Glider *glider = new Glider();

          if( glider->load( &config, i ) )
            {
              if( glider->registration() == stored )
                {
                  return glider;
                }
              else
                {
                  delete glider; //this is not the glider we're looking for...
                }
            }
          else
            {
              delete glider; //loading failed!
            }

          i++;
        } //while
    } //!stored.isEmpty()

  config.endGroup();
  // if we end up here, there is no default: either loading failed, or
  // it is not set, or the set glider has been deleted.
  return static_cast<Glider *> ( 0 );
}


/* save last selected glider */
void GliderListWidget::setStoredSelection(const Glider* glider)
{
  QSettings config( QSettings::UserScope, "Cumulus" );
  config.beginGroup("Glider Selection");

  if( glider )
    {
      // store last selection
      config.setValue("lastSelected", glider->registration());
    }
  else
    {
      // reset last selection
      config.setValue("lastSelected", "");
    }

  config.endGroup();
}

void GliderListWidget::selectItemFromReg( const QString& registration )
{
  QList<QTreeWidgetItem*> result = findItems( registration, Qt::MatchExactly, 1 );

  if( result.size() > 0 )
    {
      setCurrentItem( result.at( 0 ) );
    }
}
