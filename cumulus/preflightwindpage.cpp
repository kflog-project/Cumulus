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
#include "mainwindow.h"
#include "mapcalc.h"
#include "mapconfig.h"
#include "numberEditor.h"
#include "preflightwindpage.h"
#include "rowdelegate.h"
#include "speed.h"
#include "vector.h"
#include "WindMeasurement.h"

PreFlightWindPage::PreFlightWindPage( QWidget* parent ) :
  QWidget( parent )
{
  setObjectName( "PreFlightWindPage" );
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowTitle( tr( "PreFlight - Wind" ) );

  if( MainWindow::mainWindow() )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( MainWindow::mainWindow()->size() );

#ifdef ANDROID
      // On Galaxy S3 there are size problems observed
      setMinimumSize( MainWindow::mainWindow()->size() );
      setMaximumSize( MainWindow::mainWindow()->size() );
#endif
    }

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  int msw = QFontMetrics(font()).width("9999 Km/h") + 10;
  int mdw = QFontMetrics(font()).width("999" + QString(Qt::Key_degree)) + 10;

  QVBoxLayout* windLayout = new QVBoxLayout;
  contentLayout->addLayout( windLayout, 5 );

  windLayout->setSpacing(5);
  windLayout->setMargin(5);

  // Wind row layout as one line
  QHBoxLayout* windRow = new QHBoxLayout;
  windLayout->addLayout( windRow );

  QGroupBox *windBox = new QGroupBox( tr("Manual Wind"), this );
  windRow->addWidget( windBox );

  QHBoxLayout* windRowManual = new QHBoxLayout;
  windRowManual->setSpacing(10);
  windBox->setLayout( windRowManual );

  windLayout->addSpacing( 10 );

  m_windCheckBox = new QCheckBox( tr("On/Off"), this );
  windRowManual->addWidget(m_windCheckBox);
  connect( m_windCheckBox, SIGNAL(stateChanged(int)),
           this, SLOT(slotManualWindCbStateChanged(int)));

  QLabel* label = new QLabel( tr("WD"), this );
  windRowManual->addWidget(label);

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
  windRowManual->addWidget(m_windDirection);

  label = new QLabel( tr("WS"), this );
  windRowManual->addWidget(label);

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
  windRowManual->addWidget(m_windSpeed);
  windRow->addStretch(10);

  //----------------------------------------------------------------------------
  QGroupBox* windBoxSF = new QGroupBox( tr("SF Wind"), this );
  windBoxSF->setToolTip( tr("Wind calculation in straight flight") );
  windRow->addWidget( windBoxSF );

  QHBoxLayout* hbox = new QHBoxLayout;
  windRowManual->setSpacing(10);
  windBoxSF->setLayout( hbox );

  m_enableWindInSF = new QCheckBox( tr("On/Off"), this );
  m_enableWindInSF->setToolTip( tr("Switch on/off wind calculation in straight flight") );
  m_enableWindInSF->setChecked( GeneralConfig::instance()->isSfWCEnabled() );
  hbox->addWidget( m_enableWindInSF );

  //----------------------------------------------------------------------------
  QGroupBox* windBoxExtern = new QGroupBox( tr("Ext. Wind"), this );
  windRow->addWidget( windBoxExtern );

  hbox = new QHBoxLayout;
  windRowManual->setSpacing(10);
  windBoxExtern->setLayout( hbox );

  m_useExternalWind = new QCheckBox( tr("On/Off"), this );
  hbox->addWidget( m_useExternalWind );

  connect( m_useExternalWind, SIGNAL(stateChanged(int)),
           this, SLOT(slotExternalWindCbStateChanged(int)) );

  //----------------------------------------------------------------------------
  QHBoxLayout* header = new QHBoxLayout;
  header->setSpacing( 5 );
  header->addWidget( new QLabel( tr("Wind Statistics") ) );
  header->addSpacing( 30 );
  m_averageLabel = new QLabel( this );
  header->addWidget( m_averageLabel, 5 );

  windLayout->addLayout( header );
  windLayout->addSpacing( 10 );

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
  buttonBox->setSpacing( 0 );
  buttonBox->addWidget( help, 1 );
  buttonBox->addStretch( 2 );
  buttonBox->addWidget( cancel, 1 );
  buttonBox->addSpacing( 30 );
  buttonBox->addWidget( ok, 1 );
  buttonBox->addStretch( 2 );
  buttonBox->addWidget( titlePix );
  contentLayout->addLayout( buttonBox );

  m_windCheckBox->setCheckState( GeneralConfig::instance()->isManualWindEnabled() ? Qt::Checked : Qt::Unchecked );
  slotManualWindCbStateChanged( GeneralConfig::instance()->isManualWindEnabled() ? Qt::Checked : Qt::Unchecked );
  slotExternalWindCbStateChanged( GeneralConfig::instance()->isExternalWindEnabled() ? Qt::Checked : Qt::Unchecked );
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

void PreFlightWindPage::showEvent(QShowEvent *event )
{
  m_windListStatistics->resizeColumnToContents(0);
  m_windListStatistics->resizeColumnToContents(1);
  m_windListStatistics->resizeColumnToContents(2);

  QWidget::showEvent( event );
}

void PreFlightWindPage::slotLoadWindStatistics()
{
  m_averageLabel->clear();
  m_windListStatistics->clear();

  WindStore* ws = calculator->getWindStore();

  if( ws->numberOfWindSamples() == 0 )
    {
      // No data in wind map.
      return;
    }

  int iconSize = QFontMetrics(font()).height() - 4;
  m_windListStatistics->setIconSize( QSize(iconSize, iconSize) );

  int start = ws->getFirstWindMeasurement().altitude;
  int end = ws->getLastWindMeasurement().altitude;
  int items = 0;

  double wv = 0.0;
  double wd = 0.0;

  for( int i = start; i <= end; i += 100 )
    {
      items++;

      Altitude alt( i );

      if( Altitude::getUnit() == Altitude::feet )
        {
          alt.setFeet(i);
        }

      Vector v = ws->getWind( i, true );

      if( v.isValid() )
        {
          // Add wind of altitude to the list
          QStringList sl;
          sl << QString::number(i) + " " + Altitude::getUnitText()
             << QString("%1%2").arg( v.getAngleDeg() ).arg( QString(Qt::Key_degree) )
             << v.getSpeed().getWindText( true, 0 );

          QPixmap pixmap;
          int windAngle = v.getAngleDeg();

          // Summarize wind over all samples
          if( items == 1 )
            {
              wd = v.getAngleDegDouble();
            }
          else
            {
              wd = MapCalc::bisectorOfAngles( wd, v.getAngleDegDouble() );
            }

          wv += v.getSpeed().getMps();

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

  // Calculate average speed over all samples
  wv /= items;
  Speed wva( wv );

  // prepare label text for wind average display
  QString lt = QString( "%1°/%2" ).arg( int(wd + 0.5) )
                                  .arg( wva.getWindText(true, 0) );

  m_averageLabel->setText( lt );

  m_windListStatistics->resizeColumnToContents(0);
  m_windListStatistics->resizeColumnToContents(1);
  m_windListStatistics->resizeColumnToContents(2);
}

void PreFlightWindPage::slotManualWindCbStateChanged( int state )
{
  bool enabled = ( state == Qt::Checked ) ? true : false;

  m_windDirection->setEnabled( enabled );
  m_windSpeed->setEnabled( enabled );

  if( enabled == true && m_useExternalWind->isChecked() )
    {
      m_useExternalWind->blockSignals( true );
      m_useExternalWind->setChecked( false );
      m_useExternalWind->blockSignals( false );
    }
}

void PreFlightWindPage::slotExternalWindCbStateChanged( int state )
{
  bool enabled = ( state == Qt::Checked ) ? true : false;

  if( enabled == true && m_windCheckBox->isChecked() )
    {
      m_windCheckBox->blockSignals( true );
      m_windCheckBox->setChecked( false );
      m_windDirection->setEnabled( false );
      m_windSpeed->setEnabled( false );
      m_windCheckBox->blockSignals( false );
    }
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

  bool oldExternalWindState = conf->isExternalWindEnabled();
  bool newExternalWindState = m_useExternalWind->isChecked();

  conf->setManualWindEnabled( newWindState );
  conf->setManualWindDirection( m_windDirection->value() );
  conf->setExternalWindEnabled( m_useExternalWind->isChecked() );
  conf->setSfWCEnabled( m_enableWindInSF->isChecked() );

  Speed wv;
  wv.setValueInUnit( m_windSpeed->text().toDouble(), Speed::getWindUnit() );
  conf->setManualWindSpeed( wv );

  if( newWindState == true || oldWindState != newWindState )
    {
      // Inform about a wind state or parameter change.
      emit manualWindStateChange( newWindState );
    }

  if( newExternalWindState == true || oldExternalWindState != newWindState )
    {
      // Inform about a wind state or parameter change.
      emit externalWindStateChange( newExternalWindState );
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
