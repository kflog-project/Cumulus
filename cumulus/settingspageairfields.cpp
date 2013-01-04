/***********************************************************************
 **
 **   settingspageairfields.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2008-2013 Axel Pauli
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
 * \date 2008-2013
 *
 * \version $Id$
 */

#include <QtGui>

#include "generalconfig.h"
#include "settingspageairfields.h"
#include "varspinbox.h"

#ifdef USE_NUM_PAD
#include "numberEditor.h"
#endif

#ifdef INTERNET
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
  VarSpinBox* hspin;
  QLabel* lbl = new QLabel(tr("Country Filter:"), (weltGroup));
  weltLayout->addWidget(lbl, grow, 0);

  m_countryFilter = new QLineEdit(weltGroup);
  weltLayout->addWidget(m_countryFilter, grow, 1, 1, 3);
  grow++;

  lbl = new QLabel(tr("Home Radius:"), weltGroup);
  weltLayout->addWidget(lbl, grow, 0);

#ifdef USE_NUM_PAD
  m_homeRadius = new NumberEditor( this );
  m_homeRadius->setDecimalVisible( false );
  m_homeRadius->setPmVisible( false );
  m_homeRadius->setMaxLength(4);
  m_homeRadius->setSuffix( " " + Distance::getUnitText() );
  m_homeRadius->setMaximum( 9999 );
  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "([1-9][0-9]{0,3})" ), this );
  m_homeRadius->setValidator( eValidator );
  weltLayout->addWidget(m_homeRadius, grow, 1 );
#else
  m_homeRadius = new QSpinBox;
  m_homeRadius->setRange(1, 10000);
  m_homeRadius->setSingleStep(50);
  m_homeRadius->setSuffix( " " + Distance::getUnitText() );
  hspin = new VarSpinBox(m_homeRadius);
  weltLayout->addWidget(hspin, grow, 1 );
#endif

  m_loadOutlandings = new QCheckBox( tr("Load Outlandings"), weltGroup );
  weltLayout->addWidget(m_loadOutlandings, grow, 2, Qt::AlignRight );
  grow++;

#ifdef INTERNET

  weltLayout->setRowMinimumHeight(grow++, 10);

  m_installWelt2000 = new QPushButton( tr("Install"), weltGroup );
  m_installWelt2000->setToolTip(tr("Install Welt2000 data"));
  weltLayout->addWidget(m_installWelt2000, grow, 0 );

  connect( m_installWelt2000, SIGNAL( clicked()), this, SLOT(slot_installWelt2000()) );

  m_welt2000FileName = new QLineEdit(weltGroup);
  m_welt2000FileName->setToolTip(tr("Enter Welt2000 filename as to see on the web page"));
  weltLayout->addWidget(m_welt2000FileName, grow, 1, 1, 3);

#endif

  weltLayout->setColumnStretch(2, 10);

  //----------------------------------------------------------------------------
  // JD: adding group box for diverse list display settings
  grow = 0;
  QGroupBox* listGroup = new QGroupBox(tr("List Display"), this);
  topLayout->addWidget(listGroup);
  topLayout->addStretch(10);

  QGridLayout* listLayout = new QGridLayout(listGroup);

  lbl = new QLabel(tr( "More space in AF/WP/OL lists:"), listGroup);
  listLayout->addWidget(lbl, grow, 0);
  m_afMargin = new QSpinBox;
  m_afMargin->setRange(0, 30);
  m_afMargin->setSingleStep(1);
  m_afMargin->setSuffix( tr(" Pixels") );
  hspin = new VarSpinBox(m_afMargin);
  listLayout->addWidget(hspin, grow, 1);

  grow++;
  lbl = new QLabel(tr( "More space in Emergency list:"), listGroup);
  listLayout->addWidget(lbl, grow, 0);
  m_rpMargin = new QSpinBox;
  m_rpMargin->setRange(0, 30);
  m_rpMargin->setSingleStep(1);
  m_rpMargin->setSuffix( tr(" Pixels") );
  hspin = new VarSpinBox(m_rpMargin);
  listLayout->addWidget(hspin, grow, 1);

  grow++;
  listLayout->setRowStretch(grow, 10);
  listLayout->setColumnStretch(2, 10);

#ifndef ANDROID
  // Android makes trouble, if word detection is enabled and the input is
  // changed by us.
  connect( m_countryFilter, SIGNAL(textChanged(const QString&)),
           this, SLOT(slot_filterChanged(const QString&)) );
#endif
}

SettingsPageAirfields::~SettingsPageAirfields()
{
}

void SettingsPageAirfields::showEvent(QShowEvent *)
{
}

/**
 * Called to initiate loading of the configuration file
 */
void SettingsPageAirfields::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  m_countryFilter->setText(conf->getWelt2000CountryFilter());
  // @AP: radius value is stored without considering unit.
  m_homeRadius->setValue(conf->getWelt2000HomeRadius());

  if( conf->getWelt2000LoadOutlandings() )
    {
      m_loadOutlandings->setCheckState( Qt::Checked );
    }
  else
    {
      m_loadOutlandings->setCheckState( Qt::Unchecked );
    }

#ifdef INTERNET

  m_welt2000FileName->setText( conf->getWelt2000FileName() );

#endif

  m_afMargin->setValue(conf->getListDisplayAFMargin());
  m_rpMargin->setValue(conf->getListDisplayRPMargin());

  // sets home radius enabled/disabled in dependency to filter string
  slot_filterChanged(m_countryFilter->text());
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
  QStringList clist = m_countryFilter->text().split(QRegExp("[, ]"),
                      QString::SkipEmptyParts);

  for (QStringList::Iterator it = clist.begin(); it != clist.end(); ++it)
    {
      QString s = *it;

      if (s.length() != 2 || s.contains(QRegExp("[A-Za-z]")) != 2)
        {
          QMessageBox mb( QMessageBox::Warning,
                          tr( "Please check entries" ),
                          tr("Every Welt2000 county sign must consist of two letters!<br>Allowed separators are space and comma.<br>Your modification will not be saved!"),
                          QMessageBox::Ok,
                          this );

#ifdef ANDROID

          mb.show();
          QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                           height()/2 - mb.height()/2 ));
          mb.move( pos );

#endif
          mb.exec();
          return;
        }
    }

  conf->setWelt2000CountryFilter(m_countryFilter->text().trimmed().toUpper());
  conf->setWelt2000HomeRadius(m_homeRadius->value());

  if( m_loadOutlandings->checkState() == Qt::Checked )
    {
      conf->setWelt2000LoadOutlandings( true );
    }
  else
    {
      conf->setWelt2000LoadOutlandings( false );
    }

  conf->setListDisplayAFMargin(m_afMargin->value());
  conf->setListDisplayRPMargin(m_rpMargin->value());
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
  QString wfn = m_welt2000FileName->text().trimmed();

  if( wfn.isEmpty() )
    {
      QMessageBox mb( QMessageBox::Information,
                      tr( "Welt2000 settings invalid!" ),
                      tr( "Please add a valid Welt2000 filename!"),
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      mb.exec();
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Download Welt2000?"),
                  tr( "Active Internet connection is needed!") +
                  QString("<p>") + tr("Start download now?"),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif


  if( mb.exec() == QMessageBox::No )
    {
      return;
    }

  emit downloadWelt2000( wfn );
}

#endif

/**
 * Checks, if the configuration of the Welt2000 has been changed
 */
bool SettingsPageAirfields::checkIsWelt2000Changed()
{
  bool changed = false;

  GeneralConfig *conf = GeneralConfig::instance();

  changed = changed || (conf->getWelt2000CountryFilter() != m_countryFilter->text());
  changed = changed || (conf->getWelt2000HomeRadius() != m_homeRadius->value());

  bool currentState = false;

  if( m_loadOutlandings->checkState() == Qt::Checked )
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

  changed = changed || (conf->getListDisplayAFMargin() != m_afMargin->value());
  changed = changed || (conf->getListDisplayRPMargin() != m_rpMargin->value());

  // qDebug( "SettingsPageAirfields::checkIsListDisplayChanged(): %d", changed );
  return changed;
}
