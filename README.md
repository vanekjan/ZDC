# ZDC

## Analysis of the new runs
Check your run in [STAR RunLog Browser](https://online.star.bnl.gov/RunLog/).
Sometimes it is useful to check if the run***.tgz exists (it might not for some runs). To log to HPSS from RACF, enter:
```sh
hsi
```
Data files are there in the folder:
```sh
/home/starsink/raw/daq/#YYYY/#DAY/#RUN
```
Enter number of the run, that you are going analyse to script `ZDC_code/data/transferTarFiles.cpp` as variable 	`int RunNumber` and run this script to download the run***.tgz file:
```sh
root -l -q transferTarFiles.cpp
```
Download progress could be checked at https://www.star.bnl.gov/devcgi/display_accnt.cgi.
Once the file is download, the only think you need to do is run:
```sh
./readRunList.sh 
```

## Changes needed at the beginning of the new year run
New files and folders:
```sh
touch ZDC_code/seenRuns/seenRuns_run19.lkramarik.txt
mkdir ZDC_code/data/run19.ZdcCalibration.lkramarik
mkdir ZDC_Calibration/run19.ZdcCalibration.lkramarik
mkdir /afs/rhic.bnl.gov/star/users/lkramarik/WWW/run19.ZdcCalibration
```
Here is the list of files and lines in them, that need to be modified:

`ZDC_code/config.C`:
```sh
dirOut = "/gpfs01/star/pwg/lkramarik/ZDC/ZDC_Calibration/run19.ZdcCalibration.lkramarik";
seenRuns = "/gpfs01/star/pwg/lkramarik/ZDC/ZDC_code/seenRuns/seenRuns_run19.lkramarik.txt";
filename = "/gpfs01/star/pwg/lkramarik/ZDC/ZDC_code/data/run19.ZdcCalibration.lkramarik.list";
```

`ZDC_code/StRoot/StZDCPool/StZDCOnlineMonitoring/StReadTrg.cxx`
```sh
#include <StTriggerData2019.h>
```

`ZDC_code/StRoot/StZDCPool/StZDCOnlineMonitoring/TriggerData.cxx`
```sh
#include "StDaqLib/TRG/trgStructures2019.h" 
#include "StEvent/StTriggerData2019.h"
if(td[3] == 0x46){
TriggerDataBlk2019* trgdata2019 = (TriggerDataBlk2019*)td; 
StTriggerData2019* trgd = new StTriggerData2019(trgdata2019,run); 
```

`ZDC_Calibration/ZdcCalibration/zdcTree.h`:
```sh
TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(Form("/gpfs01/star/pwg/lkramarik/ZDC/ZDC_Calibration/run19.ZdcCalibration.lkramarik/histo/run_%d.histo.root", mRunNumber));    
f = new TFile(Form("/gpfs01/star/pwg/lkramarik/ZDC/ZDC_Calibration/run19.ZdcCalibration.lkramarik/histo/run_%d.histo.root", mRunNumber));
```

And finally, some straighforward changes in these files:
```sh
ZDC_Calibration/ZdcCalibration/moveToWww.sh
ZDC_Calibration/ZdcCalibration/html_maker.cpp
ZDC_code/readRunList.sh
```
