/***********************************************************************
 **
 **   glidereditornumpad.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by Eggert Ehmke
 **                   2008-2015 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "androidstyle.h"
#include "doubleNumberEditor.h"
#include "generalconfig.h"
#include "glidereditornumpad.h"
#include "layout.h"
#include "mapview.h"
#include "mainwindow.h"
#include "numberEditor.h"
#include "polar.h"
#include "polardialog.h"
#include "speed.h"

extern MapView *_globalMapView;

GliderEditorNumPad::GliderEditorNumPad(QWidget *parent, Glider *glider ) :
  QWidget(parent),
  m_gliderCreated(false)
{
  setWindowFlags(Qt::Tool);
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  // Resize the window to the same size as the main window has. That will
  // completely hide the parent window.
  resize( MainWindow::mainWindow()->size() );

  // save current horizontal/vertical speed unit. This unit must be considered
  // during storage.
  m_currHSpeedUnit = Speed::getHorizontalUnit();
  m_currVSpeedUnit = Speed::getVerticalUnit();

  if (glider == 0)
    {
      setWindowTitle(tr("New Glider"));
      m_isNew = true;
    }
  else
    {
      setWindowTitle(tr("Edit Glider"));
      m_isNew = false;
    }

  m_glider = glider;

  QHBoxLayout* topLayout = new QHBoxLayout(this);
  QGridLayout* itemsLayout = new QGridLayout;
  itemsLayout->setHorizontalSpacing(10);
  itemsLayout->setVerticalSpacing(10);
  int row = 0;

  if( m_isNew )
    {
      itemsLayout->addWidget(new QLabel(tr("Glider Pool:"), this), row, 0);
      m_comboType = new QComboBox(this);
      m_comboType->setEditable(false);
      m_comboType->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);

#ifdef ANDROID
      QAbstractItemView* listView = m_comboType->view();
      QScrollBar* lvsb = listView->verticalScrollBar();
      lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

#ifdef QSCROLLER
      m_comboType->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
      QScroller::grabGesture( m_comboType->view()->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
      m_comboType->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
      QtScroller::grabGesture( m_comboType->view()->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

      itemsLayout->addWidget(m_comboType, row, 1, 1, 3);
      row++;
    }

  Qt::InputMethodHints imh = Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText;

  itemsLayout->addWidget(new QLabel(tr("Glider Type:"), this), row, 0);
  m_edtGType = new QLineEdit(this);
  imh |= m_edtGType->inputMethodHints();
  m_edtGType->setInputMethodHints(imh);

  connect( m_edtGType, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  itemsLayout->addWidget(m_edtGType, row, 1, 1, 3);
  row++;

  itemsLayout->addWidget(new QLabel(tr("Registration:"), this), row, 0);
  m_edtGReg = new QLineEdit(this);
  m_edtGReg->setInputMethodHints(imh);

  connect( m_edtGReg, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  itemsLayout->addWidget(m_edtGReg, row, 1);

  itemsLayout->addWidget(new QLabel(tr("Seats:"), this), row, 2);
  m_seats = new QPushButton( this );
  itemsLayout->addWidget(m_seats, row, 3);

  connect( m_seats, SIGNAL(pressed()), SLOT(slot_changeSeats()) );
  row++;

  itemsLayout->addWidget(new QLabel(tr("Call Sign:"), this), row, 0);
  m_edtGCall = new QLineEdit(this);
  m_edtGCall->setInputMethodHints(imh);

  connect( m_edtGCall, SIGNAL(returnPressed()),
           MainWindow::mainWindow(), SLOT(slotCloseSip()) );

  itemsLayout->addWidget(m_edtGCall, row, 1);

  itemsLayout->addWidget(new QLabel(tr("Wing Area:"), this), row, 2);

  m_dneWingArea = new DoubleNumberEditor(this);
  m_dneWingArea->setDecimalVisible( true );
  m_dneWingArea->setPmVisible( false );
  m_dneWingArea->setMaxLength(7);
  m_dneWingArea->setDecimals( 2 );
  QChar tsChar(Qt::Key_twosuperior);
  m_dneWingArea->setSuffix( QString(" m") + tsChar );
  itemsLayout->addWidget(m_dneWingArea, row, 3);
  row++;

  itemsLayout->setRowMinimumHeight(row++, 10);

  QGridLayout* spinboxLayout = new QGridLayout;
  spinboxLayout->setHorizontalSpacing(10);
  spinboxLayout->setVerticalSpacing(10);
  int srow = 0;

  spinboxLayout->addWidget(new QLabel("v1:", this), srow, 0);

  m_dneV1 = new DoubleNumberEditor(this);
  m_dneV1->setDecimalVisible( true );
  m_dneV1->setPmVisible( false );
  m_dneV1->setMaxLength(5);
  m_dneV1->setRange(0.0, 250.0);
  m_dneV1->setDecimals( 1 );
  m_dneV1->setSuffix( " " + Speed::getHorizontalUnitText() );
  spinboxLayout->addWidget(m_dneV1, srow, 1);

  spinboxLayout->addWidget(new QLabel("w1:", this), srow, 2);

  m_dneW1 = new DoubleNumberEditor(this);
  m_dneW1->setDecimalVisible( true );
  m_dneW1->setPmVisible( true );
  m_dneW1->setMaxLength(7);
  m_dneW1->setDecimals( 2 );
  m_dneW1->setSuffix( " " + Speed::getVerticalUnitText() );

  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "-[0-9]+\\.?[0-9]*" ), this );
  m_dneW1->setValidator(eValidator);

  spinboxLayout->addWidget(m_dneW1, srow, 3);

  spinboxLayout->addWidget(new QLabel(tr("Ref. weight:"), this), srow, 4);

  m_grossWeight = new NumberEditor( this );
  m_grossWeight->setDecimalVisible( false );
  m_grossWeight->setPmVisible( false );
  m_grossWeight->setMaxLength(4);
  m_grossWeight->setSuffix(" kg");
  m_grossWeight->setRange( 0, 9999 );
  spinboxLayout->addWidget(m_grossWeight, srow, 5);
  srow++;

  spinboxLayout->addWidget(new QLabel("v2", this), srow, 0);

  m_dneV2 = new DoubleNumberEditor(this);
  m_dneV2->setDecimalVisible( true );
  m_dneV2->setPmVisible( false );
  m_dneV2->setMaxLength(5);
  m_dneV2->setRange(0.0, 250.0);
  m_dneV2->setDecimals( 1 );
  m_dneV2->setSuffix( " " + Speed::getHorizontalUnitText() );
  spinboxLayout->addWidget(m_dneV2, srow, 1);

  spinboxLayout->addWidget(new QLabel("w2:", this), srow, 2);

  m_dneW2 = new DoubleNumberEditor(this);
  m_dneW2->setDecimalVisible( true );
  m_dneW2->setPmVisible( true );
  m_dneW2->setMaxLength(7);
  m_dneW2->setDecimals( 2 );
  m_dneW2->setSuffix( " " + Speed::getVerticalUnitText() );

  eValidator = new QRegExpValidator( QRegExp( "-[0-9]+\\.?[0-9]*" ), this );
  m_dneW2->setValidator(eValidator);

  spinboxLayout->addWidget(m_dneW2, srow, 3);

  spinboxLayout->addWidget(new QLabel(tr("Load corr.:"), this), srow, 4);

  m_addedLoad = new NumberEditor( this );
  m_addedLoad->setDecimalVisible( false );
  m_addedLoad->setPmVisible( true );
  m_addedLoad->setMaxLength(4);
  m_addedLoad->setSuffix(" kg");
  m_addedLoad->setRange( -999, 999 );
  spinboxLayout->addWidget(m_addedLoad, srow, 5);
  srow++;

  spinboxLayout->addWidget(new QLabel("v3:", this), srow, 0);

  m_dneV3 = new DoubleNumberEditor(this);
  m_dneV3->setDecimalVisible( true );
  m_dneV3->setPmVisible( false );
  m_dneV3->setMaxLength(5);
  m_dneV3->setRange(0.0, 250.0);
  m_dneV3->setDecimals( 1 );
  m_dneV3->setSuffix( " " + Speed::getHorizontalUnitText() );
  spinboxLayout->addWidget(m_dneV3, srow, 1);

  spinboxLayout->addWidget(new QLabel("w3:", this), srow, 2);

  m_dneW3 = new DoubleNumberEditor(this);
  m_dneW3->setDecimalVisible( true );
  m_dneW3->setPmVisible( true );
  m_dneW3->setMaxLength(7);
  m_dneW3->setDecimals( 2 );
  m_dneW3->setSuffix( " " + Speed::getVerticalUnitText() );

  eValidator = new QRegExpValidator( QRegExp( "-[0-9]+\\.?[0-9]*" ), this );
  m_dneW3->setValidator(eValidator);

  spinboxLayout->addWidget(m_dneW3, srow, 3);

  spinboxLayout->addWidget(new QLabel(tr("Max. water:"), this), srow, 4);

  m_addedWater = new NumberEditor( this );
  m_addedWater->setDecimalVisible( false );
  m_addedWater->setPmVisible( false );
  m_addedWater->setMaxLength(4);
  m_addedWater->setSuffix(" l");
  m_addedWater->setRange( 0, 9999 );
  spinboxLayout->addWidget(m_addedWater, srow++, 5);
  spinboxLayout->setColumnStretch(6, 10);
  spinboxLayout->setRowMinimumHeight(srow++, 20);

  m_buttonShow = new QPushButton(tr("Show Polar"));

  QHBoxLayout* buttonRow = new QHBoxLayout;
  buttonRow->setSpacing(0);
  buttonRow->addStretch( 10 );
  buttonRow->addWidget( m_buttonShow );

  spinboxLayout->addLayout(buttonRow, srow++, 0, 1, 6);
  spinboxLayout->setRowStretch( srow++, 10 );

  itemsLayout->addLayout(spinboxLayout, row++, 0, 1, 4);
  itemsLayout->setColumnStretch( 1, 10 );
  itemsLayout->setColumnStretch( 4, 20 );

  if( m_isNew )
    {
      connect (m_comboType, SIGNAL(activated(const QString&)),
               this, SLOT(slotActivated(const QString&)));
    }

  connect(m_buttonShow, SIGNAL(pressed()), this, SLOT(slotButtonShow()));

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  connect(ok, SIGNAL(pressed()), this, SLOT(accept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(reject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);

  topLayout->addLayout(itemsLayout);
  topLayout->addLayout(buttonBox);

  if( m_isNew )
    {
      readLK8000PolarData();
    }
  else
    {
      load();
    }

  show();
}

GliderEditorNumPad::~GliderEditorNumPad()
{
}

void GliderEditorNumPad::showEvent( QShowEvent *event )
{
  Q_UNUSED(event)
}

Polar* GliderEditorNumPad::getPolar()
{
  int pos = m_comboType->currentIndex();

  if ((pos >= 0) && (pos < m_polars.count()))
    {
      return &(m_polars[pos]);
    }
  else
    {
      return static_cast<Polar *> (0);
    }
}

/** Called to initiate loading of the configuration file. */
void GliderEditorNumPad::load()
{
  if (m_glider)
    {
      m_edtGType->setText(m_glider->type());
      m_edtGReg->setText(m_glider->registration());
      m_edtGCall->setText(m_glider->callSign());
      m_dneWingArea->setValue( m_glider->polar()->wingArea() );
      m_addedWater->setValue(m_glider->maxWater());

      if (m_glider->seats() == Glider::doubleSeater)
	{
	  m_seats->setText("2");
	}
      else
	{
	  m_seats->setText("1");
	}

      m_dneV1->setValue( m_glider->polar()->v1().getHorizontalValue() );
      m_dneV2->setValue( m_glider->polar()->v2().getHorizontalValue() );
      m_dneV3->setValue( m_glider->polar()->v3().getHorizontalValue() );

      m_dneW1->setValue( m_glider->polar()->w1().getVerticalValue() );
      m_dneW2->setValue( m_glider->polar()->w2().getVerticalValue() );
      m_dneW3->setValue( m_glider->polar()->w3().getVerticalValue() );

      m_grossWeight->setValue((int) m_glider->polar()->grossWeight());
      m_addedLoad->setValue((int) m_glider->polar()->addLoad() );

      // Save load values to avoid rounding errors during save
      m_currV1 = m_dneV1->value();
      m_currV2 = m_dneV2->value();
      m_currV3 = m_dneV3->value();

      m_currW1 = m_dneW1->value();
      m_currW1 = m_dneW2->value();
      m_currW1 = m_dneW3->value();
    }
}

/** called to initiate saving to the configuration file */
void GliderEditorNumPad::save()
{
  if( !m_glider )
    {
      m_glider = new Glider;
    }

  // Note: If the following strings contains a semicolon it must be replaced
  // by another character. Otherwise storing will fail.
  m_glider->setType(m_edtGType->text().trimmed().replace(";", ","));
  m_glider->setRegistration(m_edtGReg->text().trimmed().replace(";", ","));
  m_glider->setCallSign(m_edtGCall->text().trimmed().replace(";", ","));
  m_glider->setMaxWater(m_addedWater->value());

  if (m_seats->text() == "2" )
    {
      m_glider->setSeats(Glider::doubleSeater);
    }
  else
    {
      m_glider->setSeats(Glider::singleSeater);
    }

  Speed V1, V2, V3, W1, W2, W3;

  // save the current set units, maybe changed in the meantime.
  Speed::speedUnit hSpeedUnit = Speed::getHorizontalUnit();
  Speed::speedUnit vSpeedUnit = Speed::getVerticalUnit();

  // set units valid during load of speeds
  Speed::setHorizontalUnit( m_currHSpeedUnit );
  Speed::setVerticalUnit( m_currVSpeedUnit );

  // Save values only, if they were changed to avoid rounding errors.
  if( m_dneV1->value() != m_currV1 )
    {
      V1.setHorizontalValue(m_dneV1->value());
    }
  else
    {
      V1.setHorizontalValue( m_glider->polar()->v1().getHorizontalValue() );
    }

  if( m_dneV2->value() != m_currV2 )
    {
      V2.setHorizontalValue(m_dneV2->value());
    }
  else
    {
      V2.setHorizontalValue( m_glider->polar()->v2().getHorizontalValue() );
    }

  if( m_dneV3->value() != m_currV3 )
    {
      V3.setHorizontalValue(m_dneV3->value());
    }
  else
    {
      V3.setHorizontalValue( m_glider->polar()->v3().getHorizontalValue() );
    }

  if( m_dneW1->value() != m_currW1 )
    {
      W1.setVerticalValue(m_dneW1->value());
    }
  else
    {
      W1.setVerticalValue( m_glider->polar()->w1().getVerticalValue() );
    }

  if( m_dneW2->value() != m_currW2 )
    {
      W2.setVerticalValue(m_dneW2->value());
    }
  else
    {
      W2.setVerticalValue( m_glider->polar()->w2().getVerticalValue() );
    }

  if( m_dneW3->value() != m_currW3 )
    {
      W3.setVerticalValue(m_dneW3->value());
    }
  else
    {
      W3.setVerticalValue( m_glider->polar()->w3().getVerticalValue() );
    }

  // restore current speed units.
  Speed::setHorizontalUnit( hSpeedUnit );
  Speed::setVerticalUnit( vSpeedUnit );

  Polar polar( m_glider->type(),
               V1, W1, V2, W2, V3, W3,
               m_dneWingArea->value(),
               0.0,
               m_grossWeight->value(),
               m_addedLoad->value() );

  m_glider->setPolar( polar );

  // emit glider object to GliderListWidget
  if (m_isNew)
    {
      emit newGlider(m_glider);
    }
  else
    {
      emit editedGlider(m_glider);
    }

  m_isNew = false;
  // ownership of glider object is passed to GliderListWidget
  m_gliderCreated = false;
}

#if 0
void GliderEditorNumPad::readPolarData()
{
  // qDebug ("GliderEditorNumPad::readPolarData ");

#warning "location of glider.pol file is CUMULUS_ROOT/etc"

  QStringList dirs;

  dirs << GeneralConfig::instance()->getAppRoot() + "/etc/glider.pol"
      << GeneralConfig::instance()->getDataRoot() + "/etc/glider.pol"
      << GeneralConfig::instance()->getUserDataDirectory() + "/glider.pol";

  QFile file;

  for (int i = 0; i < dirs.size(); ++i)
    {
      file.setFileName(dirs.at(i));

      if (file.exists())
	{
	  break;
	}
    }

  QTextStream stream(&file);

  int lineNo = 0;

  if (file.open(QIODevice::ReadOnly))
    {
      while (!stream.atEnd())
	{
	  QString line = stream.readLine().trimmed();

	  lineNo++;

	  // ignore comments and empty lines
	  if(line[0] == '#' || line[0] == '*' || line.size() == 0 )
	    {
	      continue;
	    }

	  QStringList list = line.split(",", QString::KeepEmptyParts);

	  if( list.size() < 11 )
	    {
	      // Too less elements
	      qWarning() << "File glider.pol: Format error at line" << lineNo;
	      continue;
	    }

	  QString glidertype = list[0];

	  // the sink values are positive in this file; we need them negative
	  Speed v1, w1, v2, w2, v3, w3;
	  v1.setKph(list[1].toDouble());
	  w1.setMps(-list[2].toDouble());
	  v2.setKph(list[3].toDouble());
	  w2.setMps(-list[4].toDouble());
	  v3.setKph(list[5].toDouble());
	  w3.setMps(-list[6].toDouble());

	  double wingarea = list[7].toDouble();
	  double emptyMass = list[8].toDouble();

	  Polar polar = Polar( glidertype,
	                       v1, w1, v2, w2, v3, w3,
	                       wingarea,
	                       emptyMass, emptyMass );

	  polar.setMaxWater(list[9].toInt());
	  polar.setSeats(list[10].toInt());

	  m_polars.append( polar );
	}

      file.close();

      if( m_polars.size() )
	{
	  // sort polar data list according to their names
	  qSort( m_polars.begin(), m_polars.end(), Polar::lessThan );

	  for( int i = 0; i < m_polars.size(); i++ )
	    {
	      m_comboType->addItem( m_polars[i].name() );
	    }
	}

      QString firstGlider = m_comboType->itemText(0);
      slotActivated(firstGlider);
    }
  else
    {
      _globalMapView->slot_info(tr("Missing polar file"));
      qWarning( "Could not open polar file: %s",
                file.fileName().toLatin1().data() );
    }

#if 0

  // try to read more polars from document folder
  // these files are in winpilot format: one file per polar
  DocLnkSet docs;
  Global::findDocuments(&docs, "cumulus/polar");
  QList<DocLnk*> list = docs.children();

  // qDebug ("found %d polars in document folder", list.count());
  for (DocLnk* doc = list.first(); doc; doc = list.next())
    {
      QString path = doc->file();
      // qDebug ("Doc: %s", path.toLatin1().data());
      QFile file (path);
      QTextStream stream(&file);
      if (file.open(IO_ReadOnly))
	{
	  QString glidertype = QFileInfo(file).baseName();
	  while (!stream.eof())
	    {
	      QString line = stream.readLine();
	      // ignore comments
	      if (line[0] == '*')
		continue;
	      QStringList list = QStringList::split(',',line,TRUE);
	      m_comboType->addItem (glidertype);

	      // vertical speeds are already negative in these files !
	      Speed v1,w1,v2,w2,v3,w3;
	      double maxgross = list [0].toDouble();
	      int maxwater = list [1].toInt();
	      v1.setKph(list[2].toDouble());
	      w1.setMps(list[3].toDouble());
	      v2.setKph(list[4].toDouble());
	      w2.setMps(list[5].toDouble());
	      v3.setKph(list[6].toDouble());
	      w3.setMps(list[7].toDouble());
	      pol=new Polar(this, glidertype,v1,w1,v2,w2,v3,w3,0.0,0.0,0.0,maxgross);
	      pol->setMaxWater(maxwater);
	      m_polars.append(pol);
	      break;
	    }
	}
    }

#endif

}
#endif

void GliderEditorNumPad::readLK8000PolarData()
{
  QStringList dirs;

  // Directories where we expect polar data files. The first directory, which
  // contains polar files is taken. The rest is ignored then.
  dirs << GeneralConfig::instance()->getAppRoot() + "/etc"
       << GeneralConfig::instance()->getDataRoot() + "/etc"
       << GeneralConfig::instance()->getUserDataDirectory();

  QStringList filters;

  // File extension of LK8000 polar files
  filters << "*.plr";

  for( int i = 0; i < dirs.size(); ++i )
    {
      QDir dir( dirs.at(i) );

      if( dir.exists() )
	{
	  // Look, if polar files are to find in this directory
	  QStringList plrList = dir.entryList( filters, QDir::Files, QDir::Name );

	  if( plrList.isEmpty() )
	    {
	      continue;
	    }

	  for( int j = 0; j < plrList.size(); j++ )
	    {
	      // Read in all found polar data files.
	      Polar polar;
	      bool ok = readLK8000PolarFile( dir.absolutePath() + "/" + plrList.at(j), polar );

	      if( ok )
		{
		  m_polars.append( polar );
		}
	    }

	  if( m_polars.size() )
	    {
	      // sort polar data list according to their names
	      // qSort( m_polars.begin(), m_polars.end(), Polar::lessThan );

	      for( int i = 0; i < m_polars.size(); i++ )
		{
		  m_comboType->addItem( m_polars[i].name() );
		}

	      // Activate first glider entry in the combo box.
	      slotActivated( m_comboType->itemText(0) );
	    }

	  // We read only data from the first directory with polar files.
	  return;
	}
    }

  _globalMapView->slot_info(tr("Missing polar files"));

  qWarning( "readLK8000PolarData(): Missing polar files" );
}

/*
 * https://github.com/LK8000/LK8000/tree/master/Common/Distribution/LK8000/_Polars
 *
 * Format explanation: all lines starting with a * are comments and you can also have empty lines
 *
 * Field 1: Gross weight of the glider, excluded ballast, when the values were measured
 * Field 2: Max ballast you can load (water). It will add wing loading, separately
 * Field 3-4, 5-6, 7-8  are couples of  speed,sink rate  in km/h and m/s .
 * 	these values are used to create an interpolated curve of sink rates
 * Field 9: NEW! Normally winpilot does not have this value. Put here at all cost the glider
 *          surface area in squared meters (m2). If the polar curve you are using does not have
 *          this value, go on wikipedia to find the wing area for that glider and add it after a comma.
 *
 * Here is the REAL polar used internally, that you have to create or change, for a glider
 * that during test flight was weighting 330kg including pilots and everything, that can load
 * extra 90 liters of water ballast, that at 75km/h has a sink rate of 0.7 m/s , at 93 km/h of 0,74
 * at 185 km/h sinks at 3.1 m/s and finally that has a wing surface of 10.6 m2.
 * Thus, the polar was calculated with a default wing loading of 31.1 kg/m2

 * So this is an example
 *
 * 330,	90,	75.0,	-0.7,	93.0,	-0.74,	185.00,	-3.1, 10.6
 *
 * Speed Astir.plr file content
 *
 * LK8000 polar for: Speed Astir
 * MassDryGross[kg], MaxWaterBallast[liters], Speed1[km/h], Sink1[m/s], Speed2, Sink2, Speed3, Sink3, WingArea[m2]
 * 351,  90,  90, -0.63, 105, -0.72, 157, -2.00, 11.5   // BestLD40@105
 *
 */
bool GliderEditorNumPad::readLK8000PolarFile( const QString& fileName, Polar& polar )
{
  QFile polarFile( fileName );

  if( polarFile.exists() == false )
    {
      qWarning() << "readLK8000PolarFile:" << fileName << "does not exists!";
      return false;
    }

  if( polarFile.open(QIODevice::ReadOnly) == false )
    {
      qWarning() << "readLK8000PolarFile:" << fileName << "is not readable!";
      return false;
    }

  // We take the polar file name as gilder type name. The type description
  // is sometimes missing in the file.
  QString name = QFileInfo(fileName).completeBaseName();
  polar.setName( name );

  if( name.contains( "PAS") )
    {
      // Polar of a double seater was selected
      polar.setSeats( 2 );
    }

  QTextStream stream( &polarFile );

  int lineNo = 0;

  while( !stream.atEnd() )
    {
      QString line = stream.readLine().trimmed();
      lineNo++;

      if( line.isEmpty() || line.startsWith("*") || line.startsWith("//") )
	{
	  continue;
	}

      // Lines can contain at their right end C++ comments. This part must be
      // removed first.
      QStringList items = line.split( "//" );
      items = items.at(0).split( ",", QString::SkipEmptyParts );

      if( items.size() < 9 )
	{
	  qWarning() << "Polar file"
	             << QFileInfo(fileName).fileName()
	             << "line"
	             << lineNo
	             << "contains to less items:"
	             << line;

	  polarFile.close();
	  return false;
	}

      // Extract data, Example is from Spped Astir.
      // MassDryGross[kg], MaxWaterBallast[liters], Speed1[km/h], Sink1[m/s], Speed2, Sink2, Speed3, Sink3, WingArea[m2]
      // 351,  90,  90, -0.63, 105, -0.72, 157, -2.00, 11.5   // BestLD40@105
      for( int i = 0; i < 9; i++ )
	{
	  bool ok;
	  double dv = items.at(i).trimmed().toDouble(&ok);

	  Speed speed;

	  if( ! ok )
	    {
	      polarFile.close();
	      return false;
	    }

	  switch(i)
	  {
	    case 0:
	      polar.setGrossWeight( dv );
	      break;

	    case 1:
	      polar.setMaxWater( rint(dv) );
	      break;

	    case 2:
	      speed.setKph( dv );
	      polar.setV1( speed );
	      break;

	    case 3:
	      speed.setMps( dv );
	      polar.setW1( speed );
	      break;

	    case 4:
	      speed.setKph( dv );
	      polar.setV2( speed );
	      break;

	    case 5:
	      speed.setMps( dv );
	      polar.setW2( speed );
	      break;

	    case 6:
	      speed.setKph( dv );
	      polar.setV3( speed );
	      break;

	    case 7:
	      speed.setMps( dv );
	      polar.setW3( speed );
	      break;

	    case 8:
	      polar.setWingArea( dv );
	      break;

	    default:
	      break;
	  }
	}

      polar.recalculatePolarData();
      polarFile.close();
      return true;
    }

  polarFile.close();
  qWarning() << "readLK8000PolarFile:" << fileName << "contains no polar data!";
  return false;
}

/** called when a glider type has been selected */
void GliderEditorNumPad::slotActivated(const QString& type)
{
  if( !m_glider )
    {
      m_glider = new Glider();
      m_gliderCreated = true;
    }

  m_edtGType->setText( type );
  m_glider->setType( type );

  m_polar = getPolar();

  if( m_polar )
    {
      m_glider->setPolar( *m_polar );

      m_dneWingArea->setValue( m_polar->wingArea() );
      m_dneV1->setValue( m_polar->v1().getHorizontalValue() );
      m_dneW1->setValue( m_polar->w1().getVerticalValue() );
      m_dneV2->setValue( m_polar->v2().getHorizontalValue() );
      m_dneW2->setValue( m_polar->w2().getVerticalValue() );
      m_dneV3->setValue( m_polar->v3().getHorizontalValue() );
      m_dneW3->setValue( m_polar->w3().getVerticalValue() );

      m_grossWeight->setValue( (int) m_polar->grossWeight() );
      m_addedLoad->setValue( (int) m_polar->addLoad() );

      m_addedWater->setValue( m_polar->maxWater() );

      if( m_polar->seats() == 2 )
	{
	  m_seats->setText( "2" );
	}
      else
	{
	  m_seats->setText( "1" );
	}
    }
}

void GliderEditorNumPad::slotButtonShow()
{
  // we create a polar object on the fly to allow test of changed polar values without saving
  Speed V1, V2, V3, W1, W2, W3;
  V1.setHorizontalValue(m_dneV1->value());
  V2.setHorizontalValue(m_dneV2->value());
  V3.setHorizontalValue(m_dneV3->value());

  W1.setVerticalValue(m_dneW1->value());
  W2.setVerticalValue(m_dneW2->value());
  W3.setVerticalValue(m_dneW3->value());

  Polar polar( m_edtGType->text(),
               V1, W1, V2, W2, V3, W3,
               m_dneWingArea->value(),
               0.0,
               m_grossWeight->value(),
               m_addedLoad->value() );

  polar.setWater( m_addedWater->value() );
  PolarDialog* dlg = new PolarDialog( polar, this );
  dlg->setVisible(true);
}

void GliderEditorNumPad::slot_changeSeats()
{
  if( m_seats->text() == "1" )
    {
      m_seats->setText( "2" );
    }
  else if( m_seats->text() == "2" )
    {
      m_seats->setText( "1" );
    }
}

void GliderEditorNumPad::accept()
{
  m_edtGType->setText(m_edtGType->text().trimmed()); //remove spaces
  m_edtGReg->setText(m_edtGReg->text().trimmed()); //remove spaces

  QString title;
  QString text;

  if (m_edtGType->text().isEmpty() )
    {
      title = tr("Missing glider type!");
      text  = tr("Please enter a valid glider type.");
    }
  else if (m_edtGReg->text().isEmpty())
    {
      title = tr( "Missing registration!" );
      text  = tr( "Please enter a valid registration." );
    }
  else
    {
      save();
      QWidget::close();
      return;
    }

  QMessageBox mb( QMessageBox::Critical,
                  title,
                  text,
                  QMessageBox::Ok,
                  this );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2 - mb.width()/2, height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  mb.exec();
}

void GliderEditorNumPad::reject()
{
  // @AP: delete glider, if it was allocated in this class and not
  // emitted in accept method to avoid a memory leak.
  if (m_gliderCreated && m_glider)
    {
      delete m_glider;
      m_glider = 0;
    }

  QWidget::close();
}
