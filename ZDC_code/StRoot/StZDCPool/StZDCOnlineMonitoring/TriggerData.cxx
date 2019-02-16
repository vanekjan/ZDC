#include <StMessMgr.h>

#ifdef NEW_DAQ_READER
//#include "DAQ_READER/daqReader.h"
//#include "DAQ_READER/daq_dta.h"
//#include "DAQ_READER/daq_det.h"
#include "RTS/src/DAQ_READER/daqReader.h"
#include "RTS/src/DAQ_READER/daq_dta.h"
#include "RTS/src/DAQ_READER/daq_det.h"
#include "StDaqLib/TRG/trgStructures2018.h" // msimko
#include "StEvent/StTriggerData2018.h" // msimko
#endif

#include "TriggerData.h"


StTriggerData* TriggerData::trgdata=0;
int TriggerData::run_old=0;
int TriggerData::event_old=0;

TriggerData::TriggerData(){}

StTriggerData* TriggerData::Instance(char *datap){
  if(datap == 0) return 0;
  daqReader* daqr = (daqReader*)datap;
  int run = daqr->run;
  int event = daqr->seq;
  if(trgdata != 0 && run==run_old && event==event_old){    
    return trgdata;
  }else{
    if(trgdata != 0) delete trgdata;
    trgdata = 0;    
    daq_dta *dd= daqr->det("trg")->get("raw");
    if (dd && dd->iterate()) {
      char* td = (char*)dd->Void;
      trgdata = TriggerData::Instance(td, run, event);
    }else{	
      LOG_ERROR << "TRG RAW: trigger data doesn't exist" << endm;
    }
  }
  return trgdata;
}

StTriggerData* TriggerData::Instance(char *td, int run, int event){
  if(td == 0) return 0;
  LOG_DEBUG << TString::Format("Run=%d  event=%d   %d   %d\n",run,event,run_old,event_old) << endm;
  if(trgdata != 0 && run==run_old && event==event_old){    
    return trgdata;
  }else{
      if(trgdata != 0) delete trgdata;
      trgdata = 0;    
      LOG_DEBUG << TString::Format("TRG RAW: version = _%02x_%02x_%02x_%02x_\n",td[0],td[1],td[2],td[3]) << endm;
   // if(td[3] == 0x41){//modified by yhzhu for run2012!!
   // if(td[3] == 0x42){//modified by xuyifei (xyf)  for run2013!!
   // if(td[3] == 0x42){//modified by xuyifei (xyf)  for run2014!!
   // if(td[3] == 0x43){//modified by msimko for run2016	!!
   // if(td[3] == 0x44){//modified by msimko for run2017	!!
      if(td[3] == 0x45){//modified by msimko for run2017	!!
	TriggerDataBlk2018* trgdata2018 = (TriggerDataBlk2018*)td; // msimko
	StTriggerData2018* trgd = new StTriggerData2018(trgdata2018,run); // msimko
//        trgd->dump();//yhzhu added for run2012!
	trgdata = (StTriggerData*)trgd;
	run_old = run;
	event_old = event;
      }else{	
	LOG_ERROR << "TRG RAW: version missmatch, skipping trigger data" << endm;
      }
  }
  return trgdata;
}
