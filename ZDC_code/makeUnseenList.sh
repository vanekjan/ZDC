#!/bin/bash
runYear=${1}
ls /gpfs01/star/pwg/lkramarik/ZDC/ZDC_code/data/${runYear}.ZdcCalibration.lkramarik/ > tmp.list
sort seenRuns/seenRuns_${runYear}.lkramarik.txt tmp.list | uniq -u > unSeenRuns.list
rm tmp.list
