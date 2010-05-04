/***********************************************************************
 **
 **   glidereditor.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by Eggert Ehmke
 **                   2008-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "polardialog.h"
#include "glidereditor.h"
#include "polar.h"
#include "speed.h"
#include "mapview.h"
#include "generalconfig.h"
#include "mainwindow.h"

extern MapView     *_globalMapView;
extern MainWindow  *_globalMainWindow;

GilderEditor::GilderEditor(QWidget *parent, Glider *glider ) :
  QDialog(parent)
{
  setObjectName("SettingsPageGliderData");
  setAttribute(Qt::WA_DeleteOnClose);
  setModal(true);

  // save current horizontal/vertical speed unit. This unit must be considered
  // during storage.
  currHSpeedUnit = Speed::getHorizontalUnit();
  currVSpeedUnit = Speed::getVerticalUnit();

  if( _globalMainWindow )
    {
      // Resize the dialog to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  if (glider == 0)
    {
      setWindowTitle(tr("New Glider"));
      isNew = true;
    }
  else
    {
      setWindowTitle(tr("Edit Glider"));
      isNew = false;
    }

  gliderCreated = false;

  _glider = glider;

  QHBoxLayout* topLayout = new QHBoxLayout(this);

  QScrollArea* itemArea = new QScrollArea(this);

  QWidget* itemWidget = new QWidget();
  QGridLayout* itemsLayout = new QGridLayout(itemWidget);
  itemsLayout->setHorizontalSpacing(10);
  int row = 0;

  itemsLayout->addWidget(new QLabel(tr("Glider type:"), this), row, 0);
  comboType = new QComboBox(this);
  comboType->setEditable(true);
  comboType->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
  itemsLayout->addWidget(comboType, row, 1);

  itemsLayout->addWidget(new QLabel(tr("Seats:"), this), row, 2);
  comboSeats = new QComboBox(this);
  comboSeats->addItem(tr("Single"));
  comboSeats->addItem(tr("Double"));
  itemsLayout->addWidget(comboSeats, row, 3);
  row++;

  itemsLayout->setRowMinimumHeight(row++, 10);

  itemsLayout->addWidget(new QLabel(tr("Registration:"), this), row, 0);
  edtGReg = new QLineEdit(this);
  itemsLayout->addWidget(edtGReg, row, 1);

  itemsLayout->addWidget(new QLabel(tr("Callsign:"), this), row, 2);
  edtGCall = new QLineEdit(this);
  itemsLayout->addWidget(edtGCall, row, 3);
  row++;

  itemsLayout->setRowMinimumHeight(row++, 10);

  QGridLayout* spinboxLayout = new QGridLayout;
  spinboxLayout->setHorizontalSpacing(10);
  int srow = 0;

  spinboxLayout->addWidget(new QLabel("v1:", this), srow, 0);
  spinV1 = new QDoubleSpinBox(this);
  spinV1->setRange(0.0, 150.0);
  spinV1->setSingleStep(1.0);
  spinV1->setButtonSymbols(QSpinBox::PlusMinus);
  spinV1->setSuffix( Speed::getHorizontalUnitText() );
  spinboxLayout->addWidget(spinV1, srow, 1);

  spinboxLayout->addWidget(new QLabel("w1:", this), srow, 2);
  spinW1 = new QDoubleSpinBox(this);

  if( Speed::getVerticalUnit() !=  Speed::feetPerMinute )
    {
      spinW1->setRange(-5.0, 0);
      spinW1->setSingleStep(0.01);
    }
  else
    {
      spinW1->setRange(-1000.0, 0);
      spinW1->setSingleStep(2.0);
    }

  spinW1->setButtonSymbols(QSpinBox::PlusMinus);
  spinW1->setSuffix( Speed::getVerticalUnitText() );
  spinboxLayout->addWidget(spinW1, srow, 3);

  spinboxLayout->addWidget(new QLabel(tr("Empty weight:"), this), srow, 4);
  emptyWeight = new QSpinBox(this);
  emptyWeight->setObjectName("emptyWeight");
  emptyWeight->setMaximum(1000);
  emptyWeight->setSingleStep(5);
  emptyWeight->setButtonSymbols(QSpinBox::PlusMinus);
  emptyWeight->setSuffix("kg");
  spinboxLayout->addWidget(emptyWeight, srow, 5);
  srow++;

  spinboxLayout->addWidget(new QLabel("v2", this), srow, 0);
  spinV2 = new QDoubleSpinBox(this);
  spinV2->setRange(0.0, 200.0);
  spinV2->setSingleStep(1.0);
  spinV2->setButtonSymbols(QSpinBox::PlusMinus);
  spinV2->setSuffix( Speed::getHorizontalUnitText() );
  spinboxLayout->addWidget(spinV2, srow, 1);

  spinboxLayout->addWidget(new QLabel("w2:", this), srow, 2);
  spinW2 = new QDoubleSpinBox(this);

  if( Speed::getVerticalUnit() !=  Speed::feetPerMinute )
    {
      spinW2->setRange(-5.0, 0);
      spinW2->setSingleStep(0.01);
    }
  else
    {
      spinW2->setRange(-1000.0, 0);
      spinW2->setSingleStep(2.0);
    }

  spinW2->setButtonSymbols(QSpinBox::PlusMinus);
  spinW2->setSuffix( Speed::getVerticalUnitText() );
  spinboxLayout->addWidget(spinW2, srow, 3);

  spinboxLayout->addWidget(new QLabel(tr("Added load:"), this), srow, 4);
  addedLoad = new QSpinBox(this);
  addedLoad->setObjectName("addedLoad");
  addedLoad->setMaximum(1000);
  addedLoad->setSingleStep(5);
  addedLoad->setButtonSymbols(QSpinBox::PlusMinus);
  addedLoad->setSuffix("kg");
  spinboxLayout->addWidget(addedLoad, srow, 5);
  srow++;

  spinboxLayout->addWidget(new QLabel("v3:", this), srow, 0);
  spinV3 = new QDoubleSpinBox(this);
  spinV3->setRange(0.0, 250.0);
  spinV3->setSingleStep(1.0);
  spinV3->setButtonSymbols(QSpinBox::PlusMinus);
  spinV3->setSuffix( Speed::getHorizontalUnitText() );
  spinboxLayout->addWidget(spinV3, srow, 1);

  spinboxLayout->addWidget(new QLabel("w3:", this), srow, 2);
  spinW3 = new QDoubleSpinBox(this);

  if( Speed::getVerticalUnit() !=  Speed::feetPerMinute )
    {
      spinW3->setRange(-5.0, 0);
      spinW3->setSingleStep(0.01);
    }
  else
    {
      spinW3->setRange(-1000.0, 0);
      spinW3->setSingleStep(2.0);
    }

  spinW3->setButtonSymbols(QSpinBox::PlusMinus);
  spinW3->setSuffix( Speed::getVerticalUnitText() );
  spinboxLayout->addWidget(spinW3, srow, 3);

  spinboxLayout->addWidget(new QLabel(tr("Max. water:"), this), srow, 4);
  spinWater = new QSpinBox(this);
  spinWater->setObjectName("spinWater");
  spinWater->setMaximum(300);
  spinWater->setSingleStep(5);
  spinWater->setButtonSymbols(QSpinBox::PlusMinus);
  spinWater->setSuffix("l");
  spinboxLayout->addWidget(spinWater, srow, 5);

  itemsLayout->addLayout(spinboxLayout, row, 0, 1, 4);
  row++;

  itemsLayout->setRowMinimumHeight(row++, 10);
  itemsLayout->setColumnStretch(1, 10);

  buttonShow = new QPushButton(tr("Show Polar"), this);
  itemsLayout->addWidget(buttonShow, row, 3);

  connect (comboType, SIGNAL(activated(const QString&)),
           this, SLOT(slotActivated(const QString&)));

  connect(buttonShow, SIGNAL(clicked()), this, SLOT(slotButtonShow()));

  itemArea->setWidget(itemWidget);

  // Add ok and cancel buttons
  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(26, 26));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(26, 26));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(cancel, 2);
  buttonBox->addSpacing(20);
  buttonBox->addWidget(ok, 2);
  buttonBox->addStretch(2);

  topLayout->addWidget(itemArea);
  topLayout->addLayout(buttonBox);

  if (isNew)
    {
      readPolarData();
    }
  else
    {
      load();
    }

  show();
}

GilderEditor::~GilderEditor()
{
}

Polar* GilderEditor::getPolar()
{
  int pos = comboType->currentIndex();

  if ((pos >= 0) && (pos < _polars.count()))
    {
      return &(_polars[pos]);
    }
  else
    {
      return static_cast<Polar *> (0);
    }
}

// just a helper function
void setCurrentText(QComboBox* combo, const QString& text)
{
  int idx = combo->findText(text);

  if (idx != -1)
    {
      combo->setCurrentIndex(idx);
    }
}

/** Called to initiate loading of the configuration file. */
void GilderEditor::load()
{
  if (_glider)
    {
      setCurrentText(comboType, _glider->type());
      comboType->setEditText(_glider->type());
      edtGReg->setText(_glider->registration());
      edtGCall->setText(_glider->callSign());

      spinWater->setValue(_glider->maxWater());

      if (_glider->seats() == Glider::doubleSeater)
        {
          comboSeats->setCurrentIndex(1);
        }
      else
        {
          comboSeats->setCurrentIndex(0);
        }

      spinV1->setValue( _glider->polar()->v1().getHorizontalValue() );
      spinV2->setValue( _glider->polar()->v2().getHorizontalValue() );
      spinV3->setValue( _glider->polar()->v3().getHorizontalValue() );

      spinW1->setValue( _glider->polar()->w1().getVerticalValue() );
      spinW2->setValue( _glider->polar()->w2().getVerticalValue() );
      spinW3->setValue( _glider->polar()->w3().getVerticalValue() );

      emptyWeight->setValue((int) _glider->polar()->emptyWeight());
      double load = _glider->polar()->grossWeight() -
                    _glider->polar()->emptyWeight();

      addedLoad->setValue((int) load);

      // Save load values to avoid rounding errors during save
      currV1 = spinV1->value();
      currV2 = spinV2->value();
      currV3 = spinV3->value();

      currW1 = spinW1->value();
      currW1 = spinW2->value();
      currW1 = spinW3->value();
    }
}

/** called to initiate saving to the configuration file */
void GilderEditor::save()
{
  if( !_glider )
    {
      _glider = new Glider;
    }

  _glider->setType(comboType->currentText().trimmed());
  _glider->setRegistration(edtGReg->text().trimmed());
  _glider->setCallSign(edtGCall->text().trimmed());
  _glider->setMaxWater(spinWater->value());

  if (comboSeats->currentIndex() == 1)
    {
      _glider->setSeats(Glider::doubleSeater);
    }
  else
    {
      _glider->setSeats(Glider::singleSeater);
    }

  Speed V1, V2, V3, W1, W2, W3;

  // save the current set units, maybe changed in the meantime.
  Speed::speedUnit hSpeedUnit = Speed::getHorizontalUnit();
  Speed::speedUnit vSpeedUnit = Speed::getVerticalUnit();

  // set units valid during load of speeds
  Speed::setHorizontalUnit( currHSpeedUnit );
  Speed::setVerticalUnit( currVSpeedUnit );

  // Save values only, if they were changed to avoid rounding errors.
  if( spinV1->value() != currV1 )
    {
      V1.setHorizontalValue(spinV1->value());
    }
  else
    {
      V1.setHorizontalValue( _glider->polar()->v1().getHorizontalValue() );
    }

  if( spinV2->value() != currV2 )
    {
       V2.setHorizontalValue(spinV2->value());
    }
  else
    {
      V2.setHorizontalValue( _glider->polar()->v2().getHorizontalValue() );
    }

  if( spinV3->value() != currV3 )
     {
        V3.setHorizontalValue(spinV3->value());
     }
  else
    {
      V3.setHorizontalValue( _glider->polar()->v3().getHorizontalValue() );
    }

  if( spinW1->value() != currW1 )
     {
        W1.setVerticalValue(spinW1->value());
     }
  else
    {
      W1.setVerticalValue( _glider->polar()->w1().getVerticalValue() );
    }

  if( spinW2->value() != currW2 )
     {
        W2.setVerticalValue(spinW2->value());
     }
  else
    {
      W2.setVerticalValue( _glider->polar()->w2().getVerticalValue() );
    }

  if( spinW3->value() != currW3 )
     {
        W3.setVerticalValue(spinW3->value());
     }
  else
    {
      W3.setVerticalValue( _glider->polar()->w3().getVerticalValue() );
    }

  // restore current speed units.
  Speed::setHorizontalUnit( hSpeedUnit );
  Speed::setVerticalUnit( vSpeedUnit );

  // qDebug("_polar->emptyWeight() %f  _polar->grossWeight() %f",
  //         (float)_glider->polar()->emptyWeight(),  (float)_glider->polar()->grossWeight() );
  Polar polar( _glider->type(), V1, W1, V2, W2, V3, W3, 0.0, 0.0,
               emptyWeight->value(), emptyWeight->value() + addedLoad->value() );

  _glider->setPolar( polar );

  // emit glider object to GliderListWidget
  if (isNew)
    {
      emit
      newGlider(_glider);
    }
  else
    {
      emit
      editedGlider(_glider);
    }

  isNew = false;
  // ownership of glider object is passed to GliderListWidget
  gliderCreated = false;
}

void GilderEditor::readPolarData()
{
  // qDebug ("GilderEditor::readPolarData ");

#warning "location of glider.pol file is CUMULUS_ROOT/etc"

  QStringList dirs;

  dirs << GeneralConfig::instance()->getInstallRoot() + "/etc/glider.pol"
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

  if (file.open(QIODevice::ReadOnly))
    {
      while (!stream.atEnd())
        {
          QString line = stream.readLine().trimmed();
          // ignore comments
          if (line[0] == '#')
            {
              continue;
            }

          QStringList list = line.split(",", QString::KeepEmptyParts);
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

          Polar polar = Polar( glidertype, v1, w1, v2, w2, v3, w3,
                               wingload, wingarea, emptyMass, emptyMass );

          if (list.count() >= 12)
            {
              polar.setMaxWater(list[11].toInt());
            }
          else
            {
              polar.setMaxWater(0);
            }

          if (list.count() >= 13)
            {
              polar.setSeats(list[12].toInt());
            }
          else
            {
              polar.setSeats(1);
            }

          _polars.append( polar );
        }

      if( _polars.size() )
        {
          // sort polar data list according to their names
          qSort( _polars.begin(), _polars.end(), Polar::lessThan );

          for( int i = 0; i < _polars.size(); i++ )
            {
              comboType->addItem( _polars[i].name() );
            }
        }

      QString firstGlider = comboType->itemText(0);
      slotActivated(firstGlider);
    }
  else
    {
      _globalMapView->message(tr("Missing polar file"));
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
              _polars.append(pol);
              break;
            }
        }
    }

#endif

}

/** called when a glider type has been selected */
void GilderEditor::slotActivated(const QString& type)
{
  // qDebug ("GilderEditor::slotActivated(%s)", type.toLatin1().data());
  if( !_glider )
    {
      _glider = new Glider();
      gliderCreated = true;
    }

  _glider->setType( type );

  _polar = getPolar();

  if( _polar )
    {
      _glider->setPolar( *_polar );

      spinV1->setValue( _polar->v1().getHorizontalValue() );
      spinW1->setValue( _polar->w1().getVerticalValue() );
      spinV2->setValue( _polar->v2().getHorizontalValue() );
      spinW2->setValue( _polar->w2().getVerticalValue() );
      spinV3->setValue( _polar->v3().getHorizontalValue() );
      spinW3->setValue( _polar->w3().getVerticalValue() );

      emptyWeight->setValue( (int) _polar->emptyWeight() );
      double load = _polar->grossWeight() - _polar->emptyWeight();
      addedLoad->setValue( (int) load );
      spinWater->setValue( _polar->maxWater() );

      if( _polar->seats() == 2 )
        {
          comboSeats->setCurrentIndex( 1 );
        }
      else
        {
          comboSeats->setCurrentIndex( 0 );
        }
    }
}

void GilderEditor::slotButtonShow()
{
  // we create a polar object on the fly to allow test of changed polar values without saving
  Speed V1, V2, V3, W1, W2, W3;
  V1.setHorizontalValue(spinV1->value());
  V2.setHorizontalValue(spinV2->value());
  V3.setHorizontalValue(spinV3->value());

  W1.setVerticalValue(spinW1->value());
  W2.setVerticalValue(spinW2->value());
  W3.setVerticalValue(spinW3->value());

  Polar polar( comboType->currentText(),
               V1, W1, V2, W2, V3, W3, 0.0, 0.0,
               emptyWeight->value(),
               emptyWeight->value() + addedLoad->value() );

  polar.setWater(spinWater->value(), 0);
  PolarDialog* dlg = new PolarDialog( polar, this );
  dlg->setVisible(true);
}

void GilderEditor::accept()
{
  edtGReg->setText(edtGReg->text().trimmed()); //remove spaces

  if (edtGReg->text().isEmpty())
    {
      QMessageBox::critical(this, tr("Missing registration!"),
          tr("Please enter a valid registration."));
    }
  else if (comboType->currentText().trimmed().isEmpty())
    {
      QMessageBox::critical(this, tr("Missing glider type!"),
          tr("Please enter a valid glider type."));
    }
  else
    {
      save();
      QDialog::accept();
    }
}

void GilderEditor::reject()
{
  // @AP: delete glider, if it was allocated in this class and not
  // emitted in accept method to avoid a memory leak.
  if (gliderCreated && _glider)
    {
      delete _glider;
      _glider = 0;
    }

  QDialog::reject();
}
