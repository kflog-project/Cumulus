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

#include <Q3PtrList>
#include <QObject>
#include <QPen>
#include <QBrush>
#include <QPixmap>
#include <QString>

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
    void setShowWpLabelsExtraInfo(bool newvalue)
    {
        bShowWpLabelsExtraInfo = newvalue;
    };
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

    Q3PtrList<QColor> topographyColorList;

    Q3PtrList<QPen> airAPenList;
    Q3PtrList<QBrush> airABrushList;
    Q3PtrList<QPen> airBPenList;
    Q3PtrList<QBrush> airBBrushList;
    Q3PtrList<QPen> airCPenList;
    Q3PtrList<QBrush> airCBrushList;
    Q3PtrList<QPen> airDPenList;
    Q3PtrList<QBrush> airDBrushList;
    Q3PtrList<QPen> airElPenList;
    Q3PtrList<QBrush> airElBrushList;
    Q3PtrList<QPen> airEhPenList;
    Q3PtrList<QBrush> airEhBrushList;
    Q3PtrList<QPen> airFPenList;
    Q3PtrList<QBrush> airFBrushList;
    Q3PtrList<QPen> ctrCPenList;
    Q3PtrList<QBrush> ctrCBrushList;
    Q3PtrList<QPen> ctrDPenList;
    Q3PtrList<QBrush> ctrDBrushList;
    Q3PtrList<QPen> lowFPenList;
    Q3PtrList<QBrush> lowFBrushList;
    Q3PtrList<QPen> dangerPenList;
    Q3PtrList<QBrush> dangerBrushList;
    Q3PtrList<QPen> restrPenList;
    Q3PtrList<QBrush> restrBrushList;
    Q3PtrList<QPen> tmzPenList;
    Q3PtrList<QBrush> tmzBrushList;
    Q3PtrList<QPen> suSectorPenList;
    Q3PtrList<QBrush> suSectorBrushList;

    Q3PtrList<QPen> highwayPenList;
    Q3PtrList<QPen> roadPenList;
    Q3PtrList<QPen> trailPenList;
    Q3PtrList<QPen> railPenList;
    Q3PtrList<QPen> rail_dPenList;
    Q3PtrList<QPen> aerialcablePenList;
    Q3PtrList<QPen> riverPenList;
    Q3PtrList<QPen> river_tPenList;
    Q3PtrList<QBrush> river_tBrushList;
    Q3PtrList<QPen> canalPenList;
    Q3PtrList<QPen> cityPenList;
    Q3PtrList<QBrush> cityBrushList;
    Q3PtrList<QPen> forestPenList;
    Q3PtrList<QPen> glacierPenList;
    Q3PtrList<QPen> packicePenList;
    Q3PtrList<QBrush> forestBrushList;
    Q3PtrList<QBrush> glacierBrushList;
    Q3PtrList<QBrush> packiceBrushList;
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
