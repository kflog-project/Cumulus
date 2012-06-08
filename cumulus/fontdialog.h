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
#include <QFont>
#include <QFontDatabase>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

class FontDialog : public QDialog
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( FontDialog )

  FontDialog(QWidget *parent);

  virtual ~FontDialog() {};

 public:

  /**
   * Returns the selected font by the user.
   */
  static QFont getFont( bool& ok, const QFont &initial, QWidget *parent, QString title="" );

 private slots:

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

 private:

  /**
   * Selects the item in the list which is identical to the passed text. The
   * assumption is, that all list items are unique.
   */
  bool selectItem( QListWidget* listWidget, QString text );

  /**
   * Do select the passed font in the lists.
   *
   * \param font Font to be selected in the lists.
   */
  void selectFont( const QFont& font );

  /**
   * \return The current selected font.
   */
  QFont currentFont() const
  {
    return sampleText->font();
  };

  /**
   * Updates the sample text.
   */
  void updateSampleText();

  QFontDatabase fdb;

  QLineEdit* fontLabel;
  QLineEdit* styleLabel;
  QLineEdit* sizeLabel;
  QLineEdit* sampleText;

  QListWidget* fontList;
  QListWidget* styleList;
  QListWidget* sizeList;
};

#endif /* FONT_DIALOG_H_ */
