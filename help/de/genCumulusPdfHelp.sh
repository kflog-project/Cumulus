#!/bin/bash

# $Id$
#
# Script to generate a pdf file from the single German html help pages

Files="
cumulus.html \
cumulus-gettingstarted.html \
cumulus-maps.html \
cumulus-settings.html \
cumulus-settings-personal.html \
cumulus-settings-gps.html \
cumulus-settings-glider.html \
cumulus-settings-map-settings.html \
cumulus-settings-map-objects.html \
cumulus-settings-terrain.html \
cumulus-settings-task.html \
cumulus-settings-airfields.html \
cumulus-settings-airspace.html \
cumulus-settings-units.html \
cumulus-settings-information.html \
cumulus-settings-look-feel.html \
cumulus-preflight-settings.html \
cumulus-preflight-settings-glider.html \
cumulus-preflight-settings-task.html \
cumulus-preflight-settings-common.html \
cumulus-display.html \
cumulus-usage.html \
cumulus-manual.html \
cumulus-waypoints.html \
cumulus-tasks.html \
cumulus-flarm.html \
cumulus-file-locations.html \
cumulus-about.html"

Pictures=../../icons/cumulus.png

Cover=cumulus-cover.html

GenDir=/tmp/cumulus_pdf_gen

rm -rf $GenDir
mkdir -p $GenDir
cp $Files $Pictures $Cover $GenDir

SaveDir=`pwd`
cd $GenDir

wkhtmltopdf -t -s A4 -T 20 -B 20 -L 25 -R 25 \
 --footer-center "[page]" \
 --footer-right "[date]" \
 --disable-external-links \
 --cover $Cover \
 --toc-header-text Inhaltsverzeichnis --title Cumlus \
 $Files Cumulus.pdf

cp Cumulus.pdf $SaveDir
cd $SaveDir

rm -rf $GenDir

