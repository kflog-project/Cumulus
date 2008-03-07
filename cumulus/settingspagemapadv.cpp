/***********************************************************************
 **
 **   settingspagemapadv.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Andr� Somers, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QLabel>
#include <QGridLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QDir>

#include "settingspagemapadv.h"
#include "generalconfig.h"

/***********************************************************/
/*  map configuration page                                 */
/***********************************************************/

SettingsPageMapAdv::SettingsPageMapAdv(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("SettingsPageMapAdv");

  currentProjType = ProjectionBase::Unknown;

  QGridLayout *topLayout = new QGridLayout(this);

  int row=0;

  mapSelection = new QPushButton( tr("Maps"), this );
  topLayout->addWidget(mapSelection, row, 0 );

  connect(mapSelection, SIGNAL( clicked()), this, SLOT(slot_openFileDialog()) );

  mapDirectory = new QLineEdit( this );
  topLayout->addWidget(mapDirectory, row++, 1 );

  topLayout->addWidget(new QLabel(tr("Projection:"), this), row, 0 );
  cmbProjection=new QComboBox(this);
  topLayout->addWidget(cmbProjection, row++, 1);
  cmbProjection->addItem(tr("Lambert"));
  cmbProjection->addItem(tr("Plate Carée"));

  connect(cmbProjection, SIGNAL(activated(int)), this, SLOT(slotSelectProjection(int)));

  topLayout->addWidget(new QLabel(tr("1. St. Parallel:"), this), row, 0);
  edtLat1=new LatEdit(this);
  topLayout->addWidget(edtLat1, row++, 1);

  topLayout->addWidget(new QLabel(tr("2. St. Parallel:"), this), row, 0);
  edtLat2=new LatEdit(this);
  topLayout->addWidget(edtLat2, row++, 1);

  topLayout->addWidget(new QLabel(tr("Origin Lon.:"), this), row, 0);
  edtLon=new LongEdit(this);
  topLayout->addWidget(edtLon, row++, 1);

  topLayout->addRowSpacing(row++,10);

  //------------------------------------------------------------------------------

  QGroupBox* weltGroup = new QGroupBox( tr("Welt2000"), this );
  topLayout->addWidget( weltGroup, row, 0, 1, 2 );
  row++;

  QGridLayout* weltLayout = new QGridLayout( weltGroup, 3, 3, 5 );
  int grow=0;
  weltLayout->addRowSpacing( grow, 10 );
  grow++;

  QLabel* lbl = new QLabel( tr("Country Filter:"), (weltGroup ) );
  weltLayout->addWidget( lbl, grow, 0 );
  weltLayout->addColSpacing( grow, 10 );

  countryFilter = new QLineEdit( weltGroup );
  weltLayout->addMultiCellWidget( countryFilter, grow, grow , 1, 2 );
  grow++;

  // get current distance unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  distUnit = Distance::getUnit();

  const char *unit = "";

  // Input accepts different units 
  if( distUnit == Distance::kilometers )
    {
      unit = "km";
    }
  else if( distUnit == Distance::miles )
    {
      unit = "ml";
    }
  else // if( distUnit == Distance::nautmiles )
    {
      unit = "nm";
    }

  lbl = new QLabel( tr("Home Radius:"), (weltGroup ) );
  weltLayout->addWidget( lbl, grow, 0 );
  homeRadius = new QSpinBox( 0, 10000, 10, weltGroup );
  homeRadius->setButtonSymbols(QSpinBox::PlusMinus);
  weltLayout->addWidget( homeRadius, grow, 1 );
  weltLayout->addWidget( new QLabel( unit, weltGroup), grow, 2 );

  grow++;
  weltLayout->addRowSpacing( grow, 5 );

  //------------------------------------------------------------------------------

  topLayout->addRowSpacing(row++,10);

  chkDeleteAfterCompile = new QCheckBox(tr("Delete original maps after compiling"),
                                        this );
  topLayout->addWidget(chkDeleteAfterCompile, row, 0, 1, 2);
  row++;

  chkUnloadUnneeded = new QCheckBox(tr("Immediately unload unneeded maps"), this );
  topLayout->addWidget(chkUnloadUnneeded, row, 0, 1, 2);
  row++;

  connect( countryFilter, SIGNAL(textChanged(const QString&)),
           this, SLOT(slot_filterChanged(const QString&)) );
}


SettingsPageMapAdv::~SettingsPageMapAdv()
{}


void SettingsPageMapAdv::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  mapDirectory->setText( conf->getMapRootDir() );

  chkDeleteAfterCompile->setChecked( conf->getMapDeleteAfterCompile() );
  chkUnloadUnneeded->setChecked( conf->getMapUnload() );

  currentProjType = conf->getMapProjectionType();
  lambertV1 =       conf->getLambertParallel1();
  lambertV2 =       conf->getLambertParallel2();
  lambertOrigin =   conf->getLambertOrign();
  cylinPar =        conf->getCylinderParallel();

  // @AP: Note, that the index of the list starts with 0 but the
  // ProjectionType uses zero for unknown. So we must subtract 1
  // to get the right value.

  int projIndex = currentProjType - 1;
  cmbProjection->setCurrentItem(projIndex);
  slotSelectProjection(projIndex);

  countryFilter->setText( conf->getWelt2000CountryFilter() );
  // @AP: radius value is stored without considering unit.
  homeRadius->setValue( conf->getWelt2000HomeRadius() );

  // sets home radius enabled/disabled in dependency to filter string
  slot_filterChanged( countryFilter->text() );
}


void SettingsPageMapAdv::slot_save()
{
  // @AP: here we must overtake the new user values at first. After that
  // we can store them.
  // Check, if input string values have been changed. If not, no
  // overtake of values to avoid roundings errors. They can appear if
  // the position formats will be changed between DMS <-> DDM vise
  // versa.

  switch(cmbProjection->currentItem())
    {
    case 0:
      if( edtLat1->isInputChanged() )
        lambertV1 = edtLat1->KFLogDegree();
      if( edtLat2->isInputChanged() )
        lambertV2 = edtLat2->KFLogDegree();
      if( edtLon->isInputChanged() )
        lambertOrigin = edtLon->KFLogDegree();
      break;
    case 1:
      if( edtLat1->isInputChanged() )
        cylinPar = edtLat1->KFLogDegree();
      break;
    }

  GeneralConfig *conf = GeneralConfig::instance();

  conf->setMapRootDir( mapDirectory->text() );
  conf->setMapDeleteAfterCompile( chkDeleteAfterCompile->isChecked() );
  conf->setMapUnload( chkUnloadUnneeded->isChecked() );
  conf->setMapProjectionType( currentProjType );
  conf->setLambertParallel1( lambertV1 );
  conf->setLambertParallel2( lambertV2 );
  conf->setLambertOrign( lambertOrigin );
  conf->setCylinderParallel( cylinPar );

  // We will check, if the country entries of welt 2000 are
  // correct. If not a warning message is displayed and the
  // modifications are discarded.
  QStringList clist = countryFilter->text().split( QRegExp("[, ]"), QString::SkipEmptyParts );

  for( QStringList::Iterator it = clist.begin(); it != clist.end(); ++it )
    {
      QString s = *it;

      if( s.length() != 2 || s.contains( QRegExp("[A-Za-z]") ) != 2 )
        {
          QMessageBox::warning( this, tr("Please check entries"),
                               tr("Every welt 2000 county sign must consist of two letters! <br>Allowed separators are space and comma.<br>Your modification will not saved!"),
                                QMessageBox::Ok, QMessageBox::NoButton );
          return;
        }
    }

  conf->setWelt2000CountryFilter( countryFilter->text() );

  if( homeRadius->isEnabled() )
    {
      conf->setWelt2000HomeRadius( homeRadius->value() );
    }
  else
    {
      conf->setWelt2000HomeRadius( 0 );
    }
}

/**
 * Called if the map selection button is pressed
 */
void SettingsPageMapAdv::slot_openFileDialog()
{
  QString mapDir = QFileDialog::getExistingDirectory( this,
                                                      tr("Please select your map directory"),
                                                      QDir::homePath(),
                                                      QFileDialog::ShowDirsOnly|
                                                      QFileDialog::DontResolveSymlinks );

  if( mapDir.isEmpty() )
    {
      return; // nothing was selected by the user
    }

  mapDirectory->setText( mapDir.remove(  mapDir.size()-1, 1 ) );
}

// selection in the combo box has been changed. index is a reference
// to the current entry. initialize widgets with the internal values
// normally read from the config file.
void SettingsPageMapAdv::slotSelectProjection(int index)
{

  switch(index) {
  case 0:    // Lambert
    edtLat2->setEnabled(true);
    edtLon-> setEnabled(true);
    edtLat1->setKFLogDegree(lambertV1);
    edtLat2->setKFLogDegree(lambertV2);
    edtLon->setKFLogDegree(lambertOrigin);
    currentProjType = ProjectionBase::Lambert;
    break;
  case 1:    // Plate Car??e
  default:   // take this if index is unknown
    edtLat2->setEnabled(false);
    edtLon->setEnabled(false);
    edtLat1->setKFLogDegree(cylinPar);
    edtLat2->setKFLogDegree(0);
    edtLon->setKFLogDegree(0);
    currentProjType = ProjectionBase::Cylindric;
    break;
  }
}

/**
 * Called if the text of the filter has been changed
 */
void SettingsPageMapAdv::slot_filterChanged( const QString& text )
{
  if( text.isEmpty() )
    {
      // make widget home radius accessable, if filter string is empty
      homeRadius->setEnabled(true);
    }
  else
    {
      // make widget home radius not accessable, if filter string is defined
      homeRadius->setEnabled(false);      
    }
}


/* Called to ask is confirmation on the close is needed. */
void SettingsPageMapAdv::slot_query_close(bool& warn, QStringList& warnings)
{
  /* set warn to 'true' if the data has changed. Note that we can NOT
    just set warn equal to _changed, because that way we might erase a
    warning flag set by another page! */

  bool changed=false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed = changed || ( mapDirectory->text() != conf->getMapRootDir() );
  changed = changed || ( chkDeleteAfterCompile->isChecked() != conf->getMapDeleteAfterCompile() );
  changed = changed || ( chkUnloadUnneeded->isChecked() != conf->getMapUnload() );

  changed = changed || checkIsProjectionChanged();
  changed = changed || checkIsWelt2000Changed();

  if (changed) {
    warn=true;
    warnings.append(tr("the advanced map settings"));
  }
}

/**
 * Checks, if the configuration of the projection has been changed
 */
bool SettingsPageMapAdv::checkIsProjectionChanged()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  switch(cmbProjection->currentItem()) {
  case 0:
    changed = changed || ( edtLat1->isInputChanged() );
    changed = changed || ( edtLat2->isInputChanged() );
    changed = changed || ( edtLon->isInputChanged() );
    break;
  case 1:
    changed = changed || ( edtLat1->isInputChanged() );
    break;
  }

  changed = changed || ( conf->getMapProjectionType() != currentProjType );

  qDebug( "SettingsPageMapAdv::()checkIsProjectionChanged: %d", changed );
  return changed;
}

/**
 * Checks, if the configuration of the welt 2000 has been changed
 */
bool SettingsPageMapAdv::checkIsWelt2000Changed()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed = changed || ( conf->getWelt2000CountryFilter() != countryFilter->text() );
  changed = changed || ( conf->getWelt2000HomeRadius() != homeRadius->value() ) ;

  qDebug( "SettingsPageMapAdv::checkIsWelt2000Changed(): %d", changed );
  return changed;
}
