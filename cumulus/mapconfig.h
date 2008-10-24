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

#ifndef MAP_CONFIG_H
#define MAP_CONFIG_H

#include <QList>
#include <QObject>
#include <QPen>
#include <QBrush>
#include <QPixmap>
#include <QString>
#include <QMap>
#include <QIcon>

/**
 * This class takes care of the configuration data for displaying
 * and printing map elements. To avoid problems, there should be only
 * one element per application.
 *
 * All printing related code has been removed for Cumulus, because printing
 * will not be supported on the PDA. (André Somers)
 *
 * @author Heiner Lamprecht, Florian Ehinger
 *
 */

class MapConfig : public QObject
{
  Q_OBJECT

private:

  /**
   * Don't allow copies and assignments.
   */
  MapConfig(const MapConfig& );
  MapConfig& operator=(const MapConfig& x);

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
    const QPen& getDrawPen(unsigned int typeID)
    {
      return __getPen(typeID, scaleIndex);
    };

    /**
     * @param  type  The typeID of the element.
     *
     * @return the brush for drawing an areaelement.
     */
    const QBrush& getDrawBrush(unsigned int typeID)
    {
      return __getBrush(typeID, scaleIndex);
    };

    /**
     * @param  heighIndex  The index of the height of the isohypse.
     *
     * @return the color for a isohypse.
     */
    const QColor& getIsoColor(int heightIndex)
      {
        return topographyColorList.at(heightIndex);
      };

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
     * @returns an icon for use in an airfield list.
     */
    const QIcon& getListIcon(unsigned int typeID)
      {
        return airfieldIcon[typeID];
      };

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
    bool useSmallIcons() const
    {
      return !isSwitch;
    };

    /**
     * Read property of bool drawBearing.
     */
    bool getdrawBearing() const
    {
      return drawBearing;
    };

    /**
     * Read property of bool drawIsoLines.
     */
    bool getdrawIsoLines() const
    {
      return drawIsoLines;
    };

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
    bool getLoadIsolines() const
    {
      return bLoadRailroads;
    };

    bool getShowIsolineBorders() const
    {
      return bLoadRailroads;
    };

    bool getShowWpLabels() const
    {
      return bShowWpLabels;
    };

    void setShowWpLabels(bool show);

    bool getShowWpLabelsExtraInfo() const
    {
        return bShowWpLabelsExtraInfo;
    };

    void setShowWpLabelsExtraInfo(bool show);

    bool getLoadRoads() const
    {
      return bLoadRailroads;
    };

    bool getLoadHighways() const
    {
      return bLoadRailroads;
    };

    bool getLoadRailroads() const
    {
      return bLoadRailroads;
    };

    bool getLoadCities() const
    {
      return bLoadCities;
    };

    bool getLoadWaterways() const
    {
      return bLoadWaterways;
    };

    bool getLoadForests() const
    {
      return bLoadForests;
    };

    /**
     * The possible data types, that could be drawn.
     *
     * @see #slotSetFlightDataType
     */

    bool isRotatable( unsigned int typeID ) const;

public slots:
    /**
     * Forces MapConfig to read the configdata.
     */
    void slotReadConfig();

    /**
     * Sets the scale index an the flag for small icons. Called from
     * MapMatrix.
     *
     * @see MapMatrix#scaleAdd
     *
     * @param  index  The scale index
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
     * @param  scaleIndex  The scale index to be used.
     *
     * @return the brush
     */
    const QBrush& __getBrush(unsigned int typeID, int scaleIndex);

    /**
     * Determines the pen to be used to draw or print a given element-type.
     *
     * @param  typeID  The typeID of the element.
     * @param  scaleIndex  The scale index to be used.
     *
     * @return the pen
     */
    const QPen& __getPen(unsigned int typeID, int sIndex);

    QList<QColor> topographyColorList;

    QList<QPen> airAPenList;
    QList<QBrush> airABrushList;
    QList<QPen> airBPenList;
    QList<QBrush> airBBrushList;
    QList<QPen> airCPenList;
    QList<QBrush> airCBrushList;
    QList<QPen> airDPenList;
    QList<QBrush> airDBrushList;
    QList<QPen> airElPenList;
    QList<QBrush> airElBrushList;
    QList<QPen> airEhPenList;
    QList<QBrush> airEhBrushList;
    QList<QPen> airFPenList;
    QList<QBrush> airFBrushList;
    QList<QPen> ctrCPenList;
    QList<QBrush> ctrCBrushList;
    QList<QPen> ctrDPenList;
    QList<QBrush> ctrDBrushList;
    QList<QPen> lowFPenList;
    QList<QBrush> lowFBrushList;
    QList<QPen> dangerPenList;
    QList<QBrush> dangerBrushList;
    QList<QPen> restrPenList;
    QList<QBrush> restrBrushList;
    QList<QPen> tmzPenList;
    QList<QBrush> tmzBrushList;
    QList<QPen> suSectorPenList;
    QList<QBrush> suSectorBrushList;
    QList<QPen> highwayPenList;
    QList<QPen> roadPenList;
    QList<QPen> trailPenList;
    QList<QPen> railPenList;
    QList<QPen> rail_dPenList;
    QList<QPen> aerialcablePenList;
    QList<QPen> lakePenList;
    QList<QBrush> lakeBrushList;
    QList<QPen> riverPenList;
    QList<QPen> river_tPenList;
    QList<QBrush> river_tBrushList;
    QList<QPen> canalPenList;
    QList<QPen> cityPenList;
    QList<QBrush> cityBrushList;
    QList<QPen> forestPenList;
    QList<QPen> glacierPenList;
    QList<QPen> packicePenList;
    QList<QBrush> forestBrushList;
    QList<QBrush> glacierBrushList;
    QList<QBrush> packiceBrushList;

    /**
     * holds a collection of ready made airfield icons
     */
    QMap<unsigned int, QIcon> airfieldIcon;

    /**
     */
    bool airABorder[4];
    bool airBBorder[4];
    bool airCBorder[4];
    bool airDBorder[4];
    bool airElBorder[4];
    bool airEhBorder[4];
    bool airFBorder[4];
    bool ctrCBorder[4];
    bool ctrDBorder[4];
    bool dangerBorder[4];
    bool lowFBorder[4];
    bool restrBorder[4];
    bool tmzBorder[4];
    bool suSectorBorder[4];
    bool trailBorder[4];
    bool roadBorder[4];
    bool highwayBorder[4];
    bool railBorder[4];
    bool rail_dBorder[4];
    bool aerialcableBorder[4];
    bool lakeBorder[4];
    bool riverBorder[4];
    bool river_tBorder[4];
    bool canalBorder[4];
    bool cityBorder[4];
    bool forestBorder[4];
    bool glacierBorder[4];
    bool packiceBorder[4];

    /**
     * The current scale index for displaying the map. The index is set
     * from the MapMatrix object each time, the map is zoomed.
     *
     * @see #slotSetMatrixValues
     * @see MapMatrix#displayMatrixValues
     */
    int scaleIndex;

    /**
     * true, if small icons should be drawn. Set from the map matrix-object
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
    /** flags holding if we should load certain map objects or not */
    bool bLoadIsolines;
    bool bShowWpLabels;
    bool bShowWpLabelsExtraInfo;
    bool bLoadRoads;
    bool bLoadHighways;
    bool bLoadRailroads;
    bool bLoadCities;
    bool bLoadWaterways;
    bool bLoadForests;

    /** flag determines if the source file is deleted after compiling a map file */
    bool bDeleteAfterMapCompile;
    /** flag determines if maps should be unloaded at the first change or not */
    bool bUnloadUnneededMap;
};

#endif
