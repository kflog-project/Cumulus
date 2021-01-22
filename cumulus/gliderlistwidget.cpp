/***********************************************************************
**
**   gliderlistwidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtGui>

#include "generalconfig.h"
#include "gliderlistwidget.h"
#include "rowdelegate.h"

GliderListWidget::GliderListWidget() :
  QTreeWidget(0),
  m_added(0),
  m_changed(false)
{
  migrateGliderList();
}

GliderListWidget::GliderListWidget( QWidget *parent,
	                                  bool considerSelectionChanges ) :
  QTreeWidget(parent),
  m_added(0),
  m_changed(false)
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

  if( considerSelectionChanges == true )
    {
      connect( this , SIGNAL( currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem* )),
	       SLOT( currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem* ) ));
    }

  migrateGliderList();
}

GliderListWidget::~GliderListWidget()
{
  qDeleteAll(m_gliders);
}

void GliderListWidget::migrateGliderList()
{
  QSettings config( QSettings::UserScope, "Cumulus" );

  config.beginGroup( "Glider Selection" );
  QString stored = config.value( "lastSelected", "" ).toString();
  config.endGroup();

  if( stored.isEmpty() )
    {
      // Nothing to do
      return;
    }

  qDebug() << "GliderListWidget::migrateGliderList will be executed!";

  qDeleteAll( m_gliders );
  m_gliders.clear();

  config.beginGroup( "Glider Data" );

  QString keyname = "Glider%1";
  int i = 1;

  while( config.contains( keyname.arg( i ) ) )
    {
      Glider *glider = new Glider();

      if( glider->load( &config, i ) )
	{
	  if( glider->registration() == stored )
	    {
	      glider->setSelectionFlag( true );
	    }

	  // Remove appended marker like (...)
	  QString reg = glider->registration();

	  int index = reg.lastIndexOf('(');

	  if( index != -1 )
	    {
	      glider->setRegistration( reg.left( index ) );
	    }

	  m_gliders.append( glider );
	}
      else
	{
	  delete glider;
	}

      i++;
    }

  config.endGroup();

  // Reset last glider selection
  config.beginGroup("Glider Selection");
  config.setValue("lastSelected", "");
  config.endGroup();

  save();

  qDeleteAll( m_gliders );
  m_gliders.clear();
}

void GliderListWidget::showEvent( QShowEvent* event )
{
  resizeListColumns();
  QTreeWidget::showEvent( event );
}

/** Retrieves the gliders from the configuration file and fills the list. */
void GliderListWidget::fillList()
{
  qDeleteAll( m_gliders );
  m_gliders.clear();

  QSettings config( QSettings::UserScope, "Cumulus" );
  config.beginGroup( "Glider Data" );

  QString keyname = "Glider%1";
  int i = 1;

  while( config.contains( keyname.arg( i ) ) )
    {
      Glider *glider = new Glider();

      if( glider->load( &config, i ) )
        {
          m_gliders.append( glider );

          QStringList rowList;
          rowList << glider->type()
                  << glider->registration()
                  << glider->callSign()
                  << QString::number( glider->lastSafeID() );

          addTopLevelItem( new GliderItem( glider, rowList, 0 ) );
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
  m_changed = false;
}

void GliderListWidget::save()
{
  QSettings config( QSettings::UserScope, "Cumulus" );
  config.beginGroup("Glider Data");

  QString keyname="Glider%1";

  config.remove(""); // remove all old entries

  // store glider list in configuration file
  for( int i = 1; i <= m_gliders.count(); i++ )
    {
      // qDebug("saving glider %d",i);
      m_gliders.at( i - 1 )->safe( &config, i );
    }

  config.endGroup();
  config.sync();

  m_changed = false;
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

      int n = m_gliders.count();

      for( int i = 0; i < n; i++ )
        {
          glider = m_gliders.at( i );

          if( glider->lastSafeID() == selectedItem->text( 3 ).toInt() )
            {
              if( take )
                {
                  return m_gliders.takeAt( i );
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
          m_changed = true;
        }
    }
}

/** Called if a glider has been added. */
void GliderListWidget::slot_Added(Glider *glider)
{
  if( glider )
    {
      m_added++;
      QStringList rowList;
      rowList << glider->type()
              << glider->registration()
              << glider->callSign()
              << QString::number( -m_added );

      addTopLevelItem( new GliderItem( glider, rowList, 0 ) );
      setCurrentItem( itemAt( 0, topLevelItemCount() - 1 ) );
      sortItems( 0, Qt::AscendingOrder );
      resizeListColumns();

      glider->setID( -m_added ); //store temp ID
      m_gliders.append( glider );
      m_changed = true;
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
      int index = m_gliders.indexOf( glider );
      delete m_gliders.takeAt( index );
      m_changed = true;
    }
}

Glider* GliderListWidget::getUserSelectedGlider()
{
  QSettings config( QSettings::UserScope, "Cumulus" );

  config.beginGroup( "Glider Data" );

  QString keyname = "Glider%1";
  int i = 1;

  while( config.contains( keyname.arg( i ) ) )
    {
      Glider *glider = new Glider();

      if( glider->load( &config, i ) )
	{
	  if( glider->getSelectionFlag() == true )
	    {
	      return glider;
	    }
	  else
	    {
	      delete glider; // this is not the glider we're looking for...
	    }
	}
      else
	{
	  delete glider; // loading failed!
	}

      i++;
    }

  config.endGroup();
  // if we end up here, there is no default: either loading failed, or
  // it is not set, or the set glider has been deleted.
  return static_cast<Glider *> ( 0 );
}

void GliderListWidget::selectItemFromReg( const QString& registration )
{
  QList<QTreeWidgetItem*> result = findItems( registration, Qt::MatchExactly, 1 );

  if( result.size() > 0 )
    {
      setCurrentItem( result.at( 0 ) );
    }
}

void GliderListWidget::selectItemFromList()
{
  for( int i = 0; i < topLevelItemCount(); i++ )
    {
      GliderItem* gi = dynamic_cast<GliderItem *>(topLevelItem(i));

      if( gi == 0 )
	{
	  continue;
	}

      if( gi->getGlider()->getSelectionFlag() == true )
	{
	  setCurrentItem( gi );
	  break;
	}
    }
}

void GliderListWidget::currentItemChanged( QTreeWidgetItem* current,
					   QTreeWidgetItem* previous)
{
  GliderItem* cur = dynamic_cast<GliderItem *>(current);
  GliderItem* pre = dynamic_cast<GliderItem *>(previous);

  if( cur != 0 )
    {
      cur->getGlider()->setSelectionFlag(true);
    }

  if( pre != 0 )
    {
      pre->getGlider()->setSelectionFlag(false);
    }
}

void GliderListWidget::clearSelectionInGliderList()
{
  for( int i = 0; i < m_gliders.size(); i++ )
    {
      // Remove selection flag on all glider objects
      m_gliders.at(i)->setSelectionFlag( false );
    }
}
