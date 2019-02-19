#!/bin/bash

runList=${1:-"unSeenRuns.list"}
runDirectory="run19.ZdcCalibration.vanekjan"
runYear="run19"
tofCut=0

# untar all the tarred .dat files
pushd data/ >> /dev/null
./untarAll.sh $runDirectory
popd >> /dev/null

makeUnseenList.sh $runYear

for run in $( cat  $runList ); do
  echo '*************run '$run'*************'
  ls /star/u/vanekjan/500GBStorage/vanekjan/ZDC/ZDC_code_new/ZDC/ZDC_code/data/$runDirectory/$run/* > /star/u/vanekjan/500GBStorage/vanekjan/ZDC/ZDC_code_new/ZDC/ZDC_code/data/$runDirectory.list

  # if [ ! -f read.pid ]; then
  #   rm -f read.out
  # else
  if [ -f read.pid ]; then
    echo Previous reading failed, aborting ...
    return -1
  fi

  ./read

  pushd ../ZDC_Calibration/ZdcCalibration/ >> /dev/null
  ./run.sh $run $tofCut
  popd >> /dev/null
done

cp ../ZDC_Calibration/ZdcCalibration/moveToWww.sh ../ZDC_Calibration/$runDirectory/

pushd ../ZDC_Calibration/$runDirectory  >> /dev/null
  ./moveToWww.sh
popd >> /dev/null

echo '**************Finished****************'
