/***********************************************************************
**
**   settingspageairspacefilling.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2009-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SettingsPageAirSpaceFilling_h
#define SettingsPageAirSpaceFilling_h

#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>

//----------------------------SettingsPageAirspaceFilling-----------------------

/**
 * \class SettingsPageAirspaceFilling
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief Configuration settings for airspace fillings.
 *
 * \date 2002-2013
 *
 * \version $Id$
 *
 */
class SettingsPageAirspaceFilling: public QDialog
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SettingsPageAirspaceFilling )

 public:

  SettingsPageAirspaceFilling( QWidget *parent=0 );

  virtual ~SettingsPageAirspaceFilling();

 public slots:

  /**
   * Called to initiate saving to the configuration file.
   */
  void slot_save();

  /**
   * Called to initiate loading of the configuration file
   */
  void slot_load();

  /**
   * Called to set all spinboxes to the default value
   */
  void slot_defaults();

  /**
   * Called to reset all spinboxes to zero
   */
  void slot_reset();

 private slots:

  /**
   * Invoked if enableFilling changes value
   * @param enabled true if filling is enabled
   */
  void slot_enabledToggled(bool enabled);

  /**
   * Called to change the step width of the spin boxes
   */
  void slot_change(int newStep);

  /**
  * This slot increments the value in the spin box which has the current focus.
  */
  void slotIncrementBox();

  /**
  * This slot decrements the value in the spin box which has the current focus.
  */
  void slotDecrementBox();

 private:

  QCheckBox* enableFilling;

  QRadioButton* s1;
  QRadioButton* s2;
  QRadioButton* s3;
  QRadioButton* s4;

#ifndef MAEMO5
  QGroupBox* separations;
#else
  QWidget* separations;
#endif

  QSpinBox*  verticalNotNear;
  QSpinBox*  verticalNear;
  QSpinBox*  verticalVeryNear;
  QSpinBox*  verticalInside;
  QSpinBox*  lateralNotNear;
  QSpinBox*  lateralNear;
  QSpinBox*  lateralVeryNear;
  QSpinBox*  lateralInside;

  QPushButton *plus;
  QPushButton *minus;
  QPushButton *reset;
  QPushButton *defaults;

  /** Auto sip flag storage. */
  bool m_autoSip;
};

#endif /* SettingsPageAirSpaceFilling_h */
