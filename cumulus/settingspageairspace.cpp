/***********************************************************************
 **
 **   settingspageairspace.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Eggert Ehmke, 2009 Axel Pauli
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
#include <QGroupBox>
#include <QSignalMapper>
#include <QHBoxLayout>
#include <QColorDialog>

#include "airspace.h"
#include "basemapelement.h"
#include "distance.h"
#include "generalconfig.h"
#include "settingspageairspace.h"
#include "mapdefaults.h"

SettingsPageAirspace::SettingsPageAirspace(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("SettingsPageAirspace");
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // save current altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.

  altUnit = Altitude::getUnit();
  QString unit = (altUnit == Altitude::meters) ? " m" : " ft";

  m_warningsDlg = new SettingsPageAirspaceWarnings(this);

  m_fillingDlg = new SettingsPageAirspaceFilling(this);

  QGridLayout *topLayout = new QGridLayout(this);
  topLayout->setMargin(3);

  int row=0;

  drawOptions = new QTableWidget(7, 4, this);
  // drawOptions->setShowGrid( false );
  connect( drawOptions, SIGNAL(cellClicked ( int, int )),
           SLOT(slot_toggleCheckBox( int, int )));

  // hide vertical headers
  QHeaderView *vHeader = drawOptions->verticalHeader();
  vHeader->setVisible(false);

  QTableWidgetItem *item = new QTableWidgetItem( tr("Enable Drawing") );
  drawOptions->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr("Border") );
  drawOptions->setHorizontalHeaderItem( 1, item );

  item = new QTableWidgetItem( tr("Enable Drawing") );
  drawOptions->setHorizontalHeaderItem( 2, item );

  item = new QTableWidgetItem( tr("Border") );
  drawOptions->setHorizontalHeaderItem( 3, item );

  topLayout->addWidget(drawOptions, row, 0, 1, 3);
  row++;

  QHBoxLayout *hbox = new QHBoxLayout;

  enableForceDrawing = new QCheckBox(tr("Force airspace drawing <"), this);
  enableForceDrawing->setChecked(true);
  hbox->addWidget( enableForceDrawing );
  connect( enableForceDrawing, SIGNAL(toggled(bool)), SLOT(slot_enabledToggled(bool)));

  spinForceMargin = new QSpinBox(this);
  spinForceMargin-> setRange( 0, 99999 );
  spinForceMargin->setSingleStep( 10 );
  spinForceMargin->setButtonSymbols(QSpinBox::PlusMinus);
  spinForceMargin->setSuffix( unit );

  hbox->addWidget( spinForceMargin );
  hbox->addWidget( new QLabel(tr("above me."), this ));
  hbox->addStretch( 10 );

  cmdColorDefaults = new QPushButton(tr("Color Defaults"), this);
  hbox->addWidget( cmdColorDefaults );
  connect( cmdColorDefaults, SIGNAL(clicked()), this, SLOT(slot_setColorDefaults()) );

  topLayout->addLayout( hbox, row, 0, 1, 3 );
  row++;

  topLayout->setRowMinimumHeight( row++, 20 );

  cmdWarning = new QPushButton(tr("Airspace Warnings"), this);
  topLayout->addWidget(cmdWarning, row, 0, 1, 1, Qt::AlignLeft);
  connect (cmdWarning, SIGNAL(clicked()), m_warningsDlg, SLOT(show()));

  cmdFilling = new QPushButton(tr("Airspace filling"), this);
  topLayout->addWidget(cmdFilling, row, 2, 1, 1, Qt::AlignRight);
  connect (cmdFilling, SIGNAL(clicked()), m_fillingDlg, SLOT(show()));

  row = 0;
  int col = 0;

  drawAirspaceA = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirA) );
  drawAirspaceA->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceA );

  drawAirspaceB = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirB) );
  drawAirspaceB->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceB );

  drawAirspaceC = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirC) );
  drawAirspaceC->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceC );

  drawAirspaceD = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirD) );
  drawAirspaceD->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceD );

  drawAirspaceE = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirE) );
  drawAirspaceE->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceE );

  drawAirspaceF = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::AirF) );
  drawAirspaceF->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawAirspaceF );

  drawWaveWindow = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::WaveWindow) );
  drawWaveWindow->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawWaveWindow );

  // next column is one
  row = 0;
  col = 1;

  borderColorAirspaceA = new QWidget();
  borderColorAirspaceA->setAutoFillBackground(true);
  borderColorAirspaceA->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceA );

  borderColorAirspaceB = new QWidget();
  borderColorAirspaceB->setAutoFillBackground(true);
  borderColorAirspaceB->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceB );

  borderColorAirspaceC = new QWidget();
  borderColorAirspaceC->setAutoFillBackground(true);
  borderColorAirspaceC->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceC );

  borderColorAirspaceD = new QWidget();
  borderColorAirspaceD->setAutoFillBackground(true);
  borderColorAirspaceD->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceD );

  borderColorAirspaceE = new QWidget();
  borderColorAirspaceE->setAutoFillBackground(true);
  borderColorAirspaceE->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceE );

  borderColorAirspaceF = new QWidget();
  borderColorAirspaceF->setAutoFillBackground(true);
  borderColorAirspaceF->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorAirspaceF );

  borderColorWaveWindow = new QWidget();
  borderColorWaveWindow->setAutoFillBackground(true);
  borderColorWaveWindow->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorWaveWindow );

  // next column is two
  row = 0;
  col = 2;

  drawControlC = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::ControlC) );
  drawControlC->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawControlC );

  drawControlD = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::ControlD) );
  drawControlD->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawControlD );

  drawRestricted = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Restricted) );
  drawRestricted->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawRestricted );

  drawDanger = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Danger) );
  drawDanger->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawDanger );

  drawTMZ = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::Tmz) );
  drawTMZ->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawTMZ );

  drawLowFlight = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::LowFlight) );
  drawLowFlight->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawLowFlight );

  drawGliderSector = new QTableWidgetItem( Airspace::getTypeName(BaseMapElement::GliderSector) );
  drawGliderSector->setFlags( Qt::ItemIsEnabled );
  drawOptions->setItem( row++, col, drawGliderSector );

  // next column is three
  row = 0;
  col = 3;

  borderColorControlC = new QWidget();
  borderColorControlC->setAutoFillBackground(true);
  borderColorControlC->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorControlC );

  borderColorControlD = new QWidget();
  borderColorControlD->setAutoFillBackground(true);
  borderColorControlD->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorControlD );

  borderColorRestricted = new QWidget();
  borderColorRestricted->setAutoFillBackground(true);
  borderColorRestricted->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorRestricted );

  borderColorDanger = new QWidget();
  borderColorDanger->setAutoFillBackground(true);
  borderColorDanger->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorDanger );

  borderColorTMZ = new QWidget();
  borderColorTMZ->setAutoFillBackground(true);
  borderColorTMZ->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorTMZ );

  borderColorLowFlight = new QWidget();
  borderColorLowFlight->setAutoFillBackground(true);
  borderColorLowFlight->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorLowFlight );

  borderColorGliderSector = new QWidget();
  borderColorGliderSector->setAutoFillBackground(true);
  borderColorGliderSector->setBackgroundRole(QPalette::Window);
  drawOptions->setCellWidget( row++, col, borderColorGliderSector );

  //qDebug("ViewportSize().width()=%d", drawOptions->maximumViewportSize().width());

  drawOptions->resizeColumnsToContents();
  //drawOptions->setColumnWidth(0, 360);
  //drawOptions->setColumnWidth(1, 360);
}

SettingsPageAirspace::~SettingsPageAirspace()
{}


void SettingsPageAirspace::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();
  bool enabled = conf->getForceAirspaceDrawingEnabled();

  enableForceDrawing->setChecked(enabled);
  slot_enabledToggled(enabled);

  if( altUnit == Altitude::meters )
    { // user wants meters
      spinForceMargin->setValue((int) rint(conf->getForceAirspaceDrawingDistance().getMeters()));
    }
  else
    { // user get feet
      spinForceMargin->setValue((int) rint(conf->getForceAirspaceDrawingDistance().getFeet()));
    }

  // save spinbox value for later change check
  spinForceMarginValue = spinForceMargin->value();

  drawAirspaceA->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::AirA) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceB->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::AirB) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceC->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::AirC) ? Qt::Checked : Qt::Unchecked );
  drawControlC->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::ControlC) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceD->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::AirD) ? Qt::Checked : Qt::Unchecked );
  drawControlD->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::ControlD) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceE->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::AirE) ? Qt::Checked : Qt::Unchecked );
  drawAirspaceF->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::AirF) ? Qt::Checked : Qt::Unchecked );
  drawRestricted->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::Restricted) ? Qt::Checked : Qt::Unchecked );
  drawDanger->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::Danger) ? Qt::Checked : Qt::Unchecked );
  drawTMZ->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::Tmz) ? Qt::Checked : Qt::Unchecked );
  drawLowFlight->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::LowFlight) ? Qt::Checked : Qt::Unchecked );
  drawWaveWindow->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::WaveWindow) ? Qt::Checked : Qt::Unchecked );
  drawGliderSector->setCheckState (conf->getAirspaceDrawingEnabled(BaseMapElement::GliderSector) ? Qt::Checked : Qt::Unchecked );

  borderColorAirspaceA->setPalette( QPalette(conf->getBorderColorAirspaceA()));
  borderColorAirspaceB->setPalette( QPalette(conf->getBorderColorAirspaceB()));
  borderColorAirspaceC->setPalette( QPalette(conf->getBorderColorAirspaceC()));
  borderColorAirspaceD->setPalette( QPalette(conf->getBorderColorAirspaceD()));
  borderColorAirspaceE->setPalette( QPalette(conf->getBorderColorAirspaceE()));
  borderColorAirspaceF->setPalette( QPalette(conf->getBorderColorAirspaceF()));
  borderColorWaveWindow->setPalette( QPalette(conf->getBorderColorWaveWindow()));
  borderColorControlC->setPalette( QPalette(conf->getBorderColorControlC()));
  borderColorControlD->setPalette( QPalette(conf->getBorderColorControlD()));
  borderColorRestricted->setPalette( QPalette(conf->getBorderColorRestricted()));
  borderColorDanger->setPalette( QPalette(conf->getBorderColorDanger()));
  borderColorTMZ->setPalette( QPalette(conf->getBorderColorTMZ()));
  borderColorLowFlight->setPalette( QPalette(conf->getBorderColorLowFlight()));
  borderColorGliderSector->setPalette( QPalette(conf->getBorderColorGliderSector()));

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

  conf->setAirspaceDrawingEnabled(BaseMapElement::AirA,drawAirspaceA->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::AirB,drawAirspaceB->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::AirC,drawAirspaceC->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::ControlC,drawControlC->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::AirD,drawAirspaceD->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::ControlD,drawControlD->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::AirE,drawAirspaceE->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::AirF,drawAirspaceF->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::Restricted,drawRestricted->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::Danger,drawDanger->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::Tmz,drawTMZ->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::LowFlight,drawLowFlight->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::WaveWindow,drawWaveWindow->checkState() == Qt::Checked ? true : false);
  conf->setAirspaceDrawingEnabled(BaseMapElement::GliderSector,drawGliderSector->checkState() == Qt::Checked ? true : false);

  conf->setBorderColorAirspaceA(borderColorAirspaceA->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceB(borderColorAirspaceB->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceC(borderColorAirspaceC->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceD(borderColorAirspaceD->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceE(borderColorAirspaceE->palette().color(QPalette::Window));
  conf->setBorderColorAirspaceF(borderColorAirspaceF->palette().color(QPalette::Window));
  conf->setBorderColorWaveWindow(borderColorWaveWindow->palette().color(QPalette::Window));
  conf->setBorderColorControlC(borderColorControlC->palette().color(QPalette::Window));
  conf->setBorderColorControlD(borderColorControlD->palette().color(QPalette::Window));
  conf->setBorderColorRestricted(borderColorRestricted->palette().color(QPalette::Window));
  conf->setBorderColorDanger(borderColorDanger->palette().color(QPalette::Window));
  conf->setBorderColorTMZ(borderColorTMZ->palette().color(QPalette::Window));
  conf->setBorderColorLowFlight(borderColorLowFlight->palette().color(QPalette::Window));
  conf->setBorderColorGliderSector(borderColorGliderSector->palette().color(QPalette::Window));

  m_fillingDlg->slot_save();
  m_warningsDlg->slot_save();
}

/**
  * Called to set all colors to their default value.
  */
void SettingsPageAirspace::slot_setColorDefaults()
{
  qDebug("Reset Colors");

  borderColorAirspaceA->setPalette( QPalette(QColor(AIRA_COLOR)) );
  borderColorAirspaceB->setPalette( QPalette(QColor(AIRB_COLOR)) );
  borderColorAirspaceC->setPalette( QPalette(QColor(AIRC_COLOR)) );
  borderColorAirspaceD->setPalette( QPalette(QColor(AIRD_COLOR)) );
  borderColorAirspaceE->setPalette( QPalette(QColor(AIRE_COLOR)) );
  borderColorAirspaceF->setPalette( QPalette(QColor(AIRF_COLOR)) );
  borderColorWaveWindow->setPalette( QPalette(QColor(WAVE_WINDOW_COLOR)) );
  borderColorControlC->setPalette( QPalette(QColor(CTRC_COLOR)) );
  borderColorControlD->setPalette( QPalette(QColor(CTRD_COLOR)) );
  borderColorRestricted->setPalette( QPalette(QColor(RESTRICTED_COLOR)) );
  borderColorDanger->setPalette( QPalette(QColor(DANGER_COLOR)) );
  borderColorTMZ->setPalette( QPalette(QColor(TMZ_COLOR)) );
  borderColorLowFlight->setPalette( QPalette(QColor(LOWF_COLOR)) );
  borderColorGliderSector->setPalette( QPalette(QColor(GLIDER_SECTOR_COLOR)) );
}

/**
 * Called to toggle the check box of the clicked table cell.
 */
void SettingsPageAirspace::slot_toggleCheckBox( int row, int column )
{
  qDebug("row=%d, column=%d", row, column);

  if( column % 2 ) // only even columns are QTableWidgetItems
    {
      QWidget *cw = drawOptions->cellWidget( row, column );
      QPalette palette = cw->palette();
      QColor color = palette.color(QPalette::Window);

      // Open color chooser dialog
      QColor newColor = QColorDialog::getColor( color, this );

      if( newColor.isValid() && color != newColor )
        {
          // set new color in widget
          cw->setPalette( QPalette(newColor));
        }
    }
  else
    {
      QTableWidgetItem *item = drawOptions->item( row, column );
      item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
    }
}

/* Called to ask is confirmation on the close is needed. */
void SettingsPageAirspace::slot_query_close(bool& warn, QStringList& warnings)
{
  GeneralConfig * conf = GeneralConfig::instance();
  bool changed=false;

  changed |= spinForceMarginValue != spinForceMargin->value();
  changed |= conf->getForceAirspaceDrawingEnabled() != enableForceDrawing->isChecked();
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::AirA) != (drawAirspaceA->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::AirB) != (drawAirspaceB->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::AirC) != (drawAirspaceC->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::ControlC) != (drawControlC->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::AirD) != (drawAirspaceD->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::ControlD) != (drawControlD->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::AirE) != (drawAirspaceE->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::AirF) != (drawAirspaceF->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::Restricted) != (drawRestricted->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::Danger) != (drawDanger->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::Tmz) != (drawTMZ->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::LowFlight) != (drawLowFlight->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::WaveWindow) != (drawWaveWindow->checkState() == Qt::Checked ? true : false);
  changed |= conf->getAirspaceDrawingEnabled(BaseMapElement::GliderSector) != (drawGliderSector->checkState() == Qt::Checked ? true : false);

  changed |= conf->getBorderColorAirspaceA() != borderColorAirspaceA->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceB() != borderColorAirspaceB->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceC() != borderColorAirspaceC->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceD() != borderColorAirspaceD->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceE() != borderColorAirspaceE->palette().color(QPalette::Window);
  changed |= conf->getBorderColorAirspaceF() != borderColorAirspaceF->palette().color(QPalette::Window);
  changed |= conf->getBorderColorWaveWindow() != borderColorWaveWindow->palette().color(QPalette::Window);
  changed |= conf->getBorderColorControlC() != borderColorControlC->palette().color(QPalette::Window);
  changed |= conf->getBorderColorControlD() != borderColorControlD->palette().color(QPalette::Window);
  changed |= conf->getBorderColorRestricted() != borderColorRestricted->palette().color(QPalette::Window);
  changed |= conf->getBorderColorDanger() != borderColorDanger->palette().color(QPalette::Window);
  changed |= conf->getBorderColorTMZ() != borderColorTMZ->palette().color(QPalette::Window);
  changed |= conf->getBorderColorLowFlight() != borderColorLowFlight->palette().color(QPalette::Window);
  changed |= conf->getBorderColorGliderSector() != borderColorGliderSector->palette().color(QPalette::Window);

  if (changed)
  {
    warn=true;
    warnings.append(tr("the Airspace drawing settings"));
  }

  /*forward request to filling page */
  m_fillingDlg->slot_query_close(warn, warnings);
  m_warningsDlg->slot_query_close(warn, warnings);
}

void SettingsPageAirspace::slot_enabledToggled(bool enabled)
{
  spinForceMargin->setEnabled(enabled);
}

/******************************************************************************/
/*            Filling page                                                    */
/******************************************************************************/

SettingsPageAirspaceFilling::SettingsPageAirspaceFilling(QWidget *parent) :
  QDialog(parent, Qt::WindowStaysOnTopHint)
{
  setObjectName("SettingsPageAirspaceFilling");
  setModal(true);
  setSizeGripEnabled(true);
  setWindowTitle(tr("Airspace fill settings"));

  QVBoxLayout * topLayout = new QVBoxLayout(this);

  enableFilling = new QCheckBox(tr("Enable airspace filling"), this);
  enableFilling->setToolTip(tr("Switch on/off Airspace filling"));

  connect(enableFilling, SIGNAL(toggled(bool)), SLOT(slot_enabledToggled(bool)));
  topLayout->addWidget(enableFilling);

  // make the step width of the spin boxes configurable in fixed steps
  QGroupBox* stepGroup = new QGroupBox(tr("Spin step width"), this);
  s1  = new QRadioButton(tr("1"), stepGroup);
  s2 = new QRadioButton(tr("5"), stepGroup);
  s3  = new QRadioButton(tr("10"), stepGroup);
  s4  = new QRadioButton(tr("20"), stepGroup);

  s1->setChecked(true);

  QHBoxLayout* radioLayout = new QHBoxLayout(stepGroup);
  radioLayout->addWidget(s1);
  radioLayout->addWidget(s2);
  radioLayout->addWidget(s3);
  radioLayout->addWidget(s4);

  topLayout->addWidget(stepGroup);

  separations = new QWidget(this);
  topLayout->addWidget(separations);

  QGridLayout * mVGroupLayout = new QGridLayout(separations);
  int row=0;
  mVGroupLayout->setRowMinimumHeight ( row, 8 );
  row++;

  // suffix % appended in spin boxes as unit
  QString spinboxSuffix = " %";

  //header
  QLabel* lbl;

  // row 0
  lbl = new QLabel(tr("Distance"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);
  lbl = new QLabel(tr("Not near"), separations);
  mVGroupLayout->addWidget(lbl, row, 1);
  lbl = new QLabel(tr("Near"), separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  lbl = new QLabel(tr("Very near"), separations);
  mVGroupLayout->addWidget(lbl, row, 3);
  lbl = new QLabel(tr("Inside"), separations);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  // row 1
  lbl = new QLabel(tr("Vertical"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);

  verticalNotNear = new QSpinBox(separations);
  verticalNotNear->setRange( 0, 100 );
  verticalNotNear->setButtonSymbols(QSpinBox::PlusMinus);
  verticalNotNear->setSuffix( spinboxSuffix );
  verticalNotNear->setWrapping(true);
  mVGroupLayout->addWidget(verticalNotNear, row, 1);

  verticalNear = new QSpinBox(separations);
  verticalNear->setRange( 0, 100 );
  verticalNear->setButtonSymbols(QSpinBox::PlusMinus);
  verticalNear->setSuffix( spinboxSuffix );
  verticalNear->setWrapping(true);
  mVGroupLayout->addWidget(verticalNear, row, 2);

  verticalVeryNear = new QSpinBox(separations);
  verticalVeryNear->setRange( 0, 100 );
  verticalVeryNear->setButtonSymbols(QSpinBox::PlusMinus);
  verticalVeryNear->setSuffix( spinboxSuffix );
  verticalVeryNear->setWrapping(true);
  mVGroupLayout->addWidget(verticalVeryNear, row, 3);

  verticalInside = new QSpinBox(separations);
  verticalInside->setRange( 0, 100 );
  verticalInside->setButtonSymbols(QSpinBox::PlusMinus);
  verticalInside->setSuffix( spinboxSuffix );
  verticalInside->setWrapping(true);
  mVGroupLayout->addWidget(verticalInside, row, 4);
  row++;

  // row 2
  lbl = new QLabel(tr("Lateral"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);

  lateralNotNear = new QSpinBox(separations);
  lateralNotNear->setRange( 0, 100 );
  lateralNotNear->setButtonSymbols(QSpinBox::PlusMinus);
  lateralNotNear->setSuffix( spinboxSuffix );
  lateralNotNear->setWrapping(true);
  mVGroupLayout->addWidget(lateralNotNear, row, 1);

  lateralNear = new QSpinBox(separations);
  lateralNear->setRange( 0, 100 );
  lateralNear->setButtonSymbols(QSpinBox::PlusMinus);
  lateralNear->setSuffix( spinboxSuffix );
  lateralNear->setWrapping(true);
  mVGroupLayout->addWidget(lateralNear, row, 2);

  lateralVeryNear = new QSpinBox(separations);
  lateralVeryNear->setRange( 0, 100 );
  lateralVeryNear->setButtonSymbols(QSpinBox::PlusMinus);
  lateralVeryNear->setSuffix( spinboxSuffix );
  lateralVeryNear->setWrapping(true);
  mVGroupLayout->addWidget(lateralVeryNear, row, 3);

  lateralInside = new QSpinBox(separations);
  lateralInside->setRange( 0, 100 );
  lateralInside->setButtonSymbols(QSpinBox::PlusMinus);
  lateralInside->setSuffix( spinboxSuffix );
  lateralInside->setWrapping(true);
  mVGroupLayout->addWidget(lateralInside, row, 4);
  row++;

  topLayout->addStretch(10);

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

  QSignalMapper* signalMapper = new QSignalMapper(this);

  connect(s1, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s1, 1);
  connect(s2, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s2, 5);
  connect(s3, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s3, 10);
  connect(s4, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s4, 20);

  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slot_change(int)));
}


SettingsPageAirspaceFilling::~SettingsPageAirspaceFilling()
{}

/**
 * Called to change the step width of the spin boxes.
 */
void SettingsPageAirspaceFilling::slot_change(int newStep)
{
  verticalNotNear->setSingleStep(newStep);
  verticalNear->setSingleStep(newStep);
  verticalVeryNear->setSingleStep(newStep);
  verticalInside->setSingleStep(newStep);

  lateralNotNear->setSingleStep(newStep);
  lateralNear->setSingleStep(newStep);
  lateralVeryNear->setSingleStep(newStep);
  lateralInside->setSingleStep(newStep);
}

void SettingsPageAirspaceFilling::slot_load()
{
  GeneralConfig * conf = GeneralConfig::instance();
  bool enabled = conf->getAirspaceFillingEnabled();

  enableFilling->setChecked(enabled);
  slot_enabledToggled(enabled);

  verticalNotNear->setValue(conf->getAirspaceFillingVertical(Airspace::none));
  verticalNear->setValue(conf->getAirspaceFillingVertical(Airspace::near));
  verticalVeryNear->setValue(conf->getAirspaceFillingVertical(Airspace::veryNear));
  verticalInside->setValue(conf->getAirspaceFillingVertical(Airspace::inside));

  lateralNotNear->setValue(conf->getAirspaceFillingLateral(Airspace::none));
  lateralNear->setValue(conf->getAirspaceFillingLateral(Airspace::near));
  lateralVeryNear->setValue(conf->getAirspaceFillingLateral(Airspace::veryNear));
  lateralInside->setValue(conf->getAirspaceFillingLateral(Airspace::inside));
}

/**
 * Called to set all spinboxes to the default value
 */
void SettingsPageAirspaceFilling::slot_defaults()
{
  if( ! enableFilling->isChecked() )
    {
      // spinboxes are insensitive, do nothing
      return;
    }

  verticalNotNear->setValue(AS_FILL_NOT_NEAR);
  verticalNear->setValue(AS_FILL_NEAR);
  verticalVeryNear->setValue(AS_FILL_VERY_NEAR);
  verticalInside->setValue(AS_FILL_INSIDE);

  lateralNotNear->setValue(AS_FILL_NOT_NEAR);
  lateralNear->setValue(AS_FILL_NEAR);
  lateralVeryNear->setValue(AS_FILL_VERY_NEAR);
  lateralInside->setValue(AS_FILL_INSIDE);
}


/**
 * Called to reset all spinboxes to zero
 */
void SettingsPageAirspaceFilling::slot_reset()
{
  if( ! enableFilling->isChecked() )
    {
      // spinboxes are insensitive, do nothing
      return;
    }

  verticalNotNear->setValue(0);
  verticalNear->setValue(0);
  verticalVeryNear->setValue(0);
  verticalInside->setValue(0);

  lateralNotNear->setValue(0);
  lateralNear->setValue(0);
  lateralVeryNear->setValue(0);
  lateralInside->setValue(0);
}

void SettingsPageAirspaceFilling::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();

  conf->setAirspaceFillingEnabled(enableFilling->isChecked());

  conf->setAirspaceFillingVertical(Airspace::none,     verticalNotNear->value());
  conf->setAirspaceFillingVertical(Airspace::near,     verticalNear->value());
  conf->setAirspaceFillingVertical(Airspace::veryNear, verticalVeryNear->value());
  conf->setAirspaceFillingVertical(Airspace::inside,   verticalInside->value());
  conf->setAirspaceFillingLateral(Airspace::none,      lateralNotNear->value());
  conf->setAirspaceFillingLateral(Airspace::near,      lateralNear->value());
  conf->setAirspaceFillingLateral(Airspace::veryNear,  lateralVeryNear->value());
  conf->setAirspaceFillingLateral(Airspace::inside,    lateralInside->value());
}


void SettingsPageAirspaceFilling::slot_query_close(bool& warn, QStringList& warnings)
{
  /*set warn to 'true' if the data has changed. Note that we can NOT
    just set warn equal to _changed, because that way we might erase
    a warning flag set by another page! */
  GeneralConfig * conf = GeneralConfig::instance();
  bool changed=false;

  changed |= conf->getAirspaceFillingEnabled() != enableFilling->isChecked();
  changed |= conf->getAirspaceFillingVertical(Airspace::none)
             != verticalNotNear->value();
  changed |= conf->getAirspaceFillingVertical(Airspace::near)
             != verticalNear->value();
  changed |= conf->getAirspaceFillingVertical(Airspace::veryNear)
             != verticalVeryNear->value();
  changed |= conf->getAirspaceFillingVertical(Airspace::inside)
             != verticalInside->value();
  changed |= conf->getAirspaceFillingLateral(Airspace::none)
             != lateralNotNear->value();
  changed |= conf->getAirspaceFillingLateral(Airspace::near)
             != lateralNear->value();
  changed |= conf->getAirspaceFillingLateral(Airspace::veryNear)
             != lateralVeryNear->value();
  changed |= conf->getAirspaceFillingLateral(Airspace::inside)
             != lateralInside->value();

  if (changed)
    {
      warn=true;
      warnings.append(tr("the Airspace filling settings"));
    }
}


void SettingsPageAirspaceFilling::reject()
{
  slot_load();
  QDialog::reject();
}


void SettingsPageAirspaceFilling::slot_enabledToggled(bool enabled)
{
  separations->setEnabled(enabled);
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

  connect( enableWarning, SIGNAL(toggled(bool)), SLOT(slot_enabledToggled(bool)));
  topLayout->addWidget( enableWarning );

  // make the step width of the spin boxes configurable in different steps
  QGroupBox* stepGroup = new QGroupBox(tr("Spin step width"), this);
  s1 = new QRadioButton(tr("1"), stepGroup);
  s2 = new QRadioButton(tr("10"), stepGroup);
  s3 = new QRadioButton(tr("100"), stepGroup);
  s4 = new QRadioButton(tr("1000"), stepGroup);

  s1->setChecked(true);

  QHBoxLayout* radioLayout = new QHBoxLayout(stepGroup);
  radioLayout->addWidget(s1);
  radioLayout->addWidget(s2);
  radioLayout->addWidget(s3);
  radioLayout->addWidget(s4);

  topLayout->addWidget(stepGroup);

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
  horiWarnDist->setRange(0, 99999);
  horiWarnDist->setButtonSymbols(QSpinBox::PlusMinus);
  horiWarnDist->setSuffix( unit );
  horiWarnDist->setWrapping(true);

  mVGroupLayout->addWidget(horiWarnDist, row, 1);

  aboveWarnDist = new QSpinBox(separations);
  aboveWarnDist->setRange(0, 99999);
  aboveWarnDist->setButtonSymbols(QSpinBox::PlusMinus);
  aboveWarnDist->setSuffix( unit );
  aboveWarnDist->setWrapping(true);
  mVGroupLayout->addWidget(aboveWarnDist, row, 2);

  belowWarnDist = new QSpinBox(separations);
  belowWarnDist->setRange(0, 99999);
  belowWarnDist->setButtonSymbols(QSpinBox::PlusMinus);
  belowWarnDist->setSuffix( unit );
  belowWarnDist->setWrapping(true);
  mVGroupLayout->addWidget(belowWarnDist, row, 3);
  row++;

  // row 2
  lbl = new QLabel(tr("Very Near"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);

  horiWarnDistVN = new QSpinBox(separations);
  horiWarnDistVN->setRange(0, 99999);
  horiWarnDistVN->setButtonSymbols(QSpinBox::PlusMinus);
  horiWarnDistVN->setSuffix( unit );
  horiWarnDistVN->setWrapping(true);
  mVGroupLayout->addWidget(horiWarnDistVN, row, 1);

  aboveWarnDistVN = new QSpinBox(separations);
  aboveWarnDistVN->setRange(0, 99999);
  aboveWarnDistVN->setButtonSymbols(QSpinBox::PlusMinus);
  aboveWarnDistVN->setSuffix( unit );
  aboveWarnDistVN->setWrapping(true);
  mVGroupLayout->addWidget(aboveWarnDistVN, row, 2);

  belowWarnDistVN = new QSpinBox(separations);
  belowWarnDistVN->setRange(0, 99999);
  belowWarnDistVN->setButtonSymbols(QSpinBox::PlusMinus);
  belowWarnDistVN->setSuffix( unit );
  belowWarnDistVN->setWrapping(true);
  mVGroupLayout->addWidget(belowWarnDistVN, row, 3);
  row++;

  topLayout->addStretch(10);

  QPushButton* defaults = new QPushButton(tr("Default"));

  QDialogButtonBox* buttonBox = new QDialogButtonBox( Qt::Horizontal );
  buttonBox->addButton( defaults, QDialogButtonBox::ActionRole );
  buttonBox->addButton( QDialogButtonBox::Ok );
  buttonBox->addButton( QDialogButtonBox::Cancel );
  topLayout->addWidget( buttonBox );

  connect(defaults, SIGNAL(clicked()), this, SLOT(slot_defaults()));

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QSignalMapper* signalMapper = new QSignalMapper(this);

  connect(s1, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s1, 1);
  connect(s2, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s2, 10);
  connect(s3, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s3, 100);
  connect(s4, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s4, 1000);

  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slot_change(int)));
}


SettingsPageAirspaceWarnings::~SettingsPageAirspaceWarnings()
{}

/**
 * Called to change the step width of the spin boxes.
 */
void SettingsPageAirspaceWarnings::slot_change(int newStep)
{
  horiWarnDist->setSingleStep(newStep);
  horiWarnDistVN->setSingleStep(newStep);

  aboveWarnDist->setSingleStep(newStep);
  aboveWarnDistVN->setSingleStep(newStep);

  belowWarnDist->setSingleStep(newStep);
  belowWarnDistVN->setSingleStep(newStep);
}

void SettingsPageAirspaceWarnings::slot_load()
{
  GeneralConfig * conf = GeneralConfig::instance();
  AirspaceWarningDistance awd=conf->getAirspaceWarningDistances();
  bool enabled = conf->getAirspaceWarningEnabled();

  enableWarning->setChecked(enabled);
  slot_enabledToggled(enabled);

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

/**
 * Called to set all spinboxes to the default value
 */
void SettingsPageAirspaceWarnings::slot_defaults()
{
  if( ! enableWarning->isChecked() )
    {
      // spinboxes are insensitive, do nothing
      return;
    }

  if( altUnit == Altitude::meters )
    { // user wants meters
      horiWarnDist->setValue( 2000 );
      horiWarnDistVN->setValue( 1000 );
      aboveWarnDist->setValue( 200 );
      aboveWarnDistVN->setValue( 100 );
      belowWarnDist->setValue( 200 );
      belowWarnDistVN->setValue( 100 );
    }
  else
    { // user gets feet
      horiWarnDist->setValue( 7000 );
      horiWarnDistVN->setValue( 3500 );
      aboveWarnDist->setValue( 700 );
      aboveWarnDistVN->setValue( 350 );
      belowWarnDist->setValue( 700 );
      belowWarnDistVN->setValue( 350 );
    }
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
      warnings.append(tr("the Airspace warning settings"));
    }
}

void SettingsPageAirspaceWarnings::reject()
{
  slot_load();
  QDialog::reject();
}

void SettingsPageAirspaceWarnings::slot_enabledToggled( bool enabled )
{
  separations->setEnabled( enabled );
}
