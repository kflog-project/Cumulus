/***********************************************************************
 **
 **   glidereditornumpad.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by Eggert Ehmke
 **                   2008-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id: glidereditor.cpp 5471 2012-08-07 10:58:09Z axel $
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
      comboType = new QComboBox(this);
      comboType->setEditable(false);
      comboType->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);

#ifdef QSCROLLER
      comboType->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
      QScroller::grabGesture( comboType->view()->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
      comboType->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
      QtScroller::grabGesture( comboType->view()->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

      itemsLayout->addWidget(comboType, row, 1, 1, 3);
      row++;
    }

  itemsLayout->addWidget(new QLabel(tr("Glider Type:"), this), row, 0);
  edtGType = new QLineEdit(this);
  itemsLayout->addWidget(edtGType, row, 1, 1, 3);
  row++;

  itemsLayout->addWidget(new QLabel(tr("Registration:"), this), row, 0);
  edtGReg = new QLineEdit(this);
  itemsLayout->addWidget(edtGReg, row, 1);

  itemsLayout->addWidget(new QLabel(tr("Seats:"), this), row, 2);
  m_seats = new QPushButton( this );
  itemsLayout->addWidget(m_seats, row, 3);

  connect( m_seats, SIGNAL(pressed()), SLOT(slot_changeSeats()) );
  row++;

  itemsLayout->addWidget(new QLabel(tr("Callsign:"), this), row, 0);
  edtGCall = new QLineEdit(this);
  itemsLayout->addWidget(edtGCall, row, 1);

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
  m_dneW1->setDecimals( 1 );
  m_dneW1->setSuffix( " " + Speed::getVerticalUnitText() );

  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "-[0-9]+\\.?[0-9]*" ), this );
  m_dneW1->setValidator(eValidator);

  spinboxLayout->addWidget(m_dneW1, srow, 3);

  spinboxLayout->addWidget(new QLabel(tr("Empty weight:"), this), srow, 4);

  emptyWeight = new NumberEditor( this );
  emptyWeight->setDecimalVisible( false );
  emptyWeight->setPmVisible( false );
  emptyWeight->setMaxLength(4);
  emptyWeight->setSuffix(" kg");
  emptyWeight->setRange( 0, 9999 );
  spinboxLayout->addWidget(emptyWeight, srow, 5);
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
  m_dneW2->setDecimals( 1 );
  m_dneW2->setSuffix( " " + Speed::getVerticalUnitText() );

  eValidator = new QRegExpValidator( QRegExp( "-[0-9]+\\.?[0-9]*" ), this );
  m_dneW2->setValidator(eValidator);

  spinboxLayout->addWidget(m_dneW2, srow, 3);

  spinboxLayout->addWidget(new QLabel(tr("Added load:"), this), srow, 4);

  addedLoad = new NumberEditor( this );
  addedLoad->setDecimalVisible( false );
  addedLoad->setPmVisible( false );
  addedLoad->setMaxLength(4);
  addedLoad->setSuffix(" kg");
  addedLoad->setRange( 0, 9999 );
  spinboxLayout->addWidget(addedLoad, srow, 5);
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
  m_dneW3->setDecimals( 1 );
  m_dneW3->setSuffix( " " + Speed::getVerticalUnitText() );

  eValidator = new QRegExpValidator( QRegExp( "-[0-9]+\\.?[0-9]*" ), this );
  m_dneW3->setValidator(eValidator);

  spinboxLayout->addWidget(m_dneW3, srow, 3);

  spinboxLayout->addWidget(new QLabel(tr("Max. water:"), this), srow, 4);

  addedWater = new NumberEditor( this );
  addedWater->setDecimalVisible( false );
  addedWater->setPmVisible( false );
  addedWater->setMaxLength(4);
  addedWater->setSuffix(" l");
  addedWater->setRange( 0, 9999 );
  spinboxLayout->addWidget(addedWater, srow++, 5);
  spinboxLayout->setColumnStretch(6, 10);
  spinboxLayout->setRowMinimumHeight(srow++, 20);

  buttonShow = new QPushButton(tr("Show Polar"));

  QHBoxLayout* buttonRow = new QHBoxLayout;
  buttonRow->setSpacing(0);
  buttonRow->addStretch( 10 );
  buttonRow->addWidget( buttonShow );

  spinboxLayout->addLayout(buttonRow, srow++, 0, 1, 6);
  spinboxLayout->setRowStretch( srow++, 10 );

  itemsLayout->addLayout(spinboxLayout, row++, 0, 1, 4);
  itemsLayout->setColumnStretch( 1, 10 );
  itemsLayout->setColumnStretch( 4, 20 );

  if( m_isNew )
    {
      connect (comboType, SIGNAL(activated(const QString&)),
               this, SLOT(slotActivated(const QString&)));
    }

  connect(buttonShow, SIGNAL(pressed()), this, SLOT(slotButtonShow()));

  // Add ok and cancel buttons
  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
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

  if (m_isNew)
    {
      readPolarData();
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
  int pos = comboType->currentIndex();

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
      edtGType->setText(m_glider->type());
      edtGReg->setText(m_glider->registration());
      edtGCall->setText(m_glider->callSign());
      m_dneWingArea->setValue( m_glider->polar()->wingArea() );
      addedWater->setValue(m_glider->maxWater());

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

      emptyWeight->setValue((int) m_glider->polar()->emptyWeight());
      double load = m_glider->polar()->grossWeight() -
                    m_glider->polar()->emptyWeight();

      addedLoad->setValue((int) load);

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
  m_glider->setType(edtGType->text().trimmed().replace(";", ","));
  m_glider->setRegistration(edtGReg->text().trimmed().replace(";", ","));
  m_glider->setCallSign(edtGCall->text().trimmed().replace(";", ","));
  m_glider->setMaxWater(addedWater->value());

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

  // qDebug("m_polar->emptyWeight() %f  m_polar->grossWeight() %f",
  //         (float)m_glider->polar()->emptyWeight(),  (float)m_glider->polar()->grossWeight() );
  Polar polar( m_glider->type(),
               V1, W1, V2, W2, V3, W3,
               0.0,
               m_dneWingArea->value(),
               emptyWeight->value(),
               emptyWeight->value() + addedLoad->value() );

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

          if( list.size() < 13 )
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
          // double bestLD = list[7].toDouble(); // not used
          double wingload = list[8].toDouble();
          double wingarea = list[9].toDouble();
          double emptyMass = list[10].toDouble();

          Polar polar = Polar( glidertype,
                               v1, w1, v2, w2, v3, w3,
                               wingload, wingarea,
                               emptyMass, emptyMass );

          polar.setMaxWater(list[11].toInt());
          polar.setSeats(list[12].toInt());

          m_polars.append( polar );
        }

      file.close();

      if( m_polars.size() )
        {
          // sort polar data list according to their names
          qSort( m_polars.begin(), m_polars.end(), Polar::lessThan );

          for( int i = 0; i < m_polars.size(); i++ )
            {
              comboType->addItem( m_polars[i].name() );
            }
        }

      QString firstGlider = comboType->itemText(0);
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
              comboType->addItem (glidertype);

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

/** called when a glider type has been selected */
void GliderEditorNumPad::slotActivated(const QString& type)
{
  if( !m_glider )
    {
      m_glider = new Glider();
      m_gliderCreated = true;
    }

  edtGType->setText( type );
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

      emptyWeight->setValue( (int) m_polar->emptyWeight() );
      double load = m_polar->grossWeight() - m_polar->emptyWeight();
      addedLoad->setValue( (int) load );

      addedWater->setValue( m_polar->maxWater() );

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

  Polar polar( edtGType->text(),
               V1, W1, V2, W2, V3, W3, 0.0,
               m_dneWingArea->value(),
               emptyWeight->value(),
               emptyWeight->value() + addedLoad->value() );

  polar.setWater(addedWater->value(), 0);
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
  edtGType->setText(edtGType->text().trimmed()); //remove spaces
  edtGReg->setText(edtGReg->text().trimmed()); //remove spaces

  QString title;
  QString text;

  if (edtGType->text().isEmpty() )
    {
      title = tr("Missing glider type!");
      text  = tr("Please enter a valid glider type.");
    }
  else if (edtGReg->text().isEmpty())
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
