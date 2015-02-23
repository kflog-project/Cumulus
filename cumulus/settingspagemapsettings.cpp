/***********************************************************************
**
**   settingspagemapsettings.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
************************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "distance.h"
#include "generalconfig.h"
#include "layout.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "numberEditor.h"
#include "settingspagemapsettings.h"

#ifdef INTERNET
#include "httpclient.h"
#endif

/***********************************************************/
/*  map setting page                                       */
/***********************************************************/

SettingsPageMapSettings::SettingsPageMapSettings(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("SettingsPageMapSettings");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Map") );

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

  QHBoxLayout *contentLayout = new QHBoxLayout(this);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal );

  m_currentProjType = ProjectionBase::Unknown;
  GeneralConfig *conf = GeneralConfig::instance();
  int row=0;

  QGridLayout *topLayout = new QGridLayout(sw);

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setMargin( 0 );

#ifdef ANDROID
  QLabel* mapSelection = new QLabel( tr("Maps") + ":", this );
#else
  QPushButton* mapSelection = new QPushButton( tr("Maps"), this );
  mapSelection->setToolTip(tr("Select your personal map directory"));

  connect(mapSelection, SIGNAL( clicked()), this, SLOT(slot_openFileDialog()) );
#endif

  hbox->addWidget( mapSelection );

  Qt::InputMethodHints imh;

  mapDirectory = new QLineEdit( this );
  imh = (mapDirectory->inputMethodHints() | Qt::ImhNoPredictiveText);
  mapDirectory->setInputMethodHints(imh);

#ifdef ANDROID
  mapDirectory->setReadOnly(true);
#endif

  connect( mapDirectory, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  hbox->addWidget( mapDirectory );
  topLayout->addLayout(hbox, row++, 0, 1, 3 );

  topLayout->addWidget(new QLabel(tr("Projection:"), this), row, 0 );
  cmbProjection = new QComboBox(this);
  topLayout->addWidget(cmbProjection, row++, 1);
  cmbProjection->addItem(tr("Lambert"));
  cmbProjection->addItem(tr("Plate Carrée")); // Qt::Key_Eacute

  connect( cmbProjection, SIGNAL(activated(int)),
          this, SLOT(slot_selectProjection(int)) );

  topLayout->addWidget(new QLabel(tr("1. St. Parallel:"), this), row, 0);

  edtLat1 = new LatEditNumPad(this, conf->getHomeLat());
  topLayout->addWidget(edtLat1, row++, 1, 1, 2);

  edtLat2Label = new QLabel(tr("2. St. Parallel:"), this);
  topLayout->addWidget(edtLat2Label, row, 0);

  edtLat2 = new LatEditNumPad(this, conf->getHomeLat());
  topLayout->addWidget(edtLat2, row++, 1, 1, 2);

  edtLonLabel = new QLabel(tr("Origin Longitude:"), this);
  topLayout->addWidget(edtLonLabel, row, 0);


  edtLon = new LongEditNumPad(this, conf->getLambertOrign());
  topLayout->addWidget(edtLon, row++, 1, 1, 2);

  //------------------------------------------------------------------------------

  topLayout->setRowMinimumHeight(row++,15);

  chkProjectionFollowHome = new QCheckBox(tr("Projection follows Home Position"), this );
  topLayout->addWidget(chkProjectionFollowHome, row, 0, 1, 2);
  row++;

  chkUnloadUnneeded = new QCheckBox(tr("Unload unused maps from RAM"), this );
  topLayout->addWidget(chkUnloadUnneeded, row, 0, 1, 2);
  row++;

#ifdef INTERNET

  topLayout->setRowMinimumHeight(row++,10);

  QLabel *label = new QLabel(tr("Center Latitude:"), this);
  topLayout->addWidget(label, row, 0);

  edtCenterLat = new LatEditNumPad(this, conf->getHomeLat());
  topLayout->addWidget(edtCenterLat, row++, 1, 1, 2);

  label = new QLabel(tr("Center Longitude:"), this);
  topLayout->addWidget(label, row, 0);

  edtCenterLon = new LongEditNumPad(this, conf->getHomeLon());
  topLayout->addWidget(edtCenterLon, row++, 1, 1, 2);

  installMaps = new QPushButton( tr("Install Maps"), this );
  installMaps->setToolTip(tr("Install maps around center point"));
  topLayout->addWidget(installMaps, row, 0 );

  connect(installMaps, SIGNAL( clicked()), this, SLOT(slot_installMaps()) );

  installRadius = new NumberEditor( this );
  installRadius->setToolTip( tr("Radius around center point") );
  installRadius->setDecimalVisible( false );
  installRadius->setPmVisible( false );
  installRadius->setMaxLength(6);
  installRadius->setSuffix( " " + Distance::getUnitText() );
  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "(0|[1-9][0-9]{0,5})" ), this );
  installRadius->setValidator( eValidator );
  installRadius->setValue( GeneralConfig::instance()->getMapInstallRadius() );
  topLayout->addWidget(installRadius, row++, 1);

#endif // #ifdef INTERNET

  topLayout->setColumnStretch( 2, 10 );
  topLayout->setRowStretch( row, 10 );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
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

SettingsPageMapSettings::~SettingsPageMapSettings()
{
}

void SettingsPageMapSettings::slotAccept()
{
  save();
  QWidget::close();
}

void SettingsPageMapSettings::slotReject()
{
  QWidget::close();
}

void SettingsPageMapSettings::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // Take the first directory from the search order
  mapDirectory->setText( conf->getMapDirectories()[0] );

  chkUnloadUnneeded->setChecked( conf->getMapUnload() );
  chkProjectionFollowHome->setChecked( conf->getMapProjectionFollowsHome() );

#ifdef INTERNET

  edtCenterLat->setKFLogDegree(conf->getHomeLat());
  edtCenterLon->setKFLogDegree(conf->getHomeLon());

#endif

  m_currentProjType = conf->getMapProjectionType();
  m_lambertV1 =       conf->getLambertParallel1();
  m_lambertV2 =       conf->getLambertParallel2();
  m_lambertOrigin =   conf->getLambertOrign();
  m_cylinPar =        conf->getCylinderParallel();

  // @AP: Note, that the index of the list starts with 0 but the
  // ProjectionType uses zero for unknown. So we must subtract 1
  // to get the right value.
  int projIndex = m_currentProjType - 1;
  cmbProjection->setCurrentIndex(projIndex);
  slot_selectProjection(projIndex);
}

void SettingsPageMapSettings::save()
{
  if( checkChanges() == false )
    {
      return;
    }

  if( checkIsProjectionChanged() )
    {
      QMessageBox mb( QMessageBox::Information,
                      "Cumulus",
                      tr( "<html>"
                      "<b>Map projection was changed!</b><p>"
                      "System update can take a few seconds and more!"
                      "</html>" ),
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      // Under Android the box must be moved into the center of the desktop screen.
      // Note the box must be set as first to visible otherwise move will not work.
      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2 - mb.width()/2, height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      mb.exec();
    }

  // @AP: here we must take over the new user values at first.
  // After that we can store them.
  // Check, if input string values have been changed. If not, no
  // take over of values to avoid rounding errors. They can appear if
  // the position formats will be changed between DMS <-> DDM vice
  // versa.
  switch(cmbProjection->currentIndex())
    {
    case 0:

      if( edtLat1->isInputChanged() )
        {
          m_lambertV1 = edtLat1->KFLogDegree();
        }

      if( edtLat2->isInputChanged() )
        {
          m_lambertV2 = edtLat2->KFLogDegree();
        }

      if( edtLon->isInputChanged() )
        {
          m_lambertOrigin = edtLon->KFLogDegree();
        }

      break;

    case 1:

      if( edtLat1->isInputChanged() )
        {
          m_cylinPar = edtLat1->KFLogDegree();
        }

      break;
    }

  GeneralConfig *conf = GeneralConfig::instance();

  conf->setMapRootDir( mapDirectory->text() );
  conf->setMapUnload( chkUnloadUnneeded->isChecked() );
  conf->setMapProjectionFollowsHome( chkProjectionFollowHome->isChecked() );
#ifdef INTERNET
  conf->setMapInstallRadius( installRadius->value() );
#endif
  conf->setMapProjectionType( m_currentProjType );
  conf->setLambertParallel1( m_lambertV1 );
  conf->setLambertParallel2( m_lambertV2 );
  conf->setLambertOrign( m_lambertOrigin );
  conf->setCylinderParallel( m_cylinPar );
  conf->save();

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents |
                                   QEventLoop::ExcludeSocketNotifiers );
  QCoreApplication::flush();
  emit settingsChanged();
  QApplication::restoreOverrideCursor();
}

#ifdef INTERNET

/**
 * Called if the install maps button is pressed
 */
void SettingsPageMapSettings::slot_installMaps()
{
  if( installRadius->value() == 0 )
    {
      // Radius distance is zero, ignore request.
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Download Maps?"),
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

  // Save map directory, can be changed in the meantime.
  // Saving will create all missing map directories.
  GeneralConfig::instance()->setMapRootDir( mapDirectory->text() );

  Distance distance( Distance::convertToMeters( installRadius->value() ));
  QPoint center;
  center.setX( edtCenterLat->KFLogDegree() );
  center.setY( edtCenterLon->KFLogDegree() );

  emit downloadMapArea( center, distance );
}

#endif

/**
 * Called if the map selection button is pressed
 */
void SettingsPageMapSettings::slot_openFileDialog()
{
  QString mapDirCurrent = GeneralConfig::instance()->getMapDirectories()[0];
  QDir mapDir;

  if( ! mapDir.exists( mapDirCurrent ) )
      {
        // Fall back to default if not existing
        mapDirCurrent = QDir::homePath();
      }

  QString mapDirNew = QFileDialog::getExistingDirectory( this,
                                                         tr("Please select your map directory"),
                                                         mapDirCurrent,
                                                         QFileDialog::ShowDirsOnly );
  if( mapDirNew.isEmpty() )
    {
      return; // cancel was selected by the user
    }

  mapDirectory->setText( mapDirNew );
}

// selection in the combo box has been changed. index is a reference
// to the current entry. initialize widgets with the internal values
// normally read from the configuration file.
void SettingsPageMapSettings::slot_selectProjection(int index)
{

  switch (index)
    {
      case 0: // Lambert

        edtLat2Label->setVisible(true);
        edtLat2->setVisible(true);
        edtLonLabel->setVisible(true);
        edtLon->setVisible(true);
        chkProjectionFollowHome->setVisible(false);
        edtLat1->setKFLogDegree(m_lambertV1);
        edtLat2->setKFLogDegree(m_lambertV2);
        edtLon->setKFLogDegree(m_lambertOrigin);
        m_currentProjType = ProjectionBase::Lambert;
        break;

      case 1: // Plate Carreé
      default: // take this if index is unknown

        edtLat2Label->setVisible(false);
        edtLat2->setVisible(false);
        edtLonLabel->setVisible(false);
        edtLon->setVisible(false);
        chkProjectionFollowHome->setVisible(true);
        edtLat1->setKFLogDegree(m_cylinPar);
        edtLat2->setKFLogDegree(0);
        edtLon->setKFLogDegree(0);
        m_currentProjType = ProjectionBase::Cylindric;
        break;
    }
}

bool SettingsPageMapSettings::checkChanges()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed |= ( mapDirectory->text() != conf->getMapRootDir() );
  changed |= ( chkUnloadUnneeded->isChecked() != conf->getMapUnload() );
  changed |= ( chkProjectionFollowHome->isChecked() != conf->getMapProjectionFollowsHome() );

#ifdef INTERNET
  changed |= ( installRadius->value() != conf->getMapInstallRadius() );
#endif

  changed |= checkIsProjectionChanged();

  return changed;
}

/**
 * Checks, if the configuration of the projection has been changed
 */
bool SettingsPageMapSettings::checkIsProjectionChanged()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  switch( cmbProjection->currentIndex() )
    {
      case 0:
        changed |= ( edtLat1->isInputChanged() );
        changed |= ( edtLat2->isInputChanged() );
        changed |= ( edtLon->isInputChanged() );
        break;

      case 1:
        changed |= ( edtLat1->isInputChanged() );
        break;
    }

  changed |= ( conf->getMapProjectionType() != m_currentProjType );

  // qDebug( "SettingsPageMapSettings::()checkIsProjectionChanged: %d", changed );
  return changed;
}
