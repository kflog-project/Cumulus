/***********************************************************************
**
**   preflightgliderpage.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers, 2008 Axel Pauli
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
#include "cucalc.h"
#include "preflightgliderpage.h"

PreFlightGliderPage::PreFlightGliderPage(QWidget *parent) : QWidget(parent)
{
  setObjectName("PreFlightGliderPage");

  lastGlider = 0;

  QGridLayout* topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  list = new GliderListWidget(this);
  topLayout->addWidget(list, 0, 0, 1, 2);

  QPushButton* deselect = new QPushButton( tr("Deselect"), this );
  deselect->setToolTip( tr("Reset glider selection") );
  topLayout->addWidget( deselect, 1, 0 );

  QLabel* lblCoPilot = new QLabel(tr("Co-pilot:"),this);
  topLayout->addWidget(lblCoPilot,2, 0);
  edtCoPilot=new QLineEdit(this);
  edtCoPilot->setObjectName("edtLineEdit");
  topLayout->addWidget(edtCoPilot,2, 1);

  QLabel* lblLoad = new QLabel(tr("Added load:"),this);
  topLayout->addWidget(lblLoad,3, 0);
  spinLoad=new QSpinBox(this);
  spinLoad->setObjectName("spinLoad");
  topLayout->addWidget(spinLoad,3, 1);
  spinLoad->setButtonSymbols(QSpinBox::PlusMinus);
  spinLoad->setMinimum(0);
  spinLoad->setMaximum(1000);
  spinLoad->setSingleStep(5);

  QLabel* lblWater = new QLabel(tr("Water balast:"),this);
  topLayout->addWidget(lblWater,4, 0);
  spinWater=new QSpinBox(this);
  spinWater->setObjectName("spinWater");
  topLayout->addWidget(spinWater,4, 1);
  spinWater->setButtonSymbols(QSpinBox::PlusMinus);
  spinWater->setMinimum(0);
  spinWater->setMaximum(300);
  spinWater->setSingleStep(5);

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
  // save co pilot before new selection
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

      spinLoad->setValue( (int) (glider->polar()->grossWeight() - glider->polar()->emptyWeight()) );

      spinWater->setMaximum(glider->maxWater());
      spinWater->setEnabled(glider->maxWater()!=0);
    }
}

void PreFlightGliderPage::slot_gliderDeselected()
{
  // clear last stored glider
  lastGlider = 0;
  // clear list selection
  list->clearSelection();
}

void PreFlightGliderPage::getCurrent()
{
  extern CuCalc* calculator;
  Glider* glider = calculator->glider();

  if( glider == 0 )
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
  extern CuCalc* calculator;
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
}
