/***********************************************************************
**
**   AirfieldSelectionList.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2016-2018 by Axel Pauli
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

#include "AirfieldSelectionList.h"
#include "layout.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "rowdelegate.h"

extern MapContents *_globalMapContents;

AirfieldSelectionList::AirfieldSelectionList( QWidget *parent ) :
 QWidget(parent)
{
  setObjectName( "AirfieldSelectionList" );
  setWindowTitle(tr("Airfield Selection"));
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout* mainLayout = new QHBoxLayout( this );

  m_airfieldTreeWidget = new QTreeWidget( this );

  mainLayout->addWidget( m_airfieldTreeWidget );

  m_airfieldTreeWidget->setRootIsDecorated( false );
  m_airfieldTreeWidget->setItemsExpandable( false );
  m_airfieldTreeWidget->setSortingEnabled( true );
  m_airfieldTreeWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  m_airfieldTreeWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_airfieldTreeWidget->setAlternatingRowColors(true);
  m_airfieldTreeWidget->setColumnCount( 1 );
  m_airfieldTreeWidget->setFocusPolicy( Qt::StrongFocus );
  m_airfieldTreeWidget->setUniformRowHeights(true);
  m_airfieldTreeWidget->setHeaderLabel( tr( "Airfields" ) );

  // adapt icon size to font size
  const int iconSize = Layout::iconSize( font() );
  m_airfieldTreeWidget->setIconSize( QSize(iconSize, iconSize) );

  connect( m_airfieldTreeWidget, SIGNAL(itemSelectionChanged()),
           SLOT(slotItemSelectionChanged()) );

  // Set additional space per row
  RowDelegate* rowDelegate = new RowDelegate( m_airfieldTreeWidget, 10 );
  m_airfieldTreeWidget->setItemDelegate( rowDelegate );

  QTreeWidgetItem* headerItem = m_airfieldTreeWidget->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );

  m_airfieldTreeWidget->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_airfieldTreeWidget->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef ANDROID
  QScrollBar* lvsb = m_airfieldTreeWidget->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture(m_airfieldTreeWidget->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_airfieldTreeWidget->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  QVBoxLayout* groupLayout = new QVBoxLayout;

  m_groupBox = new QGroupBox( tr("Search Airfield"));
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

  QHBoxLayout* clearLayout = new QHBoxLayout;
  clearLayout->setSpacing(0);
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

AirfieldSelectionList::~AirfieldSelectionList()
{
}

void AirfieldSelectionList::showEvent( QShowEvent *event )
{
  fillSelectionList();
  QWidget::showEvent( event );
}

void AirfieldSelectionList::fillSelectionList()
{
  int searchList[] = { MapContents::GliderfieldList, MapContents::AirfieldList };

  m_searchInput->clear();
  m_airfieldTreeWidget->clear();

  for( int l = 0; l < 2; l++ )
    {
      for( uint loop = 0; loop < _globalMapContents->getListLength(searchList[l]); loop++ )
      {
        SinglePoint *hitElement = (SinglePoint *) _globalMapContents->getElement(searchList[l], loop );

        PointItem* item = new PointItem( hitElement );
        m_airfieldTreeWidget->addTopLevelItem( item );
      }
    }

  m_airfieldTreeWidget->sortItems( 0, Qt::AscendingOrder );
}

AirfieldSelectionList::PointItem::PointItem( SinglePoint* sp ) :
  QTreeWidgetItem(),
  point(sp)
{
  setText( 0, sp->getName() );

  // set type icon
  QPixmap pm = _globalMapConfig->getPixmap(sp->getTypeID(), false);
  setIcon( 0, QIcon( pm) );
}

void AirfieldSelectionList::slotClearSearchEntry()
{
  m_searchInput->clear();

  if( m_airfieldTreeWidget->topLevelItemCount() > 0 )
    {
      m_airfieldTreeWidget->setCurrentItem( m_airfieldTreeWidget->topLevelItem( 0 ) );
      m_airfieldTreeWidget->clearSelection();
    }
}

void AirfieldSelectionList::slotTextEdited( const QString& text )
{
  if( text.size() == 0 )
    {
      m_airfieldTreeWidget->setCurrentItem( m_airfieldTreeWidget->topLevelItem( 0 ) );
      m_airfieldTreeWidget->clearSelection();
      m_airfieldTreeWidget->scrollToItem( m_airfieldTreeWidget->topLevelItem( 0 ),
                                          QAbstractItemView::PositionAtTop);
      return;
    }

  QList<QTreeWidgetItem *> items = m_airfieldTreeWidget->findItems( text, Qt::MatchStartsWith );

  if( items.size () > 0 )
    {
      m_airfieldTreeWidget->setCurrentItem( items.at(0) );
      m_airfieldTreeWidget->scrollToItem( items.at (0),
                                          QAbstractItemView::PositionAtTop);
    }
}

void AirfieldSelectionList::slotItemSelectionChanged()
{
  QList<QTreeWidgetItem *> selList = m_airfieldTreeWidget->selectedItems();

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

void AirfieldSelectionList::slotAccept()
{
  QList<QTreeWidgetItem *> selList = m_airfieldTreeWidget->selectedItems();

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

void AirfieldSelectionList::slotReject()
{
  QWidget::close();
}
