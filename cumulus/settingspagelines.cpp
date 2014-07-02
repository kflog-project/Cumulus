/***********************************************************************
 **
 **   settingspagelines.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2013-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "colordialog.h"
#include "generalconfig.h"
#include "layout.h"
#include "map.h"
#include "numberEditor.h"
#include "rowdelegate.h"
#include "settingspagelines.h"

SettingsPageLines::SettingsPageLines(QWidget *parent) :
  QWidget( parent )
{
  setObjectName("SettingsPageLines");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Lines") );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  QVBoxLayout *vBox = new QVBoxLayout;
  contentLayout->addLayout(vBox);

  vBox->setMargin(3);

  m_drawOptions = new QTableWidget(6, 4);

  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();
  m_drawOptions->setItemDelegate( new RowDelegate( m_drawOptions, afMargin ) );

  // m_drawOptions->setShowGrid( false );

  m_drawOptions->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_drawOptions->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture( m_drawOptions->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( m_drawOptions->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  connect( m_drawOptions, SIGNAL(cellClicked ( int, int )),
           SLOT(slot_toggleCheckBox( int, int )));

  // hide vertical headers
  QHeaderView *vHeader = m_drawOptions->verticalHeader();
  vHeader->setVisible(false);

  QTableWidgetItem *item = new QTableWidgetItem( tr("Line") );
  m_drawOptions->setHorizontalHeaderItem( 0, item );

  item = new QTableWidgetItem( tr("Color") );
  m_drawOptions->setHorizontalHeaderItem( 1, item );

  item = new QTableWidgetItem( tr("Width") );
  m_drawOptions->setHorizontalHeaderItem( 2, item );

  item = new QTableWidgetItem( tr("View") );
  m_drawOptions->setHorizontalHeaderItem( 3, item );

  vBox->addWidget( m_drawOptions );

  m_cmdDefaults = new QPushButton(tr("Defaults"), this);
  vBox->addWidget( m_cmdDefaults, 0, Qt::AlignRight|Qt::AlignBottom );
  connect( m_cmdDefaults, SIGNAL(clicked()), this, SLOT(slot_setDefaults()) );

  int row = 0;
  int col = 0;

  m_headingLine = new QTableWidgetItem( tr("Heading") );
  m_headingLine->setFlags( Qt::ItemIsEnabled );
  m_drawOptions->setItem( row++, col, m_headingLine );

  m_pathLine = new QTableWidgetItem( tr("Path") );
  m_pathLine->setFlags( Qt::ItemIsEnabled );
  m_drawOptions->setItem( row++, col, m_pathLine );

  m_targetLine = new QTableWidgetItem( tr("Target") );
  m_targetLine->setFlags( Qt::ItemIsEnabled );
  m_drawOptions->setItem( row++, col, m_targetLine );

  m_trailLine = new QTableWidgetItem( tr("Trail") );
  m_trailLine->setFlags( Qt::ItemIsEnabled );
  m_drawOptions->setItem( row++, col, m_trailLine );

  m_taskFigures = new QTableWidgetItem( tr("Task Figures") );
  m_taskFigures->setFlags( Qt::ItemIsEnabled );
  m_drawOptions->setItem( row++, col, m_taskFigures );

  m_airspaceBorder = new QTableWidgetItem( tr("AS Border") );
  m_airspaceBorder->setFlags( Qt::ItemIsEnabled );
  m_drawOptions->setItem( row++, col, m_airspaceBorder );

  // next column is one
  row = 0;
  col = 1;

  // colors of lines
  m_headingLineColor = new QWidget();
  m_headingLineColor->setAutoFillBackground(true);
  m_headingLineColor->setBackgroundRole(QPalette::Window);
  m_drawOptions->setCellWidget( row++, col, m_headingLineColor );

  m_pathLineColor = new QWidget();
  m_pathLineColor->setAutoFillBackground(true);
  m_pathLineColor->setBackgroundRole(QPalette::Window);
  m_drawOptions->setCellWidget( row++, col, m_pathLineColor );

  QTableWidgetItem* liDummy = new QTableWidgetItem(tr("none"));
  liDummy->setFlags( Qt::NoItemFlags );
  liDummy->setTextAlignment(Qt::AlignCenter);
  m_drawOptions->setItem( row++, col, liDummy );

  m_trailLineColor = new QWidget();
  m_trailLineColor->setAutoFillBackground(true);
  m_trailLineColor->setBackgroundRole(QPalette::Window);
  m_drawOptions->setCellWidget( row++, col, m_trailLineColor );

  m_taskFiguresColor = new QWidget();
  m_taskFiguresColor->setAutoFillBackground(true);
  m_taskFiguresColor->setBackgroundRole(QPalette::Window);
  m_drawOptions->setCellWidget( row++, col, m_taskFiguresColor );

  liDummy = new QTableWidgetItem(tr("none"));
  liDummy->setFlags( Qt::NoItemFlags );
  liDummy->setTextAlignment(Qt::AlignCenter);
  m_drawOptions->setItem( row++, col, liDummy );

  // next column is two the line width
  row = 0;
  col = 2;

  m_headingLineWidth = createNumberEditor( this );
  m_pathLineWidth = createNumberEditor( this );
  m_targetLineWidth = createNumberEditor( this );
  m_trailLineWidth = createNumberEditor( this );
  m_taskFiguresLineWidth = createNumberEditor( this );
  m_airspaceBorderLineWidth = createNumberEditor( this );

  m_drawOptions->setCellWidget( row++, col, m_headingLineWidth );
  m_drawOptions->setCellWidget( row++, col, m_pathLineWidth );
  m_drawOptions->setCellWidget( row++, col, m_targetLineWidth );
  m_drawOptions->setCellWidget( row++, col, m_trailLineWidth );
  m_drawOptions->setCellWidget( row++, col, m_taskFiguresLineWidth );
  m_drawOptions->setCellWidget( row++, col, m_airspaceBorderLineWidth );

  connect( m_headingLineWidth, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_drawLineIcon0(const QString&)));
  connect( m_pathLineWidth, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_drawLineIcon1(const QString&)));
  connect( m_targetLineWidth, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_drawLineIcon2(const QString&)));
  connect( m_trailLineWidth, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_drawLineIcon3(const QString&)));
  connect( m_taskFiguresLineWidth, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_drawLineIcon4(const QString&)));
  connect( m_airspaceBorderLineWidth, SIGNAL(numberEdited(const QString&)),
           this, SLOT(slot_drawLineIcon5(const QString&)));

  // next column is three the line width picture
  row = 0;
  col = 3;

  for( int i = 0; i < 6; i++ )
    {
      QLabel* label = new QLabel;
      m_drawOptions->setCellWidget( row++, col, label );
    }

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

SettingsPageLines::~SettingsPageLines()
{
}

void SettingsPageLines::slotAccept()
{
  save();
  GeneralConfig::instance()->save();
  emit settingsChanged();
  QWidget::close();
}

void SettingsPageLines::slotReject()
{
  QWidget::close();
}

NumberEditor* SettingsPageLines::createNumberEditor( QWidget* parent )
{
  NumberEditor* ne = new NumberEditor( parent );
  ne->setFrameStyle( QFrame::NoFrame );
  ne->setLineWidth( 1 );
  ne->setMargin(0);
  ne->setDecimalVisible( false );
  ne->setPmVisible( false );
  ne->setMaxLength(2);
  ne->setSuffix( tr(" px") );
  ne->setRange( 1, 10 );
  ne->setTitle("1...10 Pixel");
  ne->setValue(4);
  ne->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  ne->setFixHeight( false );
  return ne;
}

void SettingsPageLines::showEvent( QShowEvent *event )
{
  // align all columns to contents before showing
  m_drawOptions->resizeColumnsToContents();
  m_drawOptions->resizeRowsToContents();
  m_drawOptions->setFocus();

  QWidget::showEvent( event );
}

void SettingsPageLines::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  m_headingLine->setCheckState( conf->getHeadingLineDrawState() ? Qt::Checked : Qt::Unchecked );
  m_pathLine->setCheckState( Qt::Checked );
  m_targetLine->setCheckState( conf->getTargetLineDrawState() ? Qt::Checked : Qt::Unchecked );
  m_trailLine->setCheckState( conf->getMapDrawTrail() ? Qt::Checked : Qt::Unchecked );
  m_taskFigures->setCheckState( Qt::Checked );
  m_airspaceBorder->setCheckState( Qt::Checked );

  m_headingLineColor->setPalette( QPalette(conf->getHeadingLineColor()));
  m_pathLineColor->setPalette( QPalette(conf->getTaskLineColor()));
  m_trailLineColor->setPalette( QPalette(conf->getMapTrailColor()));
  m_taskFiguresColor->setPalette( QPalette(conf->getTaskFiguresColor()));

  m_headingLineWidth->setValue( conf->getHeadingLineWidth() );
  m_pathLineWidth->setValue( conf->getTaskLineWidth() );
  m_targetLineWidth->setValue( conf->getTargetLineWidth() );
  m_trailLineWidth->setValue( conf->getMapTrailLineWidth() );
  m_taskFiguresLineWidth->setValue( conf->getTaskFiguresLineWidth() );
  m_airspaceBorderLineWidth->setValue( conf->getAirspaceLineWidth() );

  drawLineIcon( m_headingLineWidth->text(), 0 );
  drawLineIcon( m_pathLineWidth->text(), 1 );
  drawLineIcon( m_targetLineWidth->text(), 2 );
  drawLineIcon( m_trailLineWidth->text(), 3 );
  drawLineIcon( m_taskFiguresLineWidth->text(), 4 );
  drawLineIcon( m_airspaceBorderLineWidth->text(), 5 );
}

void SettingsPageLines::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setHeadingLineDrawState( m_headingLine->checkState() == Qt::Checked ? true : false );
  conf->setTargetLineDrawState( m_targetLine->checkState() == Qt::Checked ? true : false );
  conf->setMapDrawTrail( m_trailLine->checkState() == Qt::Checked ? true : false );

  conf->setHeadingLineColor( m_headingLineColor->palette().color(QPalette::Window) );
  conf->setTaskLineColor( m_pathLineColor->palette().color(QPalette::Window) );
  conf->setMapTrailColor( m_trailLineColor->palette().color(QPalette::Window) );
  conf->setTaskFiguresColor( m_taskFiguresColor->palette().color(QPalette::Window) );

  conf->setHeadingLineWidth( m_headingLineWidth->value() );
  conf->setTaskLineWidth( m_pathLineWidth->value() );
  conf->setTargetLineWidth( m_targetLineWidth->value() );
  conf->setMapTrailLineWidth( m_trailLineWidth->value() );
  conf->setTaskFiguresLineWidth( m_taskFiguresLineWidth->value() );
  conf->setAirspaceLineWidth( m_airspaceBorderLineWidth->value() );

  // @AP: initiate a redraw of airspaces on the map due to color modifications.
  //      Not the best solution but it is working ;-)
  Map::getInstance()->scheduleRedraw(Map::airspaces);
}

/**
  * Called to set all colors to their default value.
  */
void SettingsPageLines::slot_setDefaults()
{
  m_headingLine->setCheckState( Qt::Checked );
  m_pathLine->setCheckState( Qt::Checked );
  m_targetLine->setCheckState( Qt::Checked );
  m_trailLine->setCheckState( Qt::Checked );
  m_taskFigures->setCheckState( Qt::Checked );
  m_airspaceBorder->setCheckState( Qt::Checked );

  m_headingLineColor->setPalette( QPalette(HeadingLineColor) );
  m_pathLineColor->setPalette( QPalette(TaskLineColor) );
  m_trailLineColor->setPalette( QPalette(TrailLineColor));
  m_taskFiguresColor->setPalette( QPalette(TaskFiguresColor));

  m_headingLineWidth->setValue( HeadingLineWidth );
  m_pathLineWidth->setValue( TaskLineWidth );
  m_targetLineWidth->setValue( TargetLineWidth );
  m_trailLineWidth->setValue( TrailLineWidth );
  m_taskFiguresLineWidth->setValue( TaskFiguresLineWidth );
  m_airspaceBorderLineWidth->setValue( AirSpaceBorderLineWidth );

  drawLineIcon( m_headingLineWidth->text(), 0 );
  drawLineIcon( m_pathLineWidth->text(), 1 );
  drawLineIcon( m_targetLineWidth->text(), 2 );
  drawLineIcon( m_trailLineWidth->text(), 3 );
  drawLineIcon( m_taskFiguresLineWidth->text(), 4 );
  drawLineIcon( m_airspaceBorderLineWidth->text(), 5 );
}

/**
 * Called to toggle the check box of the clicked table cell.
 */
void SettingsPageLines::slot_toggleCheckBox( int row, int column )
{
  // qDebug("row=%d, column=%d", row, column);

  if( column == 0 )
    {
      // Checkbox has been pressed
      if( row == 1 || row == 4 || row == 5 )
        {
          // Don't change the check state here. It is always true.
          return;
        }

      QTableWidgetItem *item = m_drawOptions->item( row, column );
      item->setCheckState( item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked );
      return;
    }

  if( column == 1 )
    {
      // Color can be modified.
      if( row == 2 || row == 5 )
        {
          // Don't change the color here.
          return;
        }

      QWidget *cw = m_drawOptions->cellWidget( row, column );
      QPalette palette = cw->palette();
      QColor color = palette.color(QPalette::Window);

      // Open color chooser dialog
      QColor newColor = ColorDialog::getColor( color, this, m_drawOptions->item( row, 0 )->text() );

      if( newColor.isValid() && color != newColor )
        {
          // set new color in widget
          cw->setPalette( QPalette(newColor));

          // Update color of line view
          NumberEditor* ne = dynamic_cast<NumberEditor *>(m_drawOptions->cellWidget( row, column + 1 ) );

          if( ne )
            {
              drawLineIcon( ne->text(), row );
            }
        }

      return;
    }
}

void SettingsPageLines::slot_drawLineIcon0( const QString& number )
{
  drawLineIcon( number, 0 );
}

void SettingsPageLines::slot_drawLineIcon1( const QString& number )
{
  drawLineIcon( number, 1 );
}

void SettingsPageLines::slot_drawLineIcon2( const QString& number )
{
  drawLineIcon( number, 2 );
}

void SettingsPageLines::slot_drawLineIcon3( const QString& number )
{
  drawLineIcon( number, 3 );
}

void SettingsPageLines::slot_drawLineIcon4( const QString& number )
{
  drawLineIcon( number, 4 );
}

void SettingsPageLines::slot_drawLineIcon5( const QString& number )
{
  drawLineIcon( number, 5 );
}

void SettingsPageLines::drawLineIcon( const QString& number, const int row )
{
  // qDebug() << __PRETTY_FUNCTION__ << "Weite=" << number << "Row="<< row;

  bool ok;

  int lw = number.toInt( &ok );

  if( !ok )
    {
      return;
    }

  QLabel *cw = dynamic_cast<QLabel *>(m_drawOptions->cellWidget( row, 3 ));

  if( cw == 0 )
    {
      return;
    }

  // At this place the icon with the line must be updated.
  QFontMetrics fm( font() );
  int charWidth = fm.width(QChar('M'));

  int pmw = 4 * charWidth;
  int pmh = fm.height() + 2;

  QColor color = Qt::black;

  if( row != 2 && row != 5 )
    {
      // Set color to user selected color.
      QWidget *cw = m_drawOptions->cellWidget( row, 1 );
      QPalette palette = cw->palette();
      color = palette.color(QPalette::Window);
    }

  QPixmap pixmap( pmw, pmh );
  pixmap.fill( Qt::white );

  QPainter painter( &pixmap );
  QPen pen( color );
  pen.setWidth( lw );
  painter.setPen( pen );
  painter.setBrush( Qt::NoBrush );
  painter.drawLine( 10, pmh / 2, pmw-10, pmh / 2 );

  cw->setPixmap( pixmap );
  m_drawOptions->resizeColumnsToContents();
  m_drawOptions->resizeRowsToContents();
}
