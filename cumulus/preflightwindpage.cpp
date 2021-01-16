/***********************************************************************
**
**   preflightwindpage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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

#include "altitude.h"
#include "calculator.h"
#include "generalconfig.h"
#include "helpbrowser.h"
#include "layout.h"
#include "mapconfig.h"
#include "numberEditor.h"
#include "preflightwindpage.h"
#include "rowdelegate.h"
#include "speed.h"
#include "vector.h"
#include "windmeasurementlist.h"

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
  m_windDirection->setDecimalVisible(false);
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
  m_windSpeed->setDecimalVisible(false);
  m_windSpeed->setRange( 0, 999 );
  m_windSpeed->setMaxLength(3);

  const Speed& wv = GeneralConfig::instance()->getManualWindSpeed();
  m_windSpeed->setText( wv.getWindText( false, 0 ) );

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
  m_windListStatistics->setColumnCount(4);
  m_windListStatistics->setFocus();
  m_windListStatistics->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_windListStatistics->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );
  //m_windListStatistics->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

#ifdef ANDROID
  QScrollBar* lvsb = m_windListStatistics->verticalScrollBar();
  lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
  QScroller::grabGesture(m_windListStatistics->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_windListStatistics->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  m_windListStatistics->setItemDelegate( new RowDelegate( m_windListStatistics, afMargin ) );

  QString h0 = tr("Altitude");
  QString h1 = tr("Direction");
  QString h2 = tr("Speed");
  QStringList sl;

  sl << h0 << h1 << h2 << "";
  m_windListStatistics->setHeaderLabels(sl);

  QTreeWidgetItem* headerItem = m_windListStatistics->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );

  windLayout->addWidget( m_windListStatistics, 5 );

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap( _globalMapConfig->createGlider(315, 1.6) );

  connect(help, SIGNAL(pressed()), this, SLOT(slotHelp()));
  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(help, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);

  m_windCheckBox->setCheckState( GeneralConfig::instance()->isManualWindEnabled() ? Qt::Checked : Qt::Unchecked );
  slotWindCbStateChanged( GeneralConfig::instance()->isManualWindEnabled() ? Qt::Checked : Qt::Unchecked );

  slotLoadWindStatistics();

  // Activate reload timer for wind statistics.
  m_reloadTimer = new QTimer( this );
  m_reloadTimer->setInterval( 30000 );
  connect(m_reloadTimer, SIGNAL(timeout()), SLOT(slotLoadWindStatistics()));
  m_reloadTimer->start();
}

PreFlightWindPage::~PreFlightWindPage()
{
  m_reloadTimer->stop();
}

void PreFlightWindPage::showEvent(QShowEvent *)
{
  m_windListStatistics->resizeColumnToContents(0);
  m_windListStatistics->resizeColumnToContents(1);
  m_windListStatistics->resizeColumnToContents(2);
}

void PreFlightWindPage::slotLoadWindStatistics()
{
  m_windListStatistics->clear();

  WindMeasurementList& wml = calculator->getWindStore()->getWindMeasurementList();

  int iconSize = QFontMetrics(font()).height() - 4;
  m_windListStatistics->setIconSize( QSize(iconSize, iconSize) );

  int start, end;

  if( Altitude::getUnit() == Altitude::meters )
    {
      start = 250;
      end   = 6000;
    }
  else if( Altitude::getUnit() == Altitude::feet )
    {
      start = 1000;
      end   = 20000;
    }
  else
    {
      return;
    }

  for( int i = start; i <= end; i += start )
    {
      Altitude alt;

      if( Altitude::getUnit() == Altitude::meters )
        {
          alt.setMeters(i);
        }
      else if( Altitude::getUnit() == Altitude::feet )
        {
          alt.setFeet(i);
        }

      Vector v = wml.getWind( alt, 3600, 250 );

      if( v.isValid() )
        {
          // Add wind of altitude to the list
          QStringList sl;
          sl << QString::number(i) + " " + Altitude::getUnitText()
             << QString("%1%2").arg( v.getAngleDeg() ).arg( QString(Qt::Key_degree) )
             << v.getSpeed().getWindText( true, 0 );

          QPixmap pixmap;

          int windAngle = rint(v.getAngleDeg());

          // Wind angle must be turned by 180 degrees to get the right triangle
          // direction.
          windAngle >= 180 ? windAngle -= 180 : windAngle += 180;

          MapConfig::createTriangle( pixmap,
                                     iconSize,
                                     QColor(Qt::black),
                                     windAngle,
                                     1.0,
                                     QColor(Qt::cyan) );

          QTreeWidgetItem* item = new QTreeWidgetItem( sl );
          item->setTextAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
          item->setTextAlignment( 1, Qt::AlignRight|Qt::AlignVCenter );
          item->setTextAlignment( 2, Qt::AlignRight|Qt::AlignVCenter );

          QIcon qi;
          qi.addPixmap( pixmap );
          item->setIcon( 1, qi );

          m_windListStatistics->addTopLevelItem( item );
        }
    }

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

void PreFlightWindPage::slotHelp()
{
  QString file = "cumulus-preflight-settings-wind.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
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
      emit manualWindStateChange( newWindState );
    }

  QWidget::close();
}

void PreFlightWindPage::slotReject()
{
  QWidget::close();
}

void PreFlightWindPage::closeEvent( QCloseEvent *event )
{
  emit closingWidget();
  event->accept();
}
