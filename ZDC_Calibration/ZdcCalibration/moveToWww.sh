#!/bin/bash

wwwDir=/afs/rhic.bnl.gov/star/users/vanekjan/WWW/run19.ZdcCalibration

mkdir -p "$wwwDir"
cp *.html "$wwwDir"
cp *.htm "$wwwDir"
cp -r analysis "$wwwDir"
cp -r asym "$wwwDir"
cp -r pdf "$wwwDir"
