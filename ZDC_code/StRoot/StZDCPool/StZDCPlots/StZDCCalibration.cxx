#include <iostream>
#include <fstream>
#include <list>
using namespace std;

#include <TROOT.h>
#include <TString.h>
#include <TObjString.h>
#include <TSystem.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2F.h>
#include <TF1.h>
#include <TLatex.h>

#include <StMessMgr.h>

#include <tables/St_zdcsmdPed_Table.h>
#include <tables/St_zdcsmdGain_Table.h>
#include <tables/St_ZdcCalPars_Table.h>
#include <tables/St_zdcCalib_Table.h>

#include "StZDCUtil/StZDCDb.h"

#include "ZDCPlotsNames.h"
#include "StZDCCalibration.h"

//---------------------------------------------------------------------------
void histFitSlicesY(TH2 *hist, TF1 *f1, Int_t firstbin, Int_t lastbin, Int_t cut, Option_t *option, TObjArray* arr) {
   bool onX = false;
   TAxis& outerAxis = (onX ? *hist->GetYaxis() : *hist->GetXaxis());
   TAxis& innerAxis = (onX ? *hist->GetXaxis() : *hist->GetYaxis());

   Int_t nbins  = outerAxis.GetNbins();
   if (firstbin < 0) firstbin = 0;
   if (lastbin < 0 || lastbin > nbins + 1) lastbin = nbins + 1;
   if (lastbin < firstbin) {firstbin = 0; lastbin = nbins + 1;}
   TString opt = option;
   opt.ToLower();
   Int_t ngroup = 1;
   if (opt.Contains("g2")) {ngroup = 2; opt.ReplaceAll("g2","");}
   if (opt.Contains("g3")) {ngroup = 3; opt.ReplaceAll("g3","");}
   if (opt.Contains("g4")) {ngroup = 4; opt.ReplaceAll("g4","");}
   if (opt.Contains("g5")) {ngroup = 5; opt.ReplaceAll("g5","");}

   //default is to fit with a gaussian
   if (f1 == 0) {
      f1 = (TF1*)gROOT->GetFunction("gaus");
      if (f1 == 0) f1 = new TF1("gaus","gaus",innerAxis.GetXmin(),innerAxis.GetXmax());
      else         f1->SetRange(innerAxis.GetXmin(),innerAxis.GetXmax());
   }
   Int_t npar = f1->GetNpar();
   if (npar <= 0) return;
   Double_t *parsave = new Double_t[npar];
   f1->GetParameters(parsave);

   if (arr) {
      arr->SetOwner();
      arr->Expand(npar + 1);
   }

   //Create one histogram for each function parameter
   Int_t ipar;
   TH1D **hlist = new TH1D*[npar];
   char *name   = new char[2000];
   char *title  = new char[2000];
   const TArrayD *bins = outerAxis.GetXbins();
   for (ipar=0;ipar<npar;ipar++) {
      sprintf(name,"%s_%d",hist->GetName(),ipar);
      sprintf(title,"Fitted value of par[%d]=%s",ipar,f1->GetParName(ipar));
      delete gDirectory->FindObject(name);
      if (bins->fN == 0) {
         hlist[ipar] = new TH1D(name,title, nbins, outerAxis.GetXmin(), outerAxis.GetXmax());
      } else {
         hlist[ipar] = new TH1D(name,title, nbins,bins->fArray);
      }
      hlist[ipar]->GetXaxis()->SetTitle(outerAxis.GetTitle());
      if (arr)
         (*arr)[ipar] = hlist[ipar];
   }
   sprintf(name,"%s_chi2",hist->GetName());
   delete gDirectory->FindObject(name);
   TH1D *hchi2 = 0;
   if (bins->fN == 0) {
      hchi2 = new TH1D(name,"chisquare", nbins, outerAxis.GetXmin(), outerAxis.GetXmax());
   } else {
      hchi2 = new TH1D(name,"chisquare", nbins, bins->fArray);
   }
   hchi2->GetXaxis()->SetTitle(outerAxis.GetTitle());
   if (arr)
      (*arr)[npar] = hchi2;

   //Loop on all bins in Y, generate a projection along X
   Int_t bin;
   Long64_t nentries;
   for (bin=firstbin;bin<=lastbin;bin += ngroup) {
      TH1D *hp;
      if (onX)
         hp= hist->ProjectionX("_temp",bin,bin+ngroup-1,"e");
      else
         hp= hist->ProjectionY("_temp",bin,bin+ngroup-1,"e");
      if (hp == 0) continue;
      nentries = Long64_t(hp->GetEntries());
      if (nentries == 0 || nentries < cut) {delete hp; continue;}
      f1->SetParameters(parsave);
      hp->Fit(f1,opt.Data());
      Int_t npfits = f1->GetNumberFitPoints();
      if (npfits > npar && npfits >= cut) {
         Int_t binOn = bin + ngroup/2;
         for (ipar=0;ipar<npar;ipar++) {
            hlist[ipar]->Fill(outerAxis.GetBinCenter(binOn),f1->GetParameter(ipar));
            hlist[ipar]->SetBinError(binOn,f1->GetParError(ipar));
         }
         hchi2->Fill(outerAxis.GetBinCenter(binOn),f1->GetChisquare()/(npfits-npar));
      }
      delete hp;
   }
   delete [] parsave;
   delete [] name;
   delete [] title;
   delete [] hlist;
}

//---------------------------------------------------------------------------
void calibrateZDC(const Char_t *filenamePlotsIn
			, Bool_t &calibratePed, Bool_t &calibrateGain
			, Bool_t &calibrateTowerPed, Bool_t &calibrateTowerCalib, Bool_t &calibrateSlewing
                        , const Char_t *zdcPedCalIn
			, Int_t run, Int_t date, Int_t time
                        , const Char_t *zdcPedCalOut
                        , const Char_t *dirTablesOut
			, TObjArray *cleanup
			, TCanvas *canvasOut, TCanvas *canvasOutTower, TCanvas *canvasOutSlewing
			) {
    Bool_t calibratedPedOk = false;
    Bool_t calibratedGainOk = false;
    Bool_t calibratedSlewingOk = false;
    Bool_t calibratedTowerPedOk = false;
    Bool_t calibratedTowerCalibOk = false;
    if (filenamePlotsIn && (zdcPedCalOut || dirTablesOut) && (calibratePed || calibrateGain || calibrateTowerPed || calibrateTowerCalib || calibrateSlewing)) {
	TFile *histFile = new TFile(filenamePlotsIn, "READ");
	if (histFile && histFile->IsOpen()) {
	    zdcsmdPed_st pedTable;
	    zdcsmdGain_st gainTable;
	    ZdcCalPars_st slewingTable;
	    zdcCalib_st towerCalibTable;
	    if (calibratePed || calibrateGain) {
	    TH2F *histADC = (TH2F*)histFile->Get(zdcsmd_ADCName);
	    if (histADC) {
		histADC->SetDirectory(0);
		if (cleanup) cleanup->Add(histADC);
		if (histADC->GetXaxis()->GetNbins() == 32) {
		    LOG_INFO << "Using RunID = " << run << ", date = " << date << ", time = " << time << endm;
		    pedTable.RunID = run;
		    gainTable.RunID = run;
		    Float_t slopes[32];
		    memset(slopes, 0, sizeof(slopes));
		    Float_t meanSlopes[2][2];
		    Int_t meanSlopesN[2][2];
		    memset(meanSlopes, 0, sizeof(meanSlopes));
		    memset(meanSlopesN, 0, sizeof(meanSlopesN));
		    Float_t meanSlope = 0;
		    TPad *pad = 0;
		    if (canvasOut) {
			canvasOut->cd();
			pad = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
			if (pad) {
    			    pad->Draw();
			    pad->cd(0);
			    pad->Divide(8, 4, 0.001, 0.001);
			}
		    }
		    for (Int_t strip = 0; strip < 32;strip++) {
			pedTable.ZdcsmdPedestal[strip] = 0;
			gainTable.ZdcsmdGain[strip] = 1;
        		TH1D *h = histADC->ProjectionY(TString::Format("%s_%i", histADC->GetName(), (strip % 8) + 1), strip + 1, strip + 1, "e");
	        	if (h) {
			    h->SetDirectory(0);
			    if (cleanup) cleanup->Add(h);
			    h->SetTitle(TString::Format("ZDC SMD %s %s %i", (strip / 16) ? "West" : "East", ((strip % 16) / 8) ? "Horiz" : "Vert", (strip % 8) + 1));
    			    Float_t ped1 = h->GetMean(), width1 = h->GetRMS();
			    Float_t ped2 = 0, width2 = 0;
			    Float_t ped = ped1, width = width1;
			    Float_t slope = 0;
			    Float_t maxBinX = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
			    TF1 *funcPed = new TF1("funcPed", "gaus");
			    if (funcPed) {
				if (cleanup) cleanup->Add(funcPed);
				funcPed->SetParameter(0, 0);
				funcPed->SetParLimits(0, 0, h->GetMaximum() * 1.1);
				funcPed->SetParameter(1, maxBinX);
				funcPed->SetParLimits(1, TMath::Max(0.0, maxBinX - 5.0), maxBinX + 5.0);
				funcPed->SetParameter(2, 1);
				funcPed->SetParLimits(2, 0, 5);
				funcPed->SetRange(TMath::Max(0.0, maxBinX - 5.0), maxBinX + 5.0);
				h->Fit(funcPed, "RQN");
				ped2 = funcPed->GetParameter(1);
				width2 = funcPed->GetParameter(2);
				ped = ped2;
				width = width2;
			    }
			    Float_t gainFitLow = ped + 50;//(1 * width);
			    Float_t gainFitHigh = ped + 350;//(4 * width);
			    TF1 *funcGain = new TF1("funcGain", "expo");
			    if (funcGain) {
				if (cleanup) cleanup->Add(funcGain);
				//funcGain->SetParameter(0, 0);
				//funcGain->SetParLimits(0, 0, h->GetMaximum() * 0.3);
				//funcGain->SetParameter(1, maxBinX);
				//funcGain->SetParLimits(1, TMath::Max(0.0, maxBinX - 5.0), maxBinX + 5);
				funcGain->SetRange(gainFitLow, gainFitHigh);
			        h->Fit(funcGain, "RQN LL");
				slope = - funcGain->GetParameter(1);
			    }
			    LOG_INFO << "Slat " << (strip + 1) << ": ped1 = " << ped1 << ", width1 = " << width1 << ", ped2 = " << ped2 << ", width2 = " << width2 << ", slope=" << slope << endm;

			    if (calibratePed) {
				pedTable.ZdcsmdPedestal[strip] = ped;
				calibratedPedOk = true;
			    }
			    slopes[strip] = slope;
			    if (slope > 0) {
				meanSlope += slope;
				meanSlopes[strip / 16][(strip % 16) / 8] += slope;
				meanSlopesN[strip / 16][(strip % 16) / 8] += 1;
			    }

			    if (pad) {
				pad->cd(strip + 1);
				gPad->SetLogy();
				h->GetXaxis()->SetRangeUser(ped - 10, gainFitHigh + 10);
				h->Draw("H COLZ");
				if (calibratePed) {
				    funcPed->SetLineWidth(1);
				    funcPed->SetLineColor(kBlue);
				    funcPed->Draw("SAME");
				}
				if (calibrateGain) {
				    funcGain->SetLineWidth(1);
				    funcGain->SetLineColor(kRed);
				    funcGain->Draw("SAME");
				}
			    }
 			}
		    }
		    if (calibrateGain) {
			if (meanSlope > 0) {
			    meanSlope /= (32.0 - 2);
			    LOG_INFO << "Mean slope in 32-2 slats = " << meanSlope << endm;
			    for (Int_t strip = 0; strip < 32;strip++) {
				Float_t gain = (slopes[strip] > 0) ? (meanSlope / slopes[strip]) : 0;
				LOG_INFO << "Slat " << (strip + 1) << ": gain = " << gain << endm;
				gainTable.ZdcsmdGain[strip] = gain;
				calibratedGainOk = true;
			    }
			}
		    }
		} else {
		    LOG_ERROR << "ZDC ADC histogram " << zdcsmd_ADCName << " in file " << filenamePlotsIn << " has " << histADC->GetXaxis()->GetNbins() << " horizontal bins, must be 32" << endm;
    		}
	    } else {
		LOG_ERROR << "Cannot find ZDC ADC histogram " << zdcsmd_ADCName << " in file " << filenamePlotsIn << endm;
	    }
	    }
	    if (calibrateTowerPed || calibrateTowerCalib) {
	    TH2F *histADC = (TH2F*)histFile->Get(zdc_ADCName);
	    if (histADC) {
		histADC->SetDirectory(0);
		if (cleanup) cleanup->Add(histADC);
		if (histADC->GetXaxis()->GetNbins() == 8) {
		    LOG_INFO << "Using towers RunID = " << run << ", date = " << date << ", time = " << time << endm;
		    towerCalibTable.RunID = run;
		    Float_t slopes[8] = {0};
		    Float_t meanSlope = 0;
		    TPad *pad = 0;
		    if (canvasOutTower) {
			canvasOutTower->cd();
			pad = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
			if (pad) {
    			    pad->Draw();
			    pad->cd(0);
			    pad->Divide(4, 2, 0.001, 0.001);
			}
		    }
		    for (Int_t tower = 0; tower < 8;tower++) {
			towerCalibTable.ZdcPedestal[tower] = 0;
			towerCalibTable.ZdcCalib[tower] = 1;
        		TH1D *h = histADC->ProjectionY(TString::Format("%s_%i", histADC->GetName(), tower % 4), tower + 1, tower + 1, "e");
	        	if (h) {
			    h->SetDirectory(0);
			    if (cleanup) cleanup->Add(h);
			    TString towerName = (tower % 4) ? TString::Format("Tower %i", tower % 4) : "Sum";
			    h->SetTitle(TString::Format("ZDC %s %s", (tower / 4) ? "West" : "East", towerName.Data()));
    			    Float_t ped1 = h->GetMean(), width1 = h->GetRMS();
    			    Float_t ped2 = 0, width2 = 0;
			    Float_t ped = ped1, width = width1;
			    Float_t slope = 0;
			    Float_t maxBinX = h->GetXaxis()->GetBinCenter(h->GetMaximumBin());
			    TF1 *funcPed = new TF1("funcPed", "gaus");
			    if (funcPed) {
				if (cleanup) cleanup->Add(funcPed);
				funcPed->SetParameter(0, 0);
				funcPed->SetParLimits(0, 0, h->GetMaximum() * 1.1);
				funcPed->SetParameter(1, maxBinX);
				funcPed->SetParLimits(1, TMath::Max(0.0, maxBinX - 5.0), maxBinX + 5.0);
				funcPed->SetParameter(2, 1);
				funcPed->SetParLimits(2, 0, 20);
				funcPed->SetRange(TMath::Max(0.0, maxBinX - 5.0), maxBinX + 5.0);
				h->Fit(funcPed, "RQN");
				ped2 = funcPed->GetParameter(1);
				width2 = funcPed->GetParameter(2);
				ped = ped2;
				width = width2;
			    }
			    LOG_INFO << "Tower " << tower << ": ped1 = " << ped1 << ", width1 = " << width1 << ", ped2 = " << ped2 << ", width2 = " << width2 << endm;
			    TF1 *funcGain = new TF1("funcGain", "expo");
			    Float_t gainFitLow = ped + (4 * width);
			    Float_t gainFitHigh = ped + (10 * width);
			    if (funcGain) {
				if (cleanup) cleanup->Add(funcGain);
				//funcGain->SetParameter(0, 0);
			        //funcGain->SetParLimits(0, 0, h->GetMaximum() * 0.3);
				//funcGain->SetParameter(1, maxBinX);
				//funcGain->SetParLimits(1, TMath::Max(0.0, maxBinX - 5.0), maxBinX + 5);
				funcGain->SetRange(gainFitLow, gainFitHigh);
				h->Fit(funcGain, "RQN");
				slope = funcGain->GetParameter(1);
			    }
			    if (calibratePed) {
				towerCalibTable.ZdcPedestal[tower] = ped;
				calibratedTowerPedOk = true;
			    }
			    slopes[tower] = slope;
			    if (slope > 0) meanSlope += slope;
			    if (pad) {
				pad->cd(tower + 1);
				gPad->SetLogy();
				h->GetXaxis()->SetRangeUser(ped - 40, gainFitHigh + 40);
				h->Draw("H COLZ");
				if (calibrateTowerPed) {
				    funcPed->SetLineWidth(1);
				    funcPed->SetLineColor(kBlue);
				    funcPed->Draw("SAME");
				}
				if (calibrateTowerCalib) {
				    funcGain->SetLineWidth(1);
				    funcGain->SetLineColor(kRed);
				    funcGain->Draw("SAME");
				}
			    }
 			}
		    }
		    if (calibrateTowerCalib) {
			if (meanSlope > 0) {
			    meanSlope /= 32.0;
			    for (Int_t tower = 0;tower < 8;tower++) {
				Float_t gain = (slopes[tower] > 0) ? (meanSlope / slopes[tower]) : 0;
				Float_t calib = (gain > 0) ? (1.0 / gain) : 0;
				LOG_INFO << "Tower " << tower << ": calib = " << calib << endm;
				towerCalibTable.ZdcCalib[tower] = calib;
				calibratedTowerCalibOk = true;
			    }
			}
		    }
		} else {
		    LOG_ERROR << "Tower ADC histogram " << zdc_ADCName << " in file " << filenamePlotsIn << " has " << histADC->GetXaxis()->GetNbins() << " horizontal bins, must be 8" << endm;
    		}
	    } else {
		LOG_ERROR << "Cannot find tower ADC histogram " << zdc_ADCName << " in file " << filenamePlotsIn << endm;
	    }
	    }
	    if (calibrateSlewing) {
		TPad *pad = 0;
		if (canvasOutSlewing) {
		    canvasOutSlewing->cd();
		    pad = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
		    if (pad) {
    			pad->Draw();
			pad->cd(0);
			pad->Divide(4, 2, 0.001, 0.001);
		    }
		}
		for (Int_t eastwest = 0;eastwest < 2;eastwest++)
		for (Int_t tower = 0;tower < 4;tower++) {
		    TString nameStr = TString::Format("%s_%s_%i", zdc_ADCTDCName, eastwest ? "west" : "east", tower);
		    TH2F *histADCTDC = (TH2F*)histFile->Get(nameStr);
		    if (histADCTDC) {
			histADCTDC->SetDirectory(0);
			if (cleanup) cleanup->Add(histADCTDC);
			TObjArray slices;
			//Float_t minADC = 0, maxADC = 300;
			//Float_t minTDC = 0, maxTDC = 300;
			Float_t minADC = 90, maxADC = 150;
			Float_t minTDC = 60, maxTDC = 100;
			TF1 funcSlices("funcSlices", "gaus");
			funcSlices.SetRange(minTDC, maxTDC);
#if ROOT_VERSION_CODE >= ROOT_VERSION(5,20,0)
			histADCTDC->FitSlicesY(&funcSlices, histADCTDC->GetXaxis()->FindBin(minADC), histADCTDC->GetXaxis()->FindBin(maxADC), 0, "RQN", &slices);
#else
			histFitSlicesY(histADCTDC, &funcSlices, histADCTDC->GetXaxis()->FindBin(minADC), histADCTDC->GetXaxis()->FindBin(maxADC), 0, "RQN", &slices);
#endif
			slices.SetOwner(kFALSE);
			for (Int_t i = 0;i < slices.GetEntries();i++) if (cleanup) cleanup->Add(slices.At(i));
			TH1 *histMean = 0;
			if (slices.GetEntries() >= 2) {
			    histMean = static_cast<TH1*>(slices.At(1));
			}
			TF1 *funcSlewing = new TF1("funcSlewing", "pol3");
			if (cleanup && funcSlewing) cleanup->Add(funcSlewing);
    			if (histMean && funcSlewing) {
			    funcSlewing->SetRange(minADC, maxADC);
			    histMean->Fit(funcSlewing, "RQN");
			    Float_t p0 = funcSlewing->GetParameter(0);
			    Float_t p1 = funcSlewing->GetParameter(1);
			    Float_t p2 = funcSlewing->GetParameter(2);
			    Float_t p3 = funcSlewing->GetParameter(3);
			    if (tower == 1) {
				if (eastwest) {
				    slewingTable.WAP0 = p0;
				    slewingTable.WAP1 = p1;
				    slewingTable.WAP2 = p2;
				    slewingTable.WAP3 = p3;
				} else {
				    slewingTable.EAP0 = p0;
				    slewingTable.EAP1 = p1;
				    slewingTable.EAP2 = p2;
				    slewingTable.EAP3 = p3;
				}
				LOG_INFO << "Slewing parameters " << (eastwest ? "west" : "east") << " tower " << tower << ": " << p0 << " " << p1 << " " << p2 << " " << p3 << endm;
				calibratedSlewingOk = true;
			    }
			}
			if (pad) {
			    pad->cd((eastwest * 4) + tower + 1);
			    if (histADCTDC) {
				histADCTDC->GetXaxis()->SetRangeUser(minADC, maxADC);
				histADCTDC->GetYaxis()->SetRangeUser(minTDC, maxTDC);
				histADCTDC->Draw("H COLZ");
			    }
			    if (histMean) {
				histMean->Draw("P ][ SAME");
			    }
			    if (funcSlewing) {
				funcSlewing->SetLineWidth(1);
				funcSlewing->SetLineColor(kBlack);
				funcSlewing->Draw("SAME");
			    }
			}
		    } else {
			LOG_ERROR << "Cannot find ADC-TDC histogram " << nameStr << " in file " << filenamePlotsIn << endm;
		    }
		}
	    }
	    StZDCDb *db = new StZDCDb();
	    if (db) {
		db->setTextDb(zdcPedCalIn);
		db->SetDateTime(date, time);
		zdcsmdPed_st *pedTableDb = db->getZdcSmdPed();
		zdcsmdGain_st *gainTableDb = db->getZdcSmdGain();
		ZdcCalPars_st *slewingTableDb = db->getZdcCalPars();
		zdcCalib_st *towerCalibTableDb = db->getZdcCalib();
		if (pedTableDb && calibratePed && calibratedPedOk) {
		    *pedTableDb = pedTable;
		}
		if (gainTableDb && calibrateGain && calibratedGainOk) {
		    // we do not calibrate these parameters here, so preserve them
		    for (Int_t i = 0;i < 4;i++) gainTable.ZdcsmdCalib[i] = gainTableDb->ZdcsmdCalib[i];
		    *gainTableDb = gainTable;
		}
		if (towerCalibTableDb && ((calibrateTowerPed && calibratedTowerPedOk) || (calibrateTowerCalib && calibratedTowerCalibOk))) {
		    if (!(calibrateTowerPed && calibratedTowerPedOk)) { // we did not calibrate tower peds this time, so preserve the old ones
			for (Int_t i = 0;i < 8;i++) towerCalibTable.ZdcPedestal[i] = towerCalibTableDb->ZdcPedestal[i];
		    }
		    if (!(calibrateTowerCalib && calibratedTowerCalibOk)) { // we did not calibrate tower gains this time, so preserve the old ones
			for (Int_t i = 0;i < 8;i++) towerCalibTable.ZdcCalib[i] = towerCalibTableDb->ZdcCalib[i];
		    }
		    *towerCalibTableDb = towerCalibTable;
		}
		if (slewingTableDb && calibrateSlewing && calibratedSlewingOk) {
		    // we do not calibrate these parameters here, so preserve them
		    slewingTable.VPAR = slewingTableDb->VPAR;
		    slewingTable.OFF = slewingTableDb->OFF;
		    *slewingTableDb = slewingTable;
		}
		if (zdcPedCalOut) db->exportTextDb(zdcPedCalOut);
		UChar_t saveTables = 0;
		if (calibratePed && calibratedPedOk) saveTables |= ZDCSMDPED_TABLE;
		if (calibrateGain && calibratedGainOk) saveTables |= ZDCSMDGAIN_TABLE;
		if ((calibrateTowerPed && calibratedTowerPedOk) || (calibrateTowerCalib && calibratedTowerCalibOk)) saveTables |= ZDCCALIB_TABLE;
		if (calibrateSlewing && calibratedSlewingOk) saveTables |= ZDCCALPARS_TABLE;
		if (dirTablesOut) db->exportROOTDb(dirTablesOut, saveTables);
		delete db;
		db = 0;
	    }
	    histFile->Close();
	} else {
	    LOG_ERROR << "Cannot read file " << filenamePlotsIn << endm;
	}
	if (histFile) delete histFile;
	histFile = 0;
    }
    calibratePed &= calibratedPedOk;
    calibrateGain &= calibratedGainOk;
    calibrateSlewing &= calibratedSlewingOk;
}

