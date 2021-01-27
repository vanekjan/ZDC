#!/bin/bash
runYear=${1}
ls /star/u/vanekjan/pwg/vanekjan/ZDC/ZDC_code_old/ZDC/ZDC_code/data/${runYear}.ZdcCalibration.vanekjan/ > tmp.list
sort seenRuns/seenRuns_${runYear}.vanekjan.txt tmp.list | uniq -u > unSeenRuns.list
rm tmp.list
