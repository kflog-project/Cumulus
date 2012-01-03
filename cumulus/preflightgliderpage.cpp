/***********************************************************************
**
**   preflightgliderpage.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003      by André Somers
**                   2008-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "glider.h"
#include "calculator.h"
#include "preflightgliderpage.h"
#include "generalconfig.h"

PreFlightGliderPage::PreFlightGliderPage(QWidget *parent) :
  QWidget(parent),
  lastGlider(0),
  lastFocusWidget(0)
{
  setObjectName("PreFlightGliderPage");
  int row = 0;

  QGridLayout* topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  QLabel* lblPilot = new QLabel(tr("Pilot:"), this);
  topLayout->addWidget(lblPilot, row, 0);
  lblPilot = new QLabel(this);
  lblPilot->setText( GeneralConfig::instance()->getSurname() );
  topLayout->addWidget(lblPilot, row, 1);

  QLabel* lblLoad = new QLabel(tr("Added load:"), this);
  topLayout->addWidget(lblLoad, row, 2);
  spinLoad = new QSpinBox(this);
  topLayout->addWidget(spinLoad, row, 3);
  spinLoad->setButtonSymbols(QSpinBox::NoButtons);
  spinLoad->setFocusPolicy(Qt::StrongFocus);
  spinLoad->setRange(0, 1000);
  spinLoad->setSingleStep(5);
  spinLoad->setSuffix(" kg");
  spinLoad->setEnabled(false);

  row++;

  QLabel* lblCoPilot = new QLabel(tr("Copilot:"), this);
  topLayout->addWidget(lblCoPilot, row, 0);
  edtCoPilot = new QLineEdit(this);
  topLayout->addWidget(edtCoPilot, row, 1);

  QLabel* lblWater = new QLabel(tr("Water ballast:"), this);
  topLayout->addWidget(lblWater, row, 2);
  spinWater = new QSpinBox(this);
  topLayout->addWidget(spinWater, row, 3);
  spinWater->setButtonSymbols(QSpinBox::NoButtons);
  spinWater->setFocusPolicy(Qt::StrongFocus);
  spinWater->setRange(0, 500);
  spinWater->setSingleStep(5);
  spinWater->setSuffix(" l");
  spinWater->setEnabled(false);
  row++;

  QLabel* lblWLoad = new QLabel(tr("Wing load:"), this);
  topLayout->addWidget(lblWLoad, row, 2);
  wingLoad = new QLabel;
  wingLoad->setFocusPolicy(Qt::NoFocus);
  topLayout->addWidget(wingLoad, row, 3);
  row++;

  topLayout->setRowMinimumHeight ( row, 10 );
  row++;

  list = new GliderListWidget(this);
  list->setToolTip(tr("Select a glider to be used"));
  topLayout->addWidget(list, row, 0, 1, 4);
  row++;

  //---------------------------------------------------------------------
  QPushButton* deselect = new QPushButton( tr("Deselect"), this );
  deselect->setToolTip( tr("Clear glider selection") );

  // button size
  int size = 40;

  // take a bold font for the plus and minus sign
  QFont bFont = font();
  bFont.setBold(true);

  plus   = new QPushButton("+");
  minus  = new QPushButton("-");

  plus->setToolTip( tr("Increase spinbox value") );
  minus->setToolTip( tr("Decrease spinbox value") );

  plus->setFont(bFont);
  minus->setFont(bFont);

  plus->setMinimumSize(size, size);
  minus->setMinimumSize(size, size);

  plus->setMaximumSize(size, size);
  minus->setMaximumSize(size, size);

  // The buttons have no focus policy to avoid a focus change during click of them.
  plus->setFocusPolicy(Qt::NoFocus);
  minus->setFocusPolicy(Qt::NoFocus);

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget( deselect );
  buttonLayout->addStretch( 10 );
  buttonLayout->addWidget( plus );
  buttonLayout->addSpacing(20);
  buttonLayout->addWidget( minus );

  topLayout->addLayout( buttonLayout, row, 0, 1, 4 );

  //---------------------------------------------------------------------
  list->fillList();
  list->clearSelection();
  getCurrent();

  connect( deselect, SIGNAL(clicked()), this, SLOT(slotGliderDeselected()) );
  connect( list, SIGNAL(itemSelectionChanged()), this, SLOT(slotGliderChanged()) );
  connect( plus, SIGNAL(clicked()),  this, SLOT(slotIncrementBox()));
  connect( minus, SIGNAL(clicked()), this, SLOT(slotDecrementBox()));

  /**
   * If the plus or minus button is clicked, the focus is changed to the main
   * window. I don't know why. Therefore the previous focused widget must be
   * saved, to have an indication, if a spinbox entry should be modified.
   */
  connect( QCoreApplication::instance(), SIGNAL(focusChanged( QWidget*, QWidget*)),
           this, SLOT( slotFocusChanged( QWidget*, QWidget*)) );
}

PreFlightGliderPage::~PreFlightGliderPage()
{
}

void PreFlightGliderPage::slotGliderChanged()
{
  // save co-pilot before new selection
  if(lastGlider)
    {
      if (lastGlider->seats() == Glider::doubleSeater)
        {
          lastGlider->setCoPilot(edtCoPilot->text());
        }
      else
        {
          lastGlider->setCoPilot("");
        }
    }

  Glider* glider = list->getSelectedGlider();

  lastGlider = glider;

  if( glider )
    {
      edtCoPilot->setEnabled(glider->seats()==Glider::doubleSeater);
      edtCoPilot->setText(glider->coPilot());

      spinLoad->setValue( (int) rint(glider->polar()->grossWeight() - glider->polar()->emptyWeight()) );
      spinLoad->setEnabled(true);

      spinWater->setMaximum(glider->maxWater() );
      spinWater->setEnabled(glider->maxWater() != 0 );
      spinWater->setValue(glider->polar()->water() );
    }

  updateWingLoad();
}

void PreFlightGliderPage::slotGliderDeselected()
{
  // clear last stored glider
  lastGlider = static_cast<Glider *> (0);

  // clear list selection
  list->clearSelection();

  // clear spinboxes
  spinLoad->setValue(0);
  spinWater->setValue(0);

  spinLoad->setEnabled(false);
  spinWater->setEnabled(false);

// clear wing load label
  wingLoad->setText("");
}

void PreFlightGliderPage::getCurrent()
{
  extern Calculator* calculator;

  Glider* glider = calculator->glider();

  if( glider == static_cast<Glider *> (0) )
    {
      return;
    }

  list->selectItemFromReg( glider->registration() );

  edtCoPilot->setEnabled(glider->seats() == Glider::doubleSeater);
  edtCoPilot->setText(glider->coPilot());

  spinLoad->setValue( (int) (glider->polar()->grossWeight() - glider->polar()->emptyWeight()) );
  spinLoad->setEnabled(true);

  spinWater->setMaximum(glider->maxWater());
  spinWater->setEnabled(glider->maxWater() != 0);
  spinWater->setValue(glider->polar()->water());

  lastGlider = list->getSelectedGlider();
}

void PreFlightGliderPage::save()
{
  extern Calculator* calculator;
//  qDebug("## s00 ## calculator:  %s", calculator->gliderType().toLatin1().data() );

  Glider* glider = list->getSelectedGlider(false);

  if(glider)
    {
      if (glider->seats() == Glider::doubleSeater)
        {
          glider->setCoPilot(edtCoPilot->text());
        }
      else
        {
          glider->setCoPilot("");
        }

      glider->polar()->setGrossWeight(spinLoad->value() + glider->polar()->emptyWeight() );
      glider->polar()->setWater(spinWater->value(), 0);
      // @AP: save changed added load permanently
      list->save();
      // @AP: take glider from list
      glider = list->getSelectedGlider(true);
      calculator->setGlider(glider);
      list->setStoredSelection(glider);
    }
  else
    {
      // no selected glider, reset of stored selection
      calculator->setGlider( static_cast<Glider *> (0) );
      list->setStoredSelection( static_cast<Glider *> (0) );
    }
}

void PreFlightGliderPage::updateWingLoad()
{
  Glider* glider = list->getSelectedGlider();

  if( glider == 0 || glider->polar() == 0 || glider->polar()->wingArea() == 0.0 )
    {
      // Clear label
      wingLoad->setText("");
      return;
    }

  Polar* polar = glider->polar();

  double wload = 0.0;

  if( polar->emptyWeight() )
    {
      wload = (polar->emptyWeight() + spinLoad->value() + spinWater->value()) / polar->wingArea();
    }

  QString msg = "";

  if( wload )
    {
      msg = QString("%1 Kg/m").arg( wload, 0, 'f', 1 ) +
            QChar(Qt::Key_twosuperior);
    }

  wingLoad->setText(msg);
}

void PreFlightGliderPage::slotIncrementBox()
{
  // Look which spin box has the focus. Note, focus can be changed by clicking
  // the connected button. Therefore take old focus widget under account and
  // set the focus back to the spinbox.
  if( QApplication::focusWidget() == spinLoad || lastFocusWidget == spinLoad )
    {
      spinLoad->setValue( spinLoad->value() + spinLoad->singleStep() );
      spinLoad->setFocus();
    }
  else if( QApplication::focusWidget() == spinWater || lastFocusWidget == spinWater )
    {
      spinWater->setValue( spinWater->value() + spinWater->singleStep() );
      spinWater->setFocus();
    }
  else
    {
      return;
    }

  updateWingLoad();
}

void PreFlightGliderPage::slotDecrementBox()
{
  // Look which spin box has the focus. Note, focus can be changed by clicking
  // the connected button. Therefore take old focus widget under account and
  // set the focus back to the spinbox.
  if( QApplication::focusWidget() == spinLoad || lastFocusWidget == spinLoad )
    {
      spinLoad->setValue( spinLoad->value() - spinLoad->singleStep() );
      spinLoad->setFocus();
    }
  else if( QApplication::focusWidget() == spinWater || lastFocusWidget == spinWater )
    {
      spinWater->setValue( spinWater->value() - spinWater->singleStep() );
      spinWater->setFocus();
    }
  else
    {
      return;
    }

  updateWingLoad();
}

void PreFlightGliderPage::slotFocusChanged( QWidget* oldWidget, QWidget* newWidget)
{
  Q_UNUSED(newWidget)

  // We save the old widget, which has just lost the focus.
  lastFocusWidget = oldWidget;
}

void PreFlightGliderPage::showEvent(QShowEvent *)
{
  slotGliderChanged();
  list->setFocus();
}
