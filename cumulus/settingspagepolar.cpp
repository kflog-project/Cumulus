/***********************************************************************
 **
 **   settingspagepolar.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Eggert Ehmke, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QRadioButton>
#include <QMessageBox>
#include <QString>
#include <QDialogButtonBox>

#include "polardialog.h"
#include "settingspagepolar.h"
#include "polar.h"
#include "speed.h"
#include "mapview.h"
#include "generalconfig.h"

extern MapView *_globalMapView;

SettingsPagePolar::SettingsPagePolar(QWidget *parent, Glider *glider )
  : QDialog(parent)
{
  setObjectName("SettingsPagePolar");
  setModal(false);

  if (glider==0) {
    setWindowTitle(tr("New Glider"));
    isNew = true;
  } else {
    setWindowTitle(tr("Edit Glider"));
    isNew = false;
  }
  _glider=glider;

  QGridLayout* topLayout = new QGridLayout(this);
  int row=0;

  topLayout->addWidget( new QLabel(tr("Glider type:"), this), row, 0 );
  comboType = new QComboBox(this);
  comboType->setEditable(true);

  topLayout->addWidget( comboType, row, 1, 1, 2 );
  topLayout->setRowMinimumHeight( row++, 10 );

  bgSeats=new QGroupBox(tr("Seats"),this);
  seatsOne=new QRadioButton(tr("Single"),bgSeats);
  seatsTwo=new QRadioButton(tr("Double"),bgSeats);
  seatsOne->setChecked(true);
  bgSeats->hide();

  topLayout->addWidget(new QLabel(tr("Seats:"), this), row, 0 );
  QBoxLayout* seats_l=new QHBoxLayout;
  topLayout->addLayout(seats_l, row, 1, 1, 2 );
  row++;
  seats_l->addWidget(seatsOne);
  seats_l->addWidget(seatsTwo);

  topLayout->addWidget( new QLabel(tr("Registration:"), this), row, 0 );
  edtGReg = new QLineEdit(this);
  topLayout->addWidget(edtGReg, row, 1, 1, 2 );
  row++;

  topLayout->addWidget( new QLabel(tr("Callsign:"), this), row, 0 );
  edtGCall = new QLineEdit(this);
  topLayout->addWidget( edtGCall, row, 1, 1, 2 );
  row++;

  topLayout->addWidget(new QLabel(tr("v1 / w1:"), this),row,0);
  spinV1 = new QDoubleSpinBox(this);
  spinV1->setRange(0.0, 150.0);
  spinV1->setSingleStep(1.0);
  spinV1->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(spinV1,row,1);
  topLayout->addWidget(new QLabel(tr("km/h"), this),row,2);

  spinW1 = new QDoubleSpinBox(this);
  spinW1->setRange(-5.0, 0);
  spinW1->setSingleStep(0.01);
  spinW1->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(spinW1,row,3);
  topLayout->addWidget(new QLabel(tr("m/s"), this),row++,4);

  topLayout->addWidget(new QLabel(tr("v2 / w2:"), this),row,0);
  spinV2 = new QDoubleSpinBox(this);
  spinV2->setRange(0.0, 200.0);
  spinV2->setSingleStep(1.0);
  spinV2->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(spinV2,row,1);
  topLayout->addWidget(new QLabel(tr("km/h"), this),row,2);

  spinW2 = new QDoubleSpinBox(this);
  spinW2->setRange(-5.0, 0);
  spinW2->setSingleStep(0.01);
  spinW2->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(spinW2,row,3);
  topLayout->addWidget(new QLabel(tr("m/s"), this),row++,4);

  topLayout->addWidget(new QLabel(tr("v3 / w3:"), this),row,0);
  spinV3 = new QDoubleSpinBox(this);
  spinV3->setRange(0.0, 250.0);
  spinV3->setSingleStep(1.0);
  spinV3->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(spinV3,row,1);
  topLayout->addWidget(new QLabel(tr("km/h"), this),row,2);
  spinW3 = new QDoubleSpinBox(this);
  spinW3->setRange(-5.0, 0);
  spinW3->setSingleStep(0.01);
  spinW3->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(spinW3,row,3);
  topLayout->addWidget(new QLabel(tr("m/s"), this),row++,4);

  topLayout->addWidget(new QLabel (tr("Empty weight:"), this), row, 0);
  emptyWeight = new QSpinBox (this);
  emptyWeight->setObjectName("emptyWeight");
  emptyWeight->setMaximum(1000);
  emptyWeight->setSingleStep(5);
  emptyWeight->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(emptyWeight,row,1);
  topLayout->addWidget(new QLabel(tr("kg"), this),row++,2);

  topLayout->addWidget(new QLabel (tr("Added load:"), this), row, 0);
  addedLoad = new QSpinBox (this);
  addedLoad->setObjectName("addedLoad");
  addedLoad->setMaximum(1000);
  addedLoad->setSingleStep(5);
  addedLoad->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(addedLoad,row,1);
  topLayout->addWidget(new QLabel(tr("kg"), this),row++,2);

  topLayout->addWidget(new QLabel (tr("Max. water:"), this), row, 0);
  spinWater = new QSpinBox (this);
  spinWater->setObjectName("spinWater");
  spinWater->setMaximum(300);
  spinWater->setSingleStep(5);
  spinWater->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget (spinWater, row, 1);
  topLayout->addWidget(new QLabel(tr("l"),this),row++,2);
  
  buttonShow = new QPushButton (tr("Show Polar"), this);

  QDialogButtonBox* buttonBox = new QDialogButtonBox( Qt::Horizontal );

  // Add show, ok and cancel buttons
  buttonBox->addButton( buttonShow, QDialogButtonBox::ActionRole );
  buttonBox->addButton( QDialogButtonBox::Ok );
  buttonBox->addButton( QDialogButtonBox::Cancel );

  topLayout->addWidget( buttonBox, row, 0, 1, 4, Qt::AlignRight );

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  topLayout->setColumnStretch (0, 10);
  topLayout->setColumnStretch (1, 20);
  topLayout->setColumnStretch (2, 10);

  connect (comboType, SIGNAL(activated(const QString&)),
           this, SLOT(slotActivated(const QString&)));
  connect (buttonShow, SIGNAL(clicked()),
           this, SLOT(slotButtonShow()));

  if (isNew)
    {
      readPolarData();
    }
  else
    {
      slot_load();
    }

  show();
}


SettingsPagePolar::~SettingsPagePolar()
{
   while (!_polars.isEmpty())
     delete _polars.takeFirst();	
}


Polar* SettingsPagePolar::getPolar()
{
  int pos = comboType->currentIndex();

  if ((pos >= 0) && (pos < _polars.count()))
    return _polars.at(pos);
  else
    return static_cast<Polar *> (0);
}


// just a helper function in Qt2 / Qtopia
void setCurrentText (QComboBox* combo, const QString& text)
{
  for (int i = 0; i < combo->count(); i++) {
    if (combo->itemText(i) == text) {
      combo->setCurrentIndex(i);
      return;
    }
  }
}


/** Called to initiate loading of the configuration file. */
void SettingsPagePolar::slot_load()
{
  if (_glider) {
    setCurrentText (comboType, _glider->type());
    comboType->setEditText(_glider->type());
    edtGReg->setText(_glider->registration());
    edtGCall->setText(_glider->callsign());

    spinWater->setValue(_glider->maxWater());

    if (_glider->seats() == Glider::doubleSeater)
      seatsTwo->setChecked(true);
    else
      seatsOne->setChecked(true);

    spinV1->setValue(_glider->polar()->v1().getKph());
    spinV2->setValue(_glider->polar()->v2().getKph());
    spinV3->setValue(_glider->polar()->v3().getKph());

    spinW1->setValue(_glider->polar()->w1().getMps());
    spinW2->setValue(_glider->polar()->w2().getMps());
    spinW3->setValue(_glider->polar()->w3().getMps());

    emptyWeight->setValue((int) _glider->polar()->emptyWeight());
    double load = _glider->polar()->grossWeight() - _glider->polar()->emptyWeight();
    addedLoad->setValue((int) load);
  }
}


/** called to initiate saving to the configuration file */
void SettingsPagePolar::slot_save()
{
  if(!_glider) {
    _glider=new Glider;
  }

  _glider->setType(comboType->currentText().trimmed());
  _glider->setRegistration(edtGReg->text().trimmed());
  _glider->setCallsign(edtGCall->text().trimmed());
  _glider->setMaxWater(spinWater->value());

  if (seatsTwo->isChecked())
    _glider->setSeats(Glider::doubleSeater);
  else
    _glider->setSeats(Glider::singleSeater);

  Speed V1, V2, V3, W1, W2, W3;
  V1.setKph(spinV1->value());
  V2.setKph(spinV2->value());
  V3.setKph(spinV3->value());

  W1.setMps(spinW1->value());
  W2.setMps(spinW2->value());
  W3.setMps(spinW3->value());
  // qDebug("_polar->emptyWeight() %f  _polar->grossWeight() %f",
  //         (float)_glider->polar()->emptyWeight(),  (float)_glider->polar()->grossWeight() );
  _glider->setPolar(new Polar( _glider, _glider->type(),
                               V1, W1, V2, W2, V3, W3,
                               0.0, 0.0,
                               emptyWeight->value(),
                               emptyWeight->value() + addedLoad->value() ));
  if (isNew) {
    emit newGlider(_glider);
  } else {
    emit editedGlider(_glider);
  }
  isNew = false;
}


void SettingsPagePolar::readPolarData ()
{
  // qDebug ("SettingsPagePolar::readPolarData ");

  // @AP: After a discussion with Michael Lauer from the Opie team, we
  // agreed, that an ipkg package should nothing install on a $HOME
  // directory. Common files should be installed under
  // /opt/QtPalmtop/..., therefore I changed the position for
  // glider.pol file under Opie.

#warning location of glider.pol file is CUMULUS_ROOT/etc

  QString path1( GeneralConfig::instance()->getInstallRoot() + "/etc/glider.pol" ); // Linux X11
  QString path2( "/opt/QtPalmtop/lib/cumulus/glider.pol");   // Opie
  QString path3 (GeneralConfig::instance()->getUserDataDirectory() + "/glider.pol"); // Linux Desktop

  QFile file(path1);

  if( ! file.exists() ) {
    // try path2
    file.setFileName( path2 );
    if( ! file.exists() ) {
      // try path3
      file.setFileName( path3 );
    }
  }

  QTextStream stream(&file);
  Polar* pol;

  if (file.open(QIODevice::ReadOnly)) {
    while (!stream.atEnd()) {
      QString line = stream.readLine();
      // ignore comments
      if (line[0] == '#')
        continue;
      QStringList list = line.split( ",", QString::KeepEmptyParts );
      QString glidertype = list[0];
      comboType->addItem (glidertype);

      // the sink values are positive in this file; we need them negative
      Speed v1,w1,v2,w2,v3,w3;
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

      pol=new Polar(this, glidertype,v1,w1,v2,w2,v3,w3,
                    wingload,wingarea,emptyMass,emptyMass);
      if (list.count()>=12) {
        pol->setMaxWater(list[11].toInt());
      } else {
        pol->setMaxWater(0);
      }
      if (list.count()>=13) {
        pol->setSeats(list[12].toInt());
      }
      else
        pol->setSeats(1);
      _polars.append(pol);
    }
    QString firstGlider = comboType->itemText(0);
    slotActivated (firstGlider);
  } else {
    _globalMapView->message( tr("Missing polar file") );
    qWarning ("Could not open polar file: %s", file.fileName().toLatin1().data());
  }

#if 0

  // try to read more polars from document folder
  // these files are in winpilot format: one file per polar
  DocLnkSet docs;
  Global::findDocuments(&docs, "cumulus/polar");
  QList<DocLnk*> list = docs.children();

  // qDebug ("found %d polars in document folder", list.count());
  for (DocLnk* doc = list.first(); doc; doc = list.next()) {
    QString path = doc->file();
    // qDebug ("Doc: %s", path.toLatin1().data());
    QFile file (path);
    QTextStream stream(&file);
    if (file.open(IO_ReadOnly)) {
      QString glidertype = QFileInfo(file).baseName();
      while (!stream.eof()) {
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
void SettingsPagePolar::slotActivated(const QString& type)
{
  // qDebug ("SettingsPagePolar::slotActivated(%s)", type.toLatin1().data());

  if(!_glider) {
    _glider=new Glider();
  }

  _polar = getPolar();

  if (_polar) {
    spinV1->setValue (_polar->v1().getKph());
    spinW1->setValue (_polar->w1().getMps());
    spinV2->setValue (_polar->v2().getKph());
    spinW2->setValue (_polar->w2().getMps());
    spinV3->setValue (_polar->v3().getKph());
    spinW3->setValue (_polar->w3().getMps());

    emptyWeight->setValue((int) _polar->emptyWeight());
    double load = _polar->grossWeight() - _polar->emptyWeight();
    addedLoad->setValue((int) load);
    spinWater->setValue(_polar->maxWater());

    if (_polar->seats() == 2) {
      seatsTwo->setChecked(true);
    }
    else {
      seatsOne->setChecked(true);
    }
  }

  // @AP: make a deep copy of polare, because setPolar will remove the
  // before passed polare
  _glider->setPolar( new Polar(*_polar));
  _glider->setType (type);
}


void SettingsPagePolar::slotButtonShow()
{
  // we create a polar object on the fly to allow test of changed polar values without saving
  Speed V1, V2, V3, W1, W2, W3;
  V1.setKph(spinV1->value());
  V2.setKph(spinV2->value());
  V3.setKph(spinV3->value());

  W1.setMps(spinW1->value());
  W2.setMps(spinW2->value());
  W3.setMps(spinW3->value());

  Polar* polar = new Polar(this,comboType->currentText(),
                           V1, W1, V2, W2, V3, W3,
                           0.0, 0.0,
                           emptyWeight->value(),
                           emptyWeight->value() + addedLoad->value() );

  // polar->setWater(0, 0);
  polar->setWater(spinWater->value(), 0);
  PolarDialog* dlg = new PolarDialog (polar, this);
  dlg->show();
}


void SettingsPagePolar::accept()
{
  edtGReg->setText(edtGReg->text().trimmed()); //remove spaces

  if( edtGReg->text().isEmpty() ) {
    QMessageBox::warning(this,tr("Missing registration!"),
                         tr("<br>Please enter a valid registration.</qt>"));
  } else if( comboType->currentText().trimmed().isEmpty() ) {
    QMessageBox::warning(this,tr("Missing glider type!"),
                         tr("<br>Please enter a valid glider type.</qt>"));
  } else {
    slot_save();
    QDialog::accept();
  }
}
