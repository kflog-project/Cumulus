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

TARGET   = Cumulus
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

# Activate this feature, if class FlickCharm shall be used for kinetic finger scrolling.
CONFIG += flickcharm

# Enable this feature, if the own number key pad shall be used for number input.
CONFIG += numberpad

HEADERS = \
    aboutwidget.h \
    airfieldlistview.h \
    airfieldlistwidget.h \
    airfield.h \
    airregion.h \
    airspace.h \
    airspacewarningdistance.h \
    altimetermodedialog.h \
    altitude.h \
    androidevents.h \
    androidstyle.h \
    authdialog.h \
    basemapelement.h \
    configwidget.h \
    calculator.h \
    datatypes.h \
    distance.h \
    elevationcolorimage.h \
    filetools.h \
    flighttask.h \
    fontdialog.h \
    generalconfig.h \
    glider.h \
    glidereditor.h \
    gliderflightdialog.h \
    gliderlistwidget.h \
    gpsconandroid.h \
    gpsnmea.h \
    gpsstatusdialog.h \
    coordedit.h \
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
    openairparser.h \
    polardialog.h \
    polar.h \
    preflightwidget.h \
    preflightgliderpage.h \
    preflightmiscpage.h \
    preflighttasklist.h \
    preflightwaypointpage.h \
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
    settingspageairspace.h \
    settingspageairspacefilling.h \
    settingspageairspaceloading.h \
    settingspageairspacewarnings.h \
    settingspageglider.h \
    settingspagegps4a.h \
    settingspageinformation.h \
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
    tasklistview.h \
    taskpoint.h \
    time_cu.h \
    tpinfowidget.h \
    vario.h \
    variomodedialog.h \
    varspinbox.h \
    vector.h \
    waitscreen.h \
    waypointcatalog.h \
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
    waypoint.h \
    wpinfowidget.h

SOURCES = \
    aboutwidget.cpp \
    airfieldlistview.cpp \
    airfieldlistwidget.cpp \
    airfield.cpp \
    airregion.cpp \
    airspace.cpp \
    altimetermodedialog.cpp \
    altitude.cpp \
    androidstyle.cpp \
    authdialog.cpp \
    basemapelement.cpp \
    builddate.cpp \
    configwidget.cpp \
    calculator.cpp \
    distance.cpp \
    elevationcolorimage.cpp \
    filetools.cpp \
    flighttask.cpp \
    fontdialog.cpp \
    generalconfig.cpp \
    glider.cpp \
    glidereditor.cpp \
    gliderflightdialog.cpp \
    gliderlistwidget.cpp \
    gpsconandroid.cpp \
    gpsnmea.cpp \
    gpsstatusdialog.cpp \
    coordedit.cpp \
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
    openairparser.cpp \
    polar.cpp \
    polardialog.cpp \
    preflightwidget.cpp \
    preflightgliderpage.cpp \
    preflightmiscpage.cpp \
    preflighttasklist.cpp \
    preflightwaypointpage.cpp \
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
    settingspageairspace.cpp \
    settingspageairspacefilling.cpp \
    settingspageairspaceloading.cpp \
    settingspageairspacewarnings.cpp \
    settingspageglider.cpp \
    settingspagegps4a.cpp \
    settingspageinformation.cpp \
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
    target.h \
    taskeditor.cpp \
    tasklistview.cpp \
    taskpoint.cpp \
    time_cu.cpp \
    tpinfowidget.cpp \
    vario.cpp \
    variomodedialog.cpp \
    varspinbox.cpp \
    vector.cpp \
    waitscreen.cpp \
    waypointcatalog.cpp \
    waypointlistview.cpp \
    waypointlistwidget.cpp \
    welt2000.cpp \
    wgspoint.cpp \
    whatsthat.cpp \
    windanalyser.cpp \
    windmeasurementlist.cpp \
    windstore.cpp \
    waypoint.cpp \
    wpeditdialog.cpp \
    wpeditdialogpageaero.cpp \
    wpeditdialogpagegeneral.cpp \
    wpinfowidget.cpp

flarm {
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

    HEADERS += airspacedownloaddialog.h \
               downloadmanager.h \
               httpclient.h \
               proxydialog.h

    SOURCES += airspacedownloaddialog.cpp \
               downloadmanager.cpp \
               httpclient.cpp \
               proxydialog.cpp

    DEFINES += INTERNET
}

welt2000thread {
    DEFINES += WELT2000_THREAD
}

flickcharm {
    DEFINES += FLICK_CHARM
    
    HEADERS += flickcharm.h
    
    SOURCES += flickcharm.cpp
}

numberpad {
    DEFINES += USE_NUM_PAD

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
