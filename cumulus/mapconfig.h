/***********************************************************************
**
**   mapconfig.h
**
**   This file is part of Cumulus.
**   It is modified for Cumulus by André Somers in 2002 and
**   by Axel Pauli in 2009
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, 2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAP_CONFIG_H
#define MAP_CONFIG_H

#include <QObject>
#include <QPen>
#include <QBrush>
#include <QPixmap>
#include <QString>
#include <QMap>
#include <QIcon>

/**
 * @author Heiner Lamprecht, Florian Ehinger
 *
 * This class takes care of the configuration data for displaying
 * map elements. To avoid problems, there should be only
 * one element per application.
 *
 * All printing related code has been removed for Cumulus, because printing
 * will not be supported on the PDA. (André Somers)
 *
 * @AP: Different load options have been removed from this class to reduce
 * calling overhead. Furthermore all pointer classes have been replaced
 * by value classes.
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
        return topographyColorList[heightIndex];
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
     * The possible data types, that could be drawn.
     *
     * @see #slotSetFlightDataType
     */

    bool isRotatable( unsigned int typeID ) const;

public slots:
    /**
     * Forces MapConfig to reload its configuration data.
     */
    void slotReadConfig();

    /**
     * Airspace colors can be modified by the user in the airspace settings
     * configuration widget. In such a case all color data in the lists must
     * be cleared and reloaded.
     */
    void slotReloadAirspaceColors();

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

    // Color list for contour areas
    QColor topographyColorList[51];

    // Pen and brush lists of different map items stored in arrays
    QPen airAPenList[4];
    QBrush airABrushList[4];
    QPen airBPenList[4];
    QBrush airBBrushList[4];
    QPen airCPenList[4];
    QBrush airCBrushList[4];
    QPen airDPenList[4];
    QBrush airDBrushList[4];
    QPen airEPenList[4];
    QBrush airEBrushList[4];
    QPen waveWindowPenList[4];
    QBrush waveWindowBrushList[4];
    QPen airFPenList[4];
    QBrush airFBrushList[4];
    QPen ctrCPenList[4];
    QBrush ctrCBrushList[4];
    QPen ctrDPenList[4];
    QBrush ctrDBrushList[4];
    QPen lowFPenList[4];
    QBrush lowFBrushList[4];
    QPen dangerPenList[4];
    QBrush dangerBrushList[4];
    QPen restrPenList[4];
    QBrush restrBrushList[4];
    QPen tmzPenList[4];
    QBrush tmzBrushList[4];
    QPen gliderSectorPenList[4];
    QBrush gliderSectorBrushList[4];
    QPen highwayPenList[4];
    QPen roadPenList[4];
    QPen trailPenList[4];
    QPen railPenList[4];
    QPen rail_dPenList[4];
    QPen aerialcablePenList[4];
    QPen lakePenList[4];
    QBrush lakeBrushList[4];
    QPen riverPenList[4];
    QPen river_tPenList[4];
    QBrush river_tBrushList[4];
    QPen canalPenList[4];
    QPen cityPenList[4];
    QBrush cityBrushList[4];
    QPen forestPenList[4];
    QPen glacierPenList[4];
    QPen packicePenList[4];
    QBrush forestBrushList[4];
    QBrush glacierBrushList[4];
    QBrush packiceBrushList[4];

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
    bool airEBorder[4];
    bool waveWindowBorder[4];
    bool airFBorder[4];
    bool ctrCBorder[4];
    bool ctrDBorder[4];
    bool dangerBorder[4];
    bool lowFBorder[4];
    bool restrBorder[4];
    bool tmzBorder[4];
    bool gliderSectorBorder[4];
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

    // number of created class instances
    static short instances;
};

#endif
