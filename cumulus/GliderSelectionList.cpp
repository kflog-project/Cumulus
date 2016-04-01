/***********************************************************************
**
**   GliderSelectionList.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2016 by Axel Pauli
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

#include "GliderSelectionList.h"
#include "layout.h"
#include "mainwindow.h"
#include "rowdelegate.h"

GliderSelectionList::GliderSelectionList( QWidget *parent ) :
 QWidget(parent)
{
  setObjectName( "GliderSelectionList" );
  setWindowTitle(tr("Glider Selection"));
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout* mainLayout = new QHBoxLayout( this );

  m_ListTreeWidget = new QTreeWidget( this );

  mainLayout->addWidget( m_ListTreeWidget );

  m_ListTreeWidget->setRootIsDecorated( false );
  m_ListTreeWidget->setItemsExpandable( false );
  m_ListTreeWidget->setSortingEnabled( false );
  m_ListTreeWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  m_ListTreeWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_ListTreeWidget->setAlternatingRowColors(true);
  m_ListTreeWidget->setColumnCount( 1 );
  m_ListTreeWidget->setFocusPolicy( Qt::StrongFocus );
  m_ListTreeWidget->setUniformRowHeights(true);
  m_ListTreeWidget->setHeaderLabel( tr( "Gliders" ) );

  connect( m_ListTreeWidget, SIGNAL(itemSelectionChanged()),
           SLOT(slotItemSelectionChanged()) );

  // Set additional space per row
  RowDelegate* rowDelegate = new RowDelegate( m_ListTreeWidget, 10 );
  m_ListTreeWidget->setItemDelegate( rowDelegate );

  QTreeWidgetItem* headerItem = m_ListTreeWidget->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );

  m_ListTreeWidget->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_ListTreeWidget->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef ANDROID
  QScrollBar* lvsb = m_ListTreeWidget->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture(m_ListTreeWidget->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_ListTreeWidget->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  QVBoxLayout* groupLayout = new QVBoxLayout;

  m_groupBox = new QGroupBox( tr("Search Glider"));
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

GliderSelectionList::~GliderSelectionList()
{
}

void GliderSelectionList::showEvent( QShowEvent *event )
{
  QWidget::showEvent( event );
}

void GliderSelectionList::fillSelectionList( QList<Polar>& polarList )
{
  m_searchInput->clear();
  m_ListTreeWidget->clear();

  for( int i = 0; i < polarList.size(); i++ )
    {
      PolarItem* item = new PolarItem( &polarList[i] );
      m_ListTreeWidget->addTopLevelItem( item );
    }
}

/**
 * Called to select a certain glider entry in the list.
 */
void GliderSelectionList::selectGlider( QString gliderName )
{
  for( int i = 0; m_ListTreeWidget->topLevelItemCount(); i++ )
    {
      QTreeWidgetItem* item = m_ListTreeWidget->topLevelItem(i);

      if( item != 0 && item->text(0) == gliderName )
	{
	  //m_ListTreeWidget->setCurrentItem( item );
	  m_searchInput->setText( gliderName );
	  break;
	}
    }
}

GliderSelectionList::PolarItem::PolarItem( Polar* polar ) :
  QTreeWidgetItem(),
  m_polar(polar)
{
  setText( 0, polar->name() );
}

void GliderSelectionList::slotClearSearchEntry()
{
  m_searchInput->clear();

  if( m_ListTreeWidget->topLevelItemCount() > 0 )
    {
      m_ListTreeWidget->setCurrentItem( m_ListTreeWidget->topLevelItem( 0 ) );
      m_ListTreeWidget->clearSelection();
    }
}

void GliderSelectionList::slotTextEdited( const QString& text )
{
  if( text.size() == 0 )
    {
      m_ListTreeWidget->setCurrentItem( m_ListTreeWidget->topLevelItem( 0 ) );
      m_ListTreeWidget->clearSelection();
      m_ListTreeWidget->scrollToItem( m_ListTreeWidget->topLevelItem( 0 ),
				      QAbstractItemView::PositionAtTop);
      return;
    }

  QList<QTreeWidgetItem *> items = m_ListTreeWidget->findItems( text, Qt::MatchStartsWith );

  if( items.size () > 0 )
    {
      m_ListTreeWidget->setCurrentItem( items.at(0) );
      m_ListTreeWidget->scrollToItem( items.at (0),
				      QAbstractItemView::PositionAtTop);
    }
}

void GliderSelectionList::slotItemSelectionChanged()
{
  QList<QTreeWidgetItem *> selList = m_ListTreeWidget->selectedItems();

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

void GliderSelectionList::slotAccept()
{
  QList<QTreeWidgetItem *> selList = m_ListTreeWidget->selectedItems();

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

  PolarItem* pi = dynamic_cast<PolarItem *> ( li );

  if( pi == 0 )
    {
      QWidget::close();
      return;
    }

  if( pi->getPolar() != 0 )
    {
      emit takeThis( pi->getPolar() );
    }

  QWidget::close();
}

void GliderSelectionList::slotReject()
{
  QWidget::close();
}
