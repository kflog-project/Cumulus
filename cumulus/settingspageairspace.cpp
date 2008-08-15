/***********************************************************************
 **
 **   settingspageairspace.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Eggert Ehmke, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QHeaderView>
#include <QToolTip>

#include "airspace.h"
#include "basemapelement.h"
#include "distance.h"
#include "generalconfig.h"
#include "settingspageairspace.h"

SettingsPageAirspace::SettingsPageAirspace(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("SettingsPageAirspace");

  // save current altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.

  altUnit = Altitude::getUnit();
  QString unit = (altUnit == Altitude::meters) ? "m" : "ft";

  m_warningsDlg = new SettingsPageAirspaceWarnings(this);

  m_fillingDlg = new SettingsPageAirspaceFilling(this);

  QGridLayout *topLayout = new QGridLayout(this);
  topLayout->setMargin(3);

  int row=0;

  lvLoadOptions = new QTableWidget(14, 1, this);
  lvLoadOptions->setShowGrid( false );

  // hide vertical headers
  QHeaderView *vHeader = lvLoadOptions->verticalHeader();
  vHeader->setVisible(false);

  QTableWidgetItem *item = new QTableWidgetItem( tr("Enable Airspace Drawing") );
  lvLoadOptions->setHorizontalHeaderItem( 0, item );

  topLayout->addWidget(lvLoadOptions, row, 0, 1, 4);
  row++;

  enableForceDrawing = new QCheckBox(tr("Force drawing for airspace less than"), this);
  enableForceDrawing->setChecked(true);
  topLayout->addWidget( enableForceDrawing, row, 0, 1, 4 );
  connect( enableForceDrawing, SIGNAL(toggled(bool)), SLOT(enabledToggled(bool)));
  row++;

  spinForceMargin = new QSpinBox(this);
  spinForceMargin-> setRange( 0, 99999 );
  spinForceMargin->setSingleStep( 1 );
  spinForceMargin->setButtonSymbols(QSpinBox::PlusMinus);

  topLayout->addWidget( spinForceMargin, row, 0 );
  topLayout->addWidget(new QLabel(tr("%1 above me.").arg(unit), this), row, 1, 1, 3);
  row++;

  cmdWarning = new QPushButton(tr("Airspace Warnings"), this);
  topLayout->addWidget(cmdWarning, row, 0, 1, 2, Qt::AlignLeft);
  connect (cmdWarning, SIGNAL(clicked()), m_warningsDlg, SLOT(show()));

  cmdFilling = new QPushButton(tr("Airspace filling"), this);
  topLayout->addWidget(cmdFilling, row, 2, 1, 2, Qt::AlignRight);
  connect (cmdFilling, SIGNAL(clicked()), m_fillingDlg, SLOT(show()));

  row = 0;

  drawAirspaceA = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirA) );
  drawAirspaceA->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawAirspaceA );

  drawAirspaceB = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirB) );
  drawAirspaceB->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawAirspaceB );

  drawAirspaceC = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirC) );
  drawAirspaceC->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawAirspaceC );

  drawControlC = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::ControlC) );
  drawControlC->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawControlC );

  drawAirspaceD = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirD) );
  drawAirspaceD->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawAirspaceD );

  drawControlD = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::ControlD) );
  drawControlD->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawControlD );

  drawAirspaceElow = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirElow) );
  drawAirspaceElow->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawAirspaceElow );

  drawAirspaceEhigh = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirEhigh) );
  drawAirspaceEhigh->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawAirspaceEhigh );

  drawAirspaceF = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirF) );
  drawAirspaceF->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawAirspaceF );

  drawRestricted = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Restricted) );
  drawRestricted->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawRestricted );

  drawDanger = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Danger) );
  drawDanger->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawDanger );

  drawTMZ = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Tmz) );
  drawTMZ->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawTMZ );

  drawLowFlight = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::LowFlight) );
  drawLowFlight->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawLowFlight );

  drawSuSector = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::SuSector) );
  drawSuSector->setFlags( Qt::ItemIsEnabled|Qt::ItemIsUserCheckable );
  lvLoadOptions->setItem( row++, 0, drawSuSector );

  lvLoadOptions->sortItems( 0 );
//  lvLoadOptions->adjustSize();
//  lvLoadOptions->setColumnWidth( 0, lvLoadOptions->maximumViewportSize().width()-20 );
  lvLoadOptions->setColumnWidth(0,360);

}


SettingsPageAirspace::~SettingsPageAirspace()
{}


void SettingsPageAirspace::slot_load()
{
  GeneralConfig * conf = GeneralConfig::instance();
  bool enabled = conf->getForceAirspaceDrawingEnabled();

  enableForceDrawing->setChecked(enabled);
  enabledToggled(enabled);

  if( altUnit == Altitude::meters )
    { // user wants meters
      spinForceMargin->setValue((int) rint(conf->getForceAirspaceDrawingDistance().getMeters()));
    }
  else
    { // user get feet
      spinForceMargin->setValue((int) rint(conf->getForceAirspaceDrawingDistance().getFeet()));
    }

  drawAirspaceA->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::AirA) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceB->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::AirB) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceC->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::AirC) ? Qt::Checked : Qt::Unchecked );
  drawControlC->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::ControlC) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceD->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::AirD) ? Qt::Checked : Qt::Unchecked );
  drawControlD->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::ControlD) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceElow->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::AirElow) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceEhigh->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::AirEhigh) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceF->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::AirF) ? Qt::Checked : Qt::Unchecked );
  drawRestricted->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::Restricted) ? Qt::Checked : Qt::Unchecked );
  drawDanger->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::Danger) ? Qt::Checked : Qt::Unchecked );
  drawTMZ->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::Tmz) ? Qt::Checked : Qt::Unchecked );
  drawLowFlight->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::LowFlight) ? Qt::Checked : Qt::Unchecked );
  drawSuSector->setCheckState (conf->getAirspaceWarningEnabled(BaseMapElement::SuSector) ? Qt::Checked : Qt::Unchecked );

  m_fillingDlg->slot_load();
  m_warningsDlg->slot_load();
}


void SettingsPageAirspace::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();
  AirspaceWarningDistance awd;

  Distance forceDist;

  // @AP: Store warning distances always as meters
  if( altUnit == Altitude::meters )
    {
      forceDist.setMeters( spinForceMargin->value() );
    }
  else
    {
      forceDist.setFeet( spinForceMargin->value() );
    }

  conf->setForceAirspaceDrawingDistance(forceDist);
  conf->setForceAirspaceDrawingEnabled(enableForceDrawing->checkState() == Qt::Checked ? true : false);

  conf->setAirspaceWarningEnabled(BaseMapElement::AirA,drawAirspaceA->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::AirB,drawAirspaceB->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::AirC,drawAirspaceC->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::ControlC,drawControlC->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::AirD,drawAirspaceD->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::ControlD,drawControlD->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::AirElow,drawAirspaceElow->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::AirEhigh,drawAirspaceEhigh->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::AirF,drawAirspaceF->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::Restricted,drawRestricted->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::Danger,drawDanger->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::Tmz,drawTMZ->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::LowFlight,drawLowFlight->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceWarningEnabled(BaseMapElement::SuSector,drawSuSector->checkState() == Qt::Checked ? true : false);

  m_fillingDlg->slot_save();
  m_warningsDlg->slot_save();
}


/* Called to ask is confirmation on the close is needed. */
void SettingsPageAirspace::slot_query_close(bool& warn, QStringList& warnings)
{
  /*forward request to filling page */
  m_fillingDlg->slot_query_close(warn, warnings);
  m_warningsDlg->slot_query_close(warn, warnings);
}

void SettingsPageAirspace::enabledToggled(bool enabled)
{
  spinForceMargin->setEnabled(enabled);
}

/******************************************************************************/
/*            Filling page                                                    */
/******************************************************************************/

SettingsPageAirspaceFilling::SettingsPageAirspaceFilling(QWidget *parent) :
  QDialog(parent)
{
  setObjectName("SettingsPageAirspaceWarnings");
  setModal(true);
  setSizeGripEnabled(true);
  setWindowTitle(tr("Airspace filling settings"));

  QVBoxLayout * topLayout = new QVBoxLayout(this);

  m_enableFilling = new QCheckBox(tr("Enable airspace filling"), this);
  m_enableFilling->setToolTip(tr("Switch on/off Airspace filling"));

  connect(m_enableFilling, SIGNAL(toggled(bool)), SLOT(enabledToggled(bool)));
  topLayout->addWidget(m_enableFilling);

  m_separations = new QWidget(this);
  topLayout->addWidget(m_separations);

  QGridLayout * mVGroupLayout = new QGridLayout(m_separations);
  int row=0;
  mVGroupLayout->addItem(new QSpacerItem(0, 8), 0, 0);
  row++;

  //header
  QLabel* lbl;
  lbl = new QLabel(tr("Vertical Dist."), m_separations);
  mVGroupLayout->addWidget(lbl,row,1,1,2);
//  mVGroupLayout->addMultiCellWidget(lbl, row, row, 1, 2);
  lbl = new QLabel(tr("Lateral Dist."), m_separations);
  mVGroupLayout->addWidget(lbl,row,3,1,2);
//  mVGroupLayout->addMultiCellWidget(lbl, row, row, 3, 4);
  row++;

  lbl = new QLabel(tr("Distance"),m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  lbl = new QLabel(tr("Filling"), m_separations);
  mVGroupLayout->addWidget(lbl,row,1,1,2);
//  mVGroupLayout->addMultiCellWidget(lbl, row, row, 1, 2);
  lbl = new QLabel(tr("Filling"), m_separations);
  mVGroupLayout->addWidget(lbl,row,3,1,2);
//  mVGroupLayout->addMultiCellWidget(lbl, row, row, 3, 4);
  row++;

  //row 1
  lbl = new QLabel(tr("Not near"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  m_verticalNotNear = new QSpinBox(m_separations);
  m_verticalNotNear->setMaximum(100);
  m_verticalNotNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_verticalNotNear, row, 1);
  //m_verticalNotNear->setEnabled(false);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  m_lateralNotNear = new QSpinBox(m_separations);
  m_lateralNotNear->setMaximum(100);
  m_lateralNotNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_lateralNotNear, row, 3);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  //row 2
  lbl = new QLabel(tr("Near"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  m_verticalNear = new QSpinBox(m_separations);
  m_verticalNear->setMaximum(100);
  m_verticalNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_verticalNear, row, 1);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  m_lateralNear = new QSpinBox(m_separations);
  m_lateralNear->setMaximum(100);
  m_lateralNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_lateralNear, row, 3);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  //row 3
  lbl = new QLabel(tr("Very near"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  m_verticalVeryNear = new QSpinBox(m_separations);
  m_verticalVeryNear->setMaximum(100);
  m_verticalVeryNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_verticalVeryNear, row, 1);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  m_lateralVeryNear = new QSpinBox(m_separations);
  m_lateralVeryNear->setMaximum(100);
  m_lateralVeryNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_lateralVeryNear, row, 3);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  //row 4
  lbl = new QLabel(tr("Inside"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  m_verticalInside = new QSpinBox(m_separations);
  m_verticalInside->setMaximum(100);
  m_verticalInside->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_verticalInside, row, 1);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  m_lateralInside = new QSpinBox(m_separations);
  m_lateralInside->setMaximum(100);
  m_lateralInside->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_lateralInside, row, 3);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  topLayout->addStretch(5);

  QPushButton* reset   = new QPushButton(tr("Reset"));
  QPushButton* defaults = new QPushButton(tr("Default"));

  QDialogButtonBox* buttonBox = new QDialogButtonBox( Qt::Horizontal );

  buttonBox->addButton( reset, QDialogButtonBox::ActionRole );
  buttonBox->addButton( defaults, QDialogButtonBox::ActionRole );
  buttonBox->addButton( QDialogButtonBox::Ok );
  buttonBox->addButton( QDialogButtonBox::Cancel );

  topLayout->addWidget( buttonBox );

  connect(reset,    SIGNAL(clicked()), this, SLOT(slot_reset()));
  connect(defaults, SIGNAL(clicked()), this, SLOT(slot_defaults()));

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


SettingsPageAirspaceFilling::~SettingsPageAirspaceFilling()
{}


void SettingsPageAirspaceFilling::slot_load()
{
  GeneralConfig * conf = GeneralConfig::instance();
  bool enabled = conf->getAirspaceFillingEnabled();

  m_enableFilling->setChecked(enabled);
  enabledToggled(enabled);

  m_verticalNotNear  ->setValue(conf->getAirspaceFillingVertical(Airspace::none));
  m_verticalNear     ->setValue(conf->getAirspaceFillingVertical(Airspace::near));
  m_verticalVeryNear ->setValue(conf->getAirspaceFillingVertical(Airspace::veryNear));
  m_verticalInside   ->setValue(conf->getAirspaceFillingVertical(Airspace::inside));
  // qDebug("filling vert inside: %d", conf->getAirspaceFillingVertical(Airspace::inside));
  m_lateralNotNear     ->setValue(conf->getAirspaceFillingLateral(Airspace::none));
  m_lateralNear        ->setValue(conf->getAirspaceFillingLateral(Airspace::near));
  m_lateralVeryNear    ->setValue(conf->getAirspaceFillingLateral(Airspace::veryNear));
  m_lateralInside      ->setValue(conf->getAirspaceFillingLateral(Airspace::inside));
}

/**
 * Called to set all spinboxes to the default value
 */
void SettingsPageAirspaceFilling::slot_defaults()
{
  if( ! m_enableFilling->isChecked() )
    {
      // spinboxes are insensitive, do nothing
      return;
    }

  m_verticalNotNear->setValue(AS_FILL_NOT_NEAR);
  m_verticalNear->setValue(AS_FILL_NEAR);
  m_verticalVeryNear->setValue(AS_FILL_VERY_NEAR);
  m_verticalInside->setValue(AS_FILL_INSIDE);

  m_lateralNotNear->setValue(AS_FILL_NOT_NEAR);
  m_lateralNear->setValue(AS_FILL_NEAR);
  m_lateralVeryNear->setValue(AS_FILL_VERY_NEAR);
  m_lateralInside->setValue(AS_FILL_INSIDE);
}


/**
 * Called to reset all spinboxes to zero
 */
void SettingsPageAirspaceFilling::slot_reset()
{
  if( ! m_enableFilling->isChecked() )
    {
      // spinboxes are insensitive, do nothing
      return;
    }

  m_verticalNotNear->setValue(0);
  m_verticalNear->setValue(0);
  m_verticalVeryNear->setValue(0);
  m_verticalInside->setValue(0);

  m_lateralNotNear->setValue(0);
  m_lateralNear->setValue(0);
  m_lateralVeryNear->setValue(0);
  m_lateralInside->setValue(0);
}

void SettingsPageAirspaceFilling::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();

  conf->setAirspaceFillingEnabled(m_enableFilling->isChecked());

  conf->setAirspaceFillingVertical(Airspace::none,     m_verticalNotNear->value());
  conf->setAirspaceFillingVertical(Airspace::near,     m_verticalNear->value());
  conf->setAirspaceFillingVertical(Airspace::veryNear, m_verticalVeryNear->value());
  conf->setAirspaceFillingVertical(Airspace::inside,   m_verticalInside->value());
  conf->setAirspaceFillingLateral(Airspace::none,      m_lateralNotNear->value());
  conf->setAirspaceFillingLateral(Airspace::near,      m_lateralNear->value());
  conf->setAirspaceFillingLateral(Airspace::veryNear,  m_lateralVeryNear->value());
  conf->setAirspaceFillingLateral(Airspace::inside,    m_lateralInside->value());
}


void SettingsPageAirspaceFilling::slot_query_close(bool& warn, QStringList& warnings)
{
  /*set warn to 'true' if the data has changed. Note that we can NOT
    just set warn equal to _changed, because that way we might erase
    a warning flag set by another page! */
  GeneralConfig * conf = GeneralConfig::instance();
  bool changed=false;

  changed |= conf->getAirspaceFillingEnabled() != m_enableFilling->isChecked();
  changed |= conf->getAirspaceFillingVertical(Airspace::none)
             != m_verticalNotNear->value();
  changed |= conf->getAirspaceFillingVertical(Airspace::near)
             != m_verticalNear->value();
  changed |= conf->getAirspaceFillingVertical(Airspace::veryNear)
             != m_verticalVeryNear->value();
  changed |= conf->getAirspaceFillingVertical(Airspace::inside)
             != m_verticalInside->value();
  changed |= conf->getAirspaceFillingLateral(Airspace::none)
             != m_lateralNotNear->value();
  changed |= conf->getAirspaceFillingLateral(Airspace::near)
             != m_lateralNear->value();
  changed |= conf->getAirspaceFillingLateral(Airspace::veryNear)
             != m_lateralVeryNear->value();
  changed |= conf->getAirspaceFillingLateral(Airspace::inside)
             != m_lateralInside->value();

  if (changed)
    {
      warn=true;
      warnings.append(tr("Airspace filling settings"));
    }
}


void SettingsPageAirspaceFilling::reject()
{
  slot_load();
  QDialog::reject(); 
}


void SettingsPageAirspaceFilling::enabledToggled(bool enabled)
{
  m_separations->setEnabled(enabled);
}


/******************************************************************************/
/*            Airspace Warning page                                           */
/******************************************************************************/

SettingsPageAirspaceWarnings::SettingsPageAirspaceWarnings(QWidget *parent) :
  QDialog(parent, Qt::WindowStaysOnTopHint)
{
  setObjectName("SettingsPageAirspaceWarnings");
  setModal(true);
  setSizeGripEnabled(true);
  setWindowTitle(tr("Airspace warning settings"));

  // save current altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.

  altUnit = Altitude::getUnit();
  QString unit = (altUnit == Altitude::meters) ? " m" : " ft";

  QVBoxLayout *topLayout = new QVBoxLayout(this);

  enableWarning = new QCheckBox(tr("Enable Airspace Warning"), this);
  enableWarning->setObjectName("EnableWarnings");
  enableWarning->setChecked(true);
  enableWarning->setToolTip(tr("Switch on/off Airspace Warnings"));

  connect( enableWarning, SIGNAL(toggled(bool)), SLOT(enabledToggled(bool)));
  topLayout->addWidget( enableWarning );

  separations = new QWidget(this);
  topLayout->addWidget(separations);

  QGridLayout* mVGroupLayout = new QGridLayout(separations);
  int row=0;
  mVGroupLayout->setRowMinimumHeight ( row, 8 );
  row++;

  //header
  QLabel* lbl;

  // row 0
  lbl = new QLabel(tr("Distance"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  lbl = new QLabel(tr("Lateral"), separations);
  mVGroupLayout->addWidget(lbl, row, 1 );
  lbl = new QLabel(tr("Above"), separations);
  mVGroupLayout->addWidget(lbl, row, 2 );
  lbl = new QLabel(tr("Below"), separations);
  mVGroupLayout->addWidget(lbl, row, 3 );
  row++;

  //row 1
  lbl = new QLabel(tr("Near"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  
  horiWarnDist = new QSpinBox(separations);
  horiWarnDist->setMaximum(99999);
  horiWarnDist->setButtonSymbols(QSpinBox::PlusMinus);
  horiWarnDist->setSuffix( unit );
  mVGroupLayout->addWidget(horiWarnDist, row, 1);

  aboveWarnDist = new QSpinBox(separations);
  aboveWarnDist->setMaximum(99999);
  aboveWarnDist->setButtonSymbols(QSpinBox::PlusMinus);
  aboveWarnDist->setSuffix( unit );
  mVGroupLayout->addWidget(aboveWarnDist, row, 2);

  belowWarnDist = new QSpinBox(separations);
  belowWarnDist->setMaximum(99999);
  belowWarnDist->setButtonSymbols(QSpinBox::PlusMinus);
  belowWarnDist->setSuffix( unit );
  mVGroupLayout->addWidget(belowWarnDist, row, 3);
  row++;
  
  // row 2
  lbl = new QLabel(tr("Very Near"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);

  horiWarnDistVN = new QSpinBox(separations);
  horiWarnDistVN->setMaximum(99999);
  horiWarnDistVN->setButtonSymbols(QSpinBox::PlusMinus);
  horiWarnDistVN->setSuffix( unit );
  mVGroupLayout->addWidget(horiWarnDistVN, row, 1);

  aboveWarnDistVN = new QSpinBox(separations);
  aboveWarnDistVN->setMaximum(99999);
  aboveWarnDistVN->setButtonSymbols(QSpinBox::PlusMinus);
  aboveWarnDistVN->setSuffix( unit );
  mVGroupLayout->addWidget(aboveWarnDistVN, row, 2);

  belowWarnDistVN = new QSpinBox(separations);
  belowWarnDistVN->setMaximum(99999);
  belowWarnDistVN->setButtonSymbols(QSpinBox::PlusMinus);
  belowWarnDistVN->setSuffix( unit );
  mVGroupLayout->addWidget(belowWarnDistVN, row, 3);
  row++;

  topLayout->addStretch(10);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                 | QDialogButtonBox::Cancel);

  topLayout->addWidget( buttonBox );

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


SettingsPageAirspaceWarnings::~SettingsPageAirspaceWarnings()
{}


void SettingsPageAirspaceWarnings::slot_load()
{
  GeneralConfig * conf = GeneralConfig::instance();
  AirspaceWarningDistance awd=conf->getAirspaceWarningDistances();
  bool enabled = conf->getAirspaceWarningEnabled();

  enableWarning->setChecked(enabled);
  enabledToggled(enabled);

  if( altUnit == Altitude::meters )
    { // user wants meters
      horiWarnDist->setValue((int) rint(awd.horClose.getMeters()));
      horiWarnDistVN->setValue((int) rint(awd.horVeryClose.getMeters()));
      aboveWarnDist->setValue((int) rint(awd.verAboveClose.getMeters()));
      aboveWarnDistVN->setValue((int) rint(awd.verAboveVeryClose.getMeters()));
      belowWarnDist->setValue((int) rint(awd.verBelowClose.getMeters()));
      belowWarnDistVN->setValue((int) rint(awd.verBelowVeryClose.getMeters()));
    }
  else
    { // user gets feet
      horiWarnDist->setValue((int) rint(awd.horClose.getFeet()));
      horiWarnDistVN->setValue((int) rint(awd.horVeryClose.getFeet()));
      aboveWarnDist->setValue((int) rint(awd.verAboveClose.getFeet()));
      aboveWarnDistVN->setValue((int) rint(awd.verAboveVeryClose.getFeet()));
      belowWarnDist->setValue((int) rint(awd.verBelowClose.getFeet()));
      belowWarnDistVN->setValue((int) rint(awd.verBelowVeryClose.getFeet()));
    }

  // save loaded values for chamge control
  horiWarnDistValue = horiWarnDist->value();
  horiWarnDistVNValue = horiWarnDistVN->value();
  aboveWarnDistValue = aboveWarnDist->value();
  aboveWarnDistVNValue = aboveWarnDistVN->value();
  belowWarnDistValue = belowWarnDist->value();
  belowWarnDistVNValue = belowWarnDistVN->value();

}

void SettingsPageAirspaceWarnings::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();
  AirspaceWarningDistance awd;

  conf->setAirspaceWarningEnabled(enableWarning->isChecked());

  // @AP: Store warning distances always as meters
  if( altUnit == Altitude::meters )
    {
      awd.horClose.setMeters( horiWarnDist->value() );
      awd.horVeryClose.setMeters( horiWarnDistVN->value() );
      awd.verAboveClose.setMeters( aboveWarnDist->value() );
      awd.verAboveVeryClose.setMeters( aboveWarnDistVN->value() );
      awd.verBelowClose.setMeters( belowWarnDist->value() );
      awd.verBelowVeryClose.setMeters( belowWarnDistVN->value() );
    }
  else
    {
      awd.horClose.setFeet( horiWarnDist->value() );
      awd.horVeryClose.setFeet( horiWarnDistVN->value() );
      awd.verAboveClose.setFeet( aboveWarnDist->value() );
      awd.verAboveVeryClose.setFeet( aboveWarnDistVN->value() );
      awd.verBelowClose.setFeet( belowWarnDist->value() );
      awd.verBelowVeryClose.setFeet( belowWarnDistVN->value() );
    }

  conf->setAirspaceWarningDistances(awd);
}


void SettingsPageAirspaceWarnings::slot_query_close( bool& warn, QStringList& warnings )
{
  /* set warn to 'true' if the data has changed. Note that we can NOT
     just set warn equal to _changed, because that way we might erase
     a warning flag set by another page! */
  GeneralConfig * conf = GeneralConfig::instance();
  bool changed=false;

  changed |= conf->getAirspaceWarningEnabled() != enableWarning->isChecked();
  changed |=  horiWarnDistValue != horiWarnDist->value();
  changed |= horiWarnDistVNValue != horiWarnDistVN->value();
  changed |= aboveWarnDistValue != aboveWarnDist->value();
  changed |= aboveWarnDistVNValue != aboveWarnDistVN->value();
  changed |= belowWarnDistValue != belowWarnDist->value();
  changed |= belowWarnDistVNValue != belowWarnDistVN->value();

  if (changed)
    {
      warn=true;
      warnings.append(tr("Airspace warning settings"));
    }
}

void SettingsPageAirspaceWarnings::reject()
{
  slot_load();
  QDialog::reject(); 
}

void SettingsPageAirspaceWarnings::enabledToggled( bool enabled )
{
  separations->setEnabled( enabled );
}
