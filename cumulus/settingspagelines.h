/***********************************************************************
**
**   settingspagelines.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: settingspagelines.h 5844 2013-02-02 20:39:31Z axel $
**
***********************************************************************/

#ifndef SettingsPageLines_H
#define SettingsPageLines_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>

class NumberEditor;

/**
 * \class SettingsPageLines
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for Lines
 *
 * Configuration settings for Line drawing.
 *
 * \date 2013
 *
 * \version $Id: settingspagelines.h 5844 2013-02-02 20:39:31Z axel $
 *
 */
class SettingsPageLines : public QWidget
{
    Q_OBJECT

  private:

    Q_DISABLE_COPY ( SettingsPageLines )

  public:

    SettingsPageLines(QWidget *parent=0);

    virtual ~SettingsPageLines();

  protected:

    virtual void showEvent(QShowEvent *);

  private:

    NumberEditor* createNumberEditor( QWidget* parent=0 );

    void drawLineIcon( const QString& number, const int row );

    /** Called to load the configuration file data. */
    void load();

    /** Called to save the configuration file data.*/
    void save();

  private slots:

    /**
     * Called to toggle the check box of the clicked table cell.
     */
    void slot_toggleCheckBox( int row, int column );

    /**
     * Called, if the number editor is closed and a new value has been emitted
     * by it. The related icon in the table is repainted.
     * @param number Edited number value
     */
    void slot_drawLineIcon0( const QString& number );
    void slot_drawLineIcon1( const QString& number );
    void slot_drawLineIcon2( const QString& number );
    void slot_drawLineIcon3( const QString& number );
    void slot_drawLineIcon4( const QString& number );
    void slot_drawLineIcon5( const QString& number );

    /**
     * Called to set all items to their default value.
     */
    void slot_setDefaults();

    /**
     * Called if the Ok button is pressed.
     */
    void slotAccept();

    /**
     * Called if the Cancel button is pressed.
     */
    void slotReject();

  signals:

    /**
     * Emitted if the airspace colors have been updated
     */
    void colorsUpdated();

    /**
     * Emitted, if settings have been changed.
     */
    void settingsChanged();

  private:

    QTableWidget* m_drawOptions;
    QPushButton*  m_cmdDefaults;

    // enable/disable drawing of lines
    QTableWidgetItem* m_headingLine;
    QTableWidgetItem* m_pathLine;
    QTableWidgetItem* m_targetLine;
    QTableWidgetItem* m_trailLine;
    QTableWidgetItem* m_taskFigures;
    QTableWidgetItem* m_airspaceBorder;

    // colors of lines
    QWidget* m_headingLineColor;
    QWidget* m_pathLineColor;
    QWidget* m_trailLineColor;
    QWidget* m_taskFiguresColor;

     // line widths
    NumberEditor* m_headingLineWidth;
    NumberEditor* m_pathLineWidth;
    NumberEditor* m_targetLineWidth;
    NumberEditor* m_trailLineWidth;
    NumberEditor* m_taskFiguresLineWidth;
    NumberEditor* m_airspaceBorderLineWidth;
};

#endif
