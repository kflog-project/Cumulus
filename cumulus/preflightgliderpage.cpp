/***********************************************************************
**
**   preflightgliderpage.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003      by André Somers
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QToolTip>

#include "glider.h"
#include "calculator.h"
#include "preflightgliderpage.h"
#include "generalconfig.h"

PreFlightGliderPage::PreFlightGliderPage(QWidget *parent) : QWidget(parent)
{
  setObjectName("PreFlightGliderPage");
  lastGlider = 0;
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
  spinLoad=new QSpinBox(this);
  topLayout->addWidget(spinLoad, row, 3);
  spinLoad->setButtonSymbols(QSpinBox::PlusMinus);
  spinLoad->setMinimum(0);
  spinLoad->setMaximum(1000);
  spinLoad->setSingleStep(5);
  spinLoad->setSuffix(" kg");
  row++;

  QLabel* lblCoPilot = new QLabel(tr("Copilot:"), this);
  topLayout->addWidget(lblCoPilot, row, 0);
  edtCoPilot=new QLineEdit(this);
  topLayout->addWidget(edtCoPilot, row, 1);

  QLabel* lblWater = new QLabel(tr("Water ballast:"), this);
  topLayout->addWidget(lblWater, row, 2);
  spinWater=new QSpinBox(this);
  topLayout->addWidget(spinWater, row, 3);
  spinWater->setButtonSymbols(QSpinBox::PlusMinus);
  spinWater->setMinimum(0);
  spinWater->setMaximum(300);
  spinWater->setSingleStep(5);
  spinWater->setSuffix(" l");
  row++;

  topLayout->setRowMinimumHeight ( row, 10 );
  row++;

  list = new GliderListWidget(this);
  topLayout->addWidget(list, row, 0, 1, 4);
  row++;

  QPushButton* deselect = new QPushButton( tr("Deselect"), this );
  deselect->setToolTip( tr("Clear glider selection") );
  topLayout->addWidget( deselect, row, 0 );

  list->fillList();
  list->clearSelection();
  getCurrent();

  connect(deselect, SIGNAL(clicked()),
          this, SLOT(slot_gliderDeselected()) );

  connect(list, SIGNAL(itemSelectionChanged()),
          this, SLOT(slot_gliderChanged()));
}

PreFlightGliderPage::~PreFlightGliderPage()
{}

void PreFlightGliderPage::slot_gliderChanged()
{
  // save copilot before new selection
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

      spinWater->setMaximum(glider->maxWater());
      spinWater->setEnabled(glider->maxWater()!=0);
    }
}

void PreFlightGliderPage::slot_gliderDeselected()
{
  // clear last stored glider
  lastGlider = static_cast<Glider *> (0);
  // clear list selection
  list->clearSelection();
}

void PreFlightGliderPage::getCurrent()
{
  extern Calculator* calculator;
  Glider* glider = calculator->glider();

  if( glider == static_cast<Glider *> (0) )
    {
      return;
    }
//  qDebug("## c2 ## reg %s", glider->registration().toLatin1().data() );
  list->selectItemFromReg( glider->registration() );

  edtCoPilot->setEnabled(glider->seats() == Glider::doubleSeater);
  edtCoPilot->setText(glider->coPilot());

  spinLoad->setValue( (int) (glider->polar()->grossWeight() - glider->polar()->emptyWeight()) );

  spinWater->setMaximum(glider->maxWater());
  spinWater->setEnabled(glider->maxWater()!=0);
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
      if (glider->seats()==Glider::doubleSeater)
        {
          glider->setCoPilot(edtCoPilot->text());
        }
      else
        {
          glider->setCoPilot("");
        }

      glider->polar()->setGrossWeight(spinLoad->value() + glider->polar()->emptyWeight() );
      glider->polar()->setWater(spinWater->value(),0);
      // @AP: save changed added load permanently
      list->save();
      // @AP: take glider from list
      glider=list->getSelectedGlider(true);
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

void PreFlightGliderPage::showEvent(QShowEvent *)
{
  slot_gliderChanged();
  list->setFocus();
}
