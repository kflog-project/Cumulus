/***********************************************************************
 **
 **   settingspageairfields.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2008-2009 Axel Pauli
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

  QVBoxLayout *topLayout = new QVBoxLayout(this);

  QGroupBox* weltGroup = new QGroupBox(tr("Welt2000"), this);
  topLayout->addWidget(weltGroup);

  QGridLayout* weltLayout = new QGridLayout(weltGroup);

  int grow = 0;
  QLabel* lbl = new QLabel(tr("Country Filter:"), (weltGroup));
  weltLayout->addWidget(lbl, grow, 0);

  countryFilter = new QLineEdit(weltGroup);
  weltLayout->addWidget(countryFilter, grow, 1, 1, 3);
  grow++;

  // get current distance unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  distUnit = Distance::getUnit();

  const char *unit = "";

  // Input accepts different units
  if (distUnit == Distance::kilometers)
    {
      unit = " km";
    }
  else if (distUnit == Distance::miles)
    {
      unit = " ml";
    }
  else // if( distUnit == Distance::nautmiles )
    {
      unit = " nm";
    }

  lbl = new QLabel(tr("Home Radius:"), (weltGroup));
  weltLayout->addWidget(lbl, grow, 0);
  homeRadius = new QSpinBox(weltGroup);
  homeRadius->setRange(0, 10000);
  homeRadius->setSingleStep(10);
  homeRadius->setButtonSymbols(QSpinBox::PlusMinus);
  homeRadius->setSuffix(unit);
  weltLayout->addWidget(homeRadius, grow, 1 );

  loadOutlandings = new QCheckBox( tr("Load Outlandings"), weltGroup );
  weltLayout->addWidget(loadOutlandings, grow, 2, Qt::AlignRight );

  weltLayout->setColumnStretch(2, 10);

  // JD: adding group box for diverse list display settings

  grow = 0;
  QGroupBox* listGroup = new QGroupBox(tr("List Display"), this);
  topLayout->addWidget(listGroup);

  QGridLayout* listLayout = new QGridLayout(listGroup);

  lbl = new QLabel(tr( "Entries per Page in AF/WP/OL Lists (0 to disable):"), listGroup);
  listLayout->addWidget(lbl, grow, 0);
  pageSize = new QSpinBox(listGroup);
  pageSize->setRange(0, 100);
  pageSize->setSingleStep(5);
  pageSize->setButtonSymbols(QSpinBox::PlusMinus);
  listLayout->addWidget(pageSize, grow, 1);

  grow++;
  lbl = new QLabel(tr( "Entry Height Increase (Pixels) in AF/WP/OL Lists:"), listGroup);
  listLayout->addWidget(lbl, grow, 0);
  afMargin = new QSpinBox(listGroup);
  afMargin->setRange(0, 60);
  afMargin->setSingleStep(2);
  afMargin->setButtonSymbols(QSpinBox::PlusMinus);
  listLayout->addWidget(afMargin, grow, 1);

  grow++;
  lbl = new QLabel(tr( "Entry Height Increase (Pixels) in Emergency List:"), listGroup);
  listLayout->addWidget(lbl, grow, 0);
  rpMargin = new QSpinBox(listGroup);
  rpMargin->setRange(0, 60);
  rpMargin->setSingleStep(2);
  rpMargin->setButtonSymbols(QSpinBox::PlusMinus);
  listLayout->addWidget(rpMargin, grow, 1);

  grow++;
  listLayout->setRowStretch(grow, 10);
  listLayout->setColumnStretch(2, 10);

  connect( countryFilter, SIGNAL(textChanged(const QString&)),
           this, SLOT(slot_filterChanged(const QString&)) );
}

SettingsPageAirfields::~SettingsPageAirfields()
{
}

/**
 * Called to initiate loading of the configuration file
 */
void SettingsPageAirfields::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  countryFilter->setText(conf->getWelt2000CountryFilter());
  // @AP: radius value is stored without considering unit.
  homeRadius->setValue(conf->getWelt2000HomeRadius());

  if( conf->getWelt2000LoadOutlandings() )
    {
      loadOutlandings->setCheckState( Qt::Checked );
    }
  else
    {
      loadOutlandings->setCheckState( Qt::Unchecked );
    }

  pageSize->setValue(conf->getListDisplayPageSize());
  afMargin->setValue(conf->getListDisplayAFMargin());
  rpMargin->setValue(conf->getListDisplayRPMargin());

  // sets home radius enabled/disabled in dependency to filter string
  slot_filterChanged(countryFilter->text());
}

/**
 * Called to initiate saving to the configuration file.
 */
void SettingsPageAirfields::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // We will check, if the country entries of Welt2000 are
  // correct. If not a warning message is displayed and the
  // modifications are discarded.
  QStringList clist = countryFilter->text().split(QRegExp("[, ]"),
                      QString::SkipEmptyParts);

  for (QStringList::Iterator it = clist.begin(); it != clist.end(); ++it)
    {
      QString s = *it;

      if (s.length() != 2 || s.contains(QRegExp("[A-Za-z]")) != 2)
        {
          QMessageBox::warning(
              this,
              tr("Please check entries"),
              tr("Every Welt2000 county sign must consist of two letters!<br>Allowed separators are space and comma.<br>Your modification will not be saved!"),
              QMessageBox::Ok, QMessageBox::NoButton);
          return;
        }
    }

  conf->setWelt2000CountryFilter(countryFilter->text());
  conf->setWelt2000HomeRadius(homeRadius->value());

if( loadOutlandings->checkState() == Qt::Checked )
  {
    conf->setWelt2000LoadOutlandings( true );
  }
else
  {
    conf->setWelt2000LoadOutlandings( false );
  }

  conf->setListDisplayPageSize(pageSize->value());
  conf->setListDisplayAFMargin(afMargin->value());
  conf->setListDisplayRPMargin(rpMargin->value());
}

/**
 * Called if the text of the filter has been changed
 */
void SettingsPageAirfields::slot_filterChanged(const QString& text)
{
if (text.isEmpty())
  {
    // make widget home radius accessible, if filter string is empty
    homeRadius->setEnabled(true);
  }
else
  {
    // make widget home radius not accessible, if filter string is defined
    homeRadius->setEnabled(false);
  }
}

/* Called to ask is confirmation on the close is needed. */
void SettingsPageAirfields::slot_query_close(bool& warn, QStringList& warnings)
{
/* set warn to 'true' if the data has changed. Note that we can NOT
 just set warn equal to _changed, because that way we might erase a
 warning flag set by another page! */

bool changed = false;

changed = changed || checkIsWelt2000Changed();
changed = changed || checkIsListDisplayChanged();

if (changed)
  {
    warn = true;
    warnings.append(tr("the airfield settings"));
  }
}

/**
 * Checks, if the configuration of the Welt2000 has been changed
 */
bool SettingsPageAirfields::checkIsWelt2000Changed()
{
bool changed = false;

GeneralConfig *conf = GeneralConfig::instance();

changed = changed || (conf->getWelt2000CountryFilter() != countryFilter->text());
changed = changed || (conf->getWelt2000HomeRadius() != homeRadius->value());

bool currentState = false;

if( loadOutlandings->checkState() == Qt::Checked )
  {
    currentState = true;
  }

changed = changed || (conf->getWelt2000LoadOutlandings() != currentState);

// qDebug( "SettingsPageAirfields::checkIsWelt2000Changed(): %d", changed );
return changed;
}

/**
 * Checks if the configuration of list display has been changed
 */
bool SettingsPageAirfields::checkIsListDisplayChanged()
{
bool changed = false;
GeneralConfig *conf = GeneralConfig::instance();

changed = changed || (conf->getListDisplayPageSize() != pageSize->value());
changed = changed || (conf->getListDisplayAFMargin() != afMargin->value());
changed = changed || (conf->getListDisplayRPMargin() != rpMargin->value());

// qDebug( "SettingsPageAirfields::checkIsListDisplayChanged(): %d", changed );
return changed;
}
