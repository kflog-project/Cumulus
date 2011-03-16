/***********************************************************************
 **
 **   settingspageairfields.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2008-2010 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief Contains the airfield data settings.
 *
 */

#include <QtGui>

#include "generalconfig.h"
#include "settingspageairfields.h"

#ifdef INTERNET
#include "proxydialog.h"
#include "httpclient.h"
#endif

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

  lbl = new QLabel(tr("Home Radius:"), weltGroup);
  weltLayout->addWidget(lbl, grow, 0);
  homeRadius = new QSpinBox(weltGroup);
  homeRadius->setRange(1, 10000);
  homeRadius->setSingleStep(50);
  homeRadius->setButtonSymbols(QSpinBox::PlusMinus);
  homeRadius->setSuffix( " " + Distance::getUnitText() );
  weltLayout->addWidget(homeRadius, grow, 1 );

  loadOutlandings = new QCheckBox( tr("Load Outlandings"), weltGroup );
  weltLayout->addWidget(loadOutlandings, grow, 2, Qt::AlignRight );
  grow++;

#ifdef INTERNET

  weltLayout->setRowMinimumHeight(grow++, 15);

  editProxy = new QPushButton( tr("Set Proxy"), weltGroup );
  editProxy->setToolTip(tr("Enter Proxy data if needed"));

  connect( editProxy, SIGNAL( clicked()), this, SLOT(slot_editProxy()) );

  weltLayout->addWidget(editProxy, grow, 0);
  proxyDisplay = new QLabel(weltGroup);
  weltLayout->addWidget(proxyDisplay, grow, 1, 1, 3);
  grow++;

  installWelt2000 = new QPushButton( tr("Install Airfields"), weltGroup );
  installWelt2000->setToolTip(tr("Install Welt2000 airfields"));
  weltLayout->addWidget(installWelt2000, grow, 0 );

  connect( installWelt2000, SIGNAL( clicked()), this, SLOT(slot_installWelt2000()) );

  welt2000FileName = new QLineEdit(weltGroup);
  welt2000FileName->setToolTip(tr("Enter Welt2000 filename as to see on the web page"));
  weltLayout->addWidget(welt2000FileName, grow, 1, 1, 3);

#endif

  weltLayout->setColumnStretch(2, 10);

  //----------------------------------------------------------------------------
  // JD: adding group box for diverse list display settings

  grow = 0;
  QGroupBox* listGroup = new QGroupBox(tr("List Display"), this);
  topLayout->addWidget(listGroup);

  QGridLayout* listLayout = new QGridLayout(listGroup);

  lbl = new QLabel(tr( "Increase row height in AF/WP/OL Lists at:"), listGroup);
  listLayout->addWidget(lbl, grow, 0);
  afMargin = new QSpinBox(listGroup);
  afMargin->setRange(0, 60);
  afMargin->setSingleStep(2);
  afMargin->setButtonSymbols(QSpinBox::PlusMinus);
  afMargin->setSuffix( tr(" Pixels") );
  listLayout->addWidget(afMargin, grow, 1);

  grow++;
  lbl = new QLabel(tr( "Increase row height in Emergency List at:"), listGroup);
  listLayout->addWidget(lbl, grow, 0);
  rpMargin = new QSpinBox(listGroup);
  rpMargin->setRange(0, 60);
  rpMargin->setSingleStep(2);
  rpMargin->setButtonSymbols(QSpinBox::PlusMinus);
  rpMargin->setSuffix( tr(" Pixels") );
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
 * Set proxy, if widget is shown. It could be changed in the meantime
 * in another tabulator.
 */
void SettingsPageAirfields::showEvent(QShowEvent *)
{

#ifdef INTERNET
  proxyDisplay->setText( GeneralConfig::instance()->getProxy() );
#endif

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

#ifdef INTERNET

  proxyDisplay->setText( conf->getProxy() );
  welt2000FileName->setText( conf->getWelt2000FileName() );

#endif

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

  conf->setListDisplayAFMargin(afMargin->value());
  conf->setListDisplayRPMargin(rpMargin->value());
}

/**
 * Called if the text of the filter has been changed
 */
void SettingsPageAirfields::slot_filterChanged(const QString& text)
{
  Q_UNUSED( text )
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
      warnings.append(tr("The Airfield settings"));
    }
}

#ifdef INTERNET

/**
 * Called, if install button of Welt2000 is clicked.
 */
void SettingsPageAirfields::slot_installWelt2000()
{
  QString wfn = welt2000FileName->text().trimmed();

  if( wfn.isEmpty() )
    {
      QMessageBox::information ( this,
                                 tr("Welt2000 settings invalid!"),
                                 tr("Please add a valid Welt2000 filename!") );
      return;
    }

  int answer = QMessageBox::question( this, tr("Download Welt2000?"),
      tr("Active Internet connection is needed!") +
      QString("<p>") + tr("Start download now?"),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No );

  if( answer == QMessageBox::No )
    {
      return;
    }

  emit downloadWelt2000( wfn );
}

/**
 * Opens proxy dialog on user request.
 */
void SettingsPageAirfields::slot_editProxy()
{
  ProxyDialog *dialog = new ProxyDialog( this );

  if( dialog->exec() == QDialog::Accepted )
    {
      // update proxy display
      proxyDisplay->setText( GeneralConfig::instance()->getProxy() );
    }
}

#endif

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

  changed = changed || (conf->getListDisplayAFMargin() != afMargin->value());
  changed = changed || (conf->getListDisplayRPMargin() != rpMargin->value());

  // qDebug( "SettingsPageAirfields::checkIsListDisplayChanged(): %d", changed );
  return changed;
}
