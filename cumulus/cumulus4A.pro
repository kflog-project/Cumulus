##################################################################
# Cumulus Android project file for qmake
#
# Copyright (c): 2010 by Josua Dietze
#                2012-2018 by Axel Pauli
#
# This file is distributed under the terms of the General Public
# License. See the file COPYING for more information.
#
# Note, that the QtCreator is used for the build!
#
##################################################################

TEMPLATE = app

# Put all generated objects into an extra directory
OBJECTS_DIR = .obj
MOC_DIR     = .obj
RCC_DIR     = .obj

QT += core gui xml

# needed by Android because we build a shared library
QMAKE_CXXFLAGS += -fpic

# Qt5 needs the QtWidgets library and a extra define, to handle that.
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
  DEFINES += QT_5
}

TARGET = Cumulus

# it seems the next two lines are important for Necessitas
CONFIG += mobility

MOBILITY =

CONFIG += qt \
          warn_on \
          release

# We need this variable because the build is done in another directory by QtCreator.
CUMDIR = ../cumulus4A

ASSETDIR = /home/axel/git/kflog-project/Cumulus/cumulus/android/assets

# The next lines shall force a compilation of the date stamp file, the qrc and qm files.
rm_build_date.commands = rm -f $(OBJECTS_DIR)/builddate.o \
                         $(OBJECTS_DIR)/qrc_cumulus.* \
                         $$CUMDIR/locale/de/cumulus_de.qm \
                         $$CUMDIR/locale/de/qt_de.qm

# http://stackoverflow.com/questions/7754218/qmake-how-to-add-and-use-a-variable-into-the-pro-file
# QMake uses its own syntax for variable references.
#
# VAR = foobar => Assign value to variable when qmake is run
# $$VAR => QMake variable's value at the time qmake is run
# $${VAR} => QMake variable's value at the time qmake is run (identical but enclosed to separate from surrounding text)
# $(VAR) => Contents of an Environment variable at the time Makefile (not qmake) is run
# $$(VAR) =>Contents of an Environment variable at the time qmake (not Makefile) is run

build_help_zip.commands = rm -f $$ASSETDIR/appData.zip; cd $$CUMDIR; zip -r -o $$ASSETDIR/appData.zip help

# Note! translations must be built first because the are linked
# into the cumulus binary
# qmake hints to build the extra targets are to find here:
# http://stackoverflow.com/questions/35847243/adding-custom-target-in-qmake

translate_cumulus.target   = $$CUMDIR/locale/de/cumulus_de.qm
translate_cumulus.depends  = $$CUMDIR/locale/de/cumulus_android_de.ts
translate_cumulus.commands = lrelease -removeidentical -nounfinished \
                               $$CUMDIR/locale/de/cumulus_android_de.ts \
                               -qm $$CUMDIR/locale/de/cumulus_de.qm

translate_qt.target   = $$CUMDIR/locale/de/qt_de.qm
translate_qt.depends  = $$CUMDIR/locale/de/qt4_de.ts
translate_qt.commands = lrelease -removeidentical -nounfinished $$CUMDIR/locale/de/qt4_de.ts -qm $$CUMDIR/locale/de/qt_de.qm

PRE_TARGETDEPS += rm_build_date $$CUMDIR/locale/de/cumulus_de.qm $$CUMDIR/locale/de/qt_de.qm

POST_TARGETDEPS += build_help_zip

QMAKE_EXTRA_TARGETS += rm_build_date \
                       translate_cumulus \
                       translate_qt \
                       build_help_zip

# These defines must be set for Android to enable/disable specific code parts
DEFINES += ANDROID CUMULUS

# Workaround for old g++ arm compiler where std::isnan is not working.
DEFINES += ISNAN_WO_STD

# Enable Flarm feature, if not wanted comment out the next line with a hash
CONFIG += flarm

# Enable Internet features, if not wanted comment out the next line with a hash
CONFIG += internet

# Activate QScroller in dependency of the Qt release
lessThan(QT_MAJOR_VERSION, 5) {
  # Activate this, if Qt class QScroller is not available. QT < 5
  CONFIG += qtscroller
}
else {
  # Activate this define, if Qt class QScroller is available. QT > 4
  DEFINES += QSCROLLER
}

# Must be always enabled now otherwise you will get compile errors.
CONFIG += numberpad

#version check for Qt 4.7 / Qt 5.x
! contains(QT_VERSION, ^4\\.[78]\\..*|^5\\..*) {
  message("Cannot build Cumulus with Qt version $${QT_VERSION}.")
  error("Use at least Qt 4.7. or higher!")
}

RESOURCES = cumulus.qrc

CONFIG += resources

HEADERS = \
    aboutwidget.h \
    airfield.h \
    AirfieldListWidget.h \
    AirfieldSelectionList.h \
    airregion.h \
    airspace.h \
    AirspaceHelper.h \
    airspacewarningdistance.h \
    altimeterdialog.h \
    altitude.h \
    androidevents.h \
    androidstyle.h \
    authdialog.h \
    basemapelement.h \
    calculator.h \
    colordialog.h \
    configwidget.h \
    CuLabel.h \
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
    GliderSelectionList.h \
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
    ListViewTabs.h \
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
    OpenAip.h \
    OpenAipLoaderThread.h \
    OpenAipPoiLoader.h \
    openairparser.h \
    PointListView.h \
    polardialog.h \
    polar.h \
    preflightchecklistpage.h \
    preflightgliderpage.h \
    preflightlogbookspage.h \
    preflightmiscpage.h \
    preflightretrievepage.h \
    preflightwaypointpage.h \
    preflightwidget.h \
    preflightwindpage.h \
    projectionbase.h \
    projectioncylindric.h \
    projectionlambert.h \
    protocol.h \
    radiopoint.h \
    RadioPointListWidget.h \
    reachablelist.h \
    reachablepoint.h \
    reachpointlistview.h \
    resource.h \
    rowdelegate.h \
    runway.h \
    SinglePointListWidget.h \
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
    SettingsPagePointData.h \
    SettingsPagePointDataLoading.h \
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
    AirfieldListWidget.cpp \
    AirfieldSelectionList.cpp \
    airregion.cpp \
    airspace.cpp \
    AirspaceHelper.cpp \
    altimeterdialog.cpp \
    altitude.cpp \
    androidstyle.cpp \
    authdialog.cpp \
    basemapelement.cpp \
    builddate.cpp \
    calculator.cpp \
    colordialog.cpp \
    configwidget.cpp \
    CuLabel.cpp \
    distance.cpp \
    elevationcolorimage.cpp \
    filetools.cpp \
    flighttask.cpp \
    fontdialog.cpp \
    generalconfig.cpp \
    glider.cpp \
    gliderflightdialog.cpp \
    gliderlistwidget.cpp \
    GliderSelectionList.cpp \
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
    ListViewTabs.cpp \
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
    OpenAip.cpp \
    OpenAipLoaderThread.cpp \
    OpenAipPoiLoader.cpp \
    openairparser.cpp \
    PointListView.cpp \
    polar.cpp \
    polardialog.cpp \
    preflightchecklistpage.cpp \
    preflightgliderpage.cpp \
    preflightlogbookspage.cpp \
    preflightmiscpage.cpp \
    preflightretrievepage.cpp \
    preflightwaypointpage.cpp \
    preflightwidget.cpp \
    preflightwindpage.cpp \
    projectionbase.cpp \
    projectioncylindric.cpp \
    projectionlambert.cpp \
    radiopoint.cpp \
    RadioPointListWidget.cpp \
    reachablelist.cpp \
    reachablepoint.cpp \
    reachpointlistview.cpp \
    rowdelegate.cpp \
    runway.cpp \
    SinglePointListWidget.cpp \
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
    SettingsPagePointData.cpp \
    SettingsPagePointDataLoading.cpp \
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
               flarmlogbook.h \
               flarmradarview.h \
               flarmwidget.h \
               preflightflarmpage.h \
							 preflightflarmusbpage.h \
               SettingsPageFlarm.h

    SOURCES += flarm.cpp \
               flarmaliaslist.cpp \
               flarmbase.cpp \
               flarmbincom.cpp \
               flarmbincomandroid.cpp \
               flarmcrc.cpp \
               flarmdisplay.cpp \
               flarmlistview.cpp \
               flarmlogbook.cpp \
               flarmradarview.cpp \
               flarmwidget.cpp \
               preflightflarmpage.cpp \
               preflightflarmusbpage.cpp \
               SettingsPageFlarm.cpp
               
    DEFINES += FLARM
}

internet {
		QT += network
		
		DEFINES += INTERNET
		
    HEADERS += airspacedownloaddialog.h \
               DownloadManager.h \
               httpclient.h \
		           LiveTrack24.h \
		           LiveTrack24Logger.h \
               preflightlivetrack24page.h \
               preflightweatherpage.h
                              
		SOURCES += airspacedownloaddialog.cpp \
		           DownloadManager.cpp \
		           httpclient.cpp \
		           LiveTrack24.cpp \
		           LiveTrack24Logger.cpp \
		           preflightlivetrack24page.cpp \
               preflightweatherpage.cpp
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

# suppress passing rpath option to the linker. Android is complaining about that.
QMAKE_LFLAGS_RPATH =

LIBS += -lstdc++

TRANSLATIONS = locale/de/cumulus_android_de.ts

CODECFORSRC = UTF-8
