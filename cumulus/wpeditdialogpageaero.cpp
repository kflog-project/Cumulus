/***********************************************************************
**
**   wpeditdialogpageaero.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers,
**                   2008-2022 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>

#include <QtWidgets>

#include "altitude.h"
#include "airfield.h"
#include "doubleNumberEditor.h"
#include "layout.h"
#include "MainWindow.h"
#include "numberEditor.h"
#include "wpeditdialogpageaero.h"

WpEditDialogPageAero::WpEditDialogPageAero(QWidget *parent) :
  QWidget(parent, Qt::WindowStaysOnTopHint),
  edtFequencyListIndex(-1)
{
  setObjectName("WpEditDialogPageAero");

  signalMapperEnable = new QSignalMapper( this );
  signalMapperHeading = new QSignalMapper( this );

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  topLayout->setMargin(10 * Layout::getIntScaledDensity());

  QGridLayout *qgl = new QGridLayout;
  qgl->setMargin(0);
  topLayout->addLayout( qgl );

  Qt::InputMethodHints imh;

  edtFrequency = new DoubleNumberEditor( this );
  edtFrequency->setDecimalVisible( true );
  edtFrequency->setPmVisible( false );
  edtFrequency->setMaxLength(7);
  edtFrequency->setDecimals( 3 );
  edtFrequency->setAlignment( Qt::AlignLeft );
  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,3}\\.[0-9]{1,3})" ), this );
  edtFrequency->setValidator( eValidator );
  edtFrequency->setText("0.0");

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setSpacing(10 * Layout::getIntScaledDensity());
  hbox->addWidget( new QLabel(tr("Channel:")) );
  hbox->addWidget( edtFrequency );
  hbox->addWidget( new QLabel(tr("Ch-Type:")) );

  edtFrequencyType = new QLineEdit;
  imh = Qt::ImhUppercaseOnly | Qt::ImhDigitsOnly | Qt::ImhNoPredictiveText;
  edtFrequencyType->setInputMethodHints(imh);
  edtFrequencyType->setMaxLength(20); // limit to 20 characters

  connect( edtFrequencyType, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  hbox->addWidget( edtFrequencyType );
  qgl->addLayout(hbox, 0, 0);

  edtICAO = new QLineEdit;
  imh = Qt::ImhUppercaseOnly | Qt::ImhDigitsOnly | Qt::ImhNoPredictiveText;
  edtICAO->setInputMethodHints(imh);
  edtICAO->setMaxLength(4); // limit name to 4 characters

  connect( edtICAO, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  QFormLayout* qfl = new QFormLayout;
  qfl->addRow(tr("ICAO:"), edtICAO);
  qgl->addLayout(qfl, 0, 2);
  qgl->setRowMinimumHeight(1, 20);

  //----------------------------------------------------------------------------
  QHBoxLayout* hboxEnabRwy = new QHBoxLayout;

  for( int i = 0; i < 4; i++ )
    {
      chkRwyEnable[i] = new QCheckBox( tr("Runway %1").arg(i + 1) );
      chkRwyEnable[i]->setToolTip( tr("Check box to enable a new runway.") );
      hboxEnabRwy->addWidget( chkRwyEnable[i] );

      // prepare the signal mapper
      connect (chkRwyEnable[i], SIGNAL(clicked()), signalMapperEnable, SLOT(map()));
      signalMapperEnable->setMapping( chkRwyEnable[i], i );
    }

  connect( signalMapperEnable, SIGNAL(mapped(int)),
           this, SIGNAL(stateChangedRwyEnable(int)));

  connect( this, SIGNAL(stateChangedRwyEnable(int)),
           this, SLOT( slot_stateChangedRwyEnable( int )));

  topLayout->addLayout( hboxEnabRwy );

  //----------------------------------------------------------------------------
  for( int i = 0; i < 4; i++ )
    {
      gboxRunway[i] = new QGroupBox( tr("Runway %1").arg( i + 1 ), this );
      gboxRunway[i]->setVisible(false);
      topLayout->addWidget( gboxRunway[i] );
      topLayout->addStretch( 10 );

      QHBoxLayout* grpLayout = new QHBoxLayout;
      gboxRunway[i]->setLayout( grpLayout );

      hbox = new QHBoxLayout;
      chkRwyBoth[i] = new QCheckBox( tr("bidirectional") );
      hbox->addWidget( chkRwyBoth[i] );

      qgl = new QGridLayout;
      qgl->setMargin(0);
      grpLayout->addLayout(qgl);

      edtRwyHeading[i] = new QLineEdit( this );
      edtRwyHeading[i]->setMaxLength(3);
      edtRwyHeading[i]->setToolTip( tr("Runway designator 01...36 [L, C, R]") );
      imh = Qt::ImhUppercaseOnly | Qt::ImhNoPredictiveText;
      edtRwyHeading[i]->setInputMethodHints(imh);

      // Set minimum width
      QFontMetrics fm( font() );
      int strWidth = fm.horizontalAdvance( QString( "MMM" ) );
      edtRwyHeading[i]->setMinimumWidth( strWidth );
      edtRwyHeading[i]->setMaximumWidth( strWidth );

      // prepare the signal mapper
      connect (edtRwyHeading[i], SIGNAL(editingFinished()), signalMapperHeading, SLOT(map()));
      signalMapperHeading->setMapping( edtRwyHeading[i], i );

      qfl = new QFormLayout;
      qfl->addRow(tr("ID:"), edtRwyHeading[i]);
      qgl->addLayout( qfl, 0, 0 );

      edtRwyLength[i] = createRunwayLengthEditor();
      qfl = new QFormLayout;
      qfl->addRow(tr("Length:"), edtRwyLength[i]);
      qgl->addLayout( qfl, 0, 1 );

      edtRwyWidth[i] = createRunwayLengthEditor();
      edtRwyWidth[i]->setMaxLength(4);
      edtRwyWidth[i]->setMinWidth( 4 );
      qfl = new QFormLayout;
      qfl->addRow(tr("Width:"), edtRwyWidth[i]);
      qgl->addLayout( qfl, 0, 2 );

      cmbRwySurface[i] = new QComboBox;
      cmbRwySurface[i]->setEditable(false);
      cmbRwySurface[i]->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      cmbRwySurface[i]->view()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

      QScroller::grabGesture( cmbRwySurface[i]->view()->viewport(), QScroller::LeftMouseButtonGesture );

      qfl = new QFormLayout;
      qfl->addRow(tr("Surface:"), cmbRwySurface[i]);
      qgl->addLayout( qfl, 0, 3 );

      QHBoxLayout* hb = new QHBoxLayout();
      hb->setSpacing( 5 );

      chkRwyBoth[i] = new QCheckBox( tr("both") );
      chkRwyBoth[i]->setChecked( true );
      hb->addWidget( chkRwyBoth[i] );

      chkRwyUsable[i] = new QCheckBox( tr("usable") );
      chkRwyUsable[i]->setChecked( true );
      hb->addWidget( chkRwyUsable[i] );

      chkRwyMain[i] = new QCheckBox( tr("main RWY") );
      chkRwyMain[i]->setChecked( true );
      hb->addWidget( chkRwyMain[i] );

      chkRwyTakeoffOnly[i] = new QCheckBox( tr("takeoff only") );
      chkRwyTakeoffOnly[i]->setChecked( false );
      hb->addWidget( chkRwyTakeoffOnly[i] );

      chkRwyLandingOnly[i] = new QCheckBox( tr("landing only") );
      chkRwyLandingOnly[i]->setChecked( false );
      hb->addWidget( chkRwyLandingOnly[i] );
      qgl->addLayout( hb, 1, 0, 1, 4 );

      QButtonGroup *bg = new QButtonGroup( this );
      bg->setExclusive( true );
      bg->addButton( chkRwyTakeoffOnly[i] );
      bg->addButton( chkRwyLandingOnly[i] );
    }

  //----------------------------------------------------------------------------
  // initialize surface combo boxes
  QStringList &tlist = Runway::getSortedTranslationList();

  for( int i=0; i < 4; i++ )
    {
      for( int j=0; j < tlist.size(); j++ )
        {
          cmbRwySurface[i]->addItem( tlist.at(j) );
        }

      cmbRwySurface[i]->setCurrentIndex(cmbRwySurface[i]->count()-1);
    }

  // Connect the signal mapper to our own signal
  connect( signalMapperHeading, SIGNAL(mapped(int)),
           this, SIGNAL(rwyHeadingEditingFinished(int)));

  // Connect out own signal mapper signal to our slot.
  connect( this, SIGNAL(rwyHeadingEditingFinished(int)),
           this, SLOT(slot_rwyHeadingEdited(int)));
}

WpEditDialogPageAero::~WpEditDialogPageAero()
{}

NumberEditor* WpEditDialogPageAero::createRunwayHeadingEditor( QWidget* parent )
{
  NumberEditor* ne = new NumberEditor( parent );
  ne->setDecimalVisible( false );
  ne->setPmVisible( false );
  ne->setMaxLength(2);
  ne->setAlignment( Qt::AlignLeft );
  ne->setSpecialValueText( "--" );
  ne->setValue( 0 );
  ne->setRange( 0, 36 );
  ne->setTitle( tr("Set Runway heading") );
  ne->setTip( tr("00=unknown") + ", 01...36");
  return ne;
}

NumberEditor* WpEditDialogPageAero::createRunwayLengthEditor( QWidget* parent )
{
  NumberEditor* ne = new NumberEditor( parent );
  ne->setDecimalVisible( false );
  ne->setPmVisible( false );
  ne->setMaxLength(6);
  ne->setAlignment( Qt::AlignLeft );

  // Note! We take as runway length/width unit the altitude unit (m/ft)
  ne->setSuffix( " " + Altitude::getUnitText() );
  ne->setSpecialValueText( "-" );
  ne->setRange( 0, 999999 );
  ne->setValue( 0 );
  ne->setTitle( tr("Set Runway length/width") );
  ne->setTip( tr("0=unknown") + ", 1-9...");
  return ne;
}

void WpEditDialogPageAero::slot_rwyHeadingEdited( int idx )
{
  edtRwyHeading[idx]->setText( edtRwyHeading[idx]->text().toUpper() );

  if( checkRunwayDesignator( edtRwyHeading[idx]->text() ) == false )
      {
        reportRwyIdError( idx + 1 );
      }
}

void WpEditDialogPageAero::slot_load( Waypoint *wp )
{
  if( ! wp )
    {
      return;
    }

  edtICAO->setText(wp->icao.toUpper());

  float frequency = 0.0;
  edtFequencyListIndex = -1;

  // Only one frequency is supported. It should be the primary frequency.
  if( wp->frequencyList.size() > 0 )
    {
      // save frequency list of waypoint
      fqList = wp->getFrequencyList();
      int i = 0;

      // Only a main frequency should be shown in the display
      for( i = 0; i < wp->frequencyList.size(); i++ )
        {
          quint8 type = wp->frequencyList[i].getType();
          bool primary = wp->frequencyList[i].isPrimary();

          if( type == Frequency::Tower || type == Frequency::Info ||
              type == Frequency::Information || primary == true )
            {
              edtFequencyListIndex = i;
              break;
            }
        }

      if( edtFequencyListIndex == -1 )
        {
          // No assignment has been done, take first index.
          edtFequencyListIndex = 0;
        }

      const Frequency& fobj = wp->frequencyList.at( edtFequencyListIndex );
      frequency = fobj.getValue();
      quint8 type = fobj.getType();

      edtFrequencyType->setText( Frequency::typeAsString(type) );

      if( type == Frequency::Unknown )
        {
          // In case of unknown, look if user has defined a type.
          if( fobj.getUserType().size() > 0 )
          {
            edtFrequencyType->setText( fobj.getUserType() );
          }
        }
    }

  QString tmp = QString("%1").arg(frequency, 0, 'f', 3);

  while( tmp.size() < 7 )
    {
      // Add leading zeros, that is not supported by floating point
      // formatting.
      tmp.insert(0, "0");
    }

  edtFrequency->setText(tmp);

  // switch off both, that is the default.
  for( int i=0; i < 4; i++ )
    {
      chkRwyEnable[i]->setCheckState( Qt::Unchecked );
    }

  // chkRwy1Enable->setCheckState( Qt::Unchecked );
  for( int i=0; i < 4; i++ )
    {
      chkRwyBoth[i]->setCheckState( Qt::Unchecked );
      chkRwyUsable[i]->setCheckState( Qt::Unchecked );
      chkRwyMain[i]->setCheckState( Qt::Unchecked );
      chkRwyTakeoffOnly[i]->setCheckState( Qt::Unchecked );
      chkRwyLandingOnly[i]->setCheckState( Qt::Unchecked );
    }

  if( wp->rwyList.size() > 0 )
    {
      for( int i = 0; i < wp->rwyList.size() && i < 4; i++ )
        {
          Runway rwy = wp->rwyList.at(i);

          chkRwyEnable[i]->setCheckState( Qt::Checked );
          gboxRunway[i]->setVisible(true);
          edtRwyHeading[i]->setText( rwy.getName() );
          edtRwyLength[i]->setText( Altitude::getText((rwy.getLength()), false, 0) );
          edtRwyWidth[i]->setText( Altitude::getText((rwy.getWidth()), false, 0) );
          setSurface( cmbRwySurface[i], rwy.getSurface() );
          chkRwyUsable[i]->setChecked( rwy.isOpen() );

          if( rwy.isBidirectional() ) {
              chkRwyBoth[i]->setChecked( true );
            }

          if( rwy.isOpen() ) {
              chkRwyUsable[i]->setChecked( true );
            }

          if( rwy.isMainRunway() ) {
              chkRwyMain[i]->setChecked( true );
            }

          if( rwy.isTakeOffOnly() ) {
              chkRwyTakeoffOnly[i]->setChecked( true );
            }

          if( rwy.isLandingOnly() ) {
              chkRwyLandingOnly[i]->setChecked( true );
            }
        }
    }
}

void WpEditDialogPageAero::slot_save( Waypoint *wp )
{
  if( ! wp )
    {
      return;
    }

  wp->icao = edtICAO->text().trimmed().toUpper();

  bool ok;
  float frequency = edtFrequency->text().toFloat(&ok);

  if( ! ok )
    {
      frequency = 0.0;
    }

  QString userType = edtFrequencyType->text().trimmed();
  quint8 intType = Frequency::typeAsInt( userType );
  Frequency f;
  f.setValue( frequency );
  f.setUnit( Frequency::MHz );
  f.setType( intType );

  if( intType == Frequency::Unknown )
    {
      // User has defined an own type
      f.setUserType( userType );
    }
  else
    {
      f.setUserType( "" );
      userType.clear();
    }

  if( fqList.size() == 0 )
    {
      // Add the new frequency to the list
      fqList.append( f );
    }
  else
    {
      // Modify the frequency in the list
      fqList[edtFequencyListIndex].setValue( frequency );
      fqList[edtFequencyListIndex].setUnit( Frequency::MHz );
      fqList[edtFequencyListIndex].setType( intType );
      fqList[edtFequencyListIndex].setUserType( userType );
    }

  // Sets the edited frequency list of the waypoint.
  wp->setFequencyList( fqList );
  wp->rwyList.clear();

  for( int i = 0; i < 4; i++ )
    {
      // If the runway heading is not defined, all data are ignored!
      if( chkRwyEnable[i]->isChecked() &&
          checkRunwayDesignator( edtRwyHeading[i]->text() ) == true )
        {
          Runway rwy;
          rwy.setName( edtRwyHeading[i]->text() );
          rwy.setLength( static_cast<float> (Altitude::convertToMeters(edtRwyLength[i]->text().toDouble())));
          rwy.setWidth( static_cast<float> (Altitude::convertToMeters(edtRwyWidth[i]->text().toDouble())));
          rwy.setSurface( getSurface( cmbRwySurface[i] ) );

          chkRwyBoth[i]->isChecked() ? rwy.setTurnDirection( Runway::Both ) : rwy.setTurnDirection( Runway::OneWay );
          chkRwyUsable[i]->isChecked() ? rwy.setOperations( Runway::Active ): rwy.setOperations( Runway::Closed );
          chkRwyMain[i]->isChecked() ? rwy.setMainRunway( true ) : rwy.setMainRunway( false );
          chkRwyTakeoffOnly[i]->isChecked() ? rwy.setTakeOffOnly( true ) : rwy.setTakeOffOnly( false );
          chkRwyLandingOnly[i]->isChecked() ? rwy.setLandingOnly( true ) : rwy.setLandingOnly( false );

          int hdg = edtRwyHeading[i]->text().left(2).toInt() * 10;
          rwy.setHeading( hdg );
          wp->rwyList.append( rwy );
        }
    }
}

void WpEditDialogPageAero::reportRwyIdError( short rwyNo )
{
  QMessageBox mb( QMessageBox::Critical,
                  tr( "RWY %1 ID error" ).arg(rwyNo),
                  tr( "RWY %1: Excepting 01...36 [C, L, R]" ).arg( rwyNo ),
                  QMessageBox::Ok,
                  this );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  mb.exec();
}


/**
 * Checks all runway designators and only flags for correctness.
 *
 * @return true in case of ok otherwise false.
 */
bool WpEditDialogPageAero::checkRunways()
{
  for( int i = 0; i < 4; i++ )
    {
      if( chkRwyEnable[i]->isChecked() )
        {
          if( checkRunwayDesignator( edtRwyHeading[i]->text() ) == false )
            {
              reportRwyIdError( i + 1 );
              return false;
            }
        }
    }

  return true;
}

/**
 * Check the runway designator for 01...36 [ L, C, R ].
 *
 * Return true in case of success otherwise false.
 */
bool WpEditDialogPageAero::checkRunwayDesignator( QString id )
{
  if( id.size() < 2 ) {
      //qDebug() << "rwy < 2";
      return false;
  }

  if( id[0].isDigit() == false || id[1].isDigit() == false ) {
      //qDebug() << "rwy0-1 nix digit";
      return false;
  }

  if( QString(id[0]).toInt() > 3 ) {
      //qDebug() << "rwy > 3";
      return false;
  }

  if( id[0] == '3' && QString( id[1] ).toInt() > 6 ) {
      //qDebug() << "rwy==3 und rwy1 > 6";
      return false;
  }

  if( id.size() == 3 && id[2] != 'C' && id[2] != 'L' && id[2] != 'R' ) {
      return false;
  }

  return true;
}

int WpEditDialogPageAero::getSurface(QComboBox* cbox)
{
  if( ! cbox )
    {
      return -1;
    }

  int s = cbox->currentIndex();

  if (s != -1)
    {
      const QString &text = Runway::getSortedTranslationList().at(s);
      s = Runway::text2Item( text );
    }

  if (s == 0)
    {
      s = -1;
    }

  return s;
}

void WpEditDialogPageAero::setSurface(QComboBox* cbox, int s)
{
  if( ! cbox )
    {
      return;
    }

  if (s != -1)
    {
      s = Runway::getSortedTranslationList().indexOf(Runway::item2Text(s));
    }
  else
    {
      s = Runway::getSortedTranslationList().indexOf(Runway::item2Text(0));
    }

  cbox->setCurrentIndex(s);
}
