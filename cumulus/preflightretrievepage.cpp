/***********************************************************************
 **
 **   preflightretrievepage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2013 by Axel Pauli <kflog.cumulus@gmail.com>
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
#include "jnisupport.h"
#include "layout.h"
#include "numberEditor.h"
#include "preflightretrievepage.h"

PreFlightRetrievePage::PreFlightRetrievePage(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("PreFlightRetrievePage");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("PreFlight - Retrieve") );

  if( parent )
    {
      resize( parent->size() );
    }

  // Layout used by scroll area
  QHBoxLayout *sal = new QHBoxLayout;

  // new widget used as container for the dialog layout.
  QWidget* sw = new QWidget;

  // Scroll area
  QScrollArea* sa = new QScrollArea;
  sa->setWidgetResizable( true );
  sa->setFrameStyle( QFrame::NoFrame );
  sa->setWidget( sw );

#ifdef QSCROLLER
  QScroller::grabGesture( sa>viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // Add scroll area to its own layout
  sal->addWidget( sa );

  QHBoxLayout *contentLayout = new QHBoxLayout(this);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal, 10 );

  // Top layout's parent is the scroll widget
  QGridLayout *topLayout = new QGridLayout(sw);

  int row = 0;

  QLabel *lbl = new QLabel( tr("Call your retrieve via SMS") );
  topLayout->addWidget(lbl, row, 0, 1, 4);
  row++;

  lbl = new QLabel(tr("Mobile number:"));
  topLayout->addWidget(lbl, row, 0);

  m_mobileNumber = new NumberEditor;
  m_mobileNumber->setDecimalVisible( false );
  m_mobileNumber->setPmVisible( false );
  m_mobileNumber->setTip(tr("Mobile number"));
  m_mobileNumber->setAlignment(Qt::AlignLeft);
  topLayout->addWidget(m_mobileNumber, row, 1);

  QPushButton *clear = new QPushButton( tr("Clear") );
  topLayout->addWidget(clear, row, 2);
  topLayout->setColumnStretch(3, 10);
  row++;

  connect( clear, SIGNAL(pressed()), m_mobileNumber, SLOT(slot_Clear()));

  lbl = new QLabel(tr("Coordinate format:"));
  topLayout->addWidget(lbl, row, 0);
  m_positionFormat = new QComboBox(this);
  m_positionFormat->setEditable(false);
  topLayout->addWidget(m_positionFormat, row++, 1);

  m_positionFormat->addItem(QString("ddd") + Qt::Key_degree + "mm'ss\"");
  m_positionFormat->addItem(QString("ddd") + Qt::Key_degree + "mm.mmm'");
  m_positionFormat->addItem(QString("ddd.ddddd") + Qt::Key_degree);
  row++;
  topLayout->setRowStretch(row, 10);
  row++;
  QPushButton *callReturner = new QPushButton(tr("Call retrieve"));
  topLayout->addWidget(callReturner, row, 0);

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

  connect(callReturner, SIGNAL(pressed()), this, SLOT(slotCallReturner()));
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

  load();
}

PreFlightRetrievePage::~PreFlightRetrievePage()
{
}

void PreFlightRetrievePage::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  m_mobileNumber->setText( conf->getReturnerMobileNumber() );
  m_positionFormat->setCurrentIndex( conf->getReturnerPositionFormat() );
}

void PreFlightRetrievePage::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setReturnerMobileNumber( m_mobileNumber->text() );
  conf->setReturnerPositionFormat( m_positionFormat->currentIndex() );
 }

void PreFlightRetrievePage::slotAccept()
{
  save();
  GeneralConfig::instance()->save();
  emit closingWidget();
  QWidget::close();
}

void PreFlightRetrievePage::slotReject()
{
  emit closingWidget();
  QWidget::close();
}

void PreFlightRetrievePage::slotCallReturner()
{
  GeneralConfig *conf = GeneralConfig::instance();

  QString smsText = m_mobileNumber->text() + ";";

  if( conf->getSurname().isEmpty() )
    {
      smsText += tr("Pilot");
    }
  else
    {
      smsText += conf->getSurname();
    }

  smsText += tr(" has outlanded with glider");

  extern Calculator* calculator;

  if( calculator->glider() )
    {
      smsText += " " + calculator->glider()->registration();
    }

  const QPoint& position = calculator->getlastPosition();

  smsText += tr(" at location ") +
             WGSPoint::printPos(position.x(), true, m_positionFormat->currentIndex() ) +
             " / "
             + WGSPoint::printPos(position.y(), false, m_positionFormat->currentIndex() )
             + ". ";

  double lat = static_cast<double>(position.x()) / 600000.0;
  double lon = static_cast<double>(position.y()) / 600000.0;

  // Add location link for Google maps a.s.o
  smsText += "loc: " + QString("%1").arg( lat, 0, 'f', 5) + "," +
                       QString("%1").arg( lon, 0, 'f', 5);

  jniCallReturner( smsText );
}
