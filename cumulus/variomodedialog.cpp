/***********************************************************************
 **
 **   variomodedialog.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2004-2013 by Axel Pauli (axel@kflog.org)
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

#include "vario.h"
#include "variomodedialog.h"
#include "calculator.h"
#include "mapconfig.h"
#include "generalconfig.h"
#include "layout.h"

// set static member variable
int VarioModeDialog::noOfInstances = 0;

VarioModeDialog::VarioModeDialog(QWidget *parent) :
  QDialog( parent, Qt::WindowStaysOnTopHint ),
  m_autoSip(true)
{
  noOfInstances++;
  setObjectName("VarioModeDialog");
  setAttribute(Qt::WA_DeleteOnClose);
  setModal(true);
  setWindowTitle( tr("Set Variometer") );

  // set font size to a reasonable and usable value
  QFont cf = font();
  cf.setBold( true );
  Layout::fitDialogFont( cf );
  setFont(cf);

  QGridLayout* gridLayout = new QGridLayout(this);
  gridLayout->setMargin(10);
  gridLayout->setSpacing(15);
  int row = 0;

  //---------------------------------------------------------------------

  QLabel *label = new QLabel(tr("Time:"), this);
  gridLayout->addWidget(label, row, 0);

  spinTime = new QSpinBox(this);
  spinTime->setRange(3, 150);
  spinTime->setSuffix( " s" );
  spinTime->setButtonSymbols(QSpinBox::NoButtons);
  spinTime->setAlignment( Qt::AlignHCenter );
  spinTime->setFocus();

  gridLayout->addWidget(spinTime, row++, 1);

  //---------------------------------------------------------------------

  tek = new QCheckBox (tr("TEK Mode"), this);
  tek->setFocusPolicy(Qt::NoFocus);
  gridLayout->addWidget(tek, row++, 0, 1, 2);

  //---------------------------------------------------------------------

  TekAdj = new QLabel(tr("TEK Adjust:"), this);
  gridLayout->addWidget(TekAdj, row, 0);

  spinTEK = new QSpinBox( this );
  spinTEK->setRange( -100, 100 );
  spinTEK->setSingleStep( 1 );
  spinTEK->setSuffix( " %" );
  spinTEK->setButtonSymbols(QSpinBox::NoButtons);
  spinTEK->setAlignment( Qt::AlignHCenter );
  gridLayout->addWidget(spinTEK, row++, 1);

  //---------------------------------------------------------------------

  pplus   = new QPushButton("++", this);
  plus    = new QPushButton("+", this);
  mminus  = new QPushButton("--", this);
  minus   = new QPushButton("-", this);

  int buttonSize = Layout::getButtonSize();
  int iconSize   = buttonSize - 5;

  pplus->setMinimumSize(buttonSize, buttonSize);
  plus->setMinimumSize(buttonSize, buttonSize);
  minus->setMinimumSize(buttonSize, buttonSize);
  mminus->setMinimumSize(buttonSize, buttonSize);

  pplus->setMaximumSize(buttonSize, buttonSize);
  plus->setMaximumSize(buttonSize, buttonSize);
  minus->setMaximumSize(buttonSize, buttonSize);
  mminus->setMaximumSize(buttonSize, buttonSize);

  pplus->setFocusPolicy(Qt::NoFocus);
  plus->setFocusPolicy(Qt::NoFocus);
  minus->setFocusPolicy(Qt::NoFocus);
  mminus->setFocusPolicy(Qt::NoFocus);

  QHBoxLayout *pmLayout = new QHBoxLayout;
  pmLayout->setSpacing(5);
  pmLayout->addWidget(pplus, Qt::AlignLeft);
  pmLayout->addWidget(plus, Qt::AlignLeft);
  pmLayout->addSpacing(20);
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
  cancel->setIconSize(QSize(iconSize, iconSize));
  cancel->setMinimumSize(buttonSize, buttonSize);
  cancel->setMaximumSize(buttonSize, buttonSize);

  ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(iconSize, iconSize));
  ok->setMinimumSize(buttonSize, buttonSize);
  ok->setMaximumSize(buttonSize, buttonSize);

  QVBoxLayout *butLayout = new QVBoxLayout;
  butLayout->addWidget( cancel );
  butLayout->addStretch(10);
  butLayout->addWidget( ok );

  gridLayout->addLayout(butLayout, 0, 3, row, 1);
  gridLayout->setColumnStretch( 2, 10 );

  timer = new QTimer(this);
  timer->setSingleShot(true);

  connect(timer, SIGNAL(timeout()), this, SLOT(reject()));
  connect(tek,   SIGNAL(toggled(bool)), this, SLOT(slot_tekChanged(bool)));

  connect (ok,     SIGNAL(released()), this, SLOT(slot_accept()));
  connect (cancel, SIGNAL(released()), this, SLOT(slot_reject()));

  // Activate keyboard shortcuts for close
  QShortcut* scClose = new QShortcut( this );
  scClose->setKey( Qt::Key_Escape );
  scClose->setKey( Qt::Key_Close );

  connect( scClose, SIGNAL(activated()), this, SLOT( reject() ));

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

  // Switch off automatic software input panel popup
  m_autoSip = qApp->autoSipEnabled();
  qApp->setAutoSipEnabled( m_autoSip );

}

VarioModeDialog::~VarioModeDialog()
{
  noOfInstances--;
  qApp->setAutoSipEnabled( true );
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
      // set focus to tek if it is enabled
      spinTEK->setFocus();
    }
  else
    {
      spinTime->setFocus();
    }

  slot_setTimer();
}

void VarioModeDialog::load()
{
  // qDebug ("VarioModeDialog::load()");
  GeneralConfig *conf = GeneralConfig::instance();

  m_intTime = conf->getVarioIntegrationTime();

  if( m_intTime < 3 ) // check configuration value
    {
      m_intTime = INT_TIME; // reset to default
      conf->setVarioIntegrationTime(m_intTime);
    }

  m_TEKComp = conf->getVarioTekCompensation();
  emit newTEKMode( m_TEKComp );

  m_TEKAdjust = conf->getVarioTekAdjust();
  emit newTEKAdjust( m_TEKAdjust );

  // let us take the user's defined info display time
  m_timeout = conf->getInfoDisplayTime();

  spinTEK->setEnabled(m_TEKComp);
  spinTime->setValue( m_intTime );
  spinTEK->setValue( m_TEKAdjust );
  tek->setChecked( m_TEKComp );

  slot_tekChanged( m_TEKComp );

  spinTime->setFocus();
  slot_setTimer();
}

void VarioModeDialog::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setVarioIntegrationTime( spinTime->value() );

  if( tek->isChecked() != m_TEKComp )
    {
      m_TEKComp = tek->isChecked();
      conf->setVarioTekCompensation( m_TEKComp );
      emit newTEKMode( m_TEKComp );
    }

  if( spinTEK->value() != m_TEKAdjust )
    {
      m_TEKAdjust = spinTEK->value();
      conf->setVarioTekAdjust( m_TEKAdjust );
      emit newTEKAdjust( m_TEKAdjust );
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
  emit closingWidget();
  QDialog::accept();
}

void VarioModeDialog::slot_reject()
{
  emit closingWidget();
  QDialog::reject();
}

void VarioModeDialog::slot_setTimer()
{
  if (m_timeout > 0)
    {
      timer->start( m_timeout * 1000 );
    }
}
