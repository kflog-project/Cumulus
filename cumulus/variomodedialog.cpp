/***********************************************************************
 **
 **   variomodedialog.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2004-2010 by Axel Pauli (axel@kflog.org)
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "vario.h"
#include "variomodedialog.h"
#include "calculator.h"
#include "mapconfig.h"
#include "generalconfig.h"

extern MapConfig *_globalMapConfig;

// set static member variable
int VarioModeDialog::noOfInstances = 0;

VarioModeDialog::VarioModeDialog(QWidget *parent) :
  QDialog( parent, Qt::WindowStaysOnTopHint )
{
  noOfInstances++;
  setObjectName("VarioModeDialog");
  setAttribute(Qt::WA_DeleteOnClose);
  setModal(true);
  setWindowTitle( tr("Set Variometer") );

#ifndef MAEMO
  int minFontSize = 14;
#else
  int minFontSize = 20;
#endif

  QFont b = font();
  b.setBold(true);
  setFont(b);

  // set font size to a reasonable and usable value
  if( font().pointSize() < minFontSize )
    {
      QFont cf = font();
      cf.setPointSize( minFontSize );
      this->setFont(cf);
    }

  QGridLayout* gridLayout = new QGridLayout(this);
  gridLayout->setMargin(5);
  gridLayout->setSpacing(15);
  int row = 0;

  //---------------------------------------------------------------------

  QLabel *label = new QLabel(tr("Time:"), this);
  gridLayout->addWidget(label, row, 0);

  spinTime = new QSpinBox(this);
  spinTime->setRange(3, 150);
  spinTime->setSuffix( "s" );
  spinTime->setButtonSymbols(QSpinBox::NoButtons);
  spinTime->setFocus();

  gridLayout->addWidget(spinTime, row++, 1);

  //---------------------------------------------------------------------

  QLabel* TekLbl = new QLabel(tr("TEK Mode:"), this);
  gridLayout->addWidget(TekLbl, row, 0);

  TEK = new QCheckBox (tr(""), this);
  TEK->setFocusPolicy(Qt::NoFocus);
  gridLayout->addWidget(TEK, row++, 1);

  //---------------------------------------------------------------------

  TekAdj = new QLabel(tr("TEK Adjust:"), this);
  gridLayout->addWidget(TekAdj, row, 0);

  spinTEK = new QSpinBox( this );
  spinTEK->setRange( -100, 100 );
  spinTEK->setSingleStep( 1 );
  spinTEK->setSuffix( "%" );

  spinTEK->setButtonSymbols(QSpinBox::NoButtons);
  gridLayout->addWidget(spinTEK, row++, 1);

  //---------------------------------------------------------------------

  QPushButton *pplus   = new QPushButton("++", this);
  QPushButton *plus    = new QPushButton("+", this);
  QPushButton *mminus  = new QPushButton("--", this);
  QPushButton *minus   = new QPushButton("-", this);

  pplus->setMinimumSize(40, 40);
  plus->setMinimumSize(40, 40);
  minus->setMinimumSize(40, 40);
  mminus->setMinimumSize(40, 40);

  pplus->setMaximumSize(40, 40);
  plus->setMaximumSize(40, 40);
  minus->setMaximumSize(40, 40);
  mminus->setMaximumSize(40, 40);

  pplus->setFocusPolicy(Qt::NoFocus);
  plus->setFocusPolicy(Qt::NoFocus);
  minus->setFocusPolicy(Qt::NoFocus);
  mminus->setFocusPolicy(Qt::NoFocus);

  QHBoxLayout *pmLayout = new QHBoxLayout;
  pmLayout->setSpacing(5);
  pmLayout->addWidget(pplus, Qt::AlignLeft);
  pmLayout->addWidget(plus, Qt::AlignLeft);
  pmLayout->addStretch(100);
  pmLayout->addWidget(minus, Qt::AlignRight);
  pmLayout->addWidget(mminus, Qt::AlignRight);

  gridLayout->addLayout(pmLayout, row, 0, 1, 3);

  //---------------------------------------------------------------------

  // Align ok and cancel button at the upper and lower position of the right
  // side of the widget to have enough space between them. That shall avoid wrong
  // button pressing in turbulent air.
  cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(30, 30));
  cancel->setMinimumSize(40, 40);
  cancel->setMaximumSize(40, 40);

  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(30, 30));
  ok->setMinimumSize(40, 40);
  ok->setMaximumSize(40, 40);
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QVBoxLayout *butLayout = new QVBoxLayout;
  butLayout->addWidget( cancel );
  butLayout->addStretch();
  butLayout->addWidget( ok );

  gridLayout->addLayout(butLayout, 0, 3, row, 1);
  gridLayout->setColumnStretch( 2, 10 );

  timer = new QTimer(this);
  timer->setSingleShot(true);

  connect(timer, SIGNAL(timeout()), this, SLOT(reject()));
  connect(TEK,   SIGNAL(toggled(bool)), this, SLOT(slot_tekChanged(bool)));

  connect (ok,     SIGNAL(clicked()), this, SLOT(slot_accept()));
  connect (cancel, SIGNAL(clicked()), this, SLOT(reject()));

  QSignalMapper* signalMapper = new QSignalMapper(this);
  connect(pplus, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(pplus, 0);
  connect(plus, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(plus, 1);
  connect(minus, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(minus, 2);
  connect(mminus, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(mminus, 3);
  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slot_change(int)));

  // restart timer, if value was changed
  connect(spinTime, SIGNAL(valueChanged(int)), this, SLOT(slot_setTimer()));
  connect(spinTEK,  SIGNAL(valueChanged(int)), this, SLOT(slot_setTimer()));

  load();
}

VarioModeDialog::~VarioModeDialog()
{
  noOfInstances--;
}

void VarioModeDialog::showEvent(QShowEvent *)
{
}

void VarioModeDialog::slot_tekChanged( bool newState )
{
  TekAdj->setEnabled(newState);
  spinTEK->setEnabled(newState);

  if( newState == true )
    {
      // set focus to TEK if it is enabled
      spinTEK->setFocus();
    }
  else
    {
      spinTime->setFocus();
    }
}

void VarioModeDialog::load()
{
  // qDebug ("VarioModeDialog::load()");
  GeneralConfig *conf = GeneralConfig::instance();

  _intTime = conf->getVarioIntegrationTime();

  if( _intTime < 3 ) // check configuration value
    {
      _intTime = INT_TIME; // reset to default
      conf->setVarioIntegrationTime(_intTime);
    }

  _TEKComp = conf->getVarioTekCompensation();
  emit newTEKMode( _TEKComp );

  _TEKAdjust = conf->getVarioTekAdjust();
  emit newTEKAdjust( _TEKAdjust );

  // let us take the user's defined info display time
  _timeout = conf->getInfoDisplayTime();

  spinTEK->setEnabled(_TEKComp);
  spinTime->setValue( _intTime );
  spinTEK->setValue( _TEKAdjust );
  TEK->setChecked( _TEKComp );

  slot_tekChanged( _TEKComp );

  spinTime->setFocus();
  slot_setTimer();
}

void VarioModeDialog::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setVarioIntegrationTime( spinTime->value() );

  if( TEK->isChecked() != _TEKComp )
    {
      _TEKComp = TEK->isChecked();
      conf->setVarioTekCompensation( _TEKComp );
      emit newTEKMode( _TEKComp );
    }

  if( spinTEK->value() != _TEKAdjust )
    {
      _TEKAdjust = spinTEK->value();
      conf->setVarioTekAdjust( _TEKAdjust );
      emit newTEKAdjust( _TEKAdjust );
    }

  emit newVarioTime( spinTime->value() );
  conf->save();
}

/**
 * This method changes the value in the spin box which has the current focus.
 *
 * @param newStep value to be set in spin box
 */
void VarioModeDialog::slot_change( int newStep )
{
  // qDebug("slot_change(%d)", newStep);
  int step  = 1;
  int value = 1;

  switch(newStep)
    {
    case 0: // ++ was pressed
      step = 5;
      value = 5;
      break;
    case 1: // + was pressed
      step = 1;
      value = 1;
      break;
    case 2: // - was pressed
      step = 1;
      value = -1;
      break;
    case 3: // -- was pressed
      step = 5;
      value = -5;
      break;
    default:
      // Should normally not happen
      return;
    }

  // Look which spin box has the focus
  if( QApplication::focusWidget() == spinTime )
    {
      //qDebug() << "spinTime has focus";
      spinTime->setSingleStep(step);
      spinTime->setValue( spinTime->value() + value );
    }

  else if( QApplication::focusWidget() == spinTEK )
    {
      //qDebug() << "spinTEK has focus";
      spinTEK->setSingleStep(step);
      spinTEK->setValue( spinTEK->value() + value );
    }
  else
    {
      return;
    }

  slot_setTimer();
}

void VarioModeDialog::slot_timePlus()
{
  spinTime->setValue( spinTime->value() + spinTime->singleStep() );
}

void VarioModeDialog::slot_timeMinus()
{
  spinTime->setValue( spinTime->value() - spinTime->singleStep() );
}

void VarioModeDialog::slot_tekPlus()
{
  spinTEK->setValue( spinTEK->value() + spinTEK->singleStep() );
}

void VarioModeDialog::slot_tekMinus()
{
  spinTEK->setValue( spinTEK->value() - spinTEK->singleStep() );
}

void VarioModeDialog::slot_accept()
{
  save();
  QDialog::accept();
}

void VarioModeDialog::slot_setTimer()
{
  if (_timeout > 0)
    {
      timer->start( _timeout * 1000 );
    }
}
