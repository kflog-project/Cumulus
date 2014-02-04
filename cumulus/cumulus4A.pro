##################################################################
# Cumulus Android project file for qmake
#
# Copyright (c): 2010 by Josua Dietze
#                2012-2013 by Axel Pauli
#
# This file is distributed under the terms of the General Public
# License. See the file COPYING for more information.
#
# $Id$
#
# Note, that the SDK Necessitas is used for the build!
#
##################################################################

QT += core gui xml

# Qt5 needs the QtWidgets library
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Cumulus

TEMPLATE = app

# it seems the next two lines are important for Necessitas
CONFIG += mobility

MOBILITY =

CONFIG += qt \
          warn_on \
          release

# The next 3 lines shall force a compilation of the date stamp file
rm_build_date.commands = rm -f $(OBJECTS_DIR)/builddate.o

QMAKE_EXTRA_TARGETS += rm_build_date

PRE_TARGETDEPS += rm_build_date

# These defines must be set for Android to enable/disable specific code parts
DEFINES += ANDROID CUMULUS

# Enable Flarm feature, if not wanted comment out the next line with a hash
CONFIG += flarm

# Enable Internet features, if not wanted comment out the next line with a hash
CONFIG += internet

# Enable Welt2000 reloading via an extra thread, if not wanted comment out the
# next line with a hash
CONFIG += welt2000thread

# Enable classical menu bar, if define is set. Otherwise a context menu is used.
# DEFINES += USE_MENUBAR

# Activate this define, if Qt class QScroller is available.
# DEFINES += QSCROLLER

# Activate this, if Qt class QScroller is not available.
CONFIG += qtscroller

# Must be always enabled now otherwise you will get compile errors.
CONFIG += numberpad

#version check for Qt 4.7 / Qt 5.x
! contains(QT_VERSION, ^4\\.[78]\\..*|^5\\..*) {
  message("Cannot build Cumulus with Qt version $${QT_VERSION}.")
  error("Use at least Qt 4.7. or higher!")
}

HEADERS = \
    aboutwidget.h \
    airfield.h \
    airfieldlistview.h \
    airfieldlistwidget.h \
    airregion.h \
    airspace.h \
    AirspaceHelper.h \
    airspacewarningdistance.h \
    altimetermodedialog.h \
    altitude.h \
    androidevents.h \
    androidstyle.h \
    authdialog.h \
    basemapelement.h \
    calculator.h \
    configwidget.h \
    datatypes.h \
    distance.h \
    elevationcolorimage.h \
    filetools.h \
    flighttask.h \
    fontdialog.h \
    generalconfig.h \
    gliderflightdialog.h \
    glider.h \
    gliderlistwidget.h \
    gpsconandroid.h \
    gpsnmea.h \
    gpsstatusdialog.h \
    helpbrowser.h \
    hwinfo.h \
    igclogger.h \
    interfaceelements.h \
    isohypse.h \
    isolist.h \
    jnisupport.h \
    layout.h \
    limitedlist.h \
    lineelement.h \
    listviewfilter.h \
    listwidgetparent.h \
    logbook.h \
    mainwindow.h \
    mapcalc.h \
    mapconfig.h \
    mapcontents.h \
    mapdefaults.h \
    map.h \
    mapinfobox.h \
    mapmatrix.h \
    mapview.h \
    messagehandler.h \
    messagewidget.h \
    multilayout.h \
    openaip.h \
    openaipairfieldloader.h \
    openairparser.h \
    polardialog.h \
    polar.h \
    preflightgliderpage.h \
    preflightlogbookspage.h \
    preflightmiscpage.h \
    preflightretrievepage.h \
    preflightwaypointpage.h \
    preflightwidget.h \
    projectionbase.h \
    projectioncylindric.h \
    projectionlambert.h \
    protocol.h \
    radiopoint.h \
    reachablelist.h \
    reachablepoint.h \
    reachpointlistview.h \
    resource.h \
    rowdelegate.h \
    runway.h \
    settingspageairfields.h \
    settingspageairfieldloading.h \
    settingspageairspace.h \
    settingspageairspaceloading.h \
    settingspageglider.h \
    settingspagegps4a.h \
    settingspageinformation.h \
    settingspagelines.h \
    settingspagelooknfeel.h \
    settingspagemapobjects.h \
    settingspagemapsettings.h \
    settingspagepersonal.h \
    settingspagetask.h \
    settingspageterraincolors.h \
    settingspageunits.h \
    signalhandler.h \
    singlepoint.h \
    sonne.h \
    sound.h \
    speed.h \
    splash.h \
    target.h \
    taskeditor.h \
    taskfilemanager.h \
    taskline.h \
    tasklistview.h \
    taskpoint.h \
    taskpointeditor.h \
    taskpointtypes.h \
    time_cu.h \
    tpinfowidget.h \
    vario.h \
    variomodedialog.h \
    varspinbox.h \
    vector.h \
    waitscreen.h \
    waypointcatalog.h \
    waypoint.h \
    waypointlistview.h \
    waypointlistwidget.h \
    welt2000.h \
    wgspoint.h \
    whatsthat.h \
    windanalyser.h \
    windmeasurementlist.h \
    windstore.h \
    wpeditdialog.h \
    wpeditdialogpageaero.h \
    wpeditdialogpagegeneral.h \
    wpinfowidget.h

SOURCES = \
    aboutwidget.cpp \
    airfield.cpp \
    airfieldlistview.cpp \
    airfieldlistwidget.cpp \
    airregion.cpp \
    airspace.cpp \
    AirspaceHelper.h \
    altimetermodedialog.cpp \
    altitude.cpp \
    androidstyle.cpp \
    authdialog.cpp \
    basemapelement.cpp \
    builddate.cpp \
    calculator.cpp \
    configwidget.cpp \
    distance.cpp \
    elevationcolorimage.cpp \
    filetools.cpp \
    flighttask.cpp \
    fontdialog.cpp \
    generalconfig.cpp \
    glider.cpp \
    gliderflightdialog.cpp \
    gliderlistwidget.cpp \
    gpsconandroid.cpp \
    gpsnmea.cpp \
    gpsstatusdialog.cpp \
    helpbrowser.cpp \
    hwinfo.cpp \
    igclogger.cpp \
    isohypse.cpp \
    isolist.cpp \
    jnisupport.cpp \
    layout.cpp \
    lineelement.cpp \
    listviewfilter.cpp \
    listwidgetparent.cpp \
    logbook.cpp \
    main.cpp \
    mainwindow.cpp \
    mapcalc.cpp \
    mapconfig.cpp \
    mapcontents.cpp \
    map.cpp \
    mapinfobox.cpp \
    mapmatrix.cpp \
    mapview.cpp \
    messagehandler.cpp \
    messagewidget.cpp \
    openaip.cpp \
    openaipairfieldloader.cpp \
    openairparser.cpp \
    polar.cpp \
    polardialog.cpp \
    preflightgliderpage.cpp \
    preflightlogbookspage.cpp \
    preflightmiscpage.cpp \
    preflightretrievepage.cpp \
    preflightwaypointpage.cpp \
    preflightwidget.cpp \
    projectionbase.cpp \
    projectioncylindric.cpp \
    projectionlambert.cpp \
    radiopoint.cpp \
    reachablelist.cpp \
    reachablepoint.cpp \
    reachpointlistview.cpp \
    rowdelegate.cpp \
    runway.cpp \
    settingspageairfields.cpp \
    settingspageairfieldloading.cpp \
    settingspageairspace.cpp \
    settingspageairspaceloading.cpp \
    settingspageglider.cpp \
    settingspagegps4a.cpp \
    settingspageinformation.cpp \
    settingspagelines.cpp \
    settingspagelooknfeel.cpp \
    settingspagemapobjects.cpp \
    settingspagemapsettings.cpp \
    settingspagepersonal.cpp \
    settingspagetask.cpp \
    settingspageterraincolors.cpp \
    settingspageunits.cpp \
    signalhandler.cpp \
    singlepoint.cpp \
    sonne.cpp \
    sound.cpp \
    speed.cpp \
    splash.cpp \
    taskeditor.cpp \
    taskfilemanager.cpp \
    taskline.cpp \
    tasklistview.cpp \
    taskpoint.cpp \
    taskpointeditor.cpp \
    time_cu.cpp \
    tpinfowidget.cpp \
    vario.cpp \
    variomodedialog.cpp \
    varspinbox.cpp \
    vector.cpp \
    waitscreen.cpp \
    waypointcatalog.cpp \
    waypoint.cpp \
    waypointlistview.cpp \
    waypointlistwidget.cpp \
    welt2000.cpp \
    wgspoint.cpp \
    whatsthat.cpp \
    windanalyser.cpp \
    windmeasurementlist.cpp \
    windstore.cpp \
    wpeditdialog.cpp \
    wpeditdialogpageaero.cpp \
    wpeditdialogpagegeneral.cpp \
    wpinfowidget.cpp

flarm {
    DEFINES += FLARM

    HEADERS += flarm.h \
               flarmaliaslist.h \
               flarmbase.h \
               flarmbincom.h \
               flarmbincomandroid.h \
               flarmcrc.h \
               flarmdisplay.h \
               flarmlistview.h \
               flarmradarview.h \
               flarmwidget.h \
               preflightflarmpage.h \
               flarmlogbook.h

    SOURCES += flarm.cpp \
               flarmaliaslist.cpp \
               flarmbase.cpp \
               flarmbincom.cpp \
               flarmbincomandroid.cpp \
               flarmcrc.cpp \
               flarmdisplay.cpp \
               flarmlistview.cpp \
               flarmradarview.cpp \
               flarmwidget.cpp \
               preflightflarmpage.cpp \
               flarmlogbook.cpp

    DEFINES += FLARM
}

internet {
		QT += network
		
		DEFINES += INTERNET
		
    HEADERS += airspacedownloaddialog.h \
               downloadmanager.h \
               httpclient.h \
		           LiveTrack24.h \
		           LiveTrack24Logger.h \
               preflightlivetrack24page.h \
               preflightweatherpage.h \
               proxydialog.h
                              
		SOURCES += airspacedownloaddialog.cpp \
		           downloadmanager.cpp \
		           httpclient.cpp \
		           LiveTrack24.cpp \
		           LiveTrack24Logger.cpp \
		           preflightlivetrack24page.cpp \
               preflightweatherpage.cpp \
		           proxydialog.cpp
}

welt2000thread {
    DEFINES += WELT2000_THREAD
}

numberpad {
    HEADERS += coordeditnumpad.h \
               doubleNumberEditor.h \
               glidereditornumpad.h \
               numberEditor.h \
               numberInputPad.h \
               preflighttaskpage.h \
               settingspageairspacefillingnumpad.h \
               settingspageairspacewarningsnumpad.h
    
    SOURCES += coordeditnumpad.cpp \
               doubleNumberEditor.cpp \
               glidereditornumpad.cpp \
               numberEditor.cpp \
               numberInputPad.cpp \
               preflighttaskpage.cpp \
               settingspageairspacefillingnumpad.cpp \
               settingspageairspacewarningsnumpad.cpp
}

qtscroller {

    DEFINES += QTSCROLLER_NO_WEBKIT QTSCROLLER

    INCLUDEPATH += QtScroller QtScroller/include QtScroller/src

    HEADERS += QtScroller/src/qtflickgesture_p.h \
               QtScroller/src/qtscroller.h \
               QtScroller/src/qtscroller_p.h \
               QtScroller/src/qtscrollerfilter_p.h \
               QtScroller/src/qtscrollerproperties.h \
               QtScroller/src/qtscrollerproperties_p.h \
               QtScroller/src/qtscrollevent.h \
               QtScroller/src/qtscrollevent_p.h

    SOURCES += QtScroller/src/qtflickgesture.cpp \
               QtScroller/src/qtscroller.cpp \
               QtScroller/src/qtscrollerfilter.cpp \
               QtScroller/src/qtscrollerproperties.cpp \
               QtScroller/src/qtscrollevent.cpp
}

# Files managed and needed by Necessitas
OTHER_FILES += \
    android/res/drawable-ldpi/icon.png \
    android/res/values-pt-rBR/strings.xml \
    android/res/drawable/icon.png \
    android/res/drawable/logo.png \
    android/res/values-ms/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-et/strings.xml \
    android/res/layout/splash.xml \
    android/res/values/strings.xml \
    android/res/values/libs.xml \
    android/res/drawable-mdpi/icon.png \
    android/res/values-es/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-el/strings.xml \
    android/res/drawable-hdpi/icon.png \
    android/AndroidManifest.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/assets/appData.zip \
    android/assets/addData.zip \
    android/AndroidManifest.xml \
    android/res/drawable-hdpi/icon.png \
    android/res/values-ro/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/layout/splash.xml \
    android/res/values-nb/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values/strings.xml \
    android/res/values/libs.xml \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/drawable/logo.png \
    android/res/drawable/icon.png \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-ms/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kflog/cumulus/CumulusActivity.java \
    android/src/org/kflog/cumulus/BluetoothService.java \
    android/version.xml \
    android/res/values/cumulus.xml \
    android/res/values-de/cumulus.xml \
    android/assets/QtIndustrius-14.jar

LIBS += -lstdc++

TRANSLATIONS = cumulus_android_de.ts
