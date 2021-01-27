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
Enter number of the run, that you are going analyse to script `ZDC_code/data/transferTarFiles.cpp` as variable `int RunNumber` and run `transferTarFiles.sh` script to download the run***.tgz file:

Download progress could be checked at https://www.star.bnl.gov/devcgi/display_accnt.cgi.
Once the file is download, the only think you need to do is run:
```sh
./readRunList.sh 
```
Before running this script, update all files and create appropriate folders. For details see the section below.

## Changes needed at the beginning of the new year run
New files and folders or you can just run `prepareFolders.sh`:
```sh
touch ZDC_code/seenRuns/seenRuns_run21.vanekjan.txt
mkdir ZDC_code/data/run21.ZdcCalibration.vanekjan
mkdir ZDC_Calibration/run21.ZdcCalibration.vanekjan
mkdir /afs/rhic.bnl.gov/star/users/vanekjan/WWW/run21.ZdcCalibration
```
Here is the list of files and lines in them, that need to be modified:

`ZDC_code/config.C`:
```sh
dirOut = "/star/u/vanekjan/pwg/vanekjan/ZDC/ZDC_code_old/ZDC/ZDC_Calibration/run19.ZdcCalibration.vanekjan";
seenRuns = "/star/u/vanekjan/pwg/vanekjan/ZDC/ZDC_code_old/ZDC/ZDC_code/seenRuns/seenRuns_run19.vanekjan.txt";
filename = "/star/u/vanekjan/pwg/vanekjan/ZDC/ZDC_code_old/ZDC/ZDC_code/data/run19.ZdcCalibration.vanekjan.list";
```

`ZDC_code/StRoot/StZDCPool/StZDCOnlineMonitoring/StReadTrg.cxx`
```sh
#include <StTriggerData2021.h>
```

`ZDC_code/StRoot/StZDCPool/StZDCOnlineMonitoring/TriggerData.cxx`
```sh
#include "StDaqLib/TRG/trgStructures2021.h" 
#include "StEvent/StTriggerData2021.h"
...
if(td[3] == 0x46){
TriggerDataBlk2021* trgdata2021 = (TriggerDataBlk2021*)td; 
StTriggerData2021* trgd = new StTriggerData2021(trgdata2021,run); 
```

`ZDC_Calibration/ZdcCalibration/zdcTree.h`:
```sh
TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(Form("/star/u/vanekjan/pwg/vanekjan/ZDC/ZDC_code_old/ZDC/ZDC_Calibration/run19.ZdcCalibration.vanekjan/histo/run_%d.histo.root", mRunNumber));    
f = new TFile(Form("/star/u/vanekjan/pwg/vanekjan/ZDC/ZDC_code_old/ZDC/ZDC_Calibration/run19.ZdcCalibration.vanekjan/histo/run_%d.histo.root", mRunNumber));
```

And finally, some straighforward changes in these files:
```sh
ZDC_Calibration/ZdcCalibration/moveToWww.sh
ZDC_Calibration/ZdcCalibration/html_maker.cpp
ZDC_code/readRunList.sh
```
