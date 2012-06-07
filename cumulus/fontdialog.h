/***********************************************************************
**
**   fontdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class FontDialog
 *
 * \author Axel Pauli
 *
 * \brief This dialog is a user interface for the font selection.
 *
 * This dialog is a user interface for the font selection.
 *
 * \date 2012
 *
 * \version $Id$
 */

#ifndef FONT_DIALOG_H_
#define FONT_DIALOG_H_

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

class FontDialog : public QDialog
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( FontDialog )

 public:

  FontDialog(QWidget *parent);

  virtual ~FontDialog() {};

 private slots:
  /**
   * This slot is called, if the user has pressed the ok button.
   */
  void slotAccept();

  /**
   * This slot is called, if the user has pressed the cancel button.
   */
  void slotReject();

  /**
   * This slot is called, if the user has selected a font entry in the list.
   */
  void slotFontListClicked( QListWidgetItem* item );

  /**
   * This slot is called, if the user has selected a style entry in the list.
   */
  void slotStyleListClicked( QListWidgetItem* item );

  /**
   * This slot is called, if the user has selected a size entry in the list.
   */
  void slotSizeListClicked( QListWidgetItem* item );

  /**
   * This slot is called if the font has been changed.
   */
  void slotUpdateSampleText();

 private:

  void selectItem( QListWidget* listWidget, QString text );

  QLineEdit* fontLabel;
  QLineEdit* styleLabel;
  QLineEdit* sizeLabel;
  QLineEdit* sampleText;

  QListWidget* fontList;
  QListWidget* styleList;
  QListWidget* sizeList;

};


#endif /* FONT_DIALOG_H_ */
