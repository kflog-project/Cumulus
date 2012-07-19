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
#include "generalconfig.h"
#include "layout.h"
#include "preflightgliderpage.h"
#include "varspinbox.h"

PreFlightGliderPage::PreFlightGliderPage(QWidget *parent) :
  QWidget(parent),
  lastGlider(0)
{
  setObjectName("PreFlightGliderPage");
  int row = 0;

  QGridLayout* topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  QLabel* lblPilot = new QLabel(tr("Pilot:"), this);
  topLayout->addWidget(lblPilot, row, 0);
  edtPilot = new QLineEdit(this);
  edtPilot->setText( GeneralConfig::instance()->getSurname() );
  topLayout->addWidget(edtPilot, row, 1);

  QLabel* lblLoad = new QLabel(tr("Added load:"), this);
  topLayout->addWidget(lblLoad, row, 2);
  spinLoad = new QSpinBox(this);
  spinLoad->setRange(0, 1000);
  spinLoad->setSingleStep(5);
  spinLoad->setSuffix(" kg");
  hspinLoad = new VarSpinBox(spinLoad);
  hspinLoad->setEnabled(false);
  topLayout->addWidget(hspinLoad, row, 3);
  row++;

  QLabel* lblCoPilot = new QLabel(tr("Copilot:"), this);
  topLayout->addWidget(lblCoPilot, row, 0);
  edtCoPilot = new QLineEdit(this);
  topLayout->addWidget(edtCoPilot, row, 1);

  QLabel* lblWater = new QLabel(tr("Water ballast:"), this);
  topLayout->addWidget(lblWater, row, 2);
  spinWater = new QSpinBox(this);
  spinWater->setRange(0, 500);
  spinWater->setSingleStep(5);
  spinWater->setSuffix(" l");
  hspinWater = new VarSpinBox(spinWater);
  hspinWater->setEnabled(false);
  topLayout->addWidget(hspinWater, row, 3);
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
  topLayout->addWidget( deselect, row, 3 );

  //---------------------------------------------------------------------
  list->fillList();
  list->clearSelection();
  getCurrent();

  connect(deselect, SIGNAL(clicked()), this, SLOT(slotGliderDeselected()) );
  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(slotGliderChanged()));
  connect(spinLoad, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateWingLoad(int)));
  connect(spinWater, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateWingLoad(int)));
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
      hspinLoad->setEnabled(true);

      spinWater->setMaximum(glider->maxWater() );
      hspinWater->setEnabled(glider->maxWater() != 0 );
      spinWater->setValue(glider->polar()->water() );
    }

  slotUpdateWingLoad(0);
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

  hspinLoad->setEnabled(false);
  hspinWater->setEnabled(false);

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
  hspinLoad->setEnabled(true);

  spinWater->setMaximum(glider->maxWater());
  hspinWater->setEnabled(glider->maxWater() != 0);
  spinWater->setValue(glider->polar()->water());

  lastGlider = list->getSelectedGlider();
}

void PreFlightGliderPage::save()
{
  extern Calculator* calculator;

  GeneralConfig::instance()->setSurname(edtPilot->text().trimmed());

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
      glider = new Glider(*list->getSelectedGlider(false));
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

void PreFlightGliderPage::slotUpdateWingLoad( int value )
{
  Q_UNUSED(value)

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

void PreFlightGliderPage::showEvent(QShowEvent *)
{
  slotGliderChanged();
  list->setFocus();
}

void PreFlightGliderPage::hideEvent( QHideEvent * )
{
  // Save done changes on this page when it is left to have them available
  // on other pages. E.g. Flarm IGC setup page.
  save();
}
