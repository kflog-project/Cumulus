/***********************************************************************
 **
 **   settingspageairfields.cpp
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

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "layout.h"
#include "numberEditor.h"
#include "settingspageairfields.h"

#ifdef INTERNET
#include "httpclient.h"
#endif

SettingsPageAirfields::SettingsPageAirfields(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("SettingsPageAirfields");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Airfields") );

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
  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // Add scroll area to its own layout
  sal->addWidget( sa );

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal, 10 );

  // The parent of the layout is the scroll widget
  QVBoxLayout *topLayout = new QVBoxLayout(sw);

  QGroupBox* weltGroup = new QGroupBox(tr("Welt2000"), this);
  topLayout->addWidget(weltGroup);

  QGridLayout* weltLayout = new QGridLayout(weltGroup);

  int grow = 0;
  QLabel* lbl = new QLabel(tr("Country Filter:"), (weltGroup));
  weltLayout->addWidget(lbl, grow, 0);

  m_countryFilter = new QLineEdit(weltGroup);
  weltLayout->addWidget(m_countryFilter, grow, 1, 1, 3);
  grow++;

  lbl = new QLabel(tr("Home Radius:"), weltGroup);
  weltLayout->addWidget(lbl, grow, 0);

  m_homeRadius = new NumberEditor( this );
  m_homeRadius->setDecimalVisible( false );
  m_homeRadius->setPmVisible( false );
  m_homeRadius->setMaxLength(4);
  m_homeRadius->setSuffix( " " + Distance::getUnitText() );
  m_homeRadius->setMaximum( 9999 );
  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "([1-9][0-9]{0,3})" ), this );
  m_homeRadius->setValidator( eValidator );
  weltLayout->addWidget(m_homeRadius, grow, 1 );

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

  m_afMargin = new NumberEditor( this );
  m_afMargin->setDecimalVisible( false );
  m_afMargin->setPmVisible( false );
  m_afMargin->setMaxLength(2);
  m_afMargin->setSuffix( tr(" Pixels") );
  m_afMargin->setMaximum( 30 );
  m_afMargin->setTitle("0...30");
  eValidator = new QRegExpValidator( QRegExp( "([0-9]|[1-2][0-9]|30)" ), this );
  m_afMargin->setValidator( eValidator );
  listLayout->addWidget(m_afMargin, grow, 1 );
  grow++;
  lbl = new QLabel(tr( "More space in Emergency list:"), listGroup);
  listLayout->addWidget(lbl, grow, 0);

  m_rpMargin = new NumberEditor( this );
  m_rpMargin->setDecimalVisible( false );
  m_rpMargin->setPmVisible( false );
  m_rpMargin->setMaxLength(2);
  m_rpMargin->setSuffix( tr(" Pixels") );
  m_rpMargin->setMaximum( 30 );
  m_rpMargin->setTitle("0...30");
  eValidator = new QRegExpValidator( QRegExp( "([0-9]|[1-2][0-9]|30)" ), this );
  m_rpMargin->setValidator( eValidator );
  listLayout->addWidget(m_rpMargin, grow, 1 );
  grow++;
  listLayout->setRowStretch(grow, 10);
  listLayout->setColumnStretch(2, 10);

#ifndef ANDROID
  // Android makes trouble, if word detection is enabled and the input is
  // changed by us.
  connect( m_countryFilter, SIGNAL(textChanged(const QString&)),
           this, SLOT(slot_filterChanged(const QString&)) );
#endif

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png"));

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

SettingsPageAirfields::~SettingsPageAirfields()
{
}

void SettingsPageAirfields::slotAccept()
{
  if( checkChanges() )
    {
      if( save() == false )
        {
          return;
        }

      emit settingsChanged();
    }

  QWidget::close();
}

void SettingsPageAirfields::slotReject()
{
  QWidget::close();
}

/**
 * Called to initiate loading of the configuration file
 */
void SettingsPageAirfields::load()
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
bool SettingsPageAirfields::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // Store change state.
  bool welt2000Changed = checkIsWelt2000Changed();

  // We will check, if the country entries of Welt2000 are
  // correct. If not a warning message is displayed and the
  // modifications are discarded.
  QStringList clist = m_countryFilter->text().split(QRegExp("[, ]"),
                      QString::SkipEmptyParts);

  for( int i = 0; i < clist.size(); i++ )
    {
      const QString& s = clist.at(i);

      if( ! (s.length() == 2 && s.contains(QRegExp("[A-Za-z][A-Za-z]")) == true) )
        {
          QMessageBox mb( QMessageBox::Warning,
                          tr( "Please check entries" ),
                          tr("Every Welt2000 county sign must consist of two letters!<br>Allowed separators are space and comma!"),
                          QMessageBox::Ok,
                          this );

#ifdef ANDROID

          mb.show();
          QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                           height()/2 - mb.height()/2 ));
          mb.move( pos );

#endif
          mb.exec();
          return false;
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
  conf->save();

  if( welt2000Changed == true )
    {
      emit reloadWelt2000();
    }

  return true;
}

/**
 * Called if the text of the filter has been changed
 */
void SettingsPageAirfields::slot_filterChanged(const QString& text)
{
  Q_UNUSED( text )
}

/* Called to ask is confirmation on the close is needed. */
bool SettingsPageAirfields::checkChanges()
{
  bool changed = false;

  changed = changed || checkIsWelt2000Changed();
  changed = changed || checkIsListDisplayChanged();

  return changed;
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
