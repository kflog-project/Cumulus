/***********************************************************************
**
**   TaskPointSelectionList.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifdef QT_5
#include <QtWidgets>
#else
#include <QtGui>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "layout.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "rowdelegate.h"
#include "TaskPointSelectionList.h"

extern MapContents *_globalMapContents;

TaskPointSelectionList::TaskPointSelectionList( QWidget *parent, QString title ) :
 QWidget(parent)
{
  setObjectName( "TaskPointSelectionList" );
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  //setAttribute(Qt::WA_DeleteOnClose);

  if( title.isEmpty() == false )
    {
      setWindowTitle( title + " " + tr("Selection"));
    }

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout* mainLayout = new QHBoxLayout( this );

  m_taskpointTreeWidget = new QTreeWidget( this );

  mainLayout->addWidget( m_taskpointTreeWidget );

  m_taskpointTreeWidget->setRootIsDecorated( false );
  m_taskpointTreeWidget->setItemsExpandable( false );
  m_taskpointTreeWidget->setSortingEnabled( true );
  m_taskpointTreeWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  m_taskpointTreeWidget->setSelectionBehavior( QAbstractItemView::SelectItems );
  m_taskpointTreeWidget->setAlternatingRowColors(true);
  m_taskpointTreeWidget->setColumnCount( 2 );
  m_taskpointTreeWidget->setFocusPolicy( Qt::StrongFocus );
  m_taskpointTreeWidget->setUniformRowHeights(true);
  m_taskpointTreeWidget->setHeaderLabel( title );

  connect( m_taskpointTreeWidget, SIGNAL(itemSelectionChanged()),
           SLOT(slotItemSelectionChanged()) );

  // Set additional space per row
  RowDelegate* rowDelegate = new RowDelegate( m_taskpointTreeWidget, 10 );
  m_taskpointTreeWidget->setItemDelegate( rowDelegate );

  QTreeWidgetItem* headerItem = m_taskpointTreeWidget->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );

  m_taskpointTreeWidget->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_taskpointTreeWidget->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_taskpointTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

#ifdef ANDROID
  QScrollBar* lvsb = m_taskpointTreeWidget->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture(m_taskpointTreeWidget->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_taskpointTreeWidget->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  QVBoxLayout* groupLayout = new QVBoxLayout;

  m_groupBox = new QGroupBox( tr("Search Entry") );
  m_groupBox->setLayout( groupLayout );
  mainLayout->addWidget( m_groupBox );

  m_searchInput = new QLineEdit;
  groupLayout->addWidget( m_searchInput );

  Qt::InputMethodHints imh = (m_searchInput->inputMethodHints() | Qt::ImhNoPredictiveText);
  m_searchInput->setInputMethodHints(imh);

#ifndef ANDROID
  m_searchInput->setToolTip( tr("Enter a search string, to navigate to a certain list entry.") );
#endif

  connect( m_searchInput, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  connect( m_searchInput, SIGNAL(textEdited(const QString&)),
           SLOT(slotTextEdited(const QString&)) );

  m_RBCol0 = new QRadioButton( tr("  1  ") );
  m_RBCol0->setChecked( true );
  m_RBCol0->setEnabled( true );

  m_RBCol1 = new QRadioButton( tr("  2  ") );
  m_RBCol1->setChecked( false );
  m_RBCol1->setEnabled( true );

  connect( m_RBCol0, SIGNAL(released()), SLOT(slotClearSearchEntry()) );
  connect( m_RBCol1, SIGNAL(released()), SLOT(slotClearSearchEntry()) );

  QHBoxLayout* rbLayout = new QHBoxLayout;
  rbLayout->addWidget( m_RBCol0 );
  rbLayout->addWidget( m_RBCol1 );

  QGroupBox* rbBox = new QGroupBox( tr("Search in column") );
  rbBox->setLayout( rbLayout );

  QHBoxLayout* clearLayout = new QHBoxLayout;
  clearLayout->setSpacing(0);
  clearLayout->addWidget( rbBox );
  clearLayout->addStretch(5);
  QPushButton* clearButton = new QPushButton(tr("Clear"));

#ifndef ANDROID
  clearButton->setToolTip( tr("Click Clear to remove the search string.") );
#endif

  clearLayout->addWidget( clearButton );
  groupLayout->addLayout( clearLayout );
  groupLayout->addStretch(5);

  connect( clearButton, SIGNAL(clicked()), SLOT(slotClearSearchEntry()));

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  connect( cancel, SIGNAL(clicked()), SLOT(slotReject()) );

  m_ok = new QPushButton(this);
  m_ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  m_ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  m_ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  m_ok->setEnabled( false );

  connect( m_ok, SIGNAL(clicked()), SLOT(slotAccept()) );

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(m_ok, 1);
  buttonBox->addStretch(2);
  mainLayout->addLayout(buttonBox);
}

TaskPointSelectionList::~TaskPointSelectionList()
{
}

void TaskPointSelectionList::showEvent( QShowEvent *event )
{
  QWidget::showEvent( event );
}

void TaskPointSelectionList::fillSelectionListWithAirfields()
{
  int searchList[] = { MapContents::GliderfieldList, MapContents::AirfieldList };

  m_searchInput->clear();
  m_taskpointTreeWidget->clear();

  for( int l = 0; l < 2; l++ )
    {
      for( uint loop = 0; loop < _globalMapContents->getListLength(searchList[l]); loop++ )
        {
          Airfield* af = static_cast<Airfield *>(_globalMapContents->getElement(searchList[l], loop ));
          PointItem* item = new PointItem( af->getICAO(), af->getName(), af );
          item->setFlags( Qt::ItemIsSelectable|Qt::ItemIsEnabled );
          m_taskpointTreeWidget->addTopLevelItem( item );
        }
    }

  setTreeHeader( "ICAO", tr("Airfields") );
  m_taskpointTreeWidget->sortItems( 1, Qt::AscendingOrder );
}

void TaskPointSelectionList::fillSelectionListWithOutlandings()
{
  m_searchInput->clear();
  m_taskpointTreeWidget->clear();

  QList<Airfield> olList = _globalMapContents->getQutlandingList();

  for( int loop = 0; loop < olList.size(); loop++ )
    {
      Airfield ol = olList.at(loop);
      PointItem* item = new PointItem( ol.getWPName(), ol.getName(), &ol );
      m_taskpointTreeWidget->addTopLevelItem( item );
    }

  setTreeHeader( tr("Short"), tr("Outlandings") );
  m_taskpointTreeWidget->sortItems( 1, Qt::AscendingOrder );
}

void TaskPointSelectionList::fillSelectionListWithNavaids()
{
  m_searchInput->clear();
  m_taskpointTreeWidget->clear();

  QList<RadioPoint> rpList = _globalMapContents->getRadioPointList();

  for( int loop = 0; loop < rpList.size(); loop++ )
    {
      RadioPoint rp = rpList.at(loop);
      PointItem* item = new PointItem( rp.getWPName(), rp.getName(), &rp );
      m_taskpointTreeWidget->addTopLevelItem( item );
    }

  setTreeHeader( tr("Short"), tr("Navaids") );
  m_taskpointTreeWidget->sortItems( 1, Qt::AscendingOrder );
}

void TaskPointSelectionList::fillSelectionListWithHotspots()
{
  m_searchInput->clear();
  m_taskpointTreeWidget->clear();

  QList<SinglePoint>& hsList = _globalMapContents->getHotspotList();

  for( int loop = 0; loop < hsList.size(); loop++ )
    {
      SinglePoint hsp = hsList.at(loop);
      PointItem* item = new PointItem( hsp.getWPName(), hsp.getName(), &hsp );
      m_taskpointTreeWidget->addTopLevelItem( item );
    }

  setTreeHeader( tr("Short"), tr("Hotspots") );
  m_taskpointTreeWidget->sortItems( 1, Qt::AscendingOrder );
}

void TaskPointSelectionList::fillSelectionListWithWaypoints()
{
  m_searchInput->clear();
  m_taskpointTreeWidget->clear();

  QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

  for( int loop = 0; loop < wpList.size(); loop++ )
    {
      Waypoint wp = wpList.at(loop);
      PointItem* item = new PointItem( wp.name, wp.description, &wp );
      m_taskpointTreeWidget->addTopLevelItem( item );
    }

  setTreeHeader( tr("Short"), tr("Waypoints") );
  m_taskpointTreeWidget->sortItems( 0, Qt::AscendingOrder );
}

TaskPointSelectionList::PointItem::PointItem( QString item0, QString item1, SinglePoint* sp ) :
  QTreeWidgetItem(),
  point(sp),
  deleteSinglePoint(false)
{
  setText( 0, item0);
  setText( 1, item1);
}

TaskPointSelectionList::PointItem::PointItem( QString item0, QString item1, Waypoint* wp ) :
  QTreeWidgetItem(),
  point(0),
  deleteSinglePoint(true)
{
  // A SinglePoint object must be created due to a Waypoint is not derived from
  // a SinglePoint.
  point = new SinglePoint( wp->name,
                           wp->description,
                           static_cast<BaseMapElement::objectType>(wp->type),
                           wp->wgsPoint,
                           wp->projPoint,
                           wp->elevation,
                           wp->country,
                           wp->comment );
  setText( 0, item0);
  setText( 1, item1);
}

void TaskPointSelectionList::slotClearSearchEntry()
{
  m_searchInput->clear();

  if( m_taskpointTreeWidget->topLevelItemCount() > 0 )
    {
      m_taskpointTreeWidget->clearSelection();
      m_taskpointTreeWidget->setCurrentItem( m_taskpointTreeWidget->topLevelItem( 0 ),
                                             getSearchColumn() );
    }
}

void TaskPointSelectionList::slotTextEdited( const QString& text )
{
  if( text.size() == 0 )
    {
      m_taskpointTreeWidget->clearSelection();
      m_taskpointTreeWidget->setCurrentItem( m_taskpointTreeWidget->topLevelItem( 0 ),
                                             getSearchColumn() );
      m_taskpointTreeWidget->scrollToItem( m_taskpointTreeWidget->topLevelItem( 0 ),
					                                 QAbstractItemView::PositionAtTop );
      return;
    }

  QList<QTreeWidgetItem *> items = m_taskpointTreeWidget->findItems( text,
                                                                     Qt::MatchStartsWith,
                                                                     getSearchColumn() );
  if( items.size () > 0 )
    {
      m_taskpointTreeWidget->setCurrentItem( items.at(0), getSearchColumn() );
      m_taskpointTreeWidget->scrollToItem( items.at (0),
					                                 QAbstractItemView::PositionAtTop );
    }
}

void TaskPointSelectionList::slotItemSelectionChanged()
{
  QList<QTreeWidgetItem *> selList = m_taskpointTreeWidget->selectedItems();

  if( selList.size() == 0 )
    {
      // No items selected, disable ok button
      m_ok->setEnabled( false );
    }
  else
    {
      // Items selected, enable ok button
      m_ok->setEnabled( true );
    }
}

void TaskPointSelectionList::slotAccept()
{
  QList<QTreeWidgetItem *> selList = m_taskpointTreeWidget->selectedItems();

  if( selList.size() == 0 )
    {
      QWidget::close();
      return;
    }

  QTreeWidgetItem* li = selList.at(0);

  if( li == static_cast<QTreeWidgetItem *> (0) )
    {
      QWidget::close();
      return;
    }

  PointItem* pi = dynamic_cast<PointItem *> ( li );

  if( pi == 0 )
    {
      QWidget::close();
      return;
    }

  if( pi->getPoint() != 0 )
    {
      emit takeThisPoint( pi->getPoint() );
    }

  QWidget::close();
}

void TaskPointSelectionList::slotReject()
{
  QWidget::close();
}
