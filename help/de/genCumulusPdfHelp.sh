#!/bin/bash

# $Id$
#
# Script to generate a pdf file from the single German html help pages

# As first generate the files list

start="cumulus.html"

Files="$start"

while true
do
  next=$(awk -F'"' '/>weiter</ { print $4; exit 0; }' ${start})

  test -z "$next" && break;

  Files="$Files $next"
  start=$next

done

#for x in $Files
#do
#  echo $x
#done > html.list

echo "$(echo $Files| wc -w) linked HTML pages found. Sum of HTML pages is $(ls *.html| wc -w)."

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
 --toc-header-text Inhaltsverzeichnis --title Cumulus \
 $Files Cumulus.pdf

cp Cumulus.pdf $SaveDir
cd $SaveDir

rm -rf $GenDir
