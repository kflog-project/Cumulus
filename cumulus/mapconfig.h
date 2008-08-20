/***********************************************************************
**
**   mapconfig.h
**
**   This file is part of Cumulus.
**   It is modified for Cumulus by André Somers in 2002 and
**   by Axel pauli in 2008
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, 2007 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAPCONFIG_H
#define MAPCONFIG_H

#include <QList>
#include <QObject>
#include <QPen>
#include <QBrush>
#include <QPixmap>
#include <QString>
#include <QMap>
#include <QIcon>

/**
 * This class takes care of the configuration-data for displaying
 * and printing map-elements. To avoid problems, there should be only
 * one element per application.
 *
 * All printing-related code has been removed for Cumulus, because printing
 * will not be supported on the PDA. (André Somers)
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class MapConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new MapConfig object.
     */
    MapConfig(QObject*);

    /**
     * Destructor
     */
    virtual ~MapConfig();

    /**
     * @param  type  The typeID of the element.
     *
     * @return "true", if the current scale is smaller than the switch-scale,
     *         so that small icons should be used for displaying.
     */
    bool isBorder(unsigned int type);

    /**
     * @param  type  The typeID of the element.
     *
     * @return the pen for drawing a mapelement.
     */
    QPen getDrawPen(unsigned int typeID);

    /**
     * @param  type  The typeID of the element.
     *
     * @return the brush for drawing an areaelement.
     */
    QBrush getDrawBrush(unsigned int typeID);

    /**
     * @param  heighIndex  The index of the height of the isohypse.
     *
     * @return the color for a isohypse.
     */
    QColor getIsoColor(unsigned int heightIndex);

    /**
     * @param  iconName  The name of the icon to load
     * @returns the icon-pixmap of the element.
     */
    QPixmap getPixmap(QString iconName);

    /**
     * @param  type  The typeID of the element.
     * @param  isWinch  Used only for glidersites to determine, if the
     *                  icon should indicate that only winch-launch is
     *                  available.
     *
     * @returns the icon-pixmap of the element.
     */
    QPixmap getPixmapRotatable(unsigned int typeID, bool isWinch);

    /**
     * @param  type  The typeID of the element.
     *
     * @returns the rotatable icon-pixmap of the element.
     */

    QPixmap getPixmap(unsigned int typeID, bool isWinch = true, QColor color=Qt::black);

    /**
     * @param  type  The typeID of the element.
     * @param  isWinch  Used only for glidersites to determine, if the
     *                  icon should indicate that only winch-launch is
     *                  available.
     * @param  smallIcon  Used to select the size of the returned pixmap.
     *                  if true, a small pixmap is returned, otherwise the larger
     *                  version is returned.
     * @returns the icon-pixmap of the element.
     */
    QPixmap getPixmap(unsigned int typeID, bool isWinch, bool smallIcon);

    /**
     * @param  type  The typeID of the element.
     *
     * @returns an icon for use in airfield list.
     */
    QIcon getListIcon(unsigned int typeID);

    /**
     * @param  type  The typeID of the element.
     * @param  isWinch  Used only for glidersites to determine, if the
     *                  icon should indicate that only winch-launch is
     *                  available.
     *
     * @return the name of the pixmap of the element.
     */
    QString getPixmapName(unsigned int type, bool isWinch = true,
                          bool rotatable = false, QColor color=Qt::black);

    /**
     * @Returns true if small icons are used, else returns false.
     */
    bool useSmallIcons() const;

    /**
     * Read property of bool drawBearing.
     */
    bool getdrawBearing() const;

    /**
     * Read property of bool drawIsoLines.
     */
    bool getdrawIsoLines() const;

    /**
     * Read property of bool determing if the original mapfile
     * should be deleted after compiling it
     */
    inline bool getDeleteMapfileAfterCompile() const
    {
        return bDeleteAfterMapCompile;
    };

    /**
     * Read property of bool determing if a mapfile should be
     * unloaded immediatly if not needed anymore. Otherwise,
     * the maps are only unloaded if free memory runs out.
     */
    inline bool getUnloadUnneededMap() const
    {
        return bUnloadUnneededMap;
    };

    /**
     * Returns whether or not the object should be loaded/drawn.
     */
    bool getLoadIsolines() const;
    bool getShowIsolineBorders() const;
    bool getShowWpLabels() const;
    void setShowWpLabels(bool);
    bool getShowWpLabelsExtraInfo() const;
    void setShowWpLabelsExtraInfo(bool);
    bool getLoadRoads() const;
    bool getLoadHighways() const;
    bool getLoadRailroads() const;
    bool getLoadCities() const;
    bool getLoadWaterways() const;
    bool getLoadForests() const;

    /**
     * The possible datatypes, that could be drawn.
     *
     * @see #slotSetFlightDataType
     */
    //enum DrawFlightPoint {Vario, Speed, Altitude, Cycling, Solid};

    bool isRotatable( unsigned int typeID );

public slots:
    /**
     * Forces MapConfig to read the configdata.
     */
    void slotReadConfig();

    /**
     * Sets the scaleindex an the flag for small icons. Called from
     * MapMatrix.
     *
     * @see MapMatrix#scaleAdd
     *
     * @param  index  The scaleindex
     * @param  isSwitch  "true" if the current scale is smaller than the
     *                   switch-scale
     */
    void slotSetMatrixValues(int index, bool isSwitch);

signals:
    /**
     * Emitted each time, the config has changed.
     */
    void configChanged();

private:
    /**
     * Determines the brush to be used to draw or print a given element-type.
     *
     * @param  typeID  The typeID of the element.
     * @param  scaleIndex  The scaleindex to be used.
     *
     * @return the brush
     */
    QBrush __getBrush(unsigned int typeID, int scaleIndex);

    /**
     * Determines the pen to be used to draw or print a given element-type.
     *
     * @param  typeID  The typeID of the element.
     * @param  scaleIndex  The scaleindex to be used.
     *
     * @return the pen
     */
    QPen __getPen(unsigned int typeID, int sIndex);

    QList<QColor*> topographyColorList;

    QList<QPen*> airAPenList;
    QList<QBrush*> airABrushList;
    QList<QPen*> airBPenList;
    QList<QBrush*> airBBrushList;
    QList<QPen*> airCPenList;
    QList<QBrush*> airCBrushList;
    QList<QPen*> airDPenList;
    QList<QBrush*> airDBrushList;
    QList<QPen*> airElPenList;
    QList<QBrush*> airElBrushList;
    QList<QPen*> airEhPenList;
    QList<QBrush*> airEhBrushList;
    QList<QPen*> airFPenList;
    QList<QBrush*> airFBrushList;
    QList<QPen*> ctrCPenList;
    QList<QBrush*> ctrCBrushList;
    QList<QPen*> ctrDPenList;
    QList<QBrush*> ctrDBrushList;
    QList<QPen*> lowFPenList;
    QList<QBrush*> lowFBrushList;
    QList<QPen*> dangerPenList;
    QList<QBrush*> dangerBrushList;
    QList<QPen*> restrPenList;
    QList<QBrush*> restrBrushList;
    QList<QPen*> tmzPenList;
    QList<QBrush*> tmzBrushList;
    QList<QPen*> suSectorPenList;
    QList<QBrush*> suSectorBrushList;

    QList<QPen*> highwayPenList;
    QList<QPen*> roadPenList;
    QList<QPen*> trailPenList;
    QList<QPen*> railPenList;
    QList<QPen*> rail_dPenList;
    QList<QPen*> aerialcablePenList;
    QList<QPen*> riverPenList;
    QList<QPen*> river_tPenList;
    QList<QBrush*> river_tBrushList;
    QList<QPen*> canalPenList;
    QList<QPen*> cityPenList;
    QList<QBrush*> cityBrushList;
    QList<QPen*> forestPenList;
    QList<QPen*> glacierPenList;
    QList<QPen*> packicePenList;
    QList<QBrush*> forestBrushList;
    QList<QBrush*> glacierBrushList;
    QList<QBrush*> packiceBrushList;

    /**
     * holds a collection of ready made airfield icons
     */
    QMap<unsigned int, QIcon> airfieldIcon;

    /**
     */
    bool* airABorder;
    bool* airBBorder;
    bool* airCBorder;
    bool* airDBorder;
    bool* airElBorder;
    bool* airEhBorder;
    bool* airFBorder;
    bool* ctrCBorder;
    bool* ctrDBorder;
    bool* dangerBorder;
    bool* lowFBorder;
    bool* restrBorder;
    bool* tmzBorder;
    bool* suSectorBorder;

    bool* trailBorder;
    bool* roadBorder;
    bool* highwayBorder;
    bool* railBorder;
    bool* rail_dBorder;
    bool* aerialcableBorder;
    bool* riverBorder;
    bool* river_tBorder;
    bool* canalBorder;
    bool* cityBorder;

    bool* forestBorder;
    bool* glacierBorder;
    bool* packiceBorder;

    /**
     * The current scaleindex for displaying the map. The index is set
     * from the mapmatrix-object each time, the map is zoomed.
     *
     * @see #slotSetMatrixValues
     * @see MapMatrix#displayMatrixValues
     */
    int scaleIndex;

    /**
     * true, if small icons should be drawn. Set from the mapmatrix-object
     * each time, the map is zoomed.
     */
    bool isSwitch;

protected: // Protected attributes
    /** determines if we should draw the bearing on the map or not. */
    bool drawBearing;
    /** determines if we should draw isolines on the map or not. */
    bool drawIsoLines;
    /** determines if we should draw the dotted borders isolines on the map or not. */
    bool bShowIsolineBorders;
    /** flags holding if we should load certain mapobjects or not */
    bool bLoadIsolines;
    bool bShowWpLabels;
    bool bShowWpLabelsExtraInfo;
    bool bLoadRoads;
    bool bLoadHighways;
    bool bLoadRailroads;
    bool bLoadCities;
    bool bLoadWaterways;
    bool bLoadForests;

    /** flag determines if the sourcefile is deleted after compiling a mapfile */
    bool bDeleteAfterMapCompile;
    /** flag determines if maps should be unloaded at the first change or not */
    bool bUnloadUnneededMap;
};

#endif
