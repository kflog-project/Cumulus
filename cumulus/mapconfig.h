/***********************************************************************
**
**   mapconfig.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2001      by Heiner Lamprecht,
**                   2002      by André Somers
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
#include <QColor>

/**
 * \class MapConfig
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Map configuration class.
 *
 * This class takes care of the configuration data for displaying
 * map elements. To avoid problems, there should be only
 * one element per application.
 *
 * All printing related code has been removed for Cumulus, because printing
 * will not be supported on the PDA. (André Somers)
 *
 * Different load options have been removed from this class to reduce
 * calling overhead. Furthermore all pointer classes have been replaced
 * by value classes.
 *
 * \date 2001-2015
 *
 * \version 1.2
 */

class MapConfig : public QObject
{
  Q_OBJECT

 private:

   Q_DISABLE_COPY ( MapConfig )

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
     * @param  typeID  The type identifier of the element.
     *
     * @return "true", if the current scale is smaller than the switch-scale,
     *         so that small icons should be used for displaying.
     */
    bool isBorder(unsigned int typeID);

    /**
     * @param  typeID  The type identifier of the element.
     *
     * @return the pen for drawing a map element.
     */
    const QPen& getDrawPen(unsigned int typeID)
    {
      return __getPen(typeID, scaleIndex);
    };

    /**
     * @param  typeID  The type identifier of the element.
     *
     * @return the brush for drawing an area element.
     */
    const QBrush& getDrawBrush(unsigned int typeID)
    {
      return __getBrush(typeID, scaleIndex);
    };

    /**
     * @param  iconName  The name of the icon to load
     *
     * @param doScale Do scale the pixmap with the current set scale.
     *
     * @returns the icon-pixmap of the element.
     */
    QPixmap getPixmap( QString iconName, const bool doScale );

    /**
     * @param  typeID  The type identifier of the element.
     * @param  isWinch  Used only for glider sites to determine, if the
     *                  icon should indicate that only winch launch is
     *                  available.
     * @returns the rotatable icon-pixmap of the element.
     */
    QPixmap getPixmap(unsigned int typeID, bool isWinch = true);

    /**
     * @param  typeID   The type identifier of the element.
     * @param  isWinch  Used only for glider sites to determine, if the
     *                  icon should indicate that only winch launch is
     *                  available.
     * @param  smallIcon  Used to select the size of the returned pixmap.
     *                  if true, a small pixmap is returned, otherwise the larger
     *                  version is returned.
     * @returns The icon-pixmap of the element.
     */
    QPixmap getPixmap(unsigned int typeID, bool isWinch, bool smallIcon);

    /**
     * @param typeID The type identifier of the element.
     *
     * @returns An icon for use in an airfield list.
     */
    const QIcon& getListIcon(unsigned int typeID)
      {
        return airfieldIcon[typeID];
      };

    /**
     * @param  type  The type identifier of the element.
     *
     * @param  isWinch  Used only for glider sites to determine, if the
     *                  icon should indicate that only winch launch is
     *                  available.
     *
     * @return the name of the pixmap of the element.
     */
    QString getPixmapName( unsigned int type, bool isWinch = true );

    /**
     * @return True if small icons are used, else returns false.
     */
    bool useSmallIcons() const
    {
      return !isSwitch;
    };

    /**
     * The possible data types, that could be drawn.
     */
    bool isRotatable( unsigned int typeID ) const;

    /**
     * Returns a pixmap containing a circle in the wanted size
     * and filled with green color. The circle has no border and
     * is transparent.
     */
    QPixmap& getGreenCircle( int diameter );

    /**
      * Returns a pixmap containing a circle in the wanted size
      * and filled with magenta color. The circle has no border and
      * is transparent.
      */
    QPixmap& getMagentaCircle( int diameter );

    /**
      * Returns a pixmap containing a plus button. The button has rounded
      * corners and is transparent.
      */
    QPixmap& getPlusButton();

    /**
      * Returns a pixmap containing a minus button. The button has rounded
      * corners and is transparent.
      */
    QPixmap& getMinusButton();

    /**
      * Returns a pixmap containing a cross. That pixmap is used, if no GPS
      * connection is available.
      *
      * \return A pixmap with a cross scaled with the current set scale.
      */
    QPixmap& getCross();

    /**
      * Returns a pixmap containing a glider. That pixmap is used, if a GPS
      * connection is available to show the movement direction.
      *
      * \param heading Heading direction of glider.
      *
      * \param scale Scale factor for pixmap to be returned
      *
      * \return A pixmap with a glider scaled with the current set scale.
      */
    QPixmap createGlider( const int heading, float scale=3.5 );

    /**
      * Returns a pixmap containing an airfiled with an runway. That pixmap
      * is used as a map icon. The pixmap is scaled with the current set scale.
      *
      * \param heading Heading direction of runway.
      *
      * \param size Base size of icon.
      *
      * \param small Provide a small icon.
      *
      * \return A pixmap with an airfield scaled with the current set scale.
      */
    QPixmap createAirfield( const int heading, float size, bool small  );

    /**
      * Returns a pixmap containing a landing field with an runway. That pixmap
      * is used as a map icon. The pixmap is scaled with the current set scale.
      *
      * \param heading Heading direction of runway.
      *
      * \param size Base size of icon.
      *
      * \param small Provide a small icon.
      *
      * \return A pixmap with a landing field scaled with the current set scale.
      */
    QPixmap createLandingField( const int heading, float size, bool small  );

    /**
      * Returns a pixmap containing a circle in the wanted size
      * and filled with wanted color. The circle has no border and
      * is semi-transparent.
      */
    static void createCircle( QPixmap& pixmap, int diameter,
                              QColor color, double opacity=0.5,
                              QColor bg=Qt::transparent,
                              QPen pen=Qt::NoPen );
    /**
      * Returns a pixmap containing a square in the wanted size
      * and filled with wanted color. The square has no border and
      * is semi-transparent.
      */
    static void createSquare( QPixmap& pixmap, int size,
                              QColor color, double opacity=0.5,
                              QPen pen=Qt::NoPen );
    /**
      * Draws on a pixmap a triangle in the wanted size,
      * filled with the wanted color and rotated in the wanted direction.
      * The triangle has no border and is semi-transparent. The top of the
      * triangle is north oriented if rotation is zero.
      *
      * @param pixmap Reference to pixmap which contains the drawn triangle
      * @param size Size of the pixmap to be drawn
      * @param color fill color of triangle
      * @param rotate rotation angle in degree of triangle
      * @param opacity a value between 0.0 ... 1.0
      * @param bg background color of pixmap. Default is set to transparent.
      * @param pen to be used for outlining
      */
    static void createTriangle( QPixmap& pixmap, int size,
                                QColor color, int rotate=0,
                                double opacity=0.5,
                                QColor bg=Qt::transparent,
                                QPen pen=Qt::NoPen );

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
    const QPen& __getPen(unsigned int typeID, int scaleIndex);

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
    QPen airFirPenList[4];
    QBrush airFirBrushList[4];
    QPen ctrPenList[4];
    QBrush ctrBrushList[4];
    QPen ctrCPenList[4];
    QBrush ctrCBrushList[4];
    QPen ctrDPenList[4];
    QBrush ctrDBrushList[4];
    QPen lowFPenList[4];
    QBrush lowFBrushList[4];
    QPen dangerPenList[4];
    QBrush dangerBrushList[4];
    QPen prohibitedPenList[4];
    QBrush prohibitedBrushList[4];
    QPen restrPenList[4];
    QBrush restrBrushList[4];
    QPen rmzPenList[4];
    QBrush rmzBrushList[4];
    QPen tmzPenList[4];
    QBrush tmzBrushList[4];
    QPen gliderSectorPenList[4];
    QBrush gliderSectorBrushList[4];
    QPen motorwayPenList[4];
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
    bool airFirBorder[4];
    bool ctrBorder[4];
    bool ctrCBorder[4];
    bool ctrDBorder[4];
    bool dangerBorder[4];
    bool prohibitedBorder[4];
    bool lowFBorder[4];
    bool restrBorder[4];
    bool rmzBorder[4];
    bool tmzBorder[4];
    bool gliderSectorBorder[4];
    bool trailBorder[4];
    bool roadBorder[4];
    bool motorwayBorder[4];
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

    // Pixmaps for reach abilities
    QPixmap greenCircle;
    QPixmap magentaCircle;

    // Pixmaps for zoom buttons +/-
    QPixmap plusButton;
    QPixmap minusButton;

    // number of created class instances
    static short instances;
};

extern MapConfig* _globalMapConfig;

#endif
