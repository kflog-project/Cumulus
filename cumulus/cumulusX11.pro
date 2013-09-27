##################################################################
# Cumulus Qt5/X11 project file for qmake
#
# Compatibility for Qt5 by Eggert Ehmke
#
# Copyright (c): 2008-2013 Axel Pauli
#
# This file is distributed under the terms of the General Public
# License. See the file COPYING for more information.
#
# $Id$
##################################################################

TEMPLATE = app

# Put all generated objects into an extra directory
OBJECTS_DIR = .obj
MOC_DIR     = .obj

QT += gui xml

# Qt5 needs the QtWidgets library
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
DEFINES += QT_5
}

# CONFIG = qt warn_on release

CONFIG = debug \
         qt \
         warn_on

# The next 3 lines shall force a compilation of the date stamp file
rm_build_date.commands = rm -f $(OBJECTS_DIR)/builddate.o

QMAKE_EXTRA_TARGETS += rm_build_date

PRE_TARGETDEPS += rm_build_date

# Enable Flarm feature, if not wanted comment out the next line with a hash
CONFIG += flarm

# Enable Internet features, if not wanted comment out the next line with a hash
CONFIG += internet

# Enable bluetooth feature, if not wanted comment out the next line with a hash
CONFIG += bluetooth

# Enable Welt2000 reloading via an extra thread, if not wanted comment out the
# next line with a hash
CONFIG += welt2000thread

# Enable classical menu bar, if define is set. Otherwise a context menu is used.
# DEFINES += USE_MENUBAR

# Activate this define, if Qt class QScroller is available.
# DEFINES += QSCROLLER

# Activate this, if Qt class QScroller is not available.
# CONFIG += qtscroller

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
    airspacewarningdistance.h \
    altimetermodedialog.h \
    altitude.h \
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
    glidereditor.h \
    gliderflightdialog.h \
    glider.h \
    gliderlistwidget.h \
    gpscon.h \
    gpsnmea.h \
    gpsstatusdialog.h \
    helpbrowser.h \
    hwinfo.h \
    igclogger.h \
    interfaceelements.h \
    ipc.h \
    isohypse.h \
    isolist.h \
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
    preflightmiscpage.h \
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
    settingspagegps.h \
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
    taskpointeditor.h \
    taskpoint.h \
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
    altimetermodedialog.cpp \
    altitude.cpp \
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
    glidereditor.cpp \
    gliderflightdialog.cpp \
    gliderlistwidget.cpp \
    gpscon.cpp \
    gpsnmea.cpp \
    gpsstatusdialog.cpp \
    helpbrowser.cpp \
    hwinfo.cpp \
    igclogger.cpp \
    ipc.cpp \
    isohypse.cpp \
    isolist.cpp \
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
    preflightmiscpage.cpp \
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
    settingspagegps.cpp \
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
		           flarmdisplay.h \
		           flarmlistview.h \
		           flarmlogbook.h \
		           flarmradarview.h \
		           flarmwidget.h \
		           preflightflarmpage.h

		SOURCES += flarm.cpp \
		           flarmaliaslist.cpp \
		           flarmbase.cpp \
		           flarmdisplay.cpp \
		           flarmlistview.cpp \
               flarmlogbook.cpp \
		           flarmradarview.cpp \
		           flarmwidget.cpp \
		           preflightflarmpage.cpp
}

internet {
		QT += network
		
		DEFINES += INTERNET
		
    HEADERS += airspacedownloaddialog.h \
               downloadmanager.h \
               httpclient.h \
               preflightweatherpage.h \
               proxydialog.h
               
                              
		SOURCES += airspacedownloaddialog.cpp \
		           downloadmanager.cpp \
		           httpclient.cpp \
               preflightweatherpage.cpp \
		           proxydialog.cpp
}

bluetooth {
    DEFINES += BLUEZ
  
    HEADERS += bluetoothdevices.h
    
    SOURCES += bluetoothdevices.cpp
    
    LIBS += -lbluetooth
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

TARGET = cumulus

DESTDIR = .

INCLUDEPATH += ../

QMAKE_CXXFLAGS += -fno-default-inline \
                  -fno-inline -Wextra
    
LIBS += -lstdc++

TRANSLATIONS = cumulus_de.ts

CODECFORSRC = UTF-8
