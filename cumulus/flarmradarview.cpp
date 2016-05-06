/***********************************************************************
**
**   flarmradarview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2016 Axel Pauli
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

#include "flarm.h"
#include "flarmaliaslist.h"
#include "flarmdisplay.h"
#include "flarmradarview.h"
#include "generalconfig.h"
#include "layout.h"

/**
 * Constructor
 */
FlarmRadarView::FlarmRadarView( QWidget *parent ) :
  QWidget( parent ),
  display(0)
{
  setAttribute( Qt::WA_DeleteOnClose );
  setContentsMargins(-4,-8,-4,-8);

  pmWindOn = GeneralConfig::instance()->loadPixmap("windsack.png");
  pmWindOff = pmWindOn;

  const int lineWidth = static_cast<int>(rintf( 3.0 * Layout::getScaledDensity() ));

  QPainter painter;
  painter.begin(&pmWindOff);
  painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  QPen pen(Qt::red);
  pen.setWidth( lineWidth );
  painter.setPen( pen );
  painter.setBrush(Qt::NoBrush);
  painter.drawLine( 0, 0, pmWindOff.width(), pmWindOff.height());
  painter.drawLine( pmWindOff.width(), 0, 0, pmWindOff.height());
  painter.end();

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  display = new FlarmDisplay( this );
  topLayout->addWidget( display, 2 );

  connect( Flarm::instance(), SIGNAL(newFlarmPflaaData()),
           display, SLOT(slot_UpdateDisplay()) );

  connect( Flarm::instance(), SIGNAL(flarmPflaaDataTimeout()),
           display, SLOT(slot_ResetDisplay()) );

  QGroupBox* buttonBox = new QGroupBox( this );
  buttonBox->setContentsMargins(2, 2, 2, 2);

  zoomButton = new QPushButton;
  zoomButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("zoom32.png")));

  listButton  = new QPushButton;
  listButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("list32.png")));

  display->setUpdateInterval( 2 );
  updateButton = new QPushButton( "2s" );

  aliasButton = new QPushButton;
  aliasButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("monkey32.png")));

  windButton = new QPushButton;
  setWindButtonIcon( ! FlarmDisplay::getDrawWindArrow() );

  closeButton = new QPushButton;
  closeButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));

  addButton = new QPushButton;
  addButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("add.png")));

  if( FlarmDisplay::getSelectedObject().isEmpty() )
    {
      // The add button is invisible, if no selection is made.
      addButton->setVisible( false );
    }

  connect( zoomButton, SIGNAL(clicked() ), this, SLOT(slotZoom()) );
  connect( listButton, SIGNAL(clicked() ), this, SLOT(slotOpenListView()) );
  connect( updateButton, SIGNAL(clicked() ), this, SLOT(slotUpdateInterval()) );
  connect( aliasButton, SIGNAL(clicked() ), this, SLOT(slotOpenAliasList()) );
  connect( windButton, SIGNAL(clicked() ), this, SLOT(slotToggleWindDisplay()) );
  connect( closeButton, SIGNAL(clicked() ), this, SLOT(slotClose()) );
  connect( addButton, SIGNAL(clicked() ), this, SLOT(slotAddFlarmId()) );

  // vertical box with operator buttons
  QVBoxLayout *vbox = new QVBoxLayout;

  vbox->setSpacing(0);
  vbox->addWidget( zoomButton );
  vbox->addStretch(2);
  vbox->addWidget( listButton );
  vbox->addStretch(2);
  vbox->addWidget( updateButton );
  vbox->addStretch(2);
  vbox->addWidget( windButton );
  vbox->addStretch(2);
  vbox->addWidget( aliasButton );
  vbox->addStretch(2);
  vbox->addWidget( addButton );
  vbox->addStretch(2);
  vbox->addWidget( closeButton );
  buttonBox->setLayout( vbox );

  topLayout->addWidget( buttonBox );
}

/**
 * Destructor
 */
FlarmRadarView::~FlarmRadarView()
{
}

void FlarmRadarView::showEvent( QShowEvent* )
{
  // According to the window height the button sizes are adapted.
  int buttonSize = Layout::getButtonSize();
  int iconSize   = buttonSize - 5;
  int space      = 5 + 5*20 + 5;

  int wh = height();

  if( wh < ( (buttonSize * 7) + space ) )
    {
      // Not enough space in the window height. Recalculate button size.
      buttonSize = (wh - space -5) / 7;
      iconSize   = buttonSize - 5;
    }

  QPushButton* pba[7] = { zoomButton,
                          listButton,
                          updateButton,
                          aliasButton,
                          addButton,
			  windButton,
                          closeButton };

  for( int i = 0; i < 7; i++ )
    {
      QPushButton* pb = pba[i];
      pb->setIconSize(QSize(iconSize, iconSize));
      pb->setMinimumSize(buttonSize, buttonSize);
      pb->setMaximumSize(buttonSize, buttonSize);
    }
}

/** Called if zoom level shall be changed. */
void FlarmRadarView::slotZoom()
{
  enum FlarmDisplay::Zoom zoom = display->getZoomLevel();

  if( zoom == FlarmDisplay::Low )
    {
      display->slot_SwitchZoom( FlarmDisplay::Middle );
    }
  else if( zoom == FlarmDisplay::Middle )
    {
      display->slot_SwitchZoom( FlarmDisplay::High );
    }
  else
    {
      display->slot_SwitchZoom( FlarmDisplay::Low );
    }
}

/** Called if list view button was pressed. */
void FlarmRadarView::slotOpenListView()
{
  emit openListView();
}

/** Called if close button was pressed. */
void FlarmRadarView::slotClose()
{
  // Ask FlarmWidget to close the widget.
  emit closeRadarView();
}

/** Called if update interval button was pressed. */
void FlarmRadarView::slotUpdateInterval()
{
  QString text = updateButton->text();

  QString newText = "2s";
  int newValue = 2;

  if( text == "1s" )
    {
      newText = "2s";
      newValue = 2;
    }
  else if( text == "2s" )
    {
      newText = "3s";
      newValue = 3;
    }
  else if( text == "3s" )
    {
      newText = "1s";
      newValue = 1;
    }

  updateButton->setText( newText );
  display->setUpdateInterval( newValue );
}

/** Called if alias list button was pressed. */
void FlarmRadarView::slotOpenAliasList()
{
  emit openAliasList();
}

/** Called to change the visibility of the add Flarm Id button. */
void FlarmRadarView::slotShowAddButton( QString selectedObject )
{
  if( selectedObject.isEmpty() == true )
    {
      // The add button is invisible, if no selection is made.
      addButton->setVisible( false );
    }
  else
    {
      addButton->setVisible( true );
    }
}

/** Called to add an object to the Flarm alias list. */
void FlarmRadarView::slotAddFlarmId()
{
  QString& selectedObject = FlarmDisplay::getSelectedObject();

  if( selectedObject.isEmpty() )
    {
      // There is nothing selected, ignore call.
      return;
    }

  QHash<QString, QString>& aliasHash = FlarmAliasList::getAliasHash();

  if( aliasHash.isEmpty() )
    {
      // try to load it
      FlarmAliasList::loadAliasData();
    }

  aliasHash = FlarmAliasList::getAliasHash();

  // Look for an existing alias name
  QString alias = aliasHash.value( selectedObject, "" );

  // Add the selected Flarm Id to the alias list.
  bool ok;

#ifndef MAEMO5
  alias = QInputDialog::getText( this,
                                 tr("Add alias name"),
                                 tr("Alias name (15) for ") + selectedObject + ":",
                                 QLineEdit::Normal,
                                 alias,
                                 &ok,
                                 0,
                                 Qt::ImhNoPredictiveText );
#else
  alias = QInputDialog::getText( this,
                                 tr("Add alias name"),
                                 tr("Alias name (15) for ") + selectedObject + ":",
                                 QLineEdit::Normal,
                                 alias,
                                 &ok,
                                 0 );
#endif

  if( !ok || alias.isEmpty() )
    {
      return;
    }

  // Add an alias name to the alias list. An existing alias name will be updated.
  aliasHash.insert( selectedObject, alias.trimmed().left(FlarmAliasList::MaxAliasLength) );
  FlarmAliasList::saveAliasData();
  display->createBackground();
  display->update();
}

void FlarmRadarView::slotToggleWindDisplay()
{
  bool state = FlarmDisplay::getDrawWindArrow();

  FlarmDisplay::setDrawWindArrow( ! state );
  setWindButtonIcon( state );
}

void FlarmRadarView::setWindButtonIcon( bool onOff )
{
  if( onOff == false )
    {
      windButton->setIcon( pmWindOff );
      windButton->setToolTip( tr("Press button to switch off wind arrow drawing.") );
    }
  else
    {
      windButton->setIcon( pmWindOn );
      windButton->setToolTip( tr("Press button to switch on wind arrow drawing.") );
    }
}
