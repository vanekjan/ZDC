#!/bin/bash
baseFolder=${1}
#baseFolder=/gpfs/mnt/gpfs01/star/pwg/lkramarik/ZDC/ZDC_code/data/run18.ZdcCalibration.lkramarik

pushd $baseFolder >> /dev/null

for run in $( ls ./ ); do
  pushd $run >> /dev/null
  tarFile=run"$run".tgz

  if [ -f $tarFile ]; then
    echo Unzipping $tarFile
    tar -xzf $tarFile
    rm $tarFile
  fi

  popd >> /dev/null 
done

popd >> /dev/null

echo Done unzipping all .tgz files
