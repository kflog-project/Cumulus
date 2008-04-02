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
#include <Q3ListViewItemIterator>

#include "listviewfilter.h"
#include "generalconfig.h"

const uint buttonCount=5;

ListViewFilter::ListViewFilter(Q3ListView *lv, QWidget *parent, const char *name) : QWidget(parent,name)
{
    _lv=lv;
    QPushButton * cmd;
    QHBoxLayout * layout=new QHBoxLayout(this);
    layout->setMargin(0);

    QSignalMapper * smap=new QSignalMapper(this, "signal mapper");
    connect(smap, SIGNAL(mapped(int)), this, SLOT(cmdPush(int)));
    _rootFilter=0;
    _activeFilter=0;

    for (uint i=0; i!=buttonCount; i++) {
        cmd = new QPushButton(this);
        cmd->setMinimumWidth(25);
        layout->addWidget(cmd);
        _buttonList.append(cmd);
        smap->setMapping(cmd, i);
        connect(cmd, SIGNAL(clicked()), smap, SLOT(map()));
    }
    if (lv->childCount()<6) {
        this->hide();
    }
}


ListViewFilter::~ListViewFilter()
{
    delete _rootFilter;
}


void ListViewFilter::reset(bool forget)
{
    if (_lv==NULL) {
        return;
    }

    if(!forget) {
        restoreListViewItems();
    }
    //renew our filtertree
    delete _rootFilter;
    _activeFilter=NULL;
    _rootFilter=new ListViewFilterItem();

    //copy pointers to all items in the listview to the root filter's itemlist
    for (Q3ListViewItemIterator it ( _lv ); it.current(); it++) {
        _rootFilter->items.append( it.current() );
    }

    if (_rootFilter->items.count()<6) {
        this->hide();
    } else {
        this->show();
    }
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
        _rootFilter->addToList(_lv);
    }
}


void ListViewFilter::activateFilter(ListViewFilterItem * filter, int shrink)
{
    //make sure we have subdivided our list
    _activeFilter=filter;
    QString teststr;

    //setup buttons
    uint i;
    teststr="";
    int spacefactor=0;

    //qDebug("shrink: %d",shrink);
    if(shrink > 4) {
      spacefactor=2;
    }
    else if(shrink > 2) {
      spacefactor=shrink-2;
    }

    if (_activeFilter!=_rootFilter)  //normal filters, not the root filter
    {
        filter->devide(buttonCount-1-spacefactor);    //we can's use the first button, so devide into (buttonCount-1) parts
        _buttonList.at(0)->show();
        //_buttonList.at(0)->setText(tr("Up"));       //first button swithces to the filter above this one
        _buttonList.at(0)->setPixmap(GeneralConfig::instance()->loadPixmap("moveup.png")); // icon is more clear than text.
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
                    _buttonList.at(i)->setText((filter->subfilters.at(i-1)->from).lower());
                } else {
                    _buttonList.at(i)->setText(filter->subfilters.at(i-1)->from);
                }
            } else {
                if (shrink>=2) {
                    _buttonList.at(i)->setText((filter->subfilters.at(i-1)->from + "-" + filter->subfilters.at(i-1)->to).lower());
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
            this->activateFilter(filter,shrink);
        }
    } else {                                     //this is the root filter
        filter->devide(buttonCount);   //devide in (buttonCount) parts, as the first button can be used just as the others
        _buttonList.at(0)->setPixmap(QPixmap());
        for (i=0; i<filter->subfilters.count(); i++) {
            if (filter->subfilters.at(i)->items.count()>0) {
                _buttonList.at(i)->show();
            } else {
                _buttonList.at(i)->hide();
                continue;
            }
            
            if (filter->subfilters.at(i)->from ==  filter->subfilters.at(i)->to) {
                if (shrink>=2) {
                    _buttonList.at(i)->setText((filter->subfilters.at(i)->from).lower());
                } else {
                    _buttonList.at(i)->setText(filter->subfilters.at(i)->from);
                }
            } else {
                if (shrink>=2) {
                    _buttonList.at(i)->setText((filter->subfilters.at(i)->from + "-" + filter->subfilters.at(i)->to).lower());
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
            this->activateFilter(filter,shrink);
        }

    }

    while (i<buttonCount) {  //hide any buttons that may be left over
        _buttonList.at(i++)->hide();
    }

    _lv->setUpdatesEnabled(false);
    uint cnt=_lv->childCount();
    for (i=0;i<cnt;i++) {
        _lv->takeItem(_lv->firstChild());
    }
    for (i=0;i<filter->items.count();i++) {
        _lv->insertItem(filter->items.at(i));
        //qDebug("re-inserted item '%s'",filter->items.at(i)->text(0).latin1());
    }
    _lv->setUpdatesEnabled(true);
    _lv->triggerUpdate();
}


void ListViewFilter::off()
{
    if (_rootFilter) {
        activateFilter(_rootFilter);
    }
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


void ListViewFilterItem::addToList(Q3ListView * lv, bool isRecursive)
{
    lv->setUpdatesEnabled(false);
    for (uint i=0; i<items.count(); i++) {
        lv->insertItem( items.at(i) );
    }
    /*
    for (uint i=0; i<subfilters.count();i++)
    {
      subfilters.at(i)->addToList( lv, true );
    }  */

    if (!isRecursive) {
        lv->setUpdatesEnabled( true );
        lv->triggerUpdate();
    }
}


void ListViewFilterItem::devide(int partcount)
{
    if (_split) {
        return;  //no need to split an allready splitted filter item.
    }

    uint totalLen=items.count();
    uint avgPLen=totalLen/partcount;
    if (totalLen<6) {
        return;  //5 or less items in this list, we're not going to split this further.
    }

    subfilters.clear();   //remove any current filters in the set

    uint tmpMinDiff;
    uint tmpMinPos;
    uint startPos;
    bool reachedMax;
    uint spread;
    uint tmpDiff;
    ListViewFilterItem * itm;
    uint maxSpread=avgPLen/3;   //the maximum distance we'll search is 1/3 of the length of the average list

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
        itm->from=items.at(pos[i-1]+1)->text(0).left(diff[i-1]).upper();
        itm->to=items.at(pos[i])->text(0).left(diff[i]).upper();
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
    QString S1=s1.upper();
    QString S2=s2.upper();
    int pos=0;
    int max=MIN(S1.length(), S2.length());
    while(pos<max) {
        //  qDebug (" S1='%s' S2='%s'  level=%d", S1.latin1(), S2.latin1(), pos+1);
        if (S1.mid(pos,1)!=S2.mid(pos,1)) {
            return pos+1;
        }
        pos++;
    }
    return pos;
}
