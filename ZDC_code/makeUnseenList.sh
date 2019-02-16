#!/bin/bash
runYear=${1}
ls data/${runYear}.ZdcCalibration.lkramarik/ > tmp.list
sort seenRuns/seenRuns_${runYear}.lkramarik.txt tmp.list | uniq -u > unSeenRuns.list
rm tmp.list
