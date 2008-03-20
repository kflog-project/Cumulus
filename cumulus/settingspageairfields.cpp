/***********************************************************************
**
**   settingspageairfields.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 *
 * contains airfield related data settings
 *
 * @author Axel Pauli, axel@kflog.org
 *
 */

#include <QLabel>
#include <QGridLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>

#include "generalconfig.h"
#include "settingspageairfields.h"

SettingsPageAirfields::SettingsPageAirfields(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("SettingsPageAirfields");

  QVBoxLayout *topLayout = new QVBoxLayout( this );

  QGroupBox* weltGroup = new QGroupBox( tr("Welt2000"), this );
  topLayout->addWidget( weltGroup );

  QGridLayout* weltLayout = new QGridLayout( weltGroup );

  int grow=0;
  QLabel* lbl = new QLabel( tr("Country Filter:"), (weltGroup ) );
  weltLayout->addWidget( lbl, grow, 0 );

  countryFilter = new QLineEdit( weltGroup );
  weltLayout->addWidget( countryFilter, grow, 1 , 1, 2 );
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
  homeRadius = new QSpinBox( weltGroup );
  homeRadius->setRange( 0, 10000 );
  homeRadius->setSingleStep( 10 );
  homeRadius->setButtonSymbols(QSpinBox::PlusMinus);
  weltLayout->addWidget( homeRadius, grow, 1 );
  weltLayout->addWidget( new QLabel( unit, weltGroup), grow, 2 );

  topLayout->addStretch( 10 );

  connect( countryFilter, SIGNAL(textChanged(const QString&)),
           this, SLOT(slot_filterChanged(const QString&)) );
}

SettingsPageAirfields::~SettingsPageAirfields()
{
}

/**
 * Called to initiate loading of the configurationfile
 */
void SettingsPageAirfields::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  countryFilter->setText( conf->getWelt2000CountryFilter() );
  // @AP: radius value is stored without considering unit.
  homeRadius->setValue( conf->getWelt2000HomeRadius() );

  // sets home radius enabled/disabled in dependency to filter string
  slot_filterChanged( countryFilter->text() );
}

/**
 * Called to initiate saving to the configuration file.
 */
void SettingsPageAirfields::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

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
                               tr("Every welt 2000 county sign must consist of two letters! <br>Allowed separators are space and comma.<br>Your modification will not be saved!"),
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
 * Called if the text of the filter has been changed
 */
void SettingsPageAirfields::slot_filterChanged( const QString& text )
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
void SettingsPageAirfields::slot_query_close(bool& warn, QStringList& warnings)
{
  /* set warn to 'true' if the data has changed. Note that we can NOT
    just set warn equal to _changed, because that way we might erase a
    warning flag set by another page! */

  bool changed=false;

  changed = changed || checkIsWelt2000Changed();

  if (changed) {
    warn=true;
    warnings.append(tr("the airfield settings"));
  }
}

/**
 * Checks, if the configuration of the welt 2000 has been changed
 */
bool SettingsPageAirfields::checkIsWelt2000Changed()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed = changed || ( conf->getWelt2000CountryFilter() != countryFilter->text() );
  changed = changed || ( conf->getWelt2000HomeRadius() != homeRadius->value() ) ;

  // qDebug( "SettingsPageAirfields::checkIsWelt2000Changed(): %d", changed );
  return changed;
}


