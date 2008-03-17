# Cumulus Maemo project file for qmake
# $Id$

QT          += qt3support
TEMPLATE    = app
CONFIG      = qt warn_on release
#CONFIG      = debug qt warn_on

HEADERS     = \
    airfieldlistview.h \
    airport.h \
    airregion.h \
    airspace.h \
    altimetermodedialog.h \
    altitude.h \
    basemapelement.h \
    colorlistviewitem.h \
    configdialog.h \
    cucalc.h \
    cumulusapp.h \
    degreespinbox.h \
    distance.h \
    filetools.h \
    flighttask.h \
    generalconfig.h \
    gliderflightdialog.h \
    glider.h \
    gliderlist.h \
    glidersite.h \
    gpscon.h \
    gpsnmea.h \
    gpsstatusdialog.h \
    coordedit.h \
    helpbrowser.h \
    hwinfo.h \
    igclogger.h \
    interfaceelements.h \
    ipc.h \
    isohypse.h \
    isolist.h \
    limitedlist.h \
    lineelement.h \
    listviewfilter.h \
    mapcalc.h \
    mapconfig.h \
    mapcontents.h \
    mapdefaults.h \
    mapelementlist.h \
    map.h \
    mapinfobox.h \
    mapmatrix.h \
    mapview.h \
    messagehandler.h \
    multilayout.h \
    openairparser.h \
    polardialog.h \
    polar.h \
    preflightdialog.h \
    preflightgliderpage.h \
    preflightmiscpage.h \
    projectionbase.h \
    projectioncylindric.h \
    projectionlambert.h \
    protocol.h \
    radiopoint.h \
    reachablelist.h \
    reachpointlistview.h \
    resource.h \
    runway.h \
    settingspageairspace.h \
    settingspagegps.h \
    settingspageinformation.h \
    settingspagemapadv.h \
    settingspagemap.h \
    settingspagepersonal.h \
    settingspagepolar.h \
    settingspagesector.h \
    settingspageunits.h \
    signalhandler.h \
    singlepoint.h \
    sonne.h \
    sound.h \
    speed.h \
    target.h \
    taskdialog.h \
    tasklist.h \
    tasklistview.h \
    tpinfowidget.h \
    vario.h \
    variomodedialog.h \
    vector.h \
    waitscreen.h \
    waypointcatalog.h \
    waypointlistview.h \
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

SOURCES     = \
    airfieldlistview.cpp \
    airport.cpp \
    airregion.cpp \
    airspace.cpp \
    altimetermodedialog.cpp \
    altitude.cpp \
    basemapelement.cpp \
    colorlistviewitem.cpp \
    configdialog.cpp \
    cucalc.cpp \
    cumulusapp.cpp \
    degreespinbox.cpp \
    distance.cpp \
    filetools.cpp \
    flighttask.cpp \
    generalconfig.cpp \
    glider.cpp \
    gliderflightdialog.cpp \
    gliderlist.cpp \
    glidersite.cpp \
    gpscon.cpp \
    gpsnmea.cpp \
    gpsstatusdialog.cpp \
    coordedit.cpp \
    helpbrowser.cpp \
    hwinfo.cpp \
    igclogger.cpp \
    ipc.cpp \
    isohypse.cpp \
    isolist.cpp \
    lineelement.cpp \
    listviewfilter.cpp \
    main.cpp \
    mapcalc.cpp \
    mapconfig.cpp \
    mapcontents.cpp \
    map.cpp \
    mapelementlist.cpp \
    mapinfobox.cpp \
    mapmatrix.cpp \
    mapview.cpp \
    messagehandler.cpp \
    openairparser.cpp \
    polar.cpp \
    polardialog.cpp \
    preflightdialog.cpp \
    preflightgliderpage.cpp \
    preflightmiscpage.cpp \
    projectionbase.cpp \
    projectioncylindric.cpp \
    projectionlambert.cpp \
    radiopoint.cpp \
    reachablelist.cpp \
    reachpointlistview.cpp \
    settingspageairspace.cpp \
    settingspagegps.cpp \
    settingspageinformation.cpp \
    settingspagemapadv.cpp \
    settingspagemap.cpp \
    settingspagepersonal.cpp \
    settingspagepolar.cpp \
    settingspagesector.cpp \
    settingspageunits.cpp \
    signalhandler.cpp \
    singlepoint.cpp \
    sonne.cpp \
    sound.cpp \
    speed.cpp \
    taskdialog.cpp \
    tasklist.cpp \
    tasklistview.cpp \
    tpinfowidget.cpp \
    vario.cpp \
    variomodedialog.cpp \
    vector.cpp \
    waitscreen.cpp \
    waypointcatalog.cpp \
    waypointlistview.cpp \
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

INTERFACES  =
TARGET      = cumulus
DESTDIR     = .
INCLUDEPATH += ../ 
DEFINES += MAEMO
LIBS += -lstdc++

TRANSLATIONS    = \
    cumulus_de.ts \
    cumulus_nl.ts \
    cumulus_it.ts \
    cumulus_sp.ts \
    cumulus_fr.ts

