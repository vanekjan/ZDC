#include <iostream>
#include <fstream>
#include <list>
using namespace std;

#include <TString.h>
#include <TPRegexp.h>
#include <TObjString.h>
#include <TSystem.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2F.h>
#include <TF1.h>
#include <TLatex.h>
#include <TDatime.h>
#include <TUnixTime.h>


// this needs to be always included
#include <RTS/src/DAQ_READER/daqReader.h>
#include <RTS/src/DAQ_READER/daq_dta.h>

// only the detectors we will use need to be included
// for their structure definitions...
#include <RTS/src/DAQ_TRG/daq_trg.h>
#include <StMessMgr.h>

#include <GenericFile.h>

#include <St_db_Maker/St_db_Maker.h>

#include <StTriggerData.h>
//#include <StTriggerData2018.h>
#include <StTriggerData2019.h>

#include <tables/St_zdcsmdPed_Table.h>
#include <tables/St_zdcsmdGain_Table.h>
#include <tables/St_ZdcCalPars_Table.h>

#include "StZDCUtil/StZDCDb.h"
#include "StZDCUtil/StZDCUtil.h"

#include "StZDCPool/StZDCPlots/ZDCPlotsNames.h"
#include "StZDCPool/StZDCPlots/ZDCPlots.h"
#include "StZDCPool/StZDCPlots/ZDCPlotsPresenter.h"
#include "StZDCPool/StZDCPlots/StZDCCalibration.h"

#include "StZDCPool/StZDCPolarimetryPlots/ZDCPolarimetryPlotsNames.h"
#include "StZDCPool/StZDCPolarimetryPlots/ZDCPolarimetryPlots.h"
#include "StZDCPool/StZDCPolarimetryPlots/ZDCPolarimetryPlotsPresenter.h"
#include "StZDCPool/StZDCPolarimetryPlots/ZDCPolarimetryPlotsUtil.h"

#include "StZDCMonUtil.h"
#include "TriggerData.h"
#include "StReadTrg.h"

ClassImp(StReadTrg);

St_db_Maker *dbMaker = 0;

typedef list<TCanvas*> canvases_list_type;

void StReadTrg::readTrgData(ZDCPlots *plots, const StTriggerData *trgData, const UInt_t *runTriggersToProcessMask, Int_t date, Int_t time, Int_t prepost, Int_t *nPreFound, Int_t *nPostFound, Bool_t *eventAccepted) {
  if (eventAccepted) *eventAccepted = false;
  if (trgData) {
    Bool_t trgMaskMatched = false;
    if (runTriggersToProcessMask) {
      const UInt_t *mask = runTriggersToProcessMask;
      trgMaskMatched = (*mask == 0);
      do {
	trgMaskMatched |= (*mask) && ((*mask & trgData->tcuBits()) == *mask);
	mask++;
      } while (*mask && !trgMaskMatched);
    } else {
      trgMaskMatched = true;
    }
    if (trgMaskMatched) {
      if (nPreFound) *nPreFound = trgData->numberOfPreXing();
      if (nPostFound) *nPostFound = trgData->numberOfPostXing();
      if (dbMaker) {
	dbMaker->SetDateTime(date, time);
	dbMaker->Make();
      }
      if (plots) plots->processEvent(trgData, date, time, prepost);
      if (eventAccepted) *eventAccepted = true;
    } else {
      LOG_DEBUG << "TCU bits " << trgData->tcuBits() << " do not match requested trigger masks" << endm;
    }
  } else {
    LOG_ERROR << "No StTriggerData structure" << endm;
  }
}

void swapI(UInt_t *var) {
  *var =
    (*var & 0xff000000) >> 24 |
    (*var & 0x00ff0000) >> 8  |
    (*var & 0x0000ff00) << 8  |
    (*var & 0x000000ff) << 24 ;
}

void updateDateTime(int date, int time, Int_t *pdate, Int_t *ptime) {
  if (pdate && ptime && (((*pdate) == 0) || ((*pdate) > date) || (((*pdate) == date) && ((*ptime) > time)) || ((*ptime) == 0))) *ptime = time;
  if (pdate && (((*pdate) > date) || ((*pdate) == 0))) *pdate = date;
}

void StReadTrg::readDaq(ZDCPlots *plots, const Char_t *filename, Int_t nEvents, const UInt_t *runTriggersToProcessMask, Int_t *prun, Int_t *pdate, Int_t *ptime, Int_t *pevts, Int_t *pevtsAccepted, Int_t prepost, Int_t *nPreFound, Int_t *nPostFound) {
  LOG_INFO << "DAQ file: start reading filename=" << filename << ", nEvents=" << nEvents << endm;
  Int_t iEvent = pevts ? *pevts : 0;
  Int_t iEventAccepted = pevtsAccepted ? *pevtsAccepted : 0;
  if (!((iEvent < nEvents) || (nEvents <= 0))) return;
  daqReader *evp = 0;                  // tha main guy
  evp = new daqReader(const_cast<Char_t*>(filename)) ;     // create it with the filename argument..
  LOG_DEBUG << "daqReader created evp=" << evp << endm;
  if (prun) *prun = 0;
  Char_t *buffer = 0;
  UInt_t bufferLength = 0;
  for(;;) {
    Char_t *ret = evp->get(0);
    if (ret) {
      if(evp->status) {
	LOG_ERROR << "evp status is non-null [" << evp->status << endm;
	continue ;
      }
    } else {    // something other than an event, check what.
      switch (evp->status) {
	case EVP_STAT_OK:   // just a burp!
	  continue;
	case EVP_STAT_EOR:
	  LOG_DEBUG << "End of Run/File" << endm;
	  if(evp->IsEvp()) {   // but event pool, keep trying...
	    LOG_DEBUG << "Wait a second..." << endm;
	    sleep(1);
	    continue;
	  }
	  break;        // file, we're done...
	case EVP_STAT_EVT:
	  LOG_WARN << "Problem getting event - skipping" << endm;
	  sleep(1);
	  continue;
	case EVP_STAT_CRIT:
	  LOG_FATAL << "evp->status CRITICAL (?)" << endm;
	  return;
      }
    }
    if(evp->status == EVP_STAT_EOR) {
      LOG_INFO << "End of File reached..." << endm;
      break ; // of the for() loop...
    }
    Int_t thisDate = 0, thisTime = 0;
    TUnixTime unixTime = evp->evt_time;
    unixTime.GetGTime(thisDate, thisTime);
    updateDateTime(thisDate, thisTime, pdate, ptime);
    if (prun && (*prun <= 0)) *prun = evp->run;
    LOG_INFO << "File name \"" << evp->file_name << "\": sequence " << evp->seq 
      << ": token " << evp->token << ", trgcmd " << evp->trgcmd << ", daqcmd " 
      << evp->daqcmd << " (evp status " << evp->status << ")" << endm;
    StTriggerData *triggerData = TriggerData::Instance(reinterpret_cast<Char_t*>(evp));
    if (triggerData) {
      Bool_t eventAccepted = false;
      readTrgData(plots, triggerData, runTriggersToProcessMask, thisDate, thisTime, prepost, nPreFound, nPostFound, &eventAccepted);
      if (eventAccepted) iEventAccepted++;
    } else {
      LOG_ERROR << "Cannot get StTriggerData" << endm;
    }
    iEvent++;
    if ((iEvent == nEvents) && (nEvents > 0)) break; // for loop
  }
  if (pevts) *pevts = iEvent;
  if (pevtsAccepted) *pevtsAccepted = iEventAccepted;
  if (buffer) delete [] buffer;
  buffer = 0;
  bufferLength = 0;
  if (evp) delete evp;
  evp = 0;
}

void StReadTrg::readDat(ZDCPlots *plots, const Char_t *filename, Int_t nEvents, const UInt_t *runTriggersToProcessMask, Int_t *prun, Int_t *pdate, Int_t *ptime, Int_t *pevts, Int_t *pevtsAccepted, Int_t prepost, Int_t *nPreFound, Int_t *nPostFound) {
  LOG_INFO << "DAT file: start reading filename=" << filename << ", nEvents=" << nEvents << endm;
  Int_t iEvent = pevts ? *pevts : 0;
  Int_t iEventAccepted = pevtsAccepted ? *pevtsAccepted : 0;
  if (!((iEvent < nEvents) || (nEvents <= 0))) return;
  TString fname(filename);
  TString runStr = "run";
  Int_t i1 = fname.Index(runStr);
  if (i1 != kNPOS) {
    i1 += runStr.Length();
    Int_t i2 = fname.Index(".", i1);
    if (i2 == kNPOS) i2 = fname.Length() - i1;
    TString runStr = fname(i1, i2 - i1);
    Int_t run = runStr.Atoi();
    if (prun && (*prun <= 0)) *prun = run;
    Int_t year = (run / 1000000) - 1;
    Int_t thisDate = 0;
    Int_t thisTime = 0;
    if (pdate) thisDate = *pdate;
    if (ptime) thisTime = *ptime;
    LOG_INFO << "Run " << runStr << " / " << run << ", year " << year << endm;
    ifstream ifstr(filename);
    if (ifstr.good()) {
      Char_t *buffer = 0;
      UInt_t bufferLength = 0;
      while (ifstr.good() && ((iEvent < nEvents) || (nEvents <= 0))) {
	LOG_INFO << "Reading event " << iEvent << endm;
	UInt_t version = 0, length = 0;
	ifstr.read(reinterpret_cast<Char_t*>(&version), sizeof(version));
	ifstr.read(reinterpret_cast<Char_t*>(&length), sizeof(length));
	UInt_t versionRaw = version, lengthRaw = length;
	swapI(&version);
	swapI(&length);
	if (ifstr.good()) {
	  LOG_INFO << "version=" << TString::Format("0x%X", version) << ", length=" << TString::Format("0x%X", length) << endm;
	  if (!buffer || (bufferLength < length)) {
	    if (buffer) delete [] buffer;
	    buffer = 0;
	    bufferLength = 0;
	    buffer = new Char_t[length];
	    bufferLength = length;
	  }
	  if (buffer && (bufferLength >= length)) {
	    UInt_t *bufferI = reinterpret_cast<UInt_t*>(buffer);
	    bufferI[0] = versionRaw;
	    bufferI[1] = lengthRaw;
	    ifstr.read(&buffer[sizeof(version) + sizeof(length)], length - sizeof(version) - sizeof(length));
	    if (ifstr.good() || ifstr.eof()) {
	      updateDateTime(thisDate, thisTime, pdate, ptime);
	      StTriggerData *triggerData = TriggerData::Instance(reinterpret_cast<Char_t*>(buffer), run, iEvent);
	      if (triggerData) {
		Bool_t eventAccepted = false;
		readTrgData(plots, triggerData, runTriggersToProcessMask, thisDate, thisTime, prepost, nPreFound, nPostFound, &eventAccepted);
		if (eventAccepted) iEventAccepted++;
	      } else {
		LOG_ERROR << "Cannot get StTriggerData" << endm;
	      }
	      iEvent++;
	    }
	  } else {
	    LOG_ERROR << "Cannot allocate memory for length=" << length << endm;
	  }
	} else {
	  if (ifstr.eof()) {
	    LOG_INFO << "Reached end of file " << filename << endm;
	  } else {
	    LOG_ERROR << "Bad format (read version=" << version << ", length=" << length << ")" << endm;
	  }
	}
      }
      if (buffer) delete [] buffer;
      buffer = 0;
      bufferLength = 0;
      if (pevts) *pevts = iEvent;
      if (pevtsAccepted) *pevtsAccepted = iEventAccepted;
    } else {
      LOG_ERROR << "Cannot open file for reading " << filename << endm;
    }
  } else {
    LOG_ERROR << "Cannot find run number in filename " << fname << endm;
  }
}

TString getRunMask(TString filename, Int_t *prun) {
  TString mask = filename;
  if (filename.EndsWith(".daq")) {
    Int_t last = mask.Last('.');
    if (last >= 0) {
      mask.Replace(last, mask.Length() - last, "");
      last = mask.Last('_');
      if (last >= 0) {
	mask.Replace(last, mask.Length() - last, "");
	if (prun) {
	  TString maskRun = mask;
	  Int_t lastRun = maskRun.Last('_');
	  if (lastRun >= 0) {
	    maskRun.Replace(lastRun, maskRun.Length() - lastRun, "");
	    lastRun = maskRun.Last('_');
	    if (lastRun >= 0) {
	      *prun = TString(maskRun(lastRun + 1, maskRun.Length() - (lastRun + 1))).Atoi();
	    }
	  }
	}
	mask += "_*";
      }
      mask += ".daq";
    }
  } else if (filename.EndsWith(".dat")) {
    Int_t last = mask.Last('.');
    if (last >= 0) {
      mask.Replace(last, mask.Length() - last, "");
      last = mask.Last('.');
      if (last >= 0) {
	mask.Replace(last, mask.Length() - last, "");
	if (prun) {
	  TString maskRun = mask;
	  Int_t lastRun = maskRun.Last('n');
	  if (lastRun >= 0) {
	    *prun = TString(maskRun(lastRun + 1, maskRun.Length() - (lastRun + 1))).Atoi();
	  }
	}
	mask += ".*";
      }
      mask += ".dat";
    }
  }
  return mask;
}

void unparseFilename(TString &fname, const TString &filenameStr, const TString &nameStr, const TString &dateStr, const TString &timeStr) {
  fname = filenameStr;
  if (nameStr != "") {
    fname += " ";
    fname += nameStr;
  }
  if ((dateStr != "") && (timeStr != "")) {
    fname += " ";
    fname += dateStr;
    fname += ".";
    fname += timeStr;
  }
}

void parseFilename(const TString &fname, TString &filenameStr, TString &nameStr, TString &dateStr, TString &timeStr) {
  filenameStr = fname;
  nameStr = "";
  dateStr = "";
  timeStr = "";
  Int_t lastArg = -1;
  lastArg = filenameStr.Last(' ');
  if (lastArg != -1) {
    TString fnameD = filenameStr;
    fnameD.Remove(0, lastArg + 1);
    Int_t timeArg = fnameD.Last('.');
    if (timeArg != -1) {
      TString fnameT = fnameD;
      fnameD.Remove(timeArg, fnameD.Length() - timeArg);
      fnameT.Remove(0, timeArg + 1);
      if (fnameT.IsDigit() && fnameD.IsDigit()) {
	dateStr = fnameD;
	timeStr = fnameT;
	filenameStr.Remove(lastArg, filenameStr.Length() - lastArg);
      }	
    }
  }
  lastArg = filenameStr.Last(' ');
  if (lastArg != -1) {
    TString fnameN = filenameStr;
    fnameN.Remove(0, lastArg + 1);
    if (fnameN.IsAscii()) {
      nameStr = fnameN;
      filenameStr.Remove(lastArg, filenameStr.Length() - lastArg);
    }
  }
}

void listDirectory(const TString &dirName, TList &dirList) {
  LOG_INFO << "Listing directory " << dirName << endm;
  void *dir = gSystem->OpenDirectory(dirName);
  while (const Char_t *dirEntry = gSystem->GetDirEntry(dir)) {
    TString dirEntryStr = dirEntry;
    if ((dirEntryStr != ".") && (dirEntryStr != "..")) {
      TString fileStr = dirName + "/" + dirEntryStr;
      dirList.Add(new TObjString(fileStr));
    }
  }
  gSystem->FreeDirectory(dir);
  dir = 0;
}

typedef map<TString, TList*> dir_list_type;

void getFileList(dir_list_type &dirList, TList &fileList, const TString &fname, Bool_t searchForOtherRunFiles) {
  TString filenameStr;
  TString nameStr;
  TString dateStr;
  TString timeStr;
  parseFilename(fname, filenameStr, nameStr, dateStr, timeStr);
  if (filenameStr.EndsWith(".list")) {
    //LOG_DEBUG << "Reading file list " << filenameStr << endm;
    TList readList;
    readList.SetOwner();
    ifstream fstr(filenameStr);
    if (fstr.good()) {
      Char_t buf[2048];
      while (fstr.good()) {
	fstr.getline(buf, sizeof(buf));
	buf[sizeof(buf)/sizeof(buf[0]) - 1] = 0;
	//LOG_DEBUG << "Read line " << buf << endm;
	if (buf[0]) readList.Add(new TObjString(buf));
      }
      fstr.close();
      //LOG_DEBUG << "Finished reading file list " << filenameStr << endm;
    } else {
      LOG_ERROR << "Cannot open file list " << filenameStr << endm;
    }
    TIter nextFile(&readList);
    while ( TObjString *nextFilename = static_cast<TObjString*>(nextFile()) ) {
      TString nextFilenameStr = nextFilename->GetString();
      TString filenameStrNext;
      TString nameStrNext;
      TString dateStrNext;
      TString timeStrNext;
      parseFilename(nextFilenameStr, filenameStrNext, nameStrNext, dateStrNext, timeStrNext);
      if (nameStrNext == "") nameStrNext = nameStr;
      if (dateStrNext == "") dateStrNext = dateStr;
      if (timeStrNext == "") timeStrNext = timeStr;
      unparseFilename(nextFilenameStr, filenameStrNext, nameStrNext, dateStrNext, timeStrNext);
      //LOG_DEBUG << "Process line " << nextFilenameStr << endm;
      getFileList(dirList, fileList, nextFilenameStr, searchForOtherRunFiles);
    }
  } else if (filenameStr.EndsWith("/*.dat") || filenameStr.EndsWith("/*.daq")) {
    //LOG_DEBUG << "Searching for files " << filenameStr << endm;
    TString dirName = gSystem->DirName(filenameStr);
    Bool_t foundDirList = (dirList.find(dirName) != dirList.end());
    TList* &thisDirList = dirList[dirName];
    if (foundDirList) {
      //LOG_DEBUG << "Re-using directory listing for " << dirName << ": " << (thisDirList ? thisDirList->GetSize() : -1) << " entries" << endm;
    } else {
      thisDirList = new TList();
      if (thisDirList) {
	thisDirList->SetOwner();
	listDirectory(dirName, *thisDirList);
      }
    }
    if (thisDirList) {
      TIter nextFile(thisDirList);
      while ( TObjString *nextFilename = static_cast<TObjString*>(nextFile()) ) {
	TString nextFilenameStr = nextFilename->GetString();
	if ((filenameStr.EndsWith("/*.dat") && nextFilenameStr.EndsWith(".dat")) ||
	    (filenameStr.EndsWith("/*.daq") && nextFilenameStr.EndsWith(".daq"))) {
	  //LOG_DEBUG << "Found file " << nextFilenameStr << endm;
	  unparseFilename(nextFilenameStr, nextFilenameStr, nameStr, dateStr, timeStr);
	  getFileList(dirList, fileList, nextFilenameStr, searchForOtherRunFiles);
	}
      }
    }
  } else if (filenameStr.EndsWith(".dat") || filenameStr.EndsWith(".daq")) {
    TString mask = getRunMask(filenameStr);
    if (((mask == filenameStr) && mask.Contains("*")) || searchForOtherRunFiles) {
      //LOG_DEBUG << "Searching for " << filenameStr << endm;
      if (filenameStr.EndsWith(".dat")) {
	for (Int_t i = 1;i < 300;i++) {
	  TString nextRunFilenameStr = mask;
	  nextRunFilenameStr.ReplaceAll(".*.", TString::Format(".%i.", i));
	  unparseFilename(nextRunFilenameStr, nextRunFilenameStr, nameStr, dateStr, timeStr);
	  getFileList(dirList, fileList, nextRunFilenameStr, false);
	}
      } else if (filenameStr.EndsWith(".daq")) {
	//LOG_DEBUG << "Filename: " << filenameStr << ", mask: " << mask << endm;
	TString dirName = gSystem->DirName(filenameStr);
	//LOG_DEBUG << "Getting directory listing for " << dirName << endm;
	//LOG_DEBUG << "Looking in the existing " << dirList.size() << " listings" << endm;
	Bool_t foundDirList = !(dirList.find(dirName) == dirList.end());
	TList* &usedList = dirList[dirName];
	if (foundDirList) {
	  //LOG_DEBUG << "Re-using directory listing for " << dirName << ": " << (usedList ? usedList->GetSize() : -1) << " entries" << endm;
	} else {
	  usedList = new TList();
	  if (usedList) {
	    usedList->SetOwner();
	    listDirectory(dirName, *usedList);
	  }
	}
	if (usedList) {
	  //LOG_DEBUG << "Using directory listing for " << dirName << ": " << usedList->GetSize() << " entries" << endm;
	  TIter nextDirFile(usedList);
	  while ( TObjString *nextDirFilename = static_cast<TObjString*>(nextDirFile()) ) {
	    TString nextDirFilenameStr = nextDirFilename->GetString();
	    TString entryMask = getRunMask(nextDirFilenameStr);
	    //LOG_DEBUG << "dir entry: " << nextDirFilenameStr << ", mask: " << entryMask << endm;
	    if (entryMask == mask) {
	      //LOG_INFO << "Found run file " << nextDirFilenameStr << endm;
	      unparseFilename(nextDirFilenameStr, nextDirFilenameStr, nameStr, dateStr, timeStr);
	      getFileList(dirList, fileList, nextDirFilenameStr, false);
	    }
	  }
	}
      }
    } else {
      TString nextFilenameStr;
      unparseFilename(nextFilenameStr, filenameStr, nameStr, dateStr, timeStr);
      //LOG_DEBUG << "Adding line " << nextFilenameStr << endm;
      fileList.Add(new TObjString(nextFilenameStr));
    }
  }
}

void saveCanvases(canvases_list_type &canvases, const TString &formats, const Char_t *dirOut, const Char_t *runStr, const TString &run, const TString &prepostNameStr, TString &formatsLine, TObjArray *cleanup) {
  LOG_DEBUG << "Saving canvases to (" << formats << "), " << dirOut << " ..." << endm;
  for (canvases_list_type::const_iterator iter = canvases.begin();iter != canvases.end();++iter) {
    TCanvas *c = *iter;
    if (c) {
      c->cd();
      TLatex *title = new TLatex(0.05, 0.05, c->GetTitle());
      if (title) {
	if (cleanup) cleanup->Add(title);
	title->SetTextAlign(11);
	title->SetTextSize(0.03);
	title->Draw();
      }
    }
  }
  Bool_t psRequested = false;
  Bool_t psSaved = false;
  Bool_t spaceNeeded = false;
  TObjArray *formatsArr = formats.Tokenize(" ");
  if (formatsArr) for (Int_t ifmt = 0;ifmt < formatsArr->GetEntries();ifmt++) {
    TString currFormat = (static_cast<TObjString*>(formatsArr->At(ifmt)))->GetString();
    if (dirOut && dirOut[0] && (run != "") && (currFormat != "")) {
      {
	if (spaceNeeded) formatsLine += " ";
	TString outFilename = TString::Format("%s/%s_%s%s.%s", currFormat.Data(), runStr, run.Data(), prepostNameStr.Data(), currFormat.Data());
	formatsLine += TString::Format("<A href=\"%s\">%s</A>", outFilename.Data(), currFormat.Data());
	spaceNeeded = true;
      }
      if (currFormat == "ps") psRequested = true;
      Bool_t runPsToPdf = false;
      TString dirOutFormatPdf = TString::Format("%s/%s", dirOut, "pdf");
      TString outFilenamePdf = TString::Format("%s/%s_%s%s.%s", dirOutFormatPdf.Data(), runStr, run.Data(), prepostNameStr.Data(), "pdf");
      if (currFormat == "pdf") {
	currFormat = "ps";
	runPsToPdf = true;
	gSystem->mkdir(dirOutFormatPdf.Data(), kTRUE);
      }
      TString dirOutFormat = TString::Format("%s/%s", dirOut, currFormat.Data());
      gSystem->mkdir(dirOutFormat.Data(), kTRUE);
      TString outFilename = TString::Format("%s/%s_%s%s.%s", dirOutFormat.Data(), runStr, run.Data(), prepostNameStr.Data(), currFormat.Data());
      if (!((currFormat == "ps") && psSaved)) {
	TString outFilenameBegin = outFilename + "(";
	TString outFilenameEnd = outFilename + ")";
	for (canvases_list_type::const_iterator iter = canvases.begin();iter != canvases.end();++iter) {
	  TCanvas *c = *iter;
	  if (c) {
	    LOG_DEBUG << "Saving canvas " << c->GetName() << ", " << c->GetTitle() << " ..." << endm;
	    Bool_t isFirst = (iter == canvases.begin());
	    canvases_list_type::const_iterator iterTmp = iter;
	    ++iterTmp;
	    Bool_t isLast = (iterTmp == canvases.end());
	    if (currFormat == "root") {
	      TFile *f = new TFile(outFilename, isFirst ? "RECREATE" : "UPDATE");
	      if (f && f->IsOpen()) {
		f->cd();
		c->Write();
	      } else {
		LOG_ERROR << "Cannot open file for writing " << outFilename << endm;
	      }
	      if (f) delete f;
	      f = 0;
	    } else {
	      LOG_DEBUG << "Printing canvas..." << endm;
	      c->Print(isFirst ? outFilenameBegin : (isLast ? outFilenameEnd : outFilename));
	      LOG_DEBUG << "Finished printing canvas." << endm;
	    }
	    LOG_DEBUG << "Finished saving canvas." << endm;
	  }
	}
	if (currFormat == "ps") psSaved = true;
      }
      if (runPsToPdf) {
	TString psToPdfCmd = TString::Format("ps2pdf \"%s\" \"%s\"", outFilename.Data(), outFilenamePdf.Data());
	LOG_DEBUG << "ps2pdf command: " << psToPdfCmd << endm;
	gSystem->Exec(psToPdfCmd);
      }
    }
  }
  if (formatsArr) {
    formatsArr->SetOwner();
    delete formatsArr;
  }
  formatsArr = 0;
  if (!psRequested && psSaved) {
    TString dirOutFormat = TString::Format("%s/%s", dirOut, "ps");
    TString outFilename = TString::Format("%s/%s_%s%s.%s", dirOutFormat.Data(), runStr, run.Data(), prepostNameStr.Data(), "ps");
    LOG_DEBUG << "Removing intermediate PostScript file " << outFilename << endm;
    gSystem->Unlink(outFilename);
  }
  if (spaceNeeded) formatsLine += " ";
  formatsLine += TString::Format("<A href=\"%s/%s_%s%s.%s.root\">%s</A>", "histo", runStr, run.Data(), prepostNameStr.Data(), "histo", "raw");
  spaceNeeded = true;
  LOG_DEBUG << "Finished saving canvases." << endm;
}

void StReadTrg::readTrg(const Char_t *filename, Int_t nEvents
    , const Char_t *configFile
    , const Char_t *zdcPedCal
    , const Char_t *seenRuns
    , const Char_t *dirOut
    , const Char_t *publicDir
    , const Char_t *emailNotification
    , const Char_t *namesToProcess
    , const Char_t *namesToCalibratePed
    , Int_t minEventsToCalibratePed
    , const Char_t *namesToCalibrateGain
    , Int_t minEventsToCalibrateGain
    , const Char_t *namesToCalibrateTowerPed
    , Int_t minEventsToCalibrateTowerPed
    , const Char_t *namesToCalibrateTowerCalib
    , Int_t minEventsToCalibrateTowerCalib
    , const Char_t *namesToCalibrateSlewing
    , Int_t minEventsToCalibrateSlewing
    , Bool_t searchForOtherRunFiles
    , Bool_t stopRunOnFirstError
    , Bool_t deleteCanvasOnExit
, Bool_t updateInputTextDb
, Int_t refreshRateSeconds
, Int_t prepost
, const Char_t *dbconnectCdev
, const Char_t *cdevFilename
, const Char_t *namesToAddFill
, Int_t minEventsToAddFill
, const Char_t *outFormats
, const Char_t *outFillFormats
, const Char_t *tcuBitsToProcess
, Float_t normAnalyzingPower
) {
  LOG_INFO << TString::Format("Starting readTrg:\nfilename=%s\nnEvents=%i\nconfigFile=%s\nzdcPedCal=%s\nseenRuns=%s\ndirOut=%s\n\
      publicDir=%s\nemailNotification=%s\nnamesToProcess=%s\nnamesToCalibratePed=%s\nminEventsToCalibratePed=%i\n\
      namesToCalibrateGain=%s\nminEventsToCalibrateGain=%i\nnamesToCalibrateTowerPed=%s\nminEventsToCalibrateTowerPed=%i\n\
      namesToCalibrateTowerCalib=%s\nminEventsToCalibrateTowerCalib=%i\nnamesToCalibraeSlewing=%s\nminEventsToCalibrateSlewing=%i\n\
      searchForOtherRunFiles=%i\nstopRunOnFirstError=%i\nupdateInputTextDb=%i\nrefreshRateSeconds=%i\nprepost=%i\ndbconnectCdev=%s\n\
      cdevFilename=%s\nnamesToAddFill=%s\nminEventsToAddFill=%i\noutFormats=%s\noutFillFormats=%s\ntcuBitsToProcess=%s\nnormAnalyzingPower=%g",
      filename, nEvents, configFile, zdcPedCal, seenRuns, dirOut, publicDir, emailNotification, namesToProcess, \
      namesToCalibratePed, minEventsToCalibratePed, namesToCalibrateGain, minEventsToCalibrateGain, \
      namesToCalibrateTowerPed, minEventsToCalibrateTowerPed, namesToCalibrateTowerCalib, minEventsToCalibrateTowerCalib, \
      namesToCalibrateSlewing, minEventsToCalibrateSlewing, Int_t(searchForOtherRunFiles), \
      Int_t(stopRunOnFirstError), Int_t(updateInputTextDb), refreshRateSeconds, prepost, dbconnectCdev, cdevFilename, \
      namesToAddFill, minEventsToAddFill, outFormats, outFillFormats, tcuBitsToProcess, normAnalyzingPower) << endm;
  TDatime startTime;
  TH1::AddDirectory(kFALSE);
  Bool_t useShiftCrewPlots = true;
  Bool_t useExpertPlots = true;
  Bool_t usePolarimetryPlots = true;
  //Bool_t usePolarimetryPlots = false;//CJ
  TString formats = outFormats;
  TString formatsFill = outFillFormats;
  const Char_t *runLogUrl = "http://online.star.bnl.gov/admin/navigator.php?run=%i";
  const Char_t *myTitle = "ZDC Online Monitoring";
  if (dirOut && dirOut[0]) gSystem->mkdir(dirOut, kTRUE);
  TString configFileBasename;
  if (configFile && configFile[0] && dirOut && dirOut[0]) {
    configFileBasename = gSystem->BaseName(configFile);
    TString configFileOutname;
    configFileOutname = TString::Format("%s/%s", dirOut, configFileBasename.Data());
    TString copyConfigFileCmd = TString::Format("/bin/cp %s %s", configFile, configFileOutname.Data());
    LOG_INFO << "Copying config file: " << copyConfigFileCmd << endm;
    gSystem->Exec(copyConfigFileCmd);
  }
  TString dirOutDb = TString::Format("%s/%s", dirOut, "StarDb/Calibrations/zdc");
  if (dirOut && dirOut[0]) gSystem->mkdir(dirOutDb.Data(), kTRUE);
  if (!(zdcPedCal && zdcPedCal[0])) {
    dbMaker = new St_db_Maker("db", dirOutDb, "MySQL:StarDb", "$STAR/StarDb");
  }
  if (dbMaker) {
    dbMaker->Init();
  }
  LOG_DEBUG << "Initializing histograms..." << endm;
  ZDCPolarimetryPlots *zdcPlots = new ZDCPolarimetryPlots();
  if (zdcPlots) {
    zdcPlots->setTextDb(zdcPedCal);
  }
  ZDCPolarimetryPlotsPresenter *zdcPlotsPresenter = new ZDCPolarimetryPlotsPresenter();
  LOG_DEBUG << "instance created " << zdcPlots << endm;
  if (zdcPlots) {
    zdcPlots->setShiftCrewPlots(useShiftCrewPlots);
    zdcPlots->setExpertPlots(useExpertPlots);
    zdcPlots->setPolarimetryPlots(usePolarimetryPlots);
    LOG_DEBUG << "initializing..." << endm;
    zdcPlots->init();
    LOG_DEBUG << "instance initialized" << endm;
  }
  TString namesToProcessStr = namesToProcess;
  TPRegexp namesToProcessRE = namesToProcessStr;
  TString namesToCalibratePedStr = namesToCalibratePed;
  TPRegexp namesToCalibratePedRE = namesToCalibratePedStr;
  TString namesToCalibrateGainStr = namesToCalibrateGain;
  TPRegexp namesToCalibrateGainRE = namesToCalibrateGainStr;
  TString namesToCalibrateTowerPedStr = namesToCalibrateTowerPed;
  TPRegexp namesToCalibrateTowerPedRE = namesToCalibrateTowerPedStr;
  TString namesToCalibrateTowerCalibStr = namesToCalibrateTowerCalib;
  TPRegexp namesToCalibrateTowerCalibRE = namesToCalibrateTowerCalibStr;
  TString namesToCalibrateSlewingStr = namesToCalibrateSlewing;
  TPRegexp namesToCalibrateSlewingRE = namesToCalibrateSlewingStr;
  TString namesToAddFillStr = namesToAddFill;
  TPRegexp namesToAddFillRE = namesToAddFillStr;
  TList fileList;
  fileList.SetOwner();
  dir_list_type dirList;
  if (filename) {
    TString fname(filename);
    getFileList(dirList, fileList, fname, searchForOtherRunFiles);
  }
  list<Int_t> seenRunList;
  list<Int_t> failedRunList;
  if (seenRuns && seenRuns[0]) {
    LOG_INFO << "Reading seen runs list " << seenRuns << endm;
    ifstream fstr(seenRuns);
    if (fstr.good()) {
      Int_t r = 0;
      while (fstr.good()) {
	fstr >> r;
	LOG_DEBUG << "Read seen run " << r << endm;
	seenRunList.push_back(r);
      }
      fstr.close();
    } else {
      LOG_ERROR << "Cannot read seen runs list " << seenRuns << endm;
    }
  }
  TString outListLastFilename = TString::Format("%s/list_last.html", dirOut);
  {
    ofstream fstr(outListLastFilename);
  }
  TString outFilesLastFilename = TString::Format("%s/files_last.html", dirOut);
  ofstream fstr_filesLast(outFilesLastFilename);
  if (fstr_filesLast.good()) {
    fstr_filesLast << "<BR/>" << endl;
    fstr_filesLast << "<TABLE border=\"1\">" << endl;
    fstr_filesLast << "<TR> <TD align=\"right\">#</TD> <TD align=\"left\">File</TD> </TR>" << endl;
  } else {
    LOG_ERROR << "Cannot write to file " << outFilesLastFilename << endm;
  }
  TString dirOutText = TString::Format("%s/%s", dirOut, "asym");
  if (dirOut && dirOut[0]) gSystem->mkdir(dirOutText.Data(), kTRUE);
  TString outTextRunsFilename = TString::Format("%s/runs.txt", dirOutText.Data());
  TString outTextFillsFilename = TString::Format("%s/fills.txt", dirOutText.Data());
  TString outAsymRunsGifFilename = TString::Format("%s/runs_asymmetry.gif", dirOutText.Data());
  TString outAsymRunsCFilename = TString::Format("%s/runs_asymmetry.C", dirOutText.Data());
  TString outAsymRunsAllGifFilename = TString::Format("%s/runs_asymmetry_all.gif", dirOutText.Data());
  TString outAsymRunsAllCFilename = TString::Format("%s/runs_asymmetry_all.C", dirOutText.Data());
  TString outAsymFillsGifFilename = TString::Format("%s/fills_asymmetry.gif", dirOutText.Data());
  TString outAsymFillsCFilename = TString::Format("%s/fills_asymmetry.C", dirOutText.Data());
  TString outAsymFillsAllGifFilename = TString::Format("%s/fills_asymmetry_all.gif", dirOutText.Data());
  TString outAsymFillsAllCFilename = TString::Format("%s/fills_asymmetry_all.C", dirOutText.Data());
  TString outAsymRunsGifWebFilename = TString::Format("%s/runs_asymmetry.gif", "asym");
  TString outAsymRunsCWebFilename = TString::Format("%s/runs_asymmetry.C", "asym");
  TString outAsymRunsAllGifWebFilename = TString::Format("%s/runs_asymmetry_all.gif", "asym");
  TString outAsymRunsAllCWebFilename = TString::Format("%s/runs_asymmetry_all.C", "asym");
  TString outAsymFillsGifWebFilename = TString::Format("%s/fills_asymmetry.gif", "asym");
  TString outAsymFillsCWebFilename = TString::Format("%s/fills_asymmetry.C", "asym");
  TString outAsymFillsAllGifWebFilename = TString::Format("%s/fills_asymmetry_all.gif", "asym");
  TString outAsymFillsAllCWebFilename = TString::Format("%s/fills_asymmetry_all.C", "asym");
  const Int_t runTriggersToProcessMaskSize = 32;
  UInt_t runTriggersToProcessMask[runTriggersToProcessMaskSize] = {0};
  if (tcuBitsToProcess && tcuBitsToProcess[0]) {
    TString tcuBitsStr = tcuBitsToProcess;
    TObjArray *tcuBitsArr = tcuBitsStr.Tokenize(" ");
    if (tcuBitsArr) for (Int_t i = 0;i < tcuBitsArr->GetEntries();i++) {
      TString currMask = (static_cast<TObjString*>(tcuBitsArr->At(i)))->GetString();
      runTriggersToProcessMask[i] = currMask.Atoi();
    }
    if (tcuBitsArr) delete tcuBitsArr;	
  }
  Int_t totalFilesProcessed = 0;
  Int_t totalRunsProcessed = 0;
  canvases_list_type canvases;
  LOG_DEBUG << "Start looping over file list, entries: " << fileList.GetSize() << endm;
  TIter nextFile(&fileList);
  while ( TObjString *nextFilename = static_cast<TObjString*>(nextFile()) ) {
    TString nextFilenameStr = nextFilename->GetString();
    TString filenameStr;
    TString nameStr;
    TString dateStr;
    TString timeStr;
    parseFilename(nextFilenameStr, filenameStr, nameStr, dateStr, timeStr);
    LOG_DEBUG << "About to process line " << nextFilenameStr << endm;
    Bool_t runFailed = false;
    Int_t run = 0;
    TList runFileList;
    TString runTriggers;
    runFileList.SetOwner();
    TString mask = getRunMask(filenameStr, &run);
    bool processThisRun = true;
    if (find(seenRunList.begin(), seenRunList.end(), run) != seenRunList.end()) {
      processThisRun = false;
      LOG_DEBUG << "This run has already been processed (run=" << run << ")" << endm;
    } else if (find(failedRunList.begin(), failedRunList.end(), run) != failedRunList.end()) {
      processThisRun = false;
      runFailed = true;
      LOG_DEBUG << "This run has already failed (run=" << run << ")" << endm;
    }
    if (processThisRun) { 
      Bool_t triggersCommaNeeded = false;
      TIter nextListFile(&fileList);
      Bool_t triggerToProcessFound = false;
      Int_t trgIndex = 0;
      TString triggerPrev = "__dummy__";
      while ( TObjString *nextListFilename = static_cast<TObjString*>(nextListFile()) ) {
	TString nextListFilenameStr = nextListFilename->GetString();
	TString filenameStrNext;
	TString nameStrNext;
	TString dateStrNext;
	TString timeStrNext;
	parseFilename(nextListFilenameStr, filenameStrNext, nameStrNext, dateStrNext, timeStrNext);
	TString entryMask = getRunMask(filenameStrNext);
	if (entryMask == mask) {
	  if (nameStrNext == "") nameStrNext = nameStr;
	  if (dateStrNext == "") dateStrNext = dateStr;
	  if (timeStrNext == "") timeStrNext = timeStr;
	  unparseFilename(nextListFilenameStr, filenameStrNext, nameStrNext, dateStrNext, timeStrNext);
	  runFileList.Add(new TObjString(nextListFilenameStr));
	  if (nameStrNext != triggerPrev) {
	    triggerPrev = nameStrNext;
	    if (triggersCommaNeeded) runTriggers += ", ";
	    if ((namesToProcessStr == "") || (nameStrNext == "") || nameStrNext.Contains(namesToProcessRE)) {
	      triggerToProcessFound = true;
	      runTriggers += "<B>" + nameStrNext + "</B>";
	    } else {
	      runTriggers += nameStrNext;
	    }
	    if (nameStrNext != "") triggersCommaNeeded = true;
	    trgIndex++;
	  }
	}
      }
      if (!triggerToProcessFound) {
	processThisRun = false;
	runFailed = true;
	failedRunList.push_back(run);
	runFileList.Clear();
	LOG_DEBUG << "This run triggers list \"" << runTriggers << "\" does not match request list \"" << namesToProcessStr << "\"" << endm;
      }
    }
    if (processThisRun && !runFailed) {
      TIter nextRunFile(&runFileList);
      TObjString *nextRunFilename = static_cast<TObjString*>(nextRunFile());
      TString nextRunFilenameStr = nextRunFilename ? nextRunFilename->GetString() : TString("");
      TString filenameStrNext;
      TString nameStrNext;
      TString dateStrNext;
      TString timeStrNext;
      parseFilename(nextRunFilenameStr, filenameStrNext, nameStrNext, dateStrNext, timeStrNext);
      if (filenameStrNext.EndsWith(".dat")) {
	LOG_DEBUG << "Processing file for .done " << filenameStrNext << endm;
	TString doneFileName = mask;
	doneFileName.ReplaceAll(".*.dat", ".done");
	if ((doneFileName != "") && (gSystem->GetPathInfo(doneFileName, (Long_t*)0, (Long_t*)0, (Long_t*)0, (Long_t*)0) == 0)) {
	  LOG_DEBUG << "Found .done file for this run " << doneFileName << endm;
	} else {
	  /*yhzhu
	    runFailed = true;
	    failedRunList.push_back(run);
	    LOG_ERROR << "Cannot find .done file for this run " << doneFileName << ", is run finished yet?" << endm;
	    yhzhu*/
	}
      }
    }
    Int_t nPreFound = prepost;
    Int_t nPostFound = prepost;
    if (processThisRun && !runFailed && prepost) {
      TIter nextRunFile(&runFileList);
      while ( TObjString *nextRunFilename = static_cast<TObjString*>(nextRunFile()) ) {
	TString nextRunFilenameStr = nextRunFilename->GetString();
	TString filenameStrNext;
	TString nameStrNext;
	TString dateStrNext;
	TString timeStrNext;
	parseFilename(nextRunFilenameStr, filenameStrNext, nameStrNext, dateStrNext, timeStrNext);
	LOG_DEBUG << "Processing file for prepost " << filenameStrNext << endm;
	Bool_t runFileProcessed = false;
	Int_t evts_prepost = 0;
	if (filenameStrNext.EndsWith(".daq")) {
	  readDaq(0, filenameStrNext.Data(), 1, 0, 0, 0, 0, &evts_prepost, 0, 0, &nPreFound, &nPostFound);
	  runFileProcessed = true;
	} else if (filenameStrNext.EndsWith(".dat")) {
	  readDat(0, filenameStrNext.Data(), 1, 0, 0, 0, 0, &evts_prepost, 0, 0, &nPreFound, &nPostFound);
	  runFileProcessed = true;
	}
	if (runFileProcessed && (evts_prepost > 0)) {
	  break;
	} else {
	  if (stopRunOnFirstError) {
	    runFailed = true;
	    failedRunList.push_back(run);
	    break; // while loop
	  }
	}
      }
    }
    nPreFound = TMath::Min(TMath::Abs(prepost), TMath::Abs(nPreFound));
    nPostFound = TMath::Min(TMath::Abs(prepost), TMath::Abs(nPostFound));
    if (!runFailed) for (Int_t iprepost = -nPreFound;iprepost <= nPostFound;iprepost++) {
      Int_t evts = 0;
      Int_t evtsAccepted = 0;
      TDatime startTimeRun;
      TDatime startTimeRunGMT(startTimeRun.Convert(true));
      Int_t date = ((dateStr != "") && (timeStr != "")) ? dateStr.Atoi() : startTimeRunGMT.GetDate();
      Int_t time = ((dateStr != "") && (timeStr != "")) ? timeStr.Atoi() : startTimeRunGMT.GetTime();
      TString fillNumberStr;
      bool runProcessed = false;
      LOG_INFO << "Start looping over run file list, entries: " << runFileList.GetSize() << endm;
      TIter nextRunFile(&runFileList);
      while ( TObjString *nextRunFilename = static_cast<TObjString*>(nextRunFile()) ) {
	TString nextRunFilenameStr = nextRunFilename->GetString();
	TString filenameStrNext;
	TString nameStrNext;
	TString dateStrNext;
	TString timeStrNext;
	parseFilename(nextRunFilenameStr, filenameStrNext, nameStrNext, dateStrNext, timeStrNext);
	LOG_DEBUG << "Processing file " << filenameStrNext << endm;
	if (!runProcessed && (gSystem->GetPathInfo(filenameStrNext.Data(), (Long_t*)0, (Long_t*)0, (Long_t*)0, (Long_t*)0) == 0)) {
	  if (dbMaker) dbMaker->InitRun(run);
	  if (zdcPlots) zdcPlots->clear();
	  TString cdevXmlFile;
	  TString cdevXmlFileOut;
	  Int_t cdevDate = date;
	  Int_t cdevTime = time;
	  TString cdevXmlFileTmp = "./cdev_tmp.xml";
	  if (cdevFilename && cdevFilename[0]) {
	    getCdevFileTimestamp(cdevFilename, cdevDate, cdevTime);
	  } else if (dbconnectCdev && dbconnectCdev[0]) {
	    if (cdevXmlFileTmp != "") {
	      writeQueryCdev(dbconnectCdev, cdevDate, cdevTime, cdevXmlFileTmp);
	      getCdevFileTimestamp(cdevXmlFileTmp, cdevDate, cdevTime);
	    }
	  }
	  if ((cdevFilename && cdevFilename[0]) || (dbconnectCdev && dbconnectCdev[0])) {
	    TString cdevLocalDir = "./cdev";
	    gSystem->mkdir(cdevLocalDir);
	    TString cdevOutDir = TString::Format("%s/cdev", dirOut);
	    gSystem->mkdir(cdevOutDir);
	    cdevXmlFile = TString::Format("%s/cdev.%08i.%06i.xml", cdevLocalDir.Data(), cdevDate, cdevTime);
	    cdevXmlFileOut = TString::Format("%s/cdev.%08i.%06i.xml", cdevOutDir.Data(), cdevDate, cdevTime);
	  }
	  if (cdevFilename && cdevFilename[0]) {
	    if ((cdevXmlFile != "") && (cdevXmlFile != cdevFilename)) {
	      TString cdevCopyCmd = TString::Format("/bin/cp -f \"%s\" \"%s\"", cdevFilename, cdevXmlFile.Data());
	      LOG_INFO << "Copying CDEV record " << cdevFilename << " to " << cdevXmlFile << ": " << cdevCopyCmd << endm;
	      gSystem->Exec(cdevCopyCmd);
	    }
	  } else if (dbconnectCdev && dbconnectCdev[0]) {
	    if (cdevXmlFileTmp != "") {
	      TString cdevMoveTmpCmd = TString::Format("/bin/mv -f \"%s\" \"%s\"", cdevXmlFileTmp.Data(), cdevXmlFile.Data());
	      LOG_INFO << "Moving CDEV tmp record " << cdevXmlFileTmp << " to " << cdevXmlFile << ": " << cdevMoveTmpCmd << endm;
	      gSystem->Exec(cdevMoveTmpCmd);
	    } else {
	      if (cdevXmlFile && cdevXmlFile[0]) writeQueryCdev(dbconnectCdev, cdevDate, cdevTime, cdevXmlFile);
	    }
	  }
	  if ((cdevXmlFile != "") && (cdevXmlFile != cdevXmlFileOut)) {
	    TString cdevCopyOutCmd = TString::Format("/bin/cp -f \"%s\" \"%s\"", cdevXmlFile.Data(), cdevXmlFileOut.Data());
	    LOG_INFO << "Copying CDEV record to output directory " << cdevXmlFile << " to " << cdevXmlFileOut << ": " << cdevCopyOutCmd << endm;
	    gSystem->Exec(cdevCopyOutCmd);
	  }

	  {
	    TString cdevRecord;
	    ifstream ifstr(cdevXmlFile);
	    if (ifstr.good()) {
	      const Int_t buflen = 2048;
	      Char_t buf[buflen];
	      while (ifstr.good()) {
		ifstr.getline(buf, buflen);
		buf[buflen - 1] = 0;
		cdevRecord += buf;
	      }
	      ifstr.close();
	    } else {
	      LOG_ERROR << "Cannot read CDEV record file " << cdevFilename << endm;
	    }
	    Float_t fillNumberF = 0;
	    getPattern(cdevRecord, "Yell", "ring", "fillNumberM", fillNumberStr, &fillNumberF, 1);
	    fillNumberStr = TString::Format("%i", Int_t(fillNumberF));
	    LOG_DEBUG << "Read fill number " << fillNumberStr << endm;
	  }
	  if (zdcPlots) zdcPlots->setCdevXmlFile(cdevXmlFile);
	  if (zdcPlots) zdcPlots->initRun(date, time);
	}
	Bool_t runFileProcessed = false;
	Int_t evts_old = evts;
	if (filenameStrNext.EndsWith(".daq")) {
	  readDaq(zdcPlots, filenameStrNext.Data(), nEvents, runTriggersToProcessMask, &run, &date, &time, &evts, &evtsAccepted, iprepost);
	  runFileProcessed = true;
	} else if (filenameStrNext.EndsWith(".dat")) {
	  readDat(zdcPlots, filenameStrNext.Data(), nEvents, runTriggersToProcessMask, &run, &date, &time, &evts, &evtsAccepted, iprepost);
	  runFileProcessed = true;
	}
	if (runFileProcessed && (evts > evts_old)) {
	  runProcessed = true;
	  totalFilesProcessed++;
	  if (fstr_filesLast.good()) {
	    fstr_filesLast << TString::Format("<TR> <TD align=\"right\">%i</TD> <TD align=\"left\">%s</TD> </TR>", totalFilesProcessed, filenameStrNext.Data()) << endl;
	  } else {
	    LOG_ERROR << "Cannot add line to file " << outFilesLastFilename << endm;
	  }
	} else {
	  if (stopRunOnFirstError) {
	    runFailed = true;
	    failedRunList.push_back(run);
	    break; // while loop
	  }
	}
      }
      if (runProcessed) totalRunsProcessed++;
      LOG_DEBUG << "Finished reading file " << nextFilenameStr.Data() << endm;
      bool histoSaved = false;
      TString dirOutHisto = TString::Format("%s/%s", dirOut, "histo");
      if (dirOut && dirOut[0]) gSystem->mkdir(dirOutHisto.Data(), kTRUE);
      TString prepostNameStr = iprepost ? TString::Format("_prepost%+i", iprepost) : "";
      TString prepostCommentStr = iprepost ? TString::Format("prepost = %+i", iprepost) : "";
      TString prepostTitleStr = iprepost ? TString::Format(", %s", prepostCommentStr.Data()) : "";
      TString outHistFilename = TString::Format("%s/run_%i%s.%s.root", dirOutHisto.Data(), run, prepostNameStr.Data(), "histo");
      TString outFillFilename = TString::Format("%s/fill_%s.%s.root", dirOutHisto.Data(), fillNumberStr.Data(), "histo");
      Bool_t useThisRunForCalibrationPed = false;
      Bool_t useThisRunForCalibrationGain = false;
      Bool_t useThisRunForCalibrationTowerPed = false;
      Bool_t useThisRunForCalibrationTowerCalib = false;
      Bool_t useThisRunForCalibrationSlewing = false;
      Bool_t addThisRunToFill = false;
      TString outTextRunFilename = TString::Format("%s/run_%i%s.txt", dirOutText.Data(), run, prepostNameStr.Data());
      TString outTextFillFilename = TString::Format("%s/fill_%s.txt", dirOutText.Data(), fillNumberStr.Data());
      if (runProcessed) {
	TFile *histFile = new TFile(outHistFilename, "RECREATE");
	if (histFile && histFile->IsOpen()) {
	  if (zdcPlots) zdcPlots->saveHistograms(histFile);
	  histFile->Close();
	  histoSaved = true;
	} else {
	  LOG_ERROR << "Cannot write histograms to file " << outHistFilename << endm;
	}
	if (histFile) delete histFile;
	histFile = 0;
	useThisRunForCalibrationPed = !iprepost && (evtsAccepted >= minEventsToCalibratePed) && !((namesToCalibratePedStr != "") && (runTriggers != "") && !runTriggers.Contains(namesToCalibratePedRE));
	useThisRunForCalibrationGain = !iprepost && (evtsAccepted >= minEventsToCalibrateGain) && !((namesToCalibrateGainStr != "") && (runTriggers != "") && !runTriggers.Contains(namesToCalibrateGainRE));
	useThisRunForCalibrationTowerPed = !iprepost && (evtsAccepted >= minEventsToCalibrateTowerPed) && !((namesToCalibrateTowerPedStr != "") && (runTriggers != "") && !runTriggers.Contains(namesToCalibrateTowerPedRE));
	useThisRunForCalibrationTowerCalib = !iprepost && (evtsAccepted >= minEventsToCalibrateTowerCalib) && !((namesToCalibrateTowerCalibStr != "") && (runTriggers != "") && !runTriggers.Contains(namesToCalibrateTowerCalibRE));
	useThisRunForCalibrationSlewing = !iprepost && (evtsAccepted >= minEventsToCalibrateSlewing) && !((namesToCalibrateSlewingStr != "") && (runTriggers != "") && !runTriggers.Contains(namesToCalibrateSlewingRE));
	addThisRunToFill = !iprepost && (evtsAccepted >= minEventsToAddFill) && !((namesToAddFillStr != "") && (runTriggers != "") && !runTriggers.Contains(namesToAddFillRE));
	zdcPlotsPresenter->clear();
      }
      Bool_t addedToFill = false;
      Bool_t firstRunInFill = false;
      if (addThisRunToFill && (fillNumberStr != "") && (fillNumberStr != "0")) {
	if (outFillFilename != "") {
	  TString addRunToFillCmd;
	  if (gSystem->GetPathInfo(outFillFilename, (Long_t*)0, (Long_t*)0, (Long_t*)0, (Long_t*)0) == 0) {
	    addRunToFillCmd = TString::Format("/bin/cp -f \"%s\" \"%s.tmp\" && /bin/mv -f \"%s\" \"%s.old\" && hadd \"%s\" \"%s.tmp\" \"%s\" && /bin/rm -f \"%s.tmp\" && /bin/rm -f \"%s.old\"", 
		outFillFilename.Data(), outFillFilename.Data(), outFillFilename.Data(), outFillFilename.Data(), outFillFilename.Data(), 
		outFillFilename.Data(), outHistFilename.Data(), outFillFilename.Data(), outFillFilename.Data());
	    LOG_INFO << "Adding run file " << outHistFilename << " to fill " << outFillFilename << ": " << addRunToFillCmd << endm;
	  } else {
	    firstRunInFill = true;
	    addRunToFillCmd = TString::Format("/bin/cp -f \"%s\" \"%s\"", outHistFilename.Data(), outFillFilename.Data());
	    LOG_INFO << "Copying run file " << outHistFilename << " to fill " << outFillFilename << ": " << addRunToFillCmd << endm;
	  }
	  gSystem->Exec(addRunToFillCmd);
	  addedToFill = true;
	}
      }
      TString formatsLineFill;
      TString fillCommentStr;
      if (addedToFill) {
	TFile *histFile = new TFile(outFillFilename, "READ");
	if (histFile && histFile->IsOpen()) {
	  gSystem->Unlink(outTextFillFilename);
	  zdcPlotsPresenter->setAsymmetryOutputFilename(outTextFillFilename);
	  canvases_list_type canvasesFill;
	  for (Int_t i = 0;i < zdcPlotsPresenter->getMaxId();i++) {
	    TString name = TString::Format("c_fill_%s_%i", fillNumberStr.Data(), i);
	    TString title = TString::Format("%s (fill %s)", zdcPlotsPresenter->getTitle(i), fillNumberStr.Data());
	    TCanvas *c = new TCanvas(name, title);
	    Bool_t isEmpty = false;
	    zdcPlotsPresenter->displayTab(i, GenericFile(histFile), c, &isEmpty);
	    if (c && !isEmpty) {
	      canvasesFill.push_back(c);
	    }
	  }
	  saveCanvases(canvasesFill, formatsFill, dirOut, "fill", fillNumberStr, "", formatsLineFill, zdcPlotsPresenter->getCleanupArray());
	  histFile->Close();
	  for (canvases_list_type::const_iterator iter = canvasesFill.begin();iter != canvasesFill.end();++iter) {
	    TCanvas *c = *iter;
	    if (c) delete c;
	  }
	  canvasesFill.clear();
	  zdcPlotsPresenter->setAsymmetryOutputFilename("");
	  appendRunsAsymmetry(outTextFillFilename, fillNumberStr, outTextFillsFilename);
	  plotsAsymmetrySummary(outTextFillsFilename, "Fill", 10, normAnalyzingPower, outAsymFillsGifFilename, outAsymFillsCFilename);
	  plotsAsymmetrySummary(outTextFillsFilename, "Fill", -1, normAnalyzingPower, outAsymFillsAllGifFilename, outAsymFillsAllCFilename);
	} else {
	  LOG_ERROR << "Cannot read histograms from file " << outFillFilename << endm;
	}
	if (histFile) delete histFile;
	histFile = 0;
	fillCommentStr = TString::Format("added to fill %s", fillNumberStr.Data());
	if (firstRunInFill) {
	  fillCommentStr += TString(" (") + formatsLineFill + TString(")");
	}
      }
      TCanvas *c_ZDCCalibSMD = 0;
      TCanvas *c_ZDCCalibTower = 0;
      TCanvas *c_ZDCCalibSlewing = 0;
      if (useThisRunForCalibrationPed || useThisRunForCalibrationGain || useThisRunForCalibrationTowerPed || useThisRunForCalibrationTowerCalib || useThisRunForCalibrationSlewing) {
	TString dirOutTextDb = TString::Format("%s/%s", dirOut, "textdb");
	if (dirOut && dirOut[0]) gSystem->mkdir(dirOutTextDb.Data(), kTRUE);
	TString outTextDbFilename = TString::Format("%s/zdcPedCal.%08i.%06i.txt", dirOutTextDb.Data(), date, time);
	c_ZDCCalibSMD = new TCanvas(TString::Format("c_ZDCCalibSMD_%i", run), TString::Format("ZDC SMD Pedestal and Gain Calculation, (run %i)", run));
	c_ZDCCalibTower = new TCanvas(TString::Format("c_ZDCCalibTower_%i", run), TString::Format("ZDC Tower Pedestal and Gain Calculation, (run %i)", run));
	c_ZDCCalibSlewing = new TCanvas(TString::Format("c_ZDCCalibSlewing_%i", run), TString::Format("ZDC Slewing Calculation, (run %i)", run));
	Bool_t calibratedPedOk = useThisRunForCalibrationPed;
	Bool_t calibratedGainOk = useThisRunForCalibrationGain;
	Bool_t calibratedTowerPedOk = useThisRunForCalibrationTowerPed;
	Bool_t calibratedTowerCalibOk = useThisRunForCalibrationTowerCalib;
	Bool_t calibratedSlewingOk = useThisRunForCalibrationSlewing;
	calibrateZDC(outHistFilename.Data()
	    , calibratedPedOk, calibratedGainOk
	    , calibratedTowerPedOk, calibratedTowerCalibOk, calibratedSlewingOk
	    , zdcPedCal
	    , run, date, time
	    , outTextDbFilename.Data()
	    , dirOutDb.Data()
	    , zdcPlotsPresenter->getCleanupArray()
	    , c_ZDCCalibSMD, c_ZDCCalibTower, c_ZDCCalibSlewing
	    );
	if (c_ZDCCalibSMD && !((useThisRunForCalibrationPed && calibratedPedOk) || (useThisRunForCalibrationGain && calibratedGainOk))) {
	  delete c_ZDCCalibSMD;
	  c_ZDCCalibSMD = 0;
	}
	if (c_ZDCCalibTower && !((useThisRunForCalibrationTowerPed && calibratedTowerPedOk) || (useThisRunForCalibrationTowerCalib && calibratedTowerCalibOk))) {
	  delete c_ZDCCalibTower;
	  c_ZDCCalibTower = 0;
	}
	if (c_ZDCCalibSlewing && !(useThisRunForCalibrationSlewing && calibratedSlewingOk)) {
	  delete c_ZDCCalibSlewing;
	  c_ZDCCalibSlewing = 0;
	}
	if (zdcPedCal && (outTextDbFilename != "") && updateInputTextDb && 
	    ((useThisRunForCalibrationPed && calibratedPedOk) || 
	     (useThisRunForCalibrationGain && calibratedGainOk) || 
	     (useThisRunForCalibrationTowerPed && calibratedTowerPedOk) || 
	     (useThisRunForCalibrationTowerCalib && calibratedTowerCalibOk) || 
	     (useThisRunForCalibrationSlewing && calibratedSlewingOk))) {
	  LOG_INFO << "Updating local text DB " << zdcPedCal << " from " << outTextDbFilename << endm;
	  TString copyTextDbCmd = TString::Format("/bin/cp -f \"%s\" \"%s\"", outTextDbFilename.Data(), zdcPedCal);
	  LOG_DEBUG << "Executing command " << copyTextDbCmd << endm;
	  gSystem->Exec(copyTextDbCmd);
	}
      }
      bool plotsSaved = false;
      TString formatsLine;
      if (histoSaved) {
	TFile *histFile = new TFile(outHistFilename, "READ");
	if (histFile && histFile->IsOpen()) {
	  if (!iprepost)  {
	    zdcPlotsPresenter->setAsymmetryOutputFilename(outTextRunFilename);
	  } else {
	    zdcPlotsPresenter->setAsymmetryOutputFilename("");
	  }
	  for (canvases_list_type::const_iterator iter = canvases.begin();iter != canvases.end();++iter) {
	    TCanvas *c = *iter;
	    if (c) delete c;
	  }
	  canvases.clear();
	  for (Int_t i = 0;i < zdcPlotsPresenter->getMaxId();i++) {
	    TString name = TString::Format("c_%i_%i%s", run, i, prepostNameStr.Data());
	    TString title = TString::Format("%s (run %i%s)", zdcPlotsPresenter->getTitle(i), run, prepostTitleStr.Data());
	    TCanvas *c = new TCanvas(name, title);
	    Bool_t isEmpty = false;
	    zdcPlotsPresenter->displayTab(i, GenericFile(histFile), c, &isEmpty);
	    if (c && !isEmpty) {
	      canvases.push_back(c);
	    }
	  }
	  if (c_ZDCCalibSMD) canvases.push_back(c_ZDCCalibSMD);
	  if (c_ZDCCalibTower) canvases.push_back(c_ZDCCalibTower);
	  if (c_ZDCCalibSlewing) canvases.push_back(c_ZDCCalibSlewing);
	  saveCanvases(canvases, formats, dirOut, "run", TString::Format("%i", run), prepostNameStr, formatsLine, zdcPlotsPresenter->getCleanupArray());
	  plotsSaved = true;
	  histFile->Close();
	  if (deleteCanvasOnExit) {
	    for (canvases_list_type::const_iterator iter = canvases.begin();iter != canvases.end();++iter) {
	      TCanvas *c = *iter;
	      if (c) delete c;
	    }
	    canvases.clear();
	  }
	  zdcPlotsPresenter->setAsymmetryOutputFilename("");
	  if (!iprepost) {
	    appendRunsAsymmetry(outTextRunFilename, TString::Format("%i", run), outTextRunsFilename);
	    plotsAsymmetrySummary(outTextRunsFilename, "Run", 10, normAnalyzingPower, outAsymRunsGifFilename, outAsymRunsCFilename);
	    plotsAsymmetrySummary(outTextRunsFilename, "Run", -1, normAnalyzingPower, outAsymRunsAllGifFilename, outAsymRunsAllCFilename);
	  }
	} else {
	  LOG_ERROR << "Cannot read histograms from file " << outHistFilename << endm;
	}
	if (histFile) delete histFile;
	histFile = 0;
      }
      TDatime stopTimeRun;
      Int_t runTimeSec = stopTimeRun.Get() - startTimeRun.Get();
      if (runProcessed && histoSaved && plotsSaved) {
	if (iprepost == nPostFound) {
	  LOG_DEBUG << "Adding " << run << " to the list of processed runs " << seenRuns << endm;
	  seenRunList.push_back(run);
	  if (seenRuns && seenRuns[0]) {
	    ofstream fstr(seenRuns, ios_base::app);
	    if (fstr.good()) {
	      fstr << run << endl;
	      if (fstr.good()) {
		fstr.close();
	      } else {
		LOG_ERROR << "Cannot add line to file " << seenRuns << endm;
	      }
	    } else {
	      LOG_ERROR << "Cannod write to file " << seenRuns << endm;
	    }
	  }
	}
	if (dirOut && dirOut[0]) {
	  ofstream fstr(outListLastFilename, ios_base::app);
	  if (fstr.good()) {
	    TString runLogLink = TString::Format(runLogUrl, run);
	    TString comment;
	    Bool_t commaNeeded = false;
	    if (prepostCommentStr != "") {
	      if (commaNeeded) comment += ", ";
	      comment += prepostCommentStr;
	      commaNeeded = true;
	    }
	    if (fillCommentStr != "") {
	      if (commaNeeded) comment += ", ";
	      comment += fillCommentStr;
	      commaNeeded = true;
	    }
	    if (useThisRunForCalibrationPed || useThisRunForCalibrationGain || useThisRunForCalibrationTowerPed || useThisRunForCalibrationTowerCalib || useThisRunForCalibrationSlewing) {
	      TString calibMode;
	      Bool_t commaNeeded1 = false;
	      if (useThisRunForCalibrationTowerPed) {
		if (commaNeeded1) calibMode += ", ";
		calibMode += "tower ped";
		commaNeeded1 = true;
	      }
	      if (useThisRunForCalibrationTowerCalib) {
		if (commaNeeded1) calibMode += ", ";
		calibMode += "tower gain";
		commaNeeded1 = true;
	      }
	      if (useThisRunForCalibrationSlewing) {
		if (commaNeeded1) calibMode += ", ";
		calibMode += "slewing";
		commaNeeded1 = true;
	      }
	      if (useThisRunForCalibrationPed) {
		if (commaNeeded1) calibMode += ", ";
		calibMode += "SMD ped";
		commaNeeded1 = true;
	      }
	      if (useThisRunForCalibrationGain) {
		if (commaNeeded1) calibMode += ", ";
		calibMode += "SMD gain";
		commaNeeded1 = true;
	      }
	      if (commaNeeded) comment += ", ";
	      comment += TString::Format("used for calibration (%s): <A href=\"%s/zdcPedCal.%08i.%06i.txt\">%s</A>", calibMode.Data(), "textdb", date, time, "textdb");
	      commaNeeded = true;
	    }
	    TString stopTimeRunStr = stopTimeRun.AsSQLString();
	    TString listLine = TString::Format("<TR> <TD align=\"left\"><A href=\"%s\">%i</A></TD> <TD align=\"right\"><B>%i</B>/%i</TD> <TD align=\"center\">%s</TD> <TD align=\"left\">%s (%i sec)</TD> <TD align=\"center\">%s</TD> <TD align=\"left\"><B>%s</B></TD><TD align=\"left\"><A href=\"analysis.%i.htm\">analysis</A></TD> </TR>", 
		runLogLink.Data(), run, evtsAccepted, evts, formatsLine.Data(), stopTimeRunStr.Data(), runTimeSec, runTriggers.Data(), comment.Data(),run);// Add->Analysis by xyf xuyifei 
	    fstr << listLine << endl;
	    if (fstr.good()) {
	      fstr.close();
	    } else {
	      LOG_ERROR << "Cannot add line to file " << outListLastFilename << endm;
	    }
	  } else {
	    LOG_ERROR << "Cannod write to file " << outListLastFilename << endm;
	  }
	}
      }
    } // for loop over prepost
  }
  if (fstr_filesLast.good()) {
    fstr_filesLast << "</TABLE>" << endl;
  } else {
    LOG_ERROR << "Cannot write line to file " << outFilesLastFilename << ", file is probably corrupted" << endm;
  }
  fstr_filesLast.close();
  TDatime stopTime;
  Int_t totalTimeSec = stopTime.Get() - startTime.Get();
  for (dir_list_type::const_iterator iter = dirList.begin();iter != dirList.end();++iter) {
    TList *l = (*iter).second;
    if (l) delete l;
  }
  if (deleteCanvasOnExit) {
    if (zdcPlots) delete zdcPlots;
    zdcPlots = 0;
    if (zdcPlotsPresenter) delete zdcPlotsPresenter;
    zdcPlotsPresenter = 0;
  }
  if (dbMaker) {
    dbMaker->Finish();
    delete dbMaker;
    dbMaker = 0;
  }
  if (totalFilesProcessed > 0) {
    FileStat_t pageHeaderStat;
    TString outPageHeaderFilename = TString::Format("%s/pageheader.html", dirOut);
    if (dirOut && dirOut[0]/* && (gSystem->GetPathInfo(outPageHeaderFilename.Data(), pageHeaderStat) != 0)*/) {
      ofstream fstr(outPageHeaderFilename);
      if (fstr.good()) {
	TString refreshRateStr;
	TString refreshRateInfoStr;
	if (refreshRateSeconds > 0) {
	  refreshRateStr = TString::Format("<meta http-equiv=\"refresh\" content=\"%i\" />", refreshRateSeconds);
	  refreshRateInfoStr = TString::Format("<I>This page will reload in %i seconds</I><BR/><BR/>", refreshRateSeconds);
	}
	fstr << "<HTML>" << endl << "<HEAD>" << endl;
	if (refreshRateStr != "") fstr << refreshRateStr << endl;
	fstr << "<TITLE>" << myTitle << "</TITLE>" << endl;
	fstr << "</HEAD>" << endl << "<BODY>" << endl << "<H1>" << myTitle << "</H1>" << endl;
	if (refreshRateInfoStr != "") fstr << refreshRateInfoStr << endl;
	if (fstr.good()) {
	  fstr.close();
	} else {
	  LOG_ERROR << "Cannot add line to file " << outPageHeaderFilename << endm;
	}
      } else {
	LOG_ERROR << "Cannod write to file " << outPageHeaderFilename << endm;
      }
    }
    FileStat_t pageFooterStat;
    TString outPageFooterFilename = TString::Format("%s/pagefooter.html", dirOut);
    if (dirOut && dirOut[0] && (gSystem->GetPathInfo(outPageFooterFilename.Data(), pageFooterStat) != 0)) {
      ofstream fstr(outPageFooterFilename);
      if (fstr.good()) {
	fstr << "</BODY>" << endl << "</HTML>" << endl;
	if (fstr.good()) {
	  fstr.close();
	} else {
	  LOG_ERROR << "Cannot add line to file " << outPageFooterFilename << endm;
	}
      } else {
	LOG_ERROR << "Cannod write to file " << outPageFooterFilename << endm;
      }
    }
    FileStat_t headerStat;
    TString outHeaderFilename = TString::Format("%s/header.html", dirOut);
    if (dirOut && dirOut[0] && (gSystem->GetPathInfo(outHeaderFilename.Data(), headerStat) != 0)) {
      ofstream fstr(outHeaderFilename);
      if (fstr.good()) {
	TString headerLine;
	fstr << "<BR/>" << endl;
	headerLine = TString::Format("<TABLE border=\"1\">");
	fstr << headerLine << endl;
	headerLine = TString::Format("<TR> <TD align=\"left\">Run</TD> <TD align=\"right\">Events</TD> <TD align=\"center\">Plots</TD> <TD align=\"left\">Posting time</TD> <TD align=\"center\">Triggers</TD> <TD align=\"left\">Comment</TD><TD align=\"left\">Analysis</TD> </TR>");// Add -> Analysis ; by xyf xuyifei
	fstr << headerLine << endl;
	if (fstr.good()) {
	  fstr.close();
	} else {
	  LOG_ERROR << "Cannot add line to file " << outHeaderFilename << endm;
	}
      } else {
	LOG_ERROR << "Cannod write to file " << outHeaderFilename << endm;
      }
    }
    FileStat_t footerStat;
    TString outFooterFilename = TString::Format("%s/footer.html", dirOut);
    if (dirOut && dirOut[0] && (gSystem->GetPathInfo(outFooterFilename.Data(), footerStat) != 0)) {
      ofstream fstr(outFooterFilename);
      if (fstr.good()) {
	TString footerLine;
	footerLine = TString::Format("</TABLE>");
	fstr << footerLine << endl;
	if (fstr.good()) {
	  fstr.close();
	} else {
	  LOG_ERROR << "Cannot add line to file " << outFooterFilename << endm;
	}
      } else {
	LOG_ERROR << "Cannod write to file " << outFooterFilename << endm;
      }
    }
    TString outInfoFilename = TString::Format("%s/info.html", dirOut);
    if (dirOut && dirOut[0]) {
      ofstream fstr(outInfoFilename);
      if (fstr.good()) {
	////fstr << "<A href=\"" << outAsymFillsAllGifWebFilename << "\"><IMG src=\"" << outAsymFillsGifWebFilename << "\" border=\"0\" alt=\"Raw asymmetry in recent fills, click to see all fills\"/></A><A href=\"" << outAsymRunsAllGifWebFilename << "\"><IMG src=\"" << outAsymRunsGifWebFilename << "\" border=\"0\" alt=\"Raw asymmetry in recent runs, click to see all runs\"></A><BR/><BR/>" << endl;//CJ
	TString infoLine;
	TString startTimeStr = startTime.AsSQLString();
	TString stopTimeStr = stopTime.AsSQLString();
	fstr << "<TABLE border=\"0\">" << endl;
	infoLine = TString::Format("<TR> <TD align=\"right\">Last monitoring run:</TD> <TD align=\"left\">%s &mdash; %s (%i sec)</TD> <TR>", startTimeStr.Data(), stopTimeStr.Data(), totalTimeSec);
	fstr << infoLine << endl;
	infoLine = TString::Format("<TR> <TD align=\"right\">Total processed files:</TD> <TD align=\"left\">%i</TD> <TR>", totalFilesProcessed);
	fstr << infoLine << endl;
	infoLine = TString::Format("<TR> <TD align=\"right\">Total processed runs:</TD> <TD align=\"left\">%i</TD> <TR>", totalRunsProcessed);
	fstr << infoLine << endl;
	infoLine = TString::Format("<TR> <TD align=\"right\">Host:</TD> <TD align=\"left\">%s</TD> <TR>", gSystem->HostName());
	fstr << infoLine << endl;
	UserGroup_t *userInfo = gSystem->GetUserInfo();
	if (userInfo) {
	  infoLine = TString::Format("<TR> <TD align=\"right\">User:</TD> <TD align=\"left\">%s, %s</TD> <TR>", userInfo->fUser.Data(), userInfo->fRealName.Data());
	  fstr << infoLine << endl;
	  delete userInfo;
	  userInfo = 0;
	}
	Long_t fsId, blockSize, blocksTotal, blocksFree;
	if (false) {
	  infoLine = TString::Format("<TR> <TD align=\"right\">Home directory:</TD> <TD align=\"left\">%s</TD> <TR>", gSystem->HomeDirectory());
	  fstr << infoLine << endl;
	  if (gSystem->GetFsInfo(gSystem->HomeDirectory(), &fsId, &blockSize, &blocksTotal, &blocksFree) == 0) {
	    Float_t totalGb = (Float_t(blocksTotal)/1024.0) * (Float_t(blockSize)/1024.0) / 1024.0;
	    Float_t freeGb = (Float_t(blocksFree)/1024.0) * (Float_t(blockSize)/1024.0) / 1024.0;
	    infoLine = TString::Format("<TR> <TD align=\"right\">Home disk space:</TD> <TD align=\"left\">%.1f Gb of %.1f Gb free</TD> <TR>", freeGb, totalGb);
	    fstr << infoLine << endl;
	  }
	}
	if (true) {
	  infoLine = TString::Format("<TR> <TD align=\"right\">Working directory:</TD> <TD align=\"left\">%s</TD> <TR>", gSystem->WorkingDirectory());
	  fstr << infoLine << endl;
	  if (gSystem->GetFsInfo(gSystem->WorkingDirectory(), &fsId, &blockSize, &blocksTotal, &blocksFree) == 0) {
	    Float_t totalGb = (Float_t(blocksTotal)/1024.0) * (Float_t(blockSize)/1024.0) / 1024.0;
	    Float_t freeGb = (Float_t(blocksFree)/1024.0) * (Float_t(blockSize)/1024.0) / 1024.0;
	    infoLine = TString::Format("<TR> <TD align=\"right\">Working disk space:</TD> <TD align=\"left\">%.1f Gb of %.1f Gb free</TD> <TR>", freeGb, totalGb);
	    fstr << infoLine << endl;
	  }
	}
	if (true) {
	  infoLine = TString::Format("<TR> <TD align=\"right\">Output directory:</TD> <TD align=\"left\">%s</TD> <TR>", dirOut);
	  fstr << infoLine << endl;
	  if (gSystem->GetFsInfo(dirOut, &fsId, &blockSize, &blocksTotal, &blocksFree) == 0) {
	    Float_t totalGb = (Float_t(blocksTotal)/1024.0) * (Float_t(blockSize)/1024.0) / 1024.0;
	    Float_t freeGb = (Float_t(blocksFree)/1024.0) * (Float_t(blockSize)/1024.0) / 1024.0;
	    infoLine = TString::Format("<TR> <TD align=\"right\">Output disk space:</TD> <TD align=\"left\">%.1f Gb of %.1f Gb free</TD> <TR>", freeGb, totalGb);
	    fstr << infoLine << endl;
	  }
	}
#if ROOT_VERSION_CODE >= ROOT_VERSION(5,14,0)
	ProcInfo_t procInfo;
	if (gSystem->GetProcInfo(&procInfo) == 0) {
	  infoLine = TString::Format("<TR> <TD align=\"right\">Program:</TD> <TD align=\"left\">CPU system/user = %d/%d sec, MEM resident/virtual = %ld/%ld</TD> <TR>", procInfo.fCpuSys, procInfo.fCpuUser, procInfo.fMemResident, procInfo.fMemVirtual);
	  fstr << infoLine << endl;
	}
#endif
	infoLine = "<TR> <TD align=\"right\">Directories:</TD> <TD align=\"left\">";
	TObjArray *formatsArr = formats.Tokenize(" ");
	if (formatsArr) for (Int_t ifmt = 0;ifmt < formatsArr->GetEntries();ifmt++) {
	  TString currFormat = (static_cast<TObjString*>(formatsArr->At(ifmt)))->GetString();
	  if (currFormat != "") {
	    infoLine += TString::Format(" <A href=\"%s\">%s</A>", currFormat.Data(), currFormat.Data());
	  }
	}
	if (formatsArr) delete formatsArr; formatsArr = 0;
	infoLine += TString::Format(" <A href=\"%s\">%s</A>", "histo", "raw");
	infoLine += TString::Format(" <A href=\"%s\">%s</A>", "textdb", "textdb");
	infoLine += TString::Format(" <A href=\"%s\">%s</A>", "StarDb/Calibrations/zdc", "db");
	infoLine += TString::Format(" <A href=\"%s\">%s</A>", "cdev", "cdev");
	////infoLine += TString::Format(" <A href=\"%s\">%s</A>", "asym", "asym");//CJ
	infoLine += "</TD> <TR>";
	fstr << infoLine << endl;
	if (configFileBasename != "") {
	  infoLine = TString::Format("<TR> <TD align=\"right\">Config file:</TD> <TD align=\"left\"><A href=\"%s\">%s</A></TD> <TR>", configFileBasename.Data(), configFileBasename.Data());
	  fstr << infoLine << endl;
	}
	fstr << "</TABLE>" << endl;
	fstr << "<BR/>" << endl;
	if (fstr.good()) {
	  fstr.close();
	} else {
	  LOG_ERROR << "Cannot add line to file " << outInfoFilename << endm;
	}
      } else {
	LOG_ERROR << "Cannod write to file " << outInfoFilename << endm;
      }
    }

    TString outListFilename = TString::Format("\"%s\"/list.html", dirOut);
    TString makeListCmd = TString::Format("/bin/touch \"%s\" ;/bin/cat \"%s\" >> \"%s\"", outListFilename.Data(), outListLastFilename.Data(), outListFilename.Data());
    gSystem->Exec(makeListCmd);

    TString outIndexFilename = TString::Format("%s/index.html", dirOut);
    TString makeIndexCmd1 = TString::Format("/bin/cat \"%s\" \"%s\" >| \"%s\"", outPageHeaderFilename.Data(), outInfoFilename.Data(), outIndexFilename.Data());
    gSystem->Exec(makeIndexCmd1);
    TString makeIndexCmd2 = TString::Format("echo \"<A href=\\\"index_last.html\\\">Last monitoring run</A><BR/>\" >> \"%s\"", outIndexFilename.Data());
    gSystem->Exec(makeIndexCmd2);
    TString makeIndexCmd3 = TString::Format("/bin/cat \"%s\" \"%s\" \"%s\" \"%s\" >> \"%s\"", outHeaderFilename.Data(), outListFilename.Data(), outFooterFilename.Data(), outPageFooterFilename.Data(), outIndexFilename.Data());
    gSystem->Exec(makeIndexCmd3);

    TString outIndexLastFilename = TString::Format("%s/index_last.html", dirOut);
    TString makeIndexLastCmd1 = TString::Format("/bin/cat \"%s\" \"%s\" >| \"%s\"", outPageHeaderFilename.Data(), outInfoFilename.Data(), outIndexLastFilename.Data());
    gSystem->Exec(makeIndexLastCmd1);
    TString makeIndexLastCmd2 = TString::Format("echo \"<A href=\\\"index.html\\\">Full list</A><BR/>\" >> \"%s\"", outIndexLastFilename.Data());
    gSystem->Exec(makeIndexLastCmd2);
    TString makeIndexLastCmd3 = TString::Format("/bin/cat \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" >> \"%s\"", outHeaderFilename.Data(), outListLastFilename.Data(), outFooterFilename.Data(), outFilesLastFilename.Data(), outPageFooterFilename.Data(), outIndexLastFilename.Data());
    gSystem->Exec(makeIndexLastCmd3);

    if (emailNotification) {
      TString outEmailLastFilename = TString::Format("\"%s\"/email_last.html", dirOut);
      TString makeEmailLastCmd0 = TString::Format("echo \"To: %s\nSubject: %s\nMIME-version: 1.0\nContent-type: text/html; charset=us-ascii\nBody:\n\n\" >| \"%s\"", emailNotification, myTitle, outEmailLastFilename.Data());
      gSystem->Exec(makeEmailLastCmd0);
      TString makeEmailLastCmd1 = TString::Format("/bin/cat \"%s\" \"%s\" >> \"%s\"", outPageHeaderFilename.Data(), outInfoFilename.Data(), outEmailLastFilename.Data());
      gSystem->Exec(makeEmailLastCmd1);
      if (publicDir) {
	TString makeEmailLastCmd2 = TString::Format("echo \"<A href=\\\"%s/index_last.html\\\">Last monitoring run</A>\" >> \"%s\"", publicDir, outEmailLastFilename.Data());
	gSystem->Exec(makeEmailLastCmd2);
	TString makeEmailLastCmd3 = TString::Format("echo \"<A href=\\\"%s/index.html\\\">Full list</A>\" >> \"%s\"", publicDir, outEmailLastFilename.Data());
	gSystem->Exec(makeEmailLastCmd3);
      }
      TString makeEmailLastCmd4 = TString::Format("/bin/cat \"%s\" >> \"%s\"", outPageFooterFilename.Data(), outEmailLastFilename.Data());
      gSystem->Exec(makeEmailLastCmd4);
      TString sendEmailLastCmd = TString::Format("/usr/sbin/sendmail -t < \"%s\"", outEmailLastFilename.Data());
      //TString sendEmailLastCmd = TString::Format("/bin/cat \"%s\" | /bin/mail -s \"%s\" \"%s\"", outEmailLastFilename.Data(), myTitle, emailNotification);
      gSystem->Exec(sendEmailLastCmd);
    }
  }
}
