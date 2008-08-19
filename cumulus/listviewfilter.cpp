/***********************************************************************
**
**   listviewfilter.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QSignalMapper>
#include <QHBoxLayout>

#include "listviewfilter.h"
#include "generalconfig.h"

const uint buttonCount=5;

ListViewFilter::ListViewFilter(QTreeWidget *tw, QWidget *parent) : QWidget(parent)
{
  _tw=tw;
  QPushButton* cmd;
  QHBoxLayout* layout=new QHBoxLayout(this);
  layout->setMargin(0);
  setMinimumWidth( 5*25 );

  QSignalMapper* smap=new QSignalMapper(this);
  smap->setObjectName("signal_mapper");
  connect(smap, SIGNAL(mapped(int)), this, SLOT(cmdPush(int)));
  _rootFilter=0;
  _activeFilter=0;
  showIndex=0;
  recursionLevel=0;

  for (uint i=0; i!=buttonCount; i++) {
    cmd = new QPushButton(this);
    cmd->setMinimumWidth(25);
    layout->addWidget(cmd);
    _buttonList.append(cmd);
    smap->setMapping(cmd, i);
    connect(cmd, SIGNAL(clicked()), smap, SLOT(map()));
  }
  if (tw->topLevelItemCount()<6)
    this->hide();

  QStringList sl;
  sl << " " << "Previous Page" << "(click)";
  prev = new QTreeWidgetItem( sl );

  QStringList nl;
  nl << " " << "Next Page" << "(click)";
  next = new QTreeWidgetItem( nl );
}


ListViewFilter::~ListViewFilter()
{
  // JD: Never forget to take ALL items out of the WP list before
  // deleting the filter item list ! Multiple pointers to items !
  if ( _rootFilter != 0 ) {
    while ( ! _rootFilter->items.isEmpty() )
      delete _rootFilter->items.takeLast();
    delete _rootFilter;
  }
  delete next;
  delete prev;
}


void ListViewFilter::reset(bool forget)
{
  if (_tw==NULL)
    return;

  if(!forget)
    restoreListViewItems();

  //renew our filtertree
  delete _rootFilter;
  _activeFilter=NULL;
  _rootFilter=new ListViewFilterItem();

  //copy pointers to all list items into the root filter's itemlist
  for ( int i = 0; i < _tw->topLevelItemCount(); i++)
      _rootFilter->items.append( _tw->topLevelItem(i) );

  if (_rootFilter->items.count()<6)
    this->hide();
  else
    this->show();
   
  activateFilter(_rootFilter);
}


void ListViewFilter::cmdPush(int id)
{
    if (id==0 && _rootFilter!=_activeFilter) {
        activateFilter(_activeFilter->parent);
        return;
    }

    if (_rootFilter!=_activeFilter) {
        activateFilter(_activeFilter->subfilters.at(id-1));
        return;
    } else {
        activateFilter(_activeFilter->subfilters.at(id));
        return;
    }
}


void ListViewFilter::restoreListViewItems()
{
    if (_rootFilter) {
        _rootFilter->addToList(_tw);
    }
}


void ListViewFilter::activateFilter(ListViewFilterItem * filter, int shrink)
{
    //make sure we have subdivided our list
    _activeFilter=filter;
    QString teststr;
    //setup buttons
    int i;
    teststr="";
    int spacefactor=0;

    // qDebug("shrink: %d",shrink);
    if(shrink > 4) {
      spacefactor=2;
    }
    else if(shrink > 2) {
      spacefactor=shrink-2;
    }

    if (_activeFilter!=_rootFilter)  //normal filters, not the root filter
    {
        filter->divide(buttonCount-1-spacefactor);    //we can's use the first button, so divide into (buttonCount-1) parts
        _buttonList.at(0)->show();
        _buttonList.at(0)->setText("");       //first button switches to the filter above this one
        _buttonList.at(0)->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("up.png")) ); // icon is more clear than text.
        _buttonList.at(0)->setIconSize( QSize(26,26) );
        _buttonList.at(0)->setMinimumSize(1,_buttonList.at(1)->height());
        for (i=1; i<filter->subfilters.count()+1; i++)
        {
            if (filter->subfilters.at(i-1)->items.count()>0) {
                _buttonList.at(i)->show();
            } else {
                _buttonList.at(i)->hide();
                continue;
            }
            if (filter->subfilters.at(i-1)->from ==  filter->subfilters.at(i-1)->to) {
                if (shrink>=2) {
                    _buttonList.at(i)->setText((filter->subfilters.at(i-1)->from).toLower());
                } else {
                    _buttonList.at(i)->setText(filter->subfilters.at(i-1)->from);
                }
            } else {
                if (shrink>=2) {
                    _buttonList.at(i)->setText((filter->subfilters.at(i-1)->from + "-" + filter->subfilters.at(i-1)->to).toLower());
                } else if (shrink==1) {
                    _buttonList.at(i)->setText(filter->subfilters.at(i-1)->from + "-" + filter->subfilters.at(i-1)->to);
                }
                if (shrink==0) {
                    _buttonList.at(i)->setText(filter->subfilters.at(i-1)->from + " - " + filter->subfilters.at(i-1)->to);
                }

            }
            teststr+=_buttonList.at(i)->text();
        }
        
        if (_buttonList.at(0)->fontMetrics().width(teststr)>200 && spacefactor<2) {
            shrink++;
            recursionLevel++;
            this->activateFilter(filter,shrink);
        }
    } else {                                     //this is the root filter
        filter->divide(buttonCount);   //divide in (buttonCount) parts, as the first button can be used just as the others
        _buttonList.at(0)->setIcon( QIcon(QPixmap()) );
        for (i=0; i<filter->subfilters.count(); i++) {
            if (filter->subfilters.at(i)->items.count()>0) {
                _buttonList.at(i)->show();
            } else {
                _buttonList.at(i)->hide();
                continue;
            }
            
            if (filter->subfilters.at(i)->from ==  filter->subfilters.at(i)->to) {
                if (shrink>=2) {
                    _buttonList.at(i)->setText((filter->subfilters.at(i)->from).toLower());
                } else {
                    _buttonList.at(i)->setText(filter->subfilters.at(i)->from);
                }
            } else {
                if (shrink>=2) {
                    _buttonList.at(i)->setText((filter->subfilters.at(i)->from + "-" + filter->subfilters.at(i)->to).toLower());
                } else if (shrink==1) {
                    _buttonList.at(i)->setText(filter->subfilters.at(i)->from + "-" + filter->subfilters.at(i)->to);
                }
                if (shrink==0) {
                    _buttonList.at(i)->setText(filter->subfilters.at(i)->from + " - " + filter->subfilters.at(i)->to);
                }
            }
            teststr+=_buttonList.at(i)->text();
        }
        if (_buttonList.at(0)->fontMetrics().width(teststr)>200 && spacefactor<2) {
            shrink++;
            recursionLevel++;
            this->activateFilter(filter,shrink);
        }

    }
    
    //@JD: Prevent repeating list filling and clearing. Do it once after recursion.
    if (recursionLevel > 0) {
      recursionLevel--;
      return;
    }

    while (i<(int)buttonCount) {  //hide any buttons that may be left over
        _buttonList.at(i++)->hide();
    }

    // @JD, after using QTreeWidget over Q3ListView:
    // There is a bug somewhere; crash if taking out the last airfield item from _tw.
    // Workaround: always keep one item in ...

    _tw->setUpdatesEnabled(false);

    //  A "dummy" item to prevent the list from running empty
    _tw->insertTopLevelItem( 0, next );

    // workaround: stop taking at item 1
    while ( _tw->topLevelItemCount() > 1)
      _tw->takeTopLevelItem(1);

    // show first page of current filter list
    showIndex = 0;
    showPage(false);

    // showPage will enable the updating again    
}

void ListViewFilter::showPage(bool up)
{
  int pageSize = GeneralConfig::instance()->getListDisplayPageSize();
  int maxIndex = _activeFilter->items.count() - 1;
  int maxPageIndex = 0;

  if (up) {
    if ( showIndex+pageSize > maxIndex ) {
      _tw->setUpdatesEnabled(true);
      return;
    }
    showIndex += pageSize;
  } else {  // down
    showIndex -= pageSize;
    if ( showIndex < 0 )
      showIndex = 0;
  }
  maxPageIndex = pageSize-1;

  // last page of list
  if (maxIndex-showIndex <= pageSize)
    maxPageIndex = maxIndex-showIndex;

//qDebug("airfield list pageview: showIndex %d, maxIndex %d, maxPageIndex %d", showIndex, maxIndex, maxPageIndex );

  _tw->setUpdatesEnabled(false);

  // @JD: Annother strange bug: removing all counted items (except one) with a
  // "for" loop leaves the list with lots of items left. "while" loop works

  while ( _tw->topLevelItemCount() > 1)
    _tw->takeTopLevelItem(0);

  // @JD: Still we can't be sure the list looks like expected. With the next lines
  // the whole shebang is finally working. Numerous hours wasted here

  // again, a "dummy" item to prevent the list from running empty
  _tw->insertTopLevelItem(0, next);
  while ( _tw->topLevelItemCount() > 1)
    _tw->takeTopLevelItem(1);

  QList<QTreeWidgetItem*> addList;

  // not the first page? Then add "Previous" item
  if ( showIndex > 0 )
    addList << prev;

  // add the waypoint/airfield items
  for ( int i = showIndex; i <= showIndex+maxPageIndex; i++ )
    addList << _activeFilter->items.at(i);

  // insert whole list _before_ "Next" item
  _tw->insertTopLevelItems( 0, addList );

  // last page of list
  if ( maxIndex-showIndex <= pageSize )
    _tw->takeTopLevelItem( _tw->indexOfTopLevelItem(next) );

  if ( showIndex == 0 ) {
    _tw->setCurrentItem( _tw->topLevelItem(0) );
  } else
    _tw->setCurrentItem( _tw->topLevelItem(1) );

  _tw->setFocus();
  _tw->setUpdatesEnabled(true);

//qDebug("airfield list pageview: after function showIndex %d, items in list %d", showIndex, _tw->topLevelItemCount() );
}

void ListViewFilter::off()
{
  if (_rootFilter)
    activateFilter(_rootFilter);
}


ListViewFilterItem::ListViewFilterItem(ListViewFilterItem * parent)
{
    this->parent=parent;
    from="";
    to="";
    _split=false;
}


ListViewFilterItem::~ListViewFilterItem()
{
  qDeleteAll(subfilters);
  subfilters.clear();
}


void ListViewFilterItem::addToList(QTreeWidget* tw, bool isRecursive)
{
return;
  tw->setUpdatesEnabled(false);
//  for (int i=0; i<items.count(); i++) {
//    tw->addTopLevelItem( items.at(i) );
//  }
  /*
  for (uint i=0; i<subfilters.count();i++)
  {
    subfilters.at(i)->addToList( tw, true );
  }  */

  if (!isRecursive)
    tw->setUpdatesEnabled( true );
}


void ListViewFilterItem::divide(int partcount)
{
    if (_split) {
        return;  //no need to split an allready splitted filter item.
    }

    int totalLen=items.count();
    int avgPLen=totalLen/partcount;
    if (totalLen<6) {
        return;  //5 or less items in this list, we're not going to split this further.
    }

    subfilters.clear();   //remove any current filters in the set

    int tmpMinDiff;
    int tmpMinPos;
    int startPos;
    bool reachedMax;
    int spread;
    int tmpDiff;
    ListViewFilterItem * itm;
    int maxSpread=avgPLen/3;   //the maximum distance we'll search is 1/3 of the length of the average list

    int diff[partcount+1];    //initialize arrays that will hold the differences and their positions.
    int pos[partcount+1];
    diff[0]=1;    //we need to initialize the first one
    pos[0]=-1;
    diff[partcount]=1; //and the last one
    pos[partcount]=items.count()-1;

    for (int i=0;i<partcount-1;i++) {
        startPos=(i+1)*avgPLen; //our startposition is the optimal point to split the list
        tmpMinPos=startPos;
        reachedMax=false;
        spread=0;
//qDebug ("Startpos: %d, listlength: %d, maxspread: %d", startPos, items.count(), maxSpread);
        tmpMinDiff=diffLevel(items.at(startPos)->text(0),  items.at(startPos+1)->text(0)); //get the diffLevel for this position
        while (tmpMinDiff>1 && !reachedMax)   //search within the defined limits for the optimum solution (1 is best).
        {
            spread++;
            if (((int)startPos-(int)spread+1) >= 0)
            {
                tmpDiff=diffLevel(items.at(startPos-spread)->text(0),  items.at(startPos-spread+1)->text(0));
                if (tmpDiff<tmpMinDiff) {
                    tmpMinDiff=tmpDiff;
                    tmpMinPos=startPos-spread;
                }
            }
            if (startPos+spread+1 < items.count())
            {
                tmpDiff=diffLevel(items.at(startPos+spread)->text(0),  items.at(startPos+spread+1)->text(0));
                if (tmpDiff<tmpMinDiff) {
                    tmpMinDiff=tmpDiff;
                    tmpMinPos=startPos+spread;
                }
            }
            reachedMax = (spread>=maxSpread);
        }
        diff[i+1]=tmpMinDiff;          //store the position and diffLevel we found in our array
        pos[i+1]=tmpMinPos;
//qDebug("chosen split: pos %d, difflevel %d", tmpMinPos, tmpMinDiff);
    }

    for(int i=1;i<=partcount;i++)     //now, create the actual filters with the found split points.
    {
        itm = new ListViewFilterItem(this);
        itm->from=items.at(pos[i-1]+1)->text(0).left(diff[i-1]).toUpper();
        itm->to=items.at(pos[i])->text(0).left(diff[i]).toUpper();
        subfilters.append(itm);
        for (int j=pos[i-1]+1;j<=pos[i];j++)
        {
            itm->items.append(items.at(j));
        }
    }
    _split=true;
}


int ListViewFilterItem::diffLevel(const QString& s1, const QString& s2)
{
    QString S1=s1.toUpper();
    QString S2=s2.toUpper();
    int pos=0;
    int max=MIN(S1.length(), S2.length());
    while(pos<max) {
//qDebug (" S1='%s' S2='%s'  level=%d", S1.toLatin1().data(), S2.toLatin1().data(), pos+1);
        if (S1.mid(pos,1)!=S2.mid(pos,1)) {
            return pos+1;
        }
        pos++;
    }
    return pos;
}
