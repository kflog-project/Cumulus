/***********************************************************************
**
**   preflightgliderpage.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003      by André Somers
**                   2008-2013 by Axel Pauli
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

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "calculator.h"
#include "generalconfig.h"
#include "glider.h"
#include "layout.h"
#include "numberEditor.h"
#include "preflightgliderpage.h"

PreFlightGliderPage::PreFlightGliderPage(QWidget *parent) :
  QWidget(parent),
  m_lastGlider(0)
{
  setObjectName("PreFlightGliderPage");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("PreFlight - Glider") );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  QGridLayout* topLayout = new QGridLayout;
  topLayout->setMargin(5);

  contentLayout->addLayout(topLayout);

  int row = 0;
  QLabel* lblPilot = new QLabel(tr("Pilot:"), this);
  topLayout->addWidget(lblPilot, row, 0);
  m_edtPilot = new QLineEdit(this);
  m_edtPilot->setText( GeneralConfig::instance()->getSurname() );
  topLayout->addWidget(m_edtPilot, row, 1);

  QLabel* lblLoad = new QLabel(tr("Added load:"), this);
  topLayout->addWidget(lblLoad, row, 2);

  m_edtLoad = new NumberEditor;
  m_edtLoad->setDecimalVisible( false );
  m_edtLoad->setPmVisible( false );
  m_edtLoad->setMaxLength(4);
  m_edtLoad->setSuffix(" kg");
  m_edtLoad->setRange( 0, 9999 );
  m_edtLoad->setValue( 0 );
  topLayout->addWidget(m_edtLoad, row, 3);
  row++;

  QLabel* lblCoPilot = new QLabel(tr("Copilot:"), this);
  topLayout->addWidget(lblCoPilot, row, 0);
  m_edtCoPilot = new QLineEdit(this);
  topLayout->addWidget(m_edtCoPilot, row, 1);

  QLabel* lblWater = new QLabel(tr("Water ballast:"), this);
  topLayout->addWidget(lblWater, row, 2);

  m_edtWater = new NumberEditor;
  m_edtWater->setDecimalVisible( false );
  m_edtWater->setPmVisible( false );
  m_edtWater->setMaxLength(4);
  m_edtWater->setSuffix(" l");
  m_edtWater->setRange( 0, 9999 );
  m_edtWater->setValue( 0 );
  topLayout->addWidget(m_edtWater, row, 3);
  row++;

  QLabel* lblWLoad = new QLabel(tr("Wing load:"), this);
  topLayout->addWidget(lblWLoad, row, 2);
  m_wingLoad = new QLabel;
  m_wingLoad->setFocusPolicy(Qt::NoFocus);
  topLayout->addWidget(m_wingLoad, row, 3);
  row++;

  topLayout->setRowMinimumHeight ( row, 10 );
  row++;

  m_gliderList = new GliderListWidget(this);
#ifndef ANDROID
  m_gliderList->setToolTip(tr("Select a glider to be used"));
#endif

#ifdef QSCROLLER
  QScroller::grabGesture(m_gliderList->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_gliderList->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  topLayout->addWidget(m_gliderList, row, 0, 1, 4);
  row++;

  //---------------------------------------------------------------------
  QPushButton* deselect = new QPushButton( tr("Deselect"), this );
#ifndef ANDROID
  deselect->setToolTip( tr("Clear glider selection") );
#endif
  topLayout->addWidget( deselect, row, 3 );

  //---------------------------------------------------------------------
  m_gliderList->fillList();
  m_gliderList->clearSelection();
  getCurrent();

  connect(deselect, SIGNAL(clicked()), this, SLOT(slotGliderDeselected()) );
  connect(m_gliderList, SIGNAL(itemSelectionChanged()), this, SLOT(slotGliderChanged()));
  connect(m_edtLoad, SIGNAL(numberEdited(const QString&)), this, SLOT(slotNumberEdited(const QString&)));
  connect(m_edtWater, SIGNAL(numberEdited(const QString&)), this, SLOT(slotNumberEdited(const QString&)));

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png"));

  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);
}

PreFlightGliderPage::~PreFlightGliderPage()
{
}

void PreFlightGliderPage::slotGliderChanged()
{
  // save co-pilot before new selection
  if(m_lastGlider)
    {
      if (m_lastGlider->seats() == Glider::doubleSeater)
        {
          m_lastGlider->setCoPilot(m_edtCoPilot->text());
        }
      else
        {
          m_lastGlider->setCoPilot("");
        }
    }

  Glider* glider = m_gliderList->getSelectedGlider();

  m_lastGlider = glider;

  if( glider )
    {
      m_edtCoPilot->setEnabled(glider->seats()==Glider::doubleSeater);
      m_edtCoPilot->setText(glider->coPilot());

      m_edtLoad->setValue( (int) rint(glider->polar()->grossWeight() - glider->polar()->emptyWeight()) );
      m_edtWater->setMaximum(glider->maxWater() );
      m_edtWater->setValue(glider->polar()->water() );

      m_edtLoad->setEnabled(true);
      m_edtWater->setEnabled(glider->maxWater() != 0 );
    }

  slotUpdateWingLoad(0);
}

void PreFlightGliderPage::slotGliderDeselected()
{
  // clear last stored glider
  m_lastGlider = static_cast<Glider *> (0);

  // clear list selection
  m_gliderList->clearSelection();

  // clear values
  m_edtLoad->setValue(0);
  m_edtWater->setValue(0);

  m_edtLoad->setEnabled(false);
  m_edtWater->setEnabled(false);

// clear wing load label
  m_wingLoad->setText("");
}

void PreFlightGliderPage::getCurrent()
{
  extern Calculator* calculator;

  Glider* glider = calculator->glider();

  if( glider == static_cast<Glider *> (0) )
    {
      return;
    }

  m_gliderList->selectItemFromReg( glider->registration() );

  m_edtCoPilot->setEnabled(glider->seats() == Glider::doubleSeater);
  m_edtCoPilot->setText(glider->coPilot());

  m_edtLoad->setValue( (int) (glider->polar()->grossWeight() - glider->polar()->emptyWeight()) );

  m_edtLoad->setEnabled(true);
  m_edtWater->setEnabled(glider->maxWater() != 0);
  m_edtWater->setMaximum(glider->maxWater());
  m_edtWater->setValue(glider->polar()->water());

  m_lastGlider = m_gliderList->getSelectedGlider();
}

void PreFlightGliderPage::save()
{
  extern Calculator* calculator;

  GeneralConfig::instance()->setSurname(m_edtPilot->text().trimmed());

  Glider* glider = m_gliderList->getSelectedGlider(false);

  if(glider)
    {
      if (glider->seats() == Glider::doubleSeater)
        {
          glider->setCoPilot(m_edtCoPilot->text().trimmed());
        }
      else
        {
          glider->setCoPilot("");
        }

      glider->polar()->setGrossWeight(m_edtLoad->value() + glider->polar()->emptyWeight() );
      glider->polar()->setWater(m_edtWater->value(), 0);
      // @AP: save changed added load permanently
      m_gliderList->save();
      glider = new Glider(*m_gliderList->getSelectedGlider(false));
      calculator->setGlider(glider);
      m_gliderList->setStoredSelection(glider);
    }
  else
    {
      // no selected glider, reset of stored selection
      calculator->setGlider( static_cast<Glider *> (0) );
      m_gliderList->setStoredSelection( static_cast<Glider *> (0) );
    }
}

void PreFlightGliderPage::slotUpdateWingLoad( int value )
{
  Q_UNUSED(value)

  Glider* glider = m_gliderList->getSelectedGlider();

  if( glider == 0 || glider->polar() == 0 || glider->polar()->wingArea() == 0.0 )
    {
      // Clear label
      m_wingLoad->setText("");
      return;
    }

  Polar* polar = glider->polar();

  double wload = 0.0;

  if( polar->emptyWeight() )
    {
      wload = (polar->emptyWeight() + m_edtLoad->value() + m_edtWater->value()) / polar->wingArea();
    }

  QString msg = "";

  if( wload )
    {
      msg = QString("%1 Kg/m").arg( wload, 0, 'f', 1 ) +
            QChar(Qt::Key_twosuperior);
    }

  m_wingLoad->setText(msg);
}

void PreFlightGliderPage::slotNumberEdited( const QString& number )
{
  Q_UNUSED( number )

  // Updates the wing load, if load or water have been edited.
  slotUpdateWingLoad( 0 );
}

void PreFlightGliderPage::showEvent(QShowEvent *)
{
  slotGliderChanged();
  m_gliderList->setFocus();
}

void PreFlightGliderPage::slotAccept()
{
  save();
  GeneralConfig::instance()->save();
  emit settingsChanged();
  emit closingWidget();
  QWidget::close();
}

void PreFlightGliderPage::slotReject()
{
  emit closingWidget();
  QWidget::close();
}
