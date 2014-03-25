/***********************************************************************
**
**   preflightwindpage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "layout.h"
#include "numberEditor.h"
#include "preflightwindpage.h"
#include "rowdelegate.h"
#include "speed.h"

PreFlightWindPage::PreFlightWindPage( QWidget* parent ) :
  QWidget( parent )
{
  setObjectName("PreFlightWindPage");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("PreFlight - Wind") );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  int msw = QFontMetrics(font()).width("9999 Km/h") + 10;
  int mdw = QFontMetrics(font()).width("999" + QString(Qt::Key_degree)) + 10;

  QVBoxLayout* windLayout = new QVBoxLayout;
  contentLayout->addLayout( windLayout, 5 );

  windLayout->setSpacing(5);
  windLayout->setMargin(0);

  QGroupBox *windBox = new QGroupBox( tr("Manual Wind"), this );
  windLayout->addWidget( windBox );

  QHBoxLayout* windRow = new QHBoxLayout;
  windRow->setSpacing(5);
  windBox->setLayout( windRow );
  windLayout->addSpacing( 10 );

  m_windCheckBox = new QCheckBox( tr("On/Off") );
  windRow->addWidget(m_windCheckBox);
  connect( m_windCheckBox, SIGNAL(stateChanged(int)),
           this, SLOT(slotWindCbStateChanged(int)));

  QLabel* label = new QLabel( tr("WD"), this );
  windRow->addWidget(label);

  m_windDirection = new NumberEditor( this );
#ifndef ANDROID
  m_windDirection->setToolTip( tr("Wind Direction") );
#endif
  m_windDirection->setPmVisible(false);
  m_windDirection->setRange( 0, 360 );
  m_windDirection->setTip("0...360");
  m_windDirection->setMaxLength(3);
  m_windDirection->setValue( GeneralConfig::instance()->getManualWindDirection() );
  m_windDirection->setSuffix( QString(Qt::Key_degree) );
  m_windDirection->setMinimumWidth( mdw );
  windRow->addWidget(m_windDirection);

  label = new QLabel( tr("WS"), this );
  windRow->addWidget(label);

  m_windSpeed = new NumberEditor( this );
#ifndef ANDROID
  m_windSpeed->setToolTip( tr("Wind Speed") );
#endif
  m_windSpeed->setPmVisible(false);
  m_windSpeed->setRange( 0, 999 );
  m_windSpeed->setMaxLength(4);

  const Speed& wv = GeneralConfig::instance()->getManualWindSpeed();
  m_windSpeed->setText( wv.getWindText( false, 1 ) );

  m_windSpeed->setSuffix( " " + Speed::getWindUnitText() );
  m_windSpeed->setMinimumWidth( msw );
  windRow->addWidget(m_windSpeed);
  windRow->addStretch(10);

  //----------------------------------------------------------------------------
  windLayout->addWidget( new QLabel( tr("Wind Statistics") ) );

  m_windListStatistics = new QTreeWidget;

  m_windListStatistics->setRootIsDecorated(false);
  m_windListStatistics->setItemsExpandable(false);
  m_windListStatistics->setUniformRowHeights(true);
  m_windListStatistics->setAlternatingRowColors(true);
  m_windListStatistics->setSortingEnabled(false);
  m_windListStatistics->setColumnCount(3);
  m_windListStatistics->setFocus();
  m_windListStatistics->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_windListStatistics->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture(m_windListStatistics->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_windListStatistics->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  m_windListStatistics->setItemDelegate( new RowDelegate( m_windListStatistics, afMargin ) );

  QStringList sl;
  sl << tr("Arrow")
     << tr("Direction")
     << tr("Speed");

  m_windListStatistics->setHeaderLabels(sl);

  QTreeWidgetItem* headerItem = m_windListStatistics->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );

  windLayout->addWidget( m_windListStatistics, 10 );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png"));

  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);

  m_windCheckBox->setCheckState( GeneralConfig::instance()->isManualWindEnabled() ? Qt::Checked : Qt::Unchecked );
  slotWindCbStateChanged( GeneralConfig::instance()->isManualWindEnabled() ? Qt::Checked : Qt::Unchecked );
}

PreFlightWindPage::~PreFlightWindPage()
{
  // qDebug("PreFlightWindPage::~PreFlightWindPage()");
}

void PreFlightWindPage::showEvent(QShowEvent *)
{
  m_windListStatistics->resizeColumnToContents(0);
  m_windListStatistics->resizeColumnToContents(1);
  m_windListStatistics->resizeColumnToContents(2);
}

void PreFlightWindPage::slotWindCbStateChanged( int state )
{
  bool enabled = false;

  if( state == Qt::Checked )
    {
      enabled = true;
    }
  else
    {
      enabled = false;
    }

  m_windDirection->setEnabled( enabled );
  m_windSpeed->setEnabled( enabled );
}

void PreFlightWindPage::slotAccept()
{
  GeneralConfig *conf = GeneralConfig::instance();

  bool oldWindState = conf->isManualWindEnabled();
  bool newWindState = m_windCheckBox->isChecked();

  conf->setManualWindEnabled( newWindState );
  conf->setManualWindDirection( m_windDirection->value() );

  Speed wv;
  wv.setValueInUnit( m_windSpeed->text().toDouble(), Speed::getWindUnit() );
  conf->setManualWindSpeed( wv );

  if( newWindState == true || oldWindState != newWindState )
    {
      // Inform about a wind state or parameter change.
      emit manualWindStateChange( newWindCbState );
    }

  emit closingWidget();
  QWidget::close();
}

void PreFlightWindPage::slotReject()
{
  emit closingWidget();
  QWidget::close();
}
