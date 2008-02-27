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

  QGridLayout *topLayout = new QGridLayout(this, 3, 5, 3);
  int row=0;

  lvLoadOptions = new Q3ListView(this, "LoadOptionList");
  lvLoadOptions->addColumn(tr("Enable Airspace Drawing"),192);
  lvLoadOptions->setAllColumnsShowFocus(true);
  topLayout->addMultiCellWidget(lvLoadOptions, row, row, 0, 4);
  row++;

  enableForceDrawing=new QCheckBox(tr("Force drawing for airspace less than"),
                                   this, "forcedraw");
  enableForceDrawing->setChecked(true);
  topLayout->addMultiCellWidget( enableForceDrawing, row, row, 0, 4 );
  connect( enableForceDrawing, SIGNAL(toggled(bool)), SLOT(enabledToggled(bool)));
  row++;

  spinForceMargin = new QSpinBox(0, 99999, 1, this);
  spinForceMargin->setButtonSymbols(QSpinBox::PlusMinus);

  topLayout->addMultiCellWidget( spinForceMargin, row, row, 0, 0 );
  topLayout->addMultiCellWidget(new QLabel(tr("%1 above me.").arg(unit), this), row, row, 1, 4);
  row++;

  cmdWarning = new QPushButton(tr("Airspace Warnings"), this, "warningOptions");
  topLayout->addMultiCellWidget(cmdWarning, row, row, 0, 1, Qt::AlignLeft);
  connect (cmdWarning, SIGNAL(clicked()), m_warningsDlg, SLOT(show()));

  cmdFilling = new QPushButton(tr("Airspace filling"), this, "fillOptions");
  topLayout->addMultiCellWidget(cmdFilling, row, row, 3, 4, Qt::AlignRight);
  connect (cmdFilling, SIGNAL(clicked()), m_fillingDlg, SLOT(show()));
  row++;

  drawAirspaceA=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::AirA), Q3CheckListItem::CheckBox);
  drawAirspaceB=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::AirB), Q3CheckListItem::CheckBox);
  drawAirspaceC=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::AirC), Q3CheckListItem::CheckBox);
  drawControlC=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::ControlC), Q3CheckListItem::CheckBox);
  drawAirspaceD=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::AirD), Q3CheckListItem::CheckBox);
  drawControlD=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::ControlD), Q3CheckListItem::CheckBox);
  drawAirspaceElow=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::AirElow), Q3CheckListItem::CheckBox);
  drawAirspaceEhigh=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::AirEhigh), Q3CheckListItem::CheckBox);
  drawAirspaceF=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::AirF), Q3CheckListItem::CheckBox);
  drawRestricted=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::Restricted), Q3CheckListItem::CheckBox);
  drawDanger=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::Danger), Q3CheckListItem::CheckBox);
  drawTMZ=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::Tmz), Q3CheckListItem::CheckBox);
  drawLowFlight=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::LowFlight), Q3CheckListItem::CheckBox);
  drawSuSector=new Q3CheckListItem(lvLoadOptions, Airspace::getTypeName(BaseMapElement::SuSector), Q3CheckListItem::CheckBox);
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

  drawAirspaceA->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::AirA));
  drawAirspaceB->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::AirB));
  drawAirspaceC->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::AirC));
  drawControlC->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::ControlC));
  drawAirspaceD->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::AirD));
  drawControlD->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::ControlD));
  drawAirspaceElow->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::AirElow));
  drawAirspaceEhigh->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::AirEhigh));
  drawAirspaceF->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::AirF));
  drawRestricted->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::Restricted));
  drawDanger->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::Danger));
  drawTMZ->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::Tmz));
  drawLowFlight->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::LowFlight));
  drawSuSector->setOn (conf->getAirspaceWarningEnabled(BaseMapElement::SuSector));

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
  conf->setForceAirspaceDrawingEnabled(this->enableForceDrawing->isOn());

  conf->setAirspaceWarningEnabled(BaseMapElement::AirA,drawAirspaceA->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::AirB,drawAirspaceB->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::AirC,drawAirspaceC->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::ControlC,drawControlC->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::AirD,drawAirspaceD->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::ControlD,drawControlD->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::AirElow,drawAirspaceElow->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::AirEhigh,drawAirspaceEhigh->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::AirF,drawAirspaceF->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::Restricted,drawRestricted->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::Danger,drawDanger->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::Tmz,drawTMZ->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::LowFlight,drawLowFlight->isOn());
  conf->setAirspaceWarningEnabled(BaseMapElement::SuSector,drawSuSector->isOn());

  m_fillingDlg->slot_save();
  m_warningsDlg->slot_save();

  conf->save();
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
  QDialog(parent, Qt::WStyle_StaysOnTop)
{
  setWindowTitle(tr("Airspace filling settings"));
  setSizeGripEnabled(false);

  QVBoxLayout * topLayout = new QVBoxLayout(this);

  m_enableFilling = new QCheckBox(tr("Enable airspace filling"),
                                  this, "enable_airspace_filling");

  connect(m_enableFilling, SIGNAL(toggled(bool)), SLOT(enabledToggled(bool)));
  topLayout->addWidget(m_enableFilling);

  m_separations = new QWidget(this);
  topLayout->addWidget(m_separations);

  QGridLayout * mVGroupLayout = new QGridLayout(m_separations);
  int row=0;
  mVGroupLayout->addRowSpacing(0,8);
  row++;

  //header
  QLabel* lbl;
  lbl = new QLabel(tr("Vertical Dist."), m_separations);
  mVGroupLayout->addMultiCellWidget(lbl, row, row, 1, 2);
  lbl = new QLabel(tr("Total Dist."), m_separations);
  mVGroupLayout->addMultiCellWidget(lbl, row, row, 3, 4);
  row++;

  lbl = new QLabel(tr("Distance"),m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  lbl = new QLabel(tr("Filling"), m_separations);
  mVGroupLayout->addMultiCellWidget(lbl, row, row, 1, 2);
  lbl = new QLabel(tr("Filling"), m_separations);
  mVGroupLayout->addMultiCellWidget(lbl, row, row, 3, 4);
  row++;

  //row 1
  lbl = new QLabel(tr("Not near"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  m_verticalNotNear = new QSpinBox(0, 100, 1, m_separations);
  m_verticalNotNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_verticalNotNear, row, 1);
  //m_verticalNotNear->setEnabled(false);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  m_totalNotNear = new QSpinBox(0, 100, 1, m_separations);
  m_totalNotNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_totalNotNear, row, 3);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;
  //connect(m_totalNotNear, SIGNAL(valueChanged(int)),
  //        m_verticalNotNear, SLOT(setValue(int)));

  //row 2
  lbl = new QLabel(tr("Near"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  m_verticalNear = new QSpinBox(0, 100, 1, m_separations);
  m_verticalNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_verticalNear, row, 1);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  m_totalNear = new QSpinBox(0, 100, 1, m_separations);
  m_totalNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_totalNear, row, 3);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  //row 3
  lbl = new QLabel(tr("Very near"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  m_verticalVeryNear = new QSpinBox(0, 100, 1, m_separations);
  m_verticalVeryNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_verticalVeryNear, row, 1);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  m_totalVeryNear = new QSpinBox(0, 100, 1, m_separations);
  m_totalVeryNear->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_totalVeryNear, row, 3);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  //row 4
  lbl = new QLabel(tr("Inside"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  m_verticalInside = new QSpinBox(0, 100, 1, m_separations);
  m_verticalInside->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_verticalInside, row, 1);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  m_totalInside = new QSpinBox(0, 100, 1, m_separations);
  m_totalInside->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(m_totalInside, row, 3);
  lbl = new QLabel(tr("%"), m_separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  topLayout->addStretch(5);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                 | QDialogButtonBox::Cancel);

  topLayout->addWidget( buttonBox );

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
  m_totalNotNear     ->setValue(conf->getAirspaceFillingTotal(Airspace::none));
  m_totalNear        ->setValue(conf->getAirspaceFillingTotal(Airspace::near));
  m_totalVeryNear    ->setValue(conf->getAirspaceFillingTotal(Airspace::veryNear));
  m_totalInside      ->setValue(conf->getAirspaceFillingTotal(Airspace::inside));
}


void SettingsPageAirspaceFilling::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();

  conf->setAirspaceFillingEnabled(m_enableFilling->isChecked());

  conf->setAirspaceFillingVertical(Airspace::none,     m_verticalNotNear->value());
  conf->setAirspaceFillingVertical(Airspace::near,     m_verticalNear->value());
  conf->setAirspaceFillingVertical(Airspace::veryNear, m_verticalVeryNear->value());
  conf->setAirspaceFillingVertical(Airspace::inside,   m_verticalInside->value());
  conf->setAirspaceFillingTotal(Airspace::none,        m_totalNotNear->value());
  conf->setAirspaceFillingTotal(Airspace::near,        m_totalNear->value());
  conf->setAirspaceFillingTotal(Airspace::veryNear,    m_totalVeryNear->value());
  conf->setAirspaceFillingTotal(Airspace::inside,      m_totalInside->value());
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
  changed |= conf->getAirspaceFillingTotal(Airspace::none)
             != m_totalNotNear->value();
  changed |= conf->getAirspaceFillingTotal(Airspace::near)
             != m_totalNear->value();
  changed |= conf->getAirspaceFillingTotal(Airspace::veryNear)
             != m_totalVeryNear->value();
  changed |= conf->getAirspaceFillingTotal(Airspace::inside)
             != m_totalInside->value();

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
  QDialog(parent, Qt::WStyle_StaysOnTop)
{
  setObjectName("SettingsPageAirspaceWarnings");
  setModal(true);
  setWindowTitle(tr("Airspace warning settings"));

  // save current altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.

  altUnit = Altitude::getUnit();
  QString unit = (altUnit == Altitude::meters) ? "m" : "ft";

  QVBoxLayout *topLayout = new QVBoxLayout(this);

  enableWarning = new QCheckBox(tr("Enable Airspace Warning"), this, "EnableWarnings");
  enableWarning->setChecked(true);

  connect( enableWarning, SIGNAL(toggled(bool)), SLOT(enabledToggled(bool)));
  topLayout->addWidget( enableWarning );

  separations = new QWidget(this);
  topLayout->addWidget(separations);

  QGridLayout* mVGroupLayout = new QGridLayout(separations);
  int row=0;
  mVGroupLayout->addRowSpacing(0,8);
  row++;

  //header
  QLabel* lbl;

  lbl = new QLabel(tr("Distance"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  lbl = new QLabel(tr("Near"), separations);
  mVGroupLayout->addMultiCellWidget(lbl, row, row, 1, 2);
  lbl = new QLabel(tr("Very Near"), separations);
  mVGroupLayout->addMultiCellWidget(lbl, row, row, 3, 4);
  row++;

  //row 1
  lbl = new QLabel(tr("Horizontal"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  horiWarnDist = new QSpinBox(0, 99999, 1, separations);
  horiWarnDist->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(horiWarnDist, row, 1);
  lbl = new QLabel(unit, separations);
  mVGroupLayout->addWidget(lbl, row, 2);

  horiWarnDistVN = new QSpinBox(0, 99999,1, separations);
  horiWarnDistVN->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(horiWarnDistVN, row, 3);
  lbl = new QLabel(unit, separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  //row 2
  lbl = new QLabel(tr("Above"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  aboveWarnDist = new QSpinBox(0, 99999,1, separations);
  aboveWarnDist->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(aboveWarnDist, row, 1);
  lbl = new QLabel(unit, separations);
  mVGroupLayout->addWidget(lbl, row, 2);

  aboveWarnDistVN = new QSpinBox(0, 99999,1, separations);
  aboveWarnDistVN->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(aboveWarnDistVN, row, 3);
  lbl = new QLabel(unit, separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  //row 3
  lbl = new QLabel(tr("Below"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  belowWarnDist = new QSpinBox(0, 99999,1, separations);
  belowWarnDist->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(belowWarnDist, row, 1);
  lbl = new QLabel(unit, separations);
  mVGroupLayout->addWidget(lbl, row, 2);

  belowWarnDistVN = new QSpinBox(0, 99999,1, separations);
  belowWarnDistVN->setButtonSymbols(QSpinBox::PlusMinus);
  mVGroupLayout->addWidget(belowWarnDistVN, row, 3);
  lbl = new QLabel(unit, separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  topLayout->addStretch(5);

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

  conf->setAirspaceWarningEnabled(enableWarning->isOn());

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
