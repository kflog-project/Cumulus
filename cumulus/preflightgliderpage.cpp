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
#include <QGridLayout>

#include "glider.h"
#include "cucalc.h"
#include "preflightgliderpage.h"

PreFlightGliderPage::PreFlightGliderPage(QWidget *parent, const char *name ) : QWidget(parent,name)
{
  lastGlider = 0;

  QGridLayout * topLayout = new QGridLayout(this, 4,2,5);

  list = new GliderList(this, "gliderlist");
  topLayout->addMultiCellWidget(list,1,1,1,2);

  QLabel * lblCoPilot=new QLabel(tr("Co-pilot:"),this);
  topLayout->addWidget(lblCoPilot,2,1);
  edtCoPilot=new QLineEdit(this,"edtLineEdit");
  topLayout->addWidget(edtCoPilot,2,2);

  QLabel * lblLoad=new QLabel(tr("Added load:"),this);
  topLayout->addWidget(lblLoad,3,1);
  spinLoad=new QSpinBox(this,"spinLoad");
  topLayout->addWidget(spinLoad,3,2);
  spinLoad->setButtonSymbols(QSpinBox::PlusMinus);
  spinLoad->setMinValue(0);
  spinLoad->setMaxValue(1000);
  spinLoad->setLineStep(5);

  QLabel * lblWater=new QLabel(tr("Water balast:"),this);
  topLayout->addWidget(lblWater,4,1);
  spinWater=new QSpinBox(this,"spinWater");
  topLayout->addWidget(spinWater,4,2);
  spinWater->setButtonSymbols(QSpinBox::PlusMinus);
  spinWater->setMinValue(0);
  spinWater->setMaxValue(300);
  spinWater->setLineStep(5);

  connect(list,SIGNAL(selectionChanged()),
          this,SLOT(slot_gliderChanged()));
  list->fillList();

  getCurrent();

  topLayout->activate();
}

PreFlightGliderPage::~PreFlightGliderPage()
{}

void PreFlightGliderPage::slot_gliderChanged()
{

  // save co pilot before new selection
  if(lastGlider)
    {
      if (lastGlider->seats()==Glider::doubleSeater)
        {
          lastGlider->setCoPilot(edtCoPilot->text());
        }
      else
        {
          lastGlider->setCoPilot("");
        }
    }

  Glider * glider=list->getSelectedGlider();
  lastGlider=glider;

  if(glider)
    {
      edtCoPilot->setEnabled(glider->seats()==Glider::doubleSeater);
      edtCoPilot->setText(glider->coPilot());

      spinLoad->setValue( (int) (glider->polar()->grossWeight() - glider->polar()->emptyWeight()) );

      spinWater->setMaxValue(glider->maxWater());
      spinWater->setEnabled(glider->maxWater()!=0);
    }
}

void PreFlightGliderPage::getCurrent()
{
  Glider * glider=calculator->glider();
  if (glider==0)
    return;

  Q3ListViewItemIterator it(list);

  for (;it.current();++it)
    {
      //select if the registration matches
      list->setSelected(it.current(), it.current()->text(1)==glider->registration());
    }

  edtCoPilot->setEnabled(glider->seats()==Glider::doubleSeater);
  edtCoPilot->setText(glider->coPilot());

  spinLoad->setValue( (int) (glider->polar()->grossWeight() - glider->polar()->emptyWeight()) );

  spinWater->setMaxValue(glider->maxWater());
  spinWater->setEnabled(glider->maxWater()!=0);
  spinWater->setValue(glider->polar()->water());
  lastGlider = list->getSelectedGlider();
}

void PreFlightGliderPage::save()
{
  Glider * glider=list->getSelectedGlider(false);

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
}

void PreFlightGliderPage::showEvent(QShowEvent *)
{
  slot_gliderChanged();
}
