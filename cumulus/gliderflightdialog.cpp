/***********************************************************************
**
**   gliderflightdialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "gliderflightdialog.h"
#include "calculator.h"
#include "glider.h"
#include "igclogger.h"
#include "mapconfig.h"
#include "generalconfig.h"
#include "layout.h"

// set static member variable
int GliderFlightDialog::noOfInstances = 0;

GliderFlightDialog::GliderFlightDialog (QWidget *parent) :
  QDialog(parent, Qt::WindowStaysOnTopHint)
{
  noOfInstances++;
  setObjectName("GliderFlightDialog");
  setAttribute(Qt::WA_DeleteOnClose);
  setModal(true);
  setWindowTitle (tr("Flight Parameters"));

  // Mc step widths
  mcSmallStep = 0.5;
  mcBigStep   = 1.0;

  QFont b = font();
  b.setBold(true);
  setFont(b);

  // set font size to a reasonable and usable value
  if( font().pointSize() < DialogMinFontSize )
    {
      QFont cf = font();

#ifdef USE_POINT_SIZE_FONT
      cf.setPointSize( DialogMinFontSize );
#else
      cf.setPixelSize( DialogMinFontSize );
#endif
      this->setFont(cf);
    }

  QGridLayout* gridLayout = new QGridLayout(this);
  gridLayout->setMargin(5);
  gridLayout->setSpacing(15);
  int row = 0;

  ftLabel = new QLabel(tr("Flight time:"));
  ftText  = new QLabel;
  gridLayout->addWidget(ftLabel, row, 0);
  gridLayout->addWidget(ftText, row++, 1);

  //---------------------------------------------------------------------

  QLabel* lbl = new QLabel(tr("McCready:"));
  gridLayout->addWidget(lbl, row, 0);
  spinMcCready = new QDoubleSpinBox;
  spinMcCready->setRange(0.0, 20.0);
  spinMcCready->setSingleStep(0.5);
  spinMcCready->setSuffix( QString(" ") + Speed::getUnitText(Speed::getVerticalUnit()));
  spinMcCready->setButtonSymbols(QSpinBox::NoButtons);
  spinMcCready->setAlignment( Qt::AlignHCenter );
  spinMcCready->setFocus();

  connect( spinMcCready, SIGNAL(valueChanged(const QString&)),
           this, SLOT(slotSpinValueChanged(const QString&)));

  gridLayout->addWidget(spinMcCready, row++, 1);

  //---------------------------------------------------------------------

  lbl = new QLabel(tr("Water:"), this);
  gridLayout->addWidget(lbl, row, 0);
  spinWater = new QSpinBox (this);
  spinWater->setRange(0, 200);
  spinWater->setSingleStep(5);
  spinWater->setSuffix( " l" );
  spinWater->setButtonSymbols(QSpinBox::NoButtons);
  spinWater->setAlignment( Qt::AlignHCenter );

  connect( spinWater, SIGNAL(valueChanged(const QString&)),
           this, SLOT(slotSpinValueChanged(const QString&)));

  gridLayout->addWidget(spinWater, row, 1);

  buttonDump = new QPushButton (tr("Dump"), this);
  buttonDump->setMinimumHeight( ButtonSize );
  buttonDump->setFocusPolicy(Qt::NoFocus);

  QHBoxLayout *waterLayout = new QHBoxLayout;
  waterLayout->addWidget( buttonDump );
  waterLayout->addStretch(10);

  gridLayout->addLayout(waterLayout, row++, 2);

  //---------------------------------------------------------------------

  lbl = new QLabel(tr("Bugs:"), this);
  gridLayout->addWidget(lbl, row, 0);
  spinBugs = new QSpinBox (this);
  spinBugs->setRange(0, 90);
  spinBugs->setSingleStep(1);
  spinBugs->setSuffix( " %" );
  spinBugs->setButtonSymbols(QSpinBox::NoButtons);
  spinBugs->setAlignment( Qt::AlignHCenter );

  connect( spinBugs, SIGNAL(valueChanged(const QString&)),
           this, SLOT(slotSpinValueChanged(const QString&)));

  gridLayout->addWidget(spinBugs, row++, 1);

  //---------------------------------------------------------------------

  pplus  = new QPushButton("++", this);
  plus   = new QPushButton("+", this);
  mminus = new QPushButton("--", this);
  minus  = new QPushButton("-", this);

  pplus->setMinimumSize(ButtonSize, ButtonSize);
  plus->setMinimumSize(ButtonSize, ButtonSize);
  minus->setMinimumSize(ButtonSize, ButtonSize);
  mminus->setMinimumSize(ButtonSize, ButtonSize);

  pplus->setMaximumSize(ButtonSize, ButtonSize);
  plus->setMaximumSize(ButtonSize, ButtonSize);
  minus->setMaximumSize(ButtonSize, ButtonSize);
  mminus->setMaximumSize(ButtonSize, ButtonSize);

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
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setMinimumSize(ButtonSize, ButtonSize);
  cancel->setMaximumSize(ButtonSize, ButtonSize);

  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setMinimumSize(ButtonSize, ButtonSize);
  ok->setMaximumSize(ButtonSize, ButtonSize);
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QVBoxLayout *butLayout = new QVBoxLayout;
  butLayout->addWidget( cancel );
  butLayout->addStretch(10);
  butLayout->addWidget( ok );

  gridLayout->addLayout(butLayout, 1, 3, row, 1);
  gridLayout->setColumnStretch( 2, 10 );

  // @AP: let us take the user's defined info display time
  GeneralConfig *conf = GeneralConfig::instance();
  timer = new QTimer(this);
  timer->setSingleShot(true);
  _time = conf->getInfoDisplayTime();

  connect (timer, SIGNAL(timeout()), this, SLOT(slotReject()));
  connect (buttonDump, SIGNAL(released()), this, SLOT(slotDump()));
  connect (ok, SIGNAL(released()), this, SLOT(slotAccept()));
  connect (cancel, SIGNAL(released()), this, SLOT(slotReject()));

  QSignalMapper* signalMapper = new QSignalMapper(this);
  connect(pplus, SIGNAL(pressed()), signalMapper, SLOT(map()));
  signalMapper->setMapping(pplus, 0);
  connect(plus, SIGNAL(pressed()), signalMapper, SLOT(map()));
  signalMapper->setMapping(plus, 1);
  connect(minus, SIGNAL(pressed()), signalMapper, SLOT(map()));
  signalMapper->setMapping(minus, 2);
  connect(mminus, SIGNAL(pressed()), signalMapper, SLOT(map()));
  signalMapper->setMapping(mminus, 3);
  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slotChange(int)));
}

GliderFlightDialog::~GliderFlightDialog()
{
  noOfInstances--;
}

void GliderFlightDialog::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  load();

  double mcMax;

  switch (Speed::getVerticalUnit())
    {
    case Speed::knots:
      mcMax = 40.0;
      mcSmallStep = 0.5;
      mcBigStep = 1.0;
      break;
    case Speed::feetPerMinute:
      mcMax = 4000.0;
      mcSmallStep = 50.0;
      mcBigStep = 100.0;
      break;
    case Speed::metersPerSecond:
      mcMax = 20.0;
      mcSmallStep = 0.5;
      mcBigStep = 1.0;
      break;
    default:
      mcMax = 20.0;
      mcSmallStep = 0.5;
      mcBigStep = 1.0;
      break;
    }

  spinMcCready->setMaximum(mcMax);
  spinMcCready->setSingleStep(mcSmallStep);

  spinMcCready->setFocus();
  startTimer();

  slotShowFlightTime();

}

void GliderFlightDialog::load()
{
  Glider *glider = calculator->glider();

  if( glider )
    {
      spinMcCready->setEnabled(true);
      spinWater->setEnabled(true);
      spinBugs->setEnabled(true);
      buttonDump->setEnabled(true);
      pplus->setEnabled(true);
      plus->setEnabled(true);
      mminus->setEnabled(true);
      minus->setEnabled(true);

      spinWater->setMaximum( glider->maxWater() );

      if ( glider->maxWater() == 0 )
        {
          spinWater->setEnabled(false);
          buttonDump->setEnabled(false);
        }

      spinMcCready->setValue(calculator->getlastMc().getVerticalValue());
      spinWater->setValue(glider->polar()->water());
      spinBugs->setValue(glider->polar()->bugs());
    }
  else
    {
      spinMcCready->setEnabled(false);
      spinWater->setEnabled(false);
      spinBugs->setEnabled(false);
      buttonDump->setEnabled(false);
      pplus->setEnabled(false);
      plus->setEnabled(false);
      mminus->setEnabled(false);
      minus->setEnabled(false);
    }
}

void GliderFlightDialog::save()
{
  if( spinMcCready->isEnabled() && spinBugs->isEnabled() )
  {
    emit newWaterAndBugs( spinWater->value(), spinBugs->value() );

    Speed mc; mc.setVerticalValue( spinMcCready->value() );
    emit newMc( mc );
  }
}

void GliderFlightDialog::slotMcPlus()
{
  if( plus->isDown() || pplus->isDown() )
    {
      spinMcCready->stepUp();

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotMcPlus()));
    }
}

void GliderFlightDialog::slotMcMinus()
{
  if( minus->isDown() || mminus->isDown() )
    {
      spinMcCready->stepDown();

      // Start repetition timer, to check, if button is longer pressed.
       QTimer::singleShot(300, this, SLOT(slotMcMinus()));
     }
}

void GliderFlightDialog::slotWaterPlus()
{
  if( plus->isDown() || pplus->isDown() )
    {
      spinWater->stepUp();

      // Start repetition timer, to check, if button is longer pressed.
       QTimer::singleShot(300, this, SLOT(slotWaterPlus()));
     }
}

void GliderFlightDialog::slotWaterMinus()
{
  if( minus->isDown() || mminus->isDown() )
    {
      spinWater->stepDown();

      // Start repetition timer, to check, if button is longer pressed.
       QTimer::singleShot(300, this, SLOT(slotWaterMinus()));
     }
}

void GliderFlightDialog::slotBugsPlus()
{
  if( plus->isDown() || pplus->isDown() )
    {
      spinBugs->stepUp();

      // Start repetition timer, to check, if button is longer pressed.
       QTimer::singleShot(300, this, SLOT(slotBugsPlus()));
     }
}

void GliderFlightDialog::slotBugsMinus()
{
  if( minus->isDown() || mminus->isDown() )
    {
      spinBugs->stepDown();

      // Start repetition timer, to check, if button is longer pressed.
       QTimer::singleShot(300, this, SLOT(slotBugsMinus()));
     }
}

/**
 * This method changes the value in the spin box which has the current focus.
 *
 * @param newStep value to be set in spin box
 */
void GliderFlightDialog::slotChange( int newStep )
{
  // Look which spin box has the focus
  if( QApplication::focusWidget() == spinMcCready )
    {
      // qDebug() << "spinMcCready has focus";
      switch(newStep)
        {
        case 0: // ++ was pressed
          spinMcCready->setSingleStep( mcBigStep );
          slotMcPlus();
          break;
        case 1: // + was pressed
          spinMcCready->setSingleStep( mcSmallStep );
          slotMcPlus();
          break;
        case 2: // - was pressed
          spinMcCready->setSingleStep( mcSmallStep );
          slotMcMinus();
          break;
        case 3: // -- was pressed
          spinMcCready->setSingleStep( mcBigStep );
          slotMcMinus();
          break;
        }

      return;
    }

  if( QApplication::focusWidget() == spinWater )
    {
      // qDebug() << "spinWater has focus";
      switch(newStep)
        {
        case 0: // ++ was pressed
          spinWater->setSingleStep( 10 );
          slotWaterPlus();
          break;
        case 1: // + was pressed
          spinWater->setSingleStep( 1 );
          slotWaterPlus();
          break;
        case 2: // - was pressed
          spinWater->setSingleStep( 1 );
          slotWaterMinus();
          break;
        case 3: // -- was pressed
          spinWater->setSingleStep( 10 );
          slotWaterMinus();
          break;
        }

      return;
    }

  if( QApplication::focusWidget() == spinBugs )
    {
      // qDebug() << "spinBugs has focus";
      switch(newStep)
        {
        case 0: // ++ was pressed
          spinBugs->setSingleStep( 5 );
          slotBugsPlus();
          break;
        case 1: // + was pressed
          spinBugs->setSingleStep( 1 );
          slotBugsPlus();
          break;
        case 2: // - was pressed
          spinBugs->setSingleStep( 1 );
          slotBugsMinus();
          break;
        case 3: // -- was pressed
          spinBugs->setSingleStep( 5 );
          slotBugsMinus();
          break;
        }

      return;
    }
}

void GliderFlightDialog::slotSpinValueChanged( const QString& text )
{
  Q_UNUSED( text )
  // Restarts the timer after a spin box value change
  startTimer();
}

void GliderFlightDialog::slotDump()
{
  spinWater->setValue(0);
  spinWater->setFocus();
}

/** Shows the flight time. */
void GliderFlightDialog::slotShowFlightTime()
{
  QDateTime takeoff = IgcLogger::instance()->loggerStart();

  if( takeoff.isValid() )
    {
      ftLabel->setVisible(true);
      ftText->setVisible(true);

      int seconds = takeoff.secsTo( QDateTime::currentDateTime() );

      QTime time;
      time = time.addSecs( seconds );

      ftText->setText( time.toString("hh:mm:ss") );

      // Fire an update timer after 5 seconds.
      QTimer::singleShot(5000, this, SLOT(slotShowFlightTime()));
    }
  else
    {
      ftLabel->setVisible(false);
      ftText->setVisible(false);
    }
}

void GliderFlightDialog::slotAccept()
{
  save();
  timer->stop();
  emit closingWidget();
  QDialog::accept();
}

void GliderFlightDialog::slotReject()
{
  emit closingWidget();
  QDialog::reject();
}

void GliderFlightDialog::startTimer()
{
  if ( _time > 0 )
    {
      timer->start(_time * 1000);
    }
}
