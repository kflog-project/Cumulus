/***********************************************************************
**
**   fontdialog.cpp
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

#include <QtGui>

#include "fontdialog.h"
#include "layout.h"
#include "mainwindow.h"

FontDialog::FontDialog (QWidget *parent) :
  QDialog(parent, Qt::WindowStaysOnTopHint)
{
  setObjectName("FontDialog");
  // setAttribute(Qt::WA_DeleteOnClose);
  setModal(true);
  setWindowTitle(tr("Select Font"));

  fontLabel = new QLineEdit;
  fontLabel->setReadOnly(true);

  styleLabel = new QLineEdit;
  styleLabel->setReadOnly(true);

  sizeLabel = new QLineEdit;
  sizeLabel->setReadOnly(true);

  sampleText = new QLineEdit;
  sampleText->setReadOnly(true);
  sampleText->setMinimumHeight(80);
  sampleText->setAlignment(Qt::AlignCenter);
  sampleText->setText(QLatin1String("AaBbYyZz"));

  fontList  = new QListWidget;
  styleList = new QListWidget;
  sizeList  = new QListWidget;

  connect( fontList, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(slotFontListClicked(QListWidgetItem *)));
  connect( styleList, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(slotStyleListClicked(QListWidgetItem *)));
  connect( sizeList, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(slotSizeListClicked(QListWidgetItem *)));
  connect( sizeList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(slotSizeListClicked(QListWidgetItem *)));

  QGridLayout* gl = new QGridLayout(this);
  gl->setMargin(10);
  gl->setSpacing(10);
  gl->setColumnStretch(0, 30);
  gl->setColumnStretch(1, 10);
  gl->setColumnStretch(2, 5);

  gl->addWidget( new QLabel(tr("Font")), 0, 0 );
  gl->addWidget( new QLabel(tr("Style")), 0, 1 );
  gl->addWidget( new QLabel(tr("Size")), 0, 2 );

  gl->addWidget( fontLabel, 1, 0 );
  gl->addWidget( styleLabel, 1, 1 );
  gl->addWidget( sizeLabel, 1, 2 );

  gl->addWidget( fontList, 2, 0 );
  gl->addWidget( styleList, 2, 1 );
  gl->addWidget( sizeList, 2, 2 );

  QGroupBox* gBox = new QGroupBox(tr("Sample"));
  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(sampleText);
  gBox->setLayout(vbox);
  gl->addWidget( gBox, 3, 0, 1, 3 );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel |
                                                         QDialogButtonBox::Ok );
  gl->addWidget( buttonBox, 4, 0, 1, 3 );

  QPushButton *ok = buttonBox->button( QDialogButtonBox::Ok );
  ok->setDefault( true );

  QPushButton *cancel = buttonBox->button( QDialogButtonBox::Cancel );
  cancel->setAutoDefault(false);

  connect( buttonBox, SIGNAL(accepted()), this, SLOT(accept()) );
  connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()) );

  QStringList fdl = fdb.families( QFontDatabase::Latin );
  fontList->addItems( fdl );

  fdl.append(tr("Font"));
  int w = Layout::maxTextWidth( fdl, font() );
  fontLabel->setMaximumWidth( w + 30 );
  fontList->setMaximumWidth( w + 30 );

  fdl.clear();
  fdl.append(tr("Size"));
  w = Layout::maxTextWidth( fdl, font() );
  sizeLabel->setMaximumWidth( w + 30 );
  sizeList->setMaximumWidth( w + 30 );

  // select the default application font
  selectFont(font());
}

/**
 * This slot is called, if the user has selected a font entry in the list.
 */
void FontDialog::slotFontListClicked( QListWidgetItem* item )
{
  if( item == 0 )
    {
      return;
    }

  int sizeCurrentRow = sizeList->currentRow();

  fontLabel->setText(item->text());
  styleLabel->clear();
  sizeLabel->clear();
  styleList->clear();
  sizeList->clear();

  // Update now the style and size lists.
  QStringList fstl = fdb.styles( item->text() );

  if( fstl.isEmpty() )
    {
      return;
    }

  styleList->addItems(fstl);
  styleList->setCurrentRow( 0 );
  styleLabel->setText( fstl.at(0) );

  QList<int> fsl = fdb.pointSizes ( item->text(), fstl.at(0) );

  if( fsl.isEmpty() )
    {
      return;
    }

  for( int i = 0; i < fsl.size(); i++ )
    {
      sizeList->addItem( QString::number( fsl.at(i)));
    }

  sizeList->setCurrentRow( sizeCurrentRow );

  QListWidgetItem *nItem = sizeList->currentItem();

  if( nItem )
    {
      sizeLabel->setText( nItem->text() );
    }

  updateSampleText();
}

/**
 * This slot is called, if the user has selected a style entry in the list.
 */
void FontDialog::slotStyleListClicked( QListWidgetItem* item )
{
  if( item == 0 )
    {
      return;
    }

  int sizeCurrentRow = sizeList->currentRow();
  styleLabel->setText(item->text());
  sizeLabel->clear();
  sizeList->clear();

  QList<int> fsl = fdb.pointSizes ( fontLabel->text(), item->text() );

  if( fsl.isEmpty() )
    {
      return;
    }

  for( int i = 0; i < fsl.size(); i++ )
    {
      sizeList->addItem( QString::number( fsl.at(i)));
    }

  sizeList->setCurrentRow( sizeCurrentRow );

  QListWidgetItem *nItem = sizeList->currentItem();

  if( nItem )
    {
      sizeLabel->setText( nItem->text() );
    }

  updateSampleText();
}

/**
 * This slot is called, if the user has selected a size entry in the list.
 */
void FontDialog::slotSizeListClicked( QListWidgetItem* item )
{
  if( item == 0 )
    {
      return;
    }

  sizeLabel->setText( item->text() );
  updateSampleText();
}

void FontDialog::updateSampleText()
{
  if( fontLabel->text().isEmpty() || styleLabel->text().isEmpty() || sizeLabel->text().isEmpty() )
    {
      sampleText->clear();
      return;
    }

  // compute new font
  int pSize = sizeLabel->text().toInt();

  QFont newFont(fdb.font(fontLabel->text(), styleLabel->text(), pSize));

  if( newFont != sampleText->font() )
    {
      sampleText->setFont(newFont);
    }

  sampleText->setText("AaBbYyZz");
}

void FontDialog::selectFont( const QFont& font )
{
  QString family = font.family();
  QString style = fdb.styleString( font );
  QString pSize = QString::number( font.pointSize() );

  if( selectItem( fontList, family ) )
    {
      slotFontListClicked( fontList->currentItem() );
    }
  else
    {
      fontLabel->clear();
    }

  if( selectItem( styleList, style ) )
    {
      styleLabel->setText(style);
    }
  else
    {
      styleLabel->clear();
    }

  if( selectItem( sizeList, pSize ) )
    {
      sizeLabel->setText(pSize);
    }
  else
    {
      sizeLabel->clear();
    }

  updateSampleText();
}

bool FontDialog::selectItem( QListWidget* listWidget, QString text )
{
  for( int i=0; i < listWidget->count(); i++ )
    {
      if( listWidget->item(i)->text() == text )
        {
          listWidget->setCurrentRow( i );
          return true;
        }
    }

  return false;
}

QFont FontDialog::getFont( bool& ok, const QFont &initial, QWidget *parent, QString title )
{
  FontDialog dlg(parent);

  if( ! title.isEmpty() )
    {
      dlg.setWindowTitle( title );
    }

  dlg.selectFont(initial);

#ifdef ANDROID
  dlg.setVisible(true);
  dlg.resize( MainWindow::mainWindow()->size() );
#endif

  int ret = dlg.exec();


  if( ret == QDialog::Accepted )
    {
      ok = true;
      return dlg.currentFont();
    }
  else
    {
      ok = false;
      return initial;
    }
}
