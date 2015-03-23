/***********************************************************************
**
**   listviewfilter.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtGui>
#include <QHBoxLayout>

#include "layout.h"
#include "listviewfilter.h"
#include "generalconfig.h"

// Initialize static members
// Button[0] can be the one level up button
// Button[1] can be the home button
const int ListViewFilter::buttonCount = 5;

ListViewFilter::ListViewFilter(QTreeWidget *tw, QWidget *parent) : QWidget(parent)
{
  _tw = tw;
  QHBoxLayout* layout=new QHBoxLayout(this);
  layout->setContentsMargins( 0, 0, 0, 5 );
  setMinimumWidth( 5*40 );

  QSignalMapper* smap=new QSignalMapper(this);
  smap->setObjectName("signal_mapper");
  connect(smap, SIGNAL(mapped(int)), this, SLOT(slot_CmdPush(int)));
  _rootFilter=0;
  _activeFilter=0;
  _filterIndex=0;
  m_isTopButtonContained=false;

  for( int i = 0; i != buttonCount; i++)
    {
      QPushButton *cmd = new QPushButton(this);
      cmd->setMinimumWidth(40);
      layout->addWidget(cmd);
      _buttonList.append(cmd);
      smap->setMapping(cmd, i);
      connect(cmd, SIGNAL(pressed()), smap, SLOT(map()));
    }

  if( tw->topLevelItemCount() < 8 )
    {
      this->setVisible( false );
    }

  // calculate the needed icon size
  QFontMetrics qfm( font() );
  int iconSize = qfm.height() - 8;

  tw->setIconSize( QSize(iconSize, iconSize) );
}

ListViewFilter::~ListViewFilter()
{
  for( int i = 0; i < _filterList.size(); i++ )
    {
      QList<ListViewFilterItem *> &itemList = _filterList[i];

      qDeleteAll( itemList );
      itemList.clear();
    }
}

void ListViewFilter::addListItem( QTreeWidgetItem* it )
{
  if( it == static_cast<QTreeWidgetItem *> (0) )
    {
      return;
    }

  _tw->addTopLevelItem( it );
}

void ListViewFilter::removeListItem( QTreeWidgetItem* it )
{
  if( it == static_cast<QTreeWidgetItem *> (0) )
    {
      return;
    }

  // Note! According to QtForum removeItemWidget removes not the item in the tree.
  // The method takeTopLevelItem has to be used instead.
  int idx = _tw->indexOfTopLevelItem(it);

  if( idx != -1 )
    {
      QTreeWidgetItem* item = _tw->takeTopLevelItem( idx );

      if( item )
        {
          delete item;
        }
    }
}

void ListViewFilter::reset()
{
  // qDebug() << "ListViewFilter::reset()";
  clear();

  if( _tw == static_cast<QTreeWidget *> (0) || _tw->topLevelItemCount() == 0 )
    {
      return;
    }

  _tw->sortItems( 0, Qt::AscendingOrder );

  // switch on all list elements
  for( int i = 0; i < _tw->topLevelItemCount(); i++ )
    {
      _tw->topLevelItem( i )->setHidden( false );
    }

  if( _tw->topLevelItemCount() < 6 )
    {
      this->setVisible( false );
    }
  else
    {
      this->setVisible( true );
    }

  // setup a new root filter
  _rootFilter = new ListViewFilterItem( _tw, 0 );

  _rootFilter->beginIdx = 0;
  _rootFilter->endIdx   = _tw->topLevelItemCount();

  QList<ListViewFilterItem *> itemList;
  itemList.append( _rootFilter );
  _filterList.append( itemList );

  activateFilter( _rootFilter );
}

/** All filter items are deleted. Use this before a reload of that view. */
void ListViewFilter::clear()
{
  // qDebug() << "ListViewFilter::clear()";
  _filterIndex = 0;
  setTopButtonContained( false );
  _buttonList.at( 1 )->setIcon( QIcon( QPixmap() ) );

  for( int i = 0; i < _filterList.size(); i++ )
    {
      QList<ListViewFilterItem *> &itemList = _filterList[i];

      qDeleteAll( itemList );
      itemList.clear();
    }

  _filterList.clear();

  _rootFilter   = static_cast<ListViewFilterItem *> (0);
  _activeFilter = static_cast<ListViewFilterItem *> (0);

  // Switch off all filter buttons.
  for( int i = 0; i < _buttonList.count(); i++ )
    {
       _buttonList.at( i )->setVisible( false );
    }
}

/** A push button has been pressed. */
void ListViewFilter::slot_CmdPush( int id )
{
  // qDebug() << "slot_CmdPush" << "ID=" << id << "_filterIndex" << _filterIndex;

  // Reset document icon
  _buttonList.at( 1 )->setIcon( QIcon( QPixmap() ) );

  if( id == 1 && _rootFilter != _activeFilter && isTopButtonContained() )
    {
      // Top button is pressed
      setTopButtonContained( false );
      reset();
      return;
    }

  // Reset TopButtonContained flag in every case.
  setTopButtonContained( false );

  if( id == 0 && _rootFilter != _activeFilter )
    {
      // Go one level back
      _activeFilter = _activeFilter->_parent;

      // Remove subfilters
      QList<ListViewFilterItem *> subFilters = _filterList.takeAt( _filterIndex );
      qDeleteAll( subFilters );
      _filterIndex--;

      subFilters = _filterList.at( _filterIndex );

      int start = 1;

      // Show buttons
      if( _activeFilter == _rootFilter )
        {
          // Root filter has no back icon and one real button more.
          _buttonList.at( 0 )->setIcon( QIcon( QPixmap() ) );
          start = 0;
        }

      for( int i = 0; i < _buttonList.count(); i++ )
        {
          int j = i + start;

          if( j >= _buttonList.count() )
            {
              break;
            }

          if( i < subFilters.count() && subFilters.at( i )->itemCount() > 0 )
            {
              _buttonList.at( j )->setVisible( true );
              _buttonList.at( j )->setText( subFilters.at( i )->buttonText );
            }
          else
            {
              _buttonList.at( j )->setVisible( false );
            }
        }

      // show tree elements of filter
      _activeFilter->showFilterItems();
      return;
    }

  // Go one level forward, means do further splitting
  QList<ListViewFilterItem *> subFilters = _filterList.at( _filterIndex );

  if( _rootFilter != _activeFilter )
    {
      // Button 0 is the return button and has no entry in the subFilters list.
      // Therefore the id must be reduced by one.
      activateFilter( subFilters.at( id - 1 ) );
    }
  else
    {
      activateFilter( subFilters.at( id ) );
    }
}

void ListViewFilter::activateFilter( ListViewFilterItem* filter, int shrink )
{
  // qDebug() << "ListViewFilter::activateFilter: shrink=" << shrink;

    // save the active filter
  _activeFilter = filter;

  int i, start;

  // list to save subfilters
  QList<ListViewFilterItem *> subFilters;

  // Determine the possible subfilter item count. We assume minimal 8 entries
  // per subfilter page.
  int newPartCount = (filter->itemCount() / 8) + 1;

  if( newPartCount >= buttonCount )
    {
      if( _activeFilter == _rootFilter )
        {
          newPartCount = buttonCount;
        }
      else
        {
          newPartCount = buttonCount - 1;
        }
    }

  newPartCount -= shrink;

  if( newPartCount < 1 )
    {
      // That here shall prevent an endless loop.
      qWarning() << "ListViewFilter: Button count < 1, cannot create filters!";
      return;
    }

  if( _activeFilter != _rootFilter )
    {
      // Normal filters, not the root filter. The first button is reserved
      // for the return to the parent filter.
      start = 1;

      if( newPartCount >= buttonCount )
        {
          // We can't use the first button if condition is true.
          // So divide into (newPartCount-1) parts
          newPartCount--;
        }

      filter->divide( newPartCount, subFilters );

      _buttonList.at( 0 )->show();

      // first button switches to the filter above this one
      _buttonList.at( 0 )->setText( "" );

      // An icon is more clear than text.
      QPixmap upPm = GeneralConfig::instance()->loadPixmap( "up.png", true );
      _buttonList.at( 0 )->setIcon( QIcon( upPm ) );
      _buttonList.at( 0 )->setIconSize( QSize( upPm.width(), upPm.height() ) );
      _buttonList.at( 0 )->setMinimumSize( 1, _buttonList.at( 1 )->height() );
    }
  else
    {
      // This is the root filter.
      // Divide in (buttonCount) parts, as the first button can be used just as the others
      filter->divide( newPartCount, subFilters );
      _buttonList.at( 0 )->setIcon( QIcon( QPixmap() ) );
      start = 0;
    }

  // qDebug() << "SubFilters=" << subFilters.count() << "Start=" << start;

  for( i = 0; i < _buttonList.count(); i++ )
    {
      // The root filter has one filter button more as the subfilters.
      // Button 0 of a subfilter is the return button to the upper level.
      // Therefore the button array is addressed with j to consider that.
      int j = i + start;

      if( j >= _buttonList.count() )
        {
          break;
        }

      if( i < subFilters.count() && subFilters.at( i )->itemCount() > 0 )
        {
          _buttonList.at( j )->setVisible( true );
        }
      else
        {
          _buttonList.at( j )->setVisible( false );
          continue;
        }

      if( subFilters.at( i )->from == subFilters.at( i )->to )
        {
          if( shrink >= 2 )
            {
              _buttonList.at( j )->setText( (subFilters.at( i )->from).toUpper() );
            }
          else
            {
              _buttonList.at( j )->setText( subFilters.at( i )->from );
            }
        }
      else
        {
          if( shrink >= 2 )
            {
              _buttonList.at( j )->setText( (subFilters.at( i )->from + "-" +
                                             subFilters.at( i )->to).toUpper() );
            }
          else
            {
              _buttonList.at( j )->setText( subFilters.at( i )->from + "-" +
                                            subFilters.at( i )->to );
            }
         }

      // Save button text into related list view filter item.
      subFilters.at( i )->buttonText = _buttonList.at( j )->text();
    }

  // Check, if the button text has enough space in the button widget
  for( i = start; i < _buttonList.count(); i++ )
    {
      if( _buttonList.at(i)->isVisible() == false )
        {
          // Button is invisible, ignore it.
          continue;
        }

      QString buttonText = _buttonList.at( i )->text();

      int buttonWidth = _buttonList.at(i)->width();

      int textWidth = _buttonList.at( i )->fontMetrics().width( buttonText );

      /*
      qDebug() << "activateFilter: Text=" << buttonText
               << "buttonWidth" << buttonWidth
               << "textWidth" << textWidth;
      */

      if( textWidth > buttonWidth && newPartCount > 2 )
        {
          // Remove allocated list items
          qDeleteAll( subFilters );

          // Call again the splitting method
          activateFilter( filter, ++shrink );
          return;
        }
    }

  // Check, if the last split layer in reached. in this case only the one level
  // up button is contained. We add a second button to the list, a home button
  // to go back to the beginning of the displayed list.
  if( subFilters.count() == 0 && _activeFilter != _rootFilter)
    {
      _buttonList.at( 1 )->show();

      // first button switches to the filter above this one
      _buttonList.at( 1 )->setText( tr("") );

      // An icon is more clear than text.
      QPixmap upPm = GeneralConfig::instance()->loadPixmap( "dokument26.png", true );
      _buttonList.at( 1 )->setIcon( QIcon( upPm ) );
      _buttonList.at( 1 )->setIconSize( QSize( upPm.width(), upPm.height() ) );
      _buttonList.at( 1 )->setMinimumSize( 1, _buttonList.at( 2 )->height() );
      setTopButtonContained( true );
    }

  // save new subfilter set
  _filterIndex++;
  _filterList.append( subFilters );

  // show first page of current filter list
  _activeFilter->showFilterItems();
}


void ListViewFilter::off()
{
  // qDebug() << "ListViewFilter::off(): filterIndex=" << _filterIndex;

  if( ! _rootFilter )
    {
      return;
    }

  QList<ListViewFilterItem *> subFilters;

  while( _filterIndex > 1 )
    {
      subFilters = _filterList.takeAt( _filterIndex );
      qDeleteAll( subFilters );
      _filterIndex--;
    }

  // Set active filter to root filter
  _activeFilter = _rootFilter;

  // Set filter buttons
  subFilters = _filterList.at( _filterIndex );

  // Root filter has no back icon.
  _buttonList.at( 0 )->setIcon( QIcon( QPixmap() ) );
  _buttonList.at( 1 )->setIcon( QIcon( QPixmap() ) );

  // Show all buttons
  for( int i = 0; i < _buttonList.count(); i++ )
    {
      if( i < subFilters.count() && subFilters.at( i )->itemCount() > 0 )
        {
          _buttonList.at( i )->setVisible( true );
          _buttonList.at( i )->setText( subFilters.at( i )->buttonText );
        }
      else
        {
          _buttonList.at( i )->setVisible( false );
        }
    }

  // show tree elements of filter
  _rootFilter->showFilterItems();
}

ListViewFilterItem::ListViewFilterItem(QTreeWidget *tw, ListViewFilterItem *parent) :
  _parent( parent ),
  _tw(tw),
  from(""),
  to(""),
  buttonText(""),
  beginIdx(-1),
  endIdx(-1)
{
}

ListViewFilterItem::~ListViewFilterItem()
{
  // qDebug() << "~ListViewFilterItem()" << this;
}

void ListViewFilterItem::divide( int partCount, QList<ListViewFilterItem *> &subFilters )
{
  // qDebug() << "ListViewFilterItem::divide() partCount=" << partCount;

  if( partCount < 2 )
    {
      // No need to further splits because part count is too less.
      // qDebug() << "divide! partCount is less than 2";
      return;
    }

  int totalLen = itemCount();
  int avgPLen = totalLen / partCount;

  // qDebug() << "itemCount=" << itemCount() << "avgPLen=" << avgPLen;

  if( totalLen < 9 )
    {
     // 8 or less items in this list, we're not going to split this further.
      return;
    }

  int tmpMinDiff;
  int tmpMinPos;
  int startPos;
  int spread;
  int tmpDiff;
  bool reachedMax;

  // the maximum distance we'll search is 1/3 of the length of the average list
  int maxSpread = avgPLen / 3;

  // initialize arrays that will hold the differences and their positions.
  int diff[partCount + 1];
  int pos[partCount + 1];

  diff[0]         = 1; // we need to initialize the first one
  diff[partCount] = 1; // and the last one

  pos[0]         = beginIdx - 1;
  pos[partCount] = endIdx - 1;

  for( int i = 0; i < partCount - 1; i++ )
    {
      // our start position is the optimal point to split the list
      startPos = ((i + 1) * avgPLen) + beginIdx;
      tmpMinPos = startPos;
      reachedMax = false;
      spread = 0;

      // qDebug ("StartPos: %d, listLength: %d, maxSpread: %d", startPos, itemCount(), maxSpread);

      // get the diffLevel for this position
      tmpMinDiff = diffLevel( itemTextAt( startPos ), itemTextAt( startPos + 1 ) );

      while( tmpMinDiff > 1 && !reachedMax )
        {
          // search within the defined limits for the optimum solution (1 is best).
          spread++;

          if( (startPos - spread + 1) >= 0 )
            {
              tmpDiff = diffLevel( itemTextAt( startPos - spread ),
                                   itemTextAt( startPos - spread + 1 ) );

              if( tmpDiff < tmpMinDiff )
                {
                  tmpMinDiff = tmpDiff;
                  tmpMinPos = startPos - spread;
                }
            }

          if( startPos + spread + 1 < itemCount() )
            {
              tmpDiff = diffLevel( itemTextAt( startPos + spread ),
                                   itemTextAt( startPos + spread + 1 ) );

              if( tmpDiff < tmpMinDiff )
                {
                  tmpMinDiff = tmpDiff;
                  tmpMinPos = startPos + spread;
                }
            }

          reachedMax = (spread >= maxSpread);
        }

      // store the position and diffLevel we found in our array
      diff[i + 1] = tmpMinDiff;
      pos[i + 1]  = tmpMinPos;

      // qDebug( "Chosen split: pos %d, diffLevel %d", tmpMinPos, tmpMinDiff );
    }

  /*
  qDebug() << "Filter to Divide:"
           << "beginIdx=" << beginIdx
           << "endIdx" << endIdx;
  */

  // Set the begin and end diff element to its found neighbor
  diff[0]         = diff[1];
  diff[partCount] = diff[partCount-1];

  // Now, create the actual filters with the found split points and put them
  // into the result list.
  for( int i = 1; i <= partCount; i++ )
    {
      ListViewFilterItem *itm = new ListViewFilterItem( _tw, this );

      itm->from = itemTextAt( pos[i - 1] + 1 ).left( diff[i - 1] );
      itm->to   = itemTextAt( pos[i] ).left( diff[i] );

      itm->beginIdx = pos[i - 1] + 1;
      itm->endIdx   = pos[i] + 1;

      subFilters.append( itm );

      /*
      qDebug() << "Divide: i=" << i
               << "fromTxt=" << itm->from
               << "toTxt=" << itm->to
               << "beginIdx=" << itm->beginIdx
               << "endIdx" << itm->endIdx;
      */
    }
}

/** Returns the first text element at the item position */
QString ListViewFilterItem::itemTextAt( const int pos )
{
  QTreeWidgetItem *it = _tw->topLevelItem( pos );

  if( it )
    {
      // qDebug() << "Pos=" << pos << "Text=" << it->text(0);
      return it->text(0);
    }

  return "";
}

int ListViewFilterItem::diffLevel(const QString& s1, const QString& s2)
{
  QString S1 = s1.toUpper();
  QString S2 = s2.toUpper();
  int pos = 0;
  int max = qMin( S1.length(), S2.length() );

  while( pos < max )
    {
      // qDebug (" S1='%s' S2='%s' level=%d", S1.toLatin1().data(), S2.toLatin1().data(), pos+1);

      if( S1[pos] != S2[pos] )
        {
          return pos + 1;
        }

      pos++;
    }

  return pos;
}

/** Make all items of the filter visible. */
void ListViewFilterItem::showFilterItems()
{
  // switch off all list elements
  for( int i = 0; i < beginIdx && i < _tw->topLevelItemCount(); i++ )
    {
      _tw->topLevelItem( i )->setHidden( true );
    }

  // switch on all list elements
  for( int i = beginIdx; i < endIdx && i < _tw->topLevelItemCount(); i++ )
    {
      _tw->topLevelItem( i )->setHidden( false );
    }

  // switch off all list elements
  for( int i = endIdx; i < _tw->topLevelItemCount(); i++ )
    {
      _tw->topLevelItem( i )->setHidden( true );
    }

  if( itemCount() > 0 )
    {
      // Set focus at first list item
      _tw->setCurrentItem( _tw->topLevelItem( beginIdx ) );
    }
}
