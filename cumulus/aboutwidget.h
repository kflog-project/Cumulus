/***********************************************************************
**
**   aboutwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief A widget to display the about application data
 *
 * This widget displays the about application data in a tabbed window
 * decorated this a headline and an icon.
 *
*/

#ifndef ABOUT_WIDGET_H_
#define ABOUT_WIDGET_H_

#include <QWidget>
#include <QLabel>
#include <QTextBrowser>
#include <QString>

class QPixmap;

class AboutWidget : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( AboutWidget )

 public:

  AboutWidget( QWidget *parent = 0 );

  virtual ~AboutWidget() {};

  void setHeaderIcon( const QPixmap pixmap )
  {
    headerIcon->setPixmap( pixmap );
  };

  void setHeaderText( const QString& text )
  {
    headerText->setText( text );
  };

  void setAboutText( const QString& text )
  {
    about->setHtml( text );
  };

  void setTeamText( const QString& text )
  {
    team->setHtml( text );
  };

 private:

  QLabel       *headerText;
  QLabel       *headerIcon;
  QTextBrowser *about;
  QTextBrowser *team;
};

#endif /* ABOUT_WIDGET_H_ */
