
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TArrow.h>
#include <TLatex.h>
#include <TString.h>
#include <TBox.h>
#include <TObjArray.h>

#include <StMessMgr.h>

#include "ZDCPlotsNames.h"
#include "ZDCPlotsPresenter.h"

const Char_t *ZDCPlotsPresenterTitles[] = {
"ZDC",
"ZDC Towers",
"ZDC Towers different scale",
"ZDC Towers Corrected",
"ZDC E+W ADC Sum vs. BBC E+W ADC Sum",
"ZDC TDC",
"ZDC #DeltaTDC East",
"ZDC #DeltaTDC West",
"ZDC TDC vs. ADC",
"ZDC Front vs. Back Sum",
"ZDC SMD",
"ZDC SMD Occupancy",
"ZDC SMD Max XY",
"ZDC SMD ADC East Vert",
"ZDC SMD ADC East Horiz",
"ZDC SMD ADC West Vert",
"ZDC SMD ADC West Horiz",
"ZDC SMD ADC East Vert Corrected",
"ZDC SMD ADC East Horiz Corrected",
"ZDC SMD ADC West Vert Corrected",
"ZDC SMD ADC West Horiz Corrected",
"ZDC Corrected SMD Sum vs. Tower Sum East",
"ZDC Corrected SMD Sum vs. Tower Sum West",
"ZDC DSM Earliest TDC",
"ZDC DSM ADC > threshold",
"ZDC ADC Sum vs. Pre-Post",
"ZDC Tower ADC Pre-Post Correlation",
"ZDC SMD ADC Pre-Post Correlation"
};

//Float_t minTowerADC = 0;
//Float_t maxTowerADC = 800;

Float_t minTowerADC = 0;
Float_t maxTowerADC = 3900;

Float_t minTowerADCCorr = -50;
Float_t maxTowerADCCorr = minTowerADCCorr + (maxTowerADC - minTowerADC);

Float_t minTowerADCSum = minTowerADC * 3.0;
Float_t maxTowerADCSum = maxTowerADC * 3.0;

Float_t minTowerADCSumCorr = minTowerADCCorr * 3.0;
Float_t maxTowerADCSumCorr = maxTowerADCCorr * 3.0;;

Float_t minTowerTDC = 0;
Float_t maxTowerTDC = 4096;

Float_t minTowerDeltaTDC = -1000;
Float_t maxTowerDeltaTDC = +1000;

Float_t minSMDADC = 0;
Float_t maxSMDADC = 1200;

Float_t minSMDADCCorr = -20;
Float_t maxSMDADCCorr = minSMDADCCorr + (maxSMDADC - minSMDADC);

Float_t minSMDADCSumCorr = minSMDADCCorr * 8.0;
Float_t maxSMDADCSumCorr = maxSMDADCCorr * 8.0;

ClassImp(ZDCPlotsPresenter)

//-------------------------------------------------------------------
TH1 *ZDCPlotsPresenter::GetHisto(FileType &fd, const char *name) const {
    TH1 *hist = fd.file() ? (TH1*)fd.Get(name, 0) : 0;
    if (hist) {
	hist->SetDirectory(0);
	if (this->mCleanup) this->mCleanup->Add(hist);
    }
    return hist;
}

//-------------------------------------------------------------------
ZDCPlotsPresenter::ZDCPlotsPresenter()
    : mCleanup(0)
{
    this->mCleanup = new TObjArray();
    if (this->mCleanup) {
	this->mCleanup->SetOwner(kTRUE);
    }
}

//-------------------------------------------------------------------
ZDCPlotsPresenter::~ZDCPlotsPresenter() {
    this->clear();
    if (this->mCleanup) delete this->mCleanup;
    this->mCleanup = 0;
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::clear() const {
    if (this->mCleanup) {
	/*TIter next(this->mCleanup);
	TObject *object;
	while ((object = next())) {
	    cout << "deleting " << object << " " << object->GetName() << " " << object->GetTitle() << endl;
	    delete object;
	}
	this->mCleanup->SetOwner(kFALSE);
	this->mCleanup->Clear();*/
 	//this->mCleanup->ls("AAAAAA*"); // strange, but works: without this ls, the following Delete generates a seg. voilation!
	this->mCleanup->Delete();
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDC(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    Bool_t corrected = false;

    FIND_HISTO(TH1F, h76_zdc_time_east, file, h76_zdc_time_eastName, isEmpty);
    FIND_HISTO(TH1F, h77_zdc_time_west, file, h77_zdc_time_westName, isEmpty);
    FIND_HISTO(TH1F, h78_zdc_timediff_east_west, file, h78_zdc_timediff_east_westName, isEmpty);
    FIND_HISTO(TH1F, h146_zdc_Vertex_cm, file, h146_zdc_Vertex_cmName, isEmpty);
    FIND_HISTO(TH2F, zdc_ADC, file, corrected ? zdc_ADCCorrName : zdc_ADCName, isEmpty);
    
    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 3, 0.001, 0.001);

    c->cd(1);
    if (h76_zdc_time_east) {
        h76_zdc_time_east->GetXaxis()->SetRangeUser(minTowerTDC, maxTowerTDC);
        h76_zdc_time_east->Draw("H COLZ");
    }
    c->cd(2);
    if (h77_zdc_time_west) {
        h77_zdc_time_west->GetXaxis()->SetRangeUser(minTowerTDC, maxTowerTDC);
        h77_zdc_time_west->Draw("H COLZ");
    }
    c->cd(3);
    if (h78_zdc_timediff_east_west) {
        h78_zdc_timediff_east_west->Draw("H COLZ");
    }
    c->cd(4);
    if (h146_zdc_Vertex_cm) {
        h146_zdc_Vertex_cm->Draw("H COLZ");
    }
    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
            c->cd(4 + eastwest + 1);
	    Int_t tower = 0;
            Int_t itower = (eastwest * 4) + tower;
            TH1D *h = zdc_ADC ? zdc_ADC->ProjectionY(TString::Format("%s_%s_%i_general", zdc_ADC->GetName(), eastwest ? "west" : "east", tower), itower + 1, itower + 1) : 0;
            if (h) {
                h->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(h);
		h->Rebin(16);//CJ
		h->GetXaxis()->SetRangeUser(minTowerADC, maxTowerADC);
		TString towerName = tower ? TString::Format("Tower %i", tower) : "Sum";
                h->SetTitle(TString::Format("ZDC %s %s ADC, %s", eastwest ? "West" : "East", corrected ? "Corrected" : "", towerName.Data()));
		h->Draw("H COLZ");
        	gPad->SetLogy();
	    }
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCTower(Bool_t corrected, FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    FIND_HISTO(TH2F, zdc_ADC, file, corrected ? zdc_ADCCorrName : zdc_ADCName, isEmpty);

    TPad* c = new TPad("pad2", "pad2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(4, 2, 0.001, 0.001);

    if (zdc_ADC) {
        for (Int_t eastwest = 0;eastwest < 2;eastwest++)
        for (Int_t tower = 0;tower < 4;tower++) {
            c->cd((eastwest * 4) + tower + 1);
            Int_t itower = (eastwest * 4) + tower;
            TH1D *h = zdc_ADC->ProjectionY(TString::Format("%s_%s_%i", zdc_ADC->GetName(), eastwest ? "west" : "east", tower), itower + 1, itower + 1);
            if (h) {
                h->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(h);
		TString towerName = tower ? TString::Format("Tower %i", tower) : "Sum";
                h->SetTitle(TString::Format("ZDC %s %s ADC, %s", eastwest ? "West" : "East", corrected ? "Corrected" : "", towerName.Data()));
		h->Rebin(16);
		if (tower == 0) {
		    h->GetXaxis()->SetRangeUser(corrected ? minTowerADCSumCorr : minTowerADCSum, corrected ? maxTowerADCSumCorr : maxTowerADCSum);
		} else {
		    h->GetXaxis()->SetRangeUser(corrected ? minTowerADCCorr : minTowerADC, corrected ? maxTowerADCCorr : maxTowerADC);
		}
		h->Draw("H COLZ");
	    }
            gPad->SetLogy();
        }
    }
}


//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCTower_scale(Bool_t corrected, FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    FIND_HISTO(TH2F, zdc_ADC, file, corrected ? zdc_ADCCorrName : zdc_ADCName, isEmpty);

    TPad* c = new TPad("pad2", "pad2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(4, 2, 0.001, 0.001);

    if (zdc_ADC) {
        for (Int_t eastwest = 0;eastwest < 2;eastwest++)
        for (Int_t tower = 0;tower < 4;tower++) {
            c->cd((eastwest * 4) + tower + 1);
            Int_t itower = (eastwest * 4) + tower;
            TH1D *h = zdc_ADC->ProjectionY(TString::Format("%s_%s_%i", zdc_ADC->GetName(), eastwest ? "west" : "east", tower), itower + 1, itower + 1);
            if (h) {
                h->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(h);
		TString towerName = tower ? TString::Format("Tower %i", tower) : "Sum";
                h->SetTitle(TString::Format("ZDC %s %s ADC, %s", eastwest ? "West" : "East", corrected ? "Corrected" : "", towerName.Data()));
		if (tower == 0) {
		  //h->GetXaxis()->SetRangeUser(corrected ? minTowerADCSumCorr : minTowerADCSum, corrected ? maxTowerADCSumCorr : maxTowerADCSum);
		  h->GetXaxis()->SetRangeUser(corrected ? -50. : 0., corrected ? 750. : 800.);
		} else {
		  //h->GetXaxis()->SetRangeUser(corrected ? minTowerADCCorr : minTowerADC, corrected ? maxTowerADCCorr : maxTowerADC);
		  h->GetXaxis()->SetRangeUser(corrected ? -50. : 0., corrected ? 750. : 800.);
		}
                h->Draw("H COLZ");
	    }
            gPad->SetLogy();
        }
    }
}


//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCEWadcSum(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

 
    FIND_HISTO(TH1F, hADC_EastWestSum, file, hADC_EastWestSumName, isEmpty);
    FIND_HISTO(TH2F, hADC_EWSumZDCvsBBC, file, hADC_EWSumZDCvsBBCName, isEmpty);
    FIND_HISTO(TH2F, hADC_EWSumZDCvsTOFMult, file, hADC_EWSumZDCvsTOFMultName, isEmpty);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(3, 1, 0.001, 0.001);

    c->cd(1);
    if (hADC_EastWestSum) {
      //hADC_EastWestSum->GetXaxis()->SetRangeUser(0, 3000);
      hADC_EastWestSum->Draw("H COLZ");
      gPad->SetLogy();
    }
    c->cd(2);
    if (hADC_EWSumZDCvsBBC) {
      //hADC_EWSumZDCvsBBC->GetXaxis()->SetRangeUser(minTowerTDC, maxTowerTDC);
      hADC_EWSumZDCvsBBC->Draw("H COLZ");
    }
//yhzhu added!
    c->cd(3);
    if (hADC_EWSumZDCvsTOFMult) {
      //hADC_EWSumZDCvsBBC->GetXaxis()->SetRangeUser(minTowerTDC, maxTowerTDC);
      hADC_EWSumZDCvsTOFMult->Draw("H COLZ");
    }

}

    
//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCTDC(FileType file, TPad *pad, Bool_t *isEmpty) const {
  if (isEmpty) *isEmpty = true;
  if (!pad) return;
  pad->Clear();
  pad->cd(0);

  FIND_HISTO(TH2F, zdc_TDC, file, zdc_TDCName, isEmpty);

  TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
  c->Draw();
  c->cd(0);
  c->Divide(4, 2, 0.001, 0.001);

  if (zdc_TDC) {
    for (Int_t eastwest = 0;eastwest < 2;eastwest++)
      for (Int_t tower = 0;tower < 4;tower++) {
	c->cd((eastwest * 4) + tower + 1);
	Int_t itower = (eastwest * 4) + tower;
	TH1D *h = zdc_TDC->ProjectionY(TString::Format("%s_%s_%i", zdc_TDC->GetName(), eastwest ? "west" : "east", tower), itower + 1, itower + 1);
	if (h) {
	  h->SetDirectory(0);
	  if (this->mCleanup) this->mCleanup->Add(h);
	  TString towerName = tower ? TString::Format("Tower %i", tower) : "Sum";
	  h->SetTitle(TString::Format("ZDC %s TDC, %s", eastwest ? "West" : "East", towerName.Data()));
	  h->GetXaxis()->SetRangeUser(minTowerTDC, maxTowerTDC);
	  h->Draw("H COLZ");
	}
	gPad->SetLogy();
      }
  }
}


//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCTDCdiff(StBeamDirection eastwest, FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    FIND_HISTO(TH2F, zdc_TDCdiff, file, zdc_TDCdiffName, isEmpty);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(3, 2, 0.001, 0.001);

    if (zdc_TDCdiff) {
        for (Int_t dtower = 0;dtower < 6;dtower++) {
            c->cd(dtower + 1);
            Int_t itower = (eastwest * 6) + dtower;
            TH1D *h = zdc_TDCdiff->ProjectionY(TString::Format("%s_%s_%i", zdc_TDCdiff->GetName(), eastwest ? "west" : "east", dtower), itower + 1, itower + 1);
            if (h) {
                h->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(h);
		TString towerName;
		if (dtower == 0) {
		    towerName = "Sum - Tower1";
		} else if (dtower == 1) {
		    towerName = "Sum - Tower2";
		} else if (dtower == 2) {
		    towerName = "Sum - Tower3";
		} else if (dtower == 3) {
		    towerName = "Tower1 - Tower2";
		} else if (dtower == 4) {
		    towerName = "Tower1 - Tower3";
		} else if (dtower == 5) {
		    towerName = "Tower2 - Tower3";
		}
                h->SetTitle(TString::Format("ZDC %s #DeltaTDC, %s", eastwest ? "West" : "East", towerName.Data()));
		h->GetXaxis()->SetRangeUser(minTowerDeltaTDC, maxTowerDeltaTDC);
                h->Draw("H COLZ");
	    }
            gPad->SetLogy();
        }
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCADCTDC(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(4, 2, 0.001, 0.001);

    for (int eastwest = 0;eastwest < 2;eastwest++)
    for (int tower = 0;tower < 4;tower++) {
	c->cd((eastwest * 4) + tower + 1);
	FIND_HISTO(TH2F, zdc_ADCTDC, file, TString::Format("%s_%s_%i", zdc_ADCTDCName, eastwest ? "west" : "east", tower), isEmpty);
	if (zdc_ADCTDC) {
	    zdc_ADCTDC->GetXaxis()->SetRangeUser(minTowerADC, maxTowerADC);
	    zdc_ADCTDC->GetYaxis()->SetRangeUser(minTowerTDC, maxTowerTDC);
	    zdc_ADCTDC->Draw("H COLZ");
	}
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCFrontBackSum(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 1, 0.001, 0.001);

    for (int eastwest = 0;eastwest < 2;eastwest++) {
	FIND_HISTO(TH2F, zdc_FrontBackSum, file, TString::Format("%s_%s", zdc_FrontBackSumName, eastwest ? "west" : "east"), isEmpty);
	c->cd((eastwest * 1) + 1);
	if (zdc_FrontBackSum) {
	    zdc_FrontBackSum->GetXaxis()->SetRangeUser(minTowerADCSum, maxTowerADCSum);
	    zdc_FrontBackSum->GetYaxis()->SetRangeUser(minTowerADCSum, maxTowerADCSum);
	    zdc_FrontBackSum->Draw("H COLZ");
	    gPad->SetLogz();
	}
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCSMD(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 4, 0.001, 0.001);

    for (int eastwest = 0;eastwest < 2;eastwest++)
    for (int horiz = 0;horiz < 2;horiz++) {
	c->cd((horiz * 2) + eastwest + 1);
        FIND_HISTO(TH1F, zdcsmd_N, file, TString::Format(zdcsmd_NName "_%s_%s", eastwest ? "west" : "east", horiz ? "horiz" : "vert"), isEmpty);
	if (zdcsmd_N) {
    	    zdcsmd_N->Draw("H COLZ");
	}
    }

    for (int eastwest = 0;eastwest < 2;eastwest++)
    for (int horiz = 0;horiz < 2;horiz++) {
	c->cd((horiz * 2) + eastwest + 1 + 4);
        FIND_HISTO(TH1F, zdcsmd_A, file, TString::Format(zdcsmd_AName "_%s_%s", eastwest ? "west" : "east", horiz ? "horiz" : "vert"), isEmpty);
	if (zdcsmd_A) {
    	    zdcsmd_A->Draw("H COLZ");
	}
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCSmdSumTowerSum(StBeamDirection eastwest, FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 2, 0.001, 0.001);

    for (int horiz = 0;horiz < 2;horiz++) {
        FIND_HISTO(TH2F, zdcsmd_SmdSumTowerSum, file, TString::Format(zdcsmd_SmdSumTowerSumName "_%s_%s", (eastwest == west) ? "west" : "east", horiz ? "horiz" : "vert"), isEmpty);
        FIND_HISTO(TH1F, zdcsmd_SmdSumTowerSumRatio, file, TString::Format(zdcsmd_SmdSumTowerSumRatioName "_%s_%s", (eastwest == west) ? "west" : "east", horiz ? "horiz" : "vert"), isEmpty);
	c->cd(horiz + 1);
	if (zdcsmd_SmdSumTowerSum) {
    	    zdcsmd_SmdSumTowerSum->GetXaxis()->SetRangeUser(minTowerADCSumCorr, maxTowerADCSumCorr);
    	    zdcsmd_SmdSumTowerSum->GetYaxis()->SetRangeUser(minSMDADCSumCorr, maxSMDADCSumCorr);
    	    zdcsmd_SmdSumTowerSum->Draw("H COLZ");
	    gPad->SetLogz();
	}
	c->cd(horiz + 1 + 2);
	if (zdcsmd_SmdSumTowerSumRatio) {
    	    zdcsmd_SmdSumTowerSumRatio->Draw("H COLZ");
	}
    }
}

//-------------------------------------------------------------------
void drawZDCSMDLines(TH1 *h, TObjArray *cleanup) {
    if (h) {
	Int_t cl = 16;
	TLine *l = 0;
	TLatex *t = 0;
	Float_t minY = h->InheritsFrom("TH2") ? h->GetYaxis()->GetBinLowEdge(h->GetYaxis()->GetFirst()) : h->GetMinimum();
	Float_t maxY = h->InheritsFrom("TH2") ? h->GetYaxis()->GetBinUpEdge(h->GetYaxis()->GetLast()) : h->GetMaximum();
	if ((minY == 0) && (maxY == 0)) maxY = 1.0;
	for (int eastwest = 0;eastwest < 2;eastwest++)
	for (int horiz = 0;horiz < 2;horiz++) {
	    if (!((eastwest == 0) && (horiz == 0))) {
		l = new TLine(h->GetXaxis()->GetBinUpEdge((eastwest * 16) + (horiz * 8)), minY, 
		    h->GetXaxis()->GetBinUpEdge((eastwest * 16) + (horiz * 8)), maxY);
	    }
	    if (l) {
		if (cleanup) cleanup->Add(l);
		l->SetLineColor(16);
    		l->SetLineWidth(1);
    		l->Draw();
	    }
	    t = new TLatex(h->GetXaxis()->GetBinLowEdge((eastwest * 16) + (horiz * 8) + 1) + 3, 0.8 * maxY,
		TString::Format("#splitline{%s}{%s}", eastwest ? "West" : "East", horiz ? "Horiz" : "Vert"));
	    if (t) {
		if (cleanup) cleanup->Add(t);
		t->SetTextColor(cl);
		t->SetTextSize(0.05);
	        t->Draw();
	    }
	}
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCSMDADCOccupancy(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    FIND_HISTO(TH2F, zdcsmd_ADC, file, zdcsmd_ADCName, isEmpty);
    FIND_HISTO(TH1F, zdcsmd_Occupancy, file, zdcsmd_OccupancyName, isEmpty);
    FIND_HISTO(TH2F, zdcsmd_ADCCorr, file, zdcsmd_ADCCorrName, isEmpty);
    FIND_HISTO(TH1F, zdcsmd_OccupancyCorr, file, zdcsmd_OccupancyCorrName, isEmpty);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 2, 0.001, 0.001);

    c->cd(1);
    if (zdcsmd_ADC) {
	zdcsmd_ADC->GetYaxis()->SetRangeUser(minSMDADC, maxSMDADC);
	zdcsmd_ADC->Draw("H COLZ");
	gPad->SetLogz();
	drawZDCSMDLines(zdcsmd_ADC, this->mCleanup);
    }

    c->cd(2);
    if (zdcsmd_ADCCorr) {
	zdcsmd_ADCCorr->GetYaxis()->SetRangeUser(minSMDADCCorr, maxSMDADCCorr);
	zdcsmd_ADCCorr->Draw("H COLZ");
	gPad->SetLogz();
	drawZDCSMDLines(zdcsmd_ADCCorr, this->mCleanup);
    }

    c->cd(3);
    if (zdcsmd_Occupancy) {
	zdcsmd_Occupancy->GetYaxis()->UnZoom();
	zdcsmd_Occupancy->GetYaxis()->SetRangeUser(0, zdcsmd_Occupancy->GetMaximum() * 1.19);
	zdcsmd_Occupancy->Draw("H COLZ");
	drawZDCSMDLines(zdcsmd_Occupancy, this->mCleanup);
    }

    c->cd(4);
    if (zdcsmd_OccupancyCorr) {
	zdcsmd_OccupancyCorr->GetYaxis()->UnZoom();
	zdcsmd_OccupancyCorr->GetYaxis()->SetRangeUser(0, zdcsmd_OccupancyCorr->GetMaximum() * 1.19);
	zdcsmd_OccupancyCorr->Draw("H COLZ");
	drawZDCSMDLines(zdcsmd_OccupancyCorr, this->mCleanup);
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCSMDMaxXY(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 2, 0.001, 0.001);

    for (int eastwest = 0;eastwest < 2;eastwest++) {
	FIND_HISTO(TH2F, zdcsmd_MaxXYCorr, file, TString::Format(zdcsmd_MaxXYCorrName "_%s", eastwest ? "west" : "east"), isEmpty);
	FIND_HISTO(TH1F, zdcsmd_MaxXYCorrRatio, file, TString::Format(zdcsmd_MaxXYCorrRatioName "_%s", eastwest ? "west" : "east"), isEmpty);
	c->cd(eastwest + 1);
	if (zdcsmd_MaxXYCorr) {
	    zdcsmd_MaxXYCorr->GetXaxis()->SetRangeUser(minSMDADCCorr, maxSMDADCCorr);
	    zdcsmd_MaxXYCorr->GetYaxis()->SetRangeUser(minSMDADCCorr, maxSMDADCCorr);
	    zdcsmd_MaxXYCorr->Draw("H COLZ");
	    gPad->SetLogz();
	}
	c->cd(eastwest + 1 + 2);
	if (zdcsmd_MaxXYCorrRatio) {
	    zdcsmd_MaxXYCorrRatio->Draw("H COLZ");
	}
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCSMDADC(StBeamDirection eastwest, int horiz, bool corrected, FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    FIND_HISTO(TH2F, zdcsmd_ADC, file, corrected ? zdcsmd_ADCCorrName : zdcsmd_ADCName, isEmpty);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(4, 2, 0.001, 0.001);

    if (zdcsmd_ADC) {
        for (Int_t strip = 0;strip < 8;strip++) {
            c->cd(strip + 1);
            Int_t istrip = ((eastwest == east ? 0 : 1) * 16) + (horiz * 8) + strip;
            TH1D *h = zdcsmd_ADC->ProjectionY(TString::Format("%s_%i", zdcsmd_ADC->GetName(), strip + 1), istrip + 1, istrip + 1);
            if (h) {
                h->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(h);
                h->GetXaxis()->SetRangeUser(corrected ? minSMDADCCorr : minSMDADC, corrected ? maxSMDADCCorr : maxSMDADC);
                h->SetTitle(TString::Format("ZDC SMD %s %s %s ADC, strip %i", (eastwest == west) ? "West" : "East", horiz ? "Horiz" : "Vert", corrected ? "Corrected" : "", strip + 1));
                h->Draw("H COLZ");
	    }
            gPad->SetLogy();
        }
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCDSMearliestTDC(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    FIND_HISTO(TH2F, zdcdsm_earliestTDC, file, zdcdsm_earliestTDCName, isEmpty);
    FIND_HISTO(TH2F, zdcdsm_TDCdiff, file, zdcdsm_TDCdiffName, isEmpty);
    FIND_HISTO(TH2F, zdcdsm_TDCdiffL1L2, file, zdcdsm_TDCdiffL1L2Name, isEmpty);
    FIND_HISTO(TH2F, zdcdsm_TDCdiffL3, file, zdcdsm_TDCdiffL3Name, isEmpty);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(4, 2, 0.001, 0.001);

    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	FIND_HISTO(TH2F, zdcdsm_earliestTDCMaxTDC, file, TString::Format("%s_%s", zdcdsm_earliestTDCName, eastwest ? "west" : "east"), isEmpty);
	if (zdcdsm_earliestTDC) {
            c->cd((eastwest * 4) + 0 + 1);
            TH1D *h = zdcdsm_earliestTDC->ProjectionY(TString::Format("%s_%s", zdcdsm_earliestTDC->GetName(), eastwest ? "west" : "east"), eastwest + 1, eastwest + 1);
            if (h) {
                h->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(h);
                h->GetXaxis()->SetRangeUser(minTowerTDC, maxTowerTDC);
                h->SetTitle(TString::Format("ZDC DSM Earliest TDC, %s", eastwest ? "West" : "East"));
                h->Draw("H COLZ");
	    }
            gPad->SetLogy();
	}
	if (zdcdsm_earliestTDCMaxTDC) {
            c->cd((eastwest * 4) + 1 + 1);
	    zdcdsm_earliestTDCMaxTDC->Draw("H COLZ");
            gPad->SetLogz();
	}
    }
    if (zdcdsm_TDCdiff) {
	c->cd((0 * 4) + 2 + 1);
	zdcdsm_TDCdiff->Draw("H COLZ");
        gPad->SetLogz();
    }
    if (zdcdsm_TDCdiffL1L2) {
	c->cd((1 * 4) + 2 + 1);
	zdcdsm_TDCdiffL1L2->Draw("H COLZ");
        gPad->SetLogz();
    }
    if (zdcdsm_TDCdiffL3) {
	c->cd((0 * 4) + 3 + 1);
    	TH1D *hInWindow = zdcdsm_TDCdiffL3->ProjectionX(TString::Format("%s_%s", zdcdsm_TDCdiffL3->GetName(), "in_window"), 2, 2);
        if (hInWindow) {
            hInWindow->SetDirectory(0);
	    if (this->mCleanup) this->mCleanup->Add(hInWindow);
            hInWindow->SetStats(0);
            hInWindow->SetLineColor(kBlue);
            hInWindow->GetXaxis()->SetRangeUser(minTowerDeltaTDC, maxTowerDeltaTDC);
            hInWindow->SetTitle("LD301 Input, TAC diff in window ? blue : red");
            hInWindow->Draw("H COLZ");
	}
    	TH1D *hOutWindow = zdcdsm_TDCdiffL3->ProjectionX(TString::Format("%s_%s", zdcdsm_TDCdiffL3->GetName(), "out_window"), 1, 1);
        if (hOutWindow) {
            hOutWindow->SetDirectory(0);
	    if (this->mCleanup) this->mCleanup->Add(hOutWindow);
            hOutWindow->SetStats(0);
            hOutWindow->SetLineColor(kRed);
            hOutWindow->GetXaxis()->SetRangeUser(minTowerDeltaTDC, maxTowerDeltaTDC);
            hOutWindow->SetTitle("LD301 Input, TAC diff in window ? blue : red");
            hOutWindow->Draw("H COLZ SAME");
	}
        gPad->SetLogy();
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCDSMADCthresh(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    FIND_HISTO(TH2F, zdcdsm_ADCthresh, file, zdcdsm_ADCthreshName, isEmpty);
    FIND_HISTO(TH2F, zdcdsm_ADCthreshL2, file, zdcdsm_ADCthreshL2Name, isEmpty);
    FIND_HISTO(TH2F, zdcdsm_ADCthreshL3, file, zdcdsm_ADCthreshL3Name, isEmpty);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(5, 2, 0.001, 0.001);

    if (zdcdsm_ADCthresh) {
	for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	    c->cd((eastwest * 5) + 0 + 1);
    	    TH1D *hSumAbove = zdcdsm_ADCthresh->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthresh->GetName(), (eastwest == west) ? "west" : "east", "sum_above"), (eastwest * 6) + (0 * 2) + 0 + 1, (eastwest * 6) + (0 * 2) + 0 + 1);
            if (hSumAbove) {
                hSumAbove->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hSumAbove);
                hSumAbove->SetStats(0);
                hSumAbove->SetLineColor(kBlue);
                hSumAbove->GetXaxis()->SetRangeUser(minTowerADCSum, maxTowerADCSum);
                hSumAbove->SetTitle(TString::Format("ZD101 Input, Sum > threshold ? blue : red, %s;ADC Sum", (eastwest == west) ? "West" : "East"));
                hSumAbove->Draw("H COLZ");
	    }
    	    TH1D *hSumBelow = zdcdsm_ADCthresh->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthresh->GetName(), (eastwest == west) ? "west" : "east", "sum_below"), (eastwest * 6) + (0 * 2) + 1 + 1, (eastwest * 6) + (0 * 2) + 1 + 1);
            if (hSumBelow) {
                hSumBelow->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hSumBelow);
                hSumBelow->SetStats(0);
                hSumBelow->SetLineColor(kRed);
                hSumBelow->GetXaxis()->SetRangeUser(minTowerADCSum, maxTowerADCSum);
                hSumBelow->SetTitle(TString::Format("ZD101 Input, Sum > threshold ? blue : red, %s;ADC Sum", (eastwest == west) ? "West" : "East"));
		if (hSumAbove) {
            	    hSumBelow->Draw("H COLZ SAME");
		    hSumAbove->GetYaxis()->SetRangeUser(0.5/*TMath::Min(hSumAbove->GetMinimum(), hSumBelow->GetMinimum())*/, 0.5 + 2 * TMath::Max(hSumAbove->GetMaximum(), hSumBelow->GetMaximum()));
		} else {
            	    hSumBelow->Draw("H COLZ");
		}
	    }
    	    gPad->SetLogy();

	    c->cd((eastwest * 5) + 1 + 1);
    	    TH1D *hFrontAbove = zdcdsm_ADCthresh->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthresh->GetName(), (eastwest == west) ? "west" : "east", "front_above"), (eastwest * 6) + (1 * 2) + 0 + 1, (eastwest * 6) + (1 * 2) + 0 + 1);
            if (hFrontAbove) {
                hFrontAbove->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hFrontAbove);
                hFrontAbove->SetStats(0);
                hFrontAbove->SetLineColor(kBlue);
                hFrontAbove->GetXaxis()->SetRangeUser(minTowerADC, maxTowerADC);
                hFrontAbove->SetTitle(TString::Format("ZD101 Input, Front > threshold ? blue : red, %s;Front ADC Sum", (eastwest == west) ? "West" : "East"));
                hFrontAbove->Draw("H COLZ");
	    }
    	    TH1D *hFrontBelow = zdcdsm_ADCthresh->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthresh->GetName(), (eastwest == west) ? "west" : "east", "front_below"), (eastwest * 6) + (1 * 2) + 1 + 1, (eastwest * 6) + (1 * 2) + 1 + 1);
            if (hFrontBelow) {
                hFrontBelow->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hFrontBelow);
                hFrontBelow->SetLineColor(kRed);
                hFrontBelow->GetXaxis()->SetRangeUser(minTowerADC, maxTowerADC);
                hFrontBelow->SetTitle(TString::Format("ZD101 Input, Front > threshold ? blue : red, %s;Front ADC Sum", (eastwest == west) ? "West" : "East"));
                hFrontBelow->Draw("H COLZ SAME");
		if (hFrontAbove) {
            	    hFrontBelow->Draw("H COLZ SAME");
		    hFrontAbove->GetYaxis()->SetRangeUser(0.5/*TMath::Min(hFrontAbove->GetMinimum(), hFrontBelow->GetMinimum())*/, 0.5 + 2 * TMath::Max(hFrontAbove->GetMaximum(), hFrontBelow->GetMaximum()));
		} else {
            	    hFrontBelow->Draw("H COLZ");
		}
	    }
    	    gPad->SetLogy();

	    c->cd((eastwest * 5) + 2 + 1);
    	    TH1D *hBackAbove = zdcdsm_ADCthresh->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthresh->GetName(), (eastwest == west) ? "west" : "east", "back_above"), (eastwest * 6) + (2 * 2) + 0 + 1, (eastwest * 6) + (2 * 2) + 0 + 1);
            if (hBackAbove) {
                hBackAbove->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hBackAbove);
                hBackAbove->SetStats(0);
                hBackAbove->SetLineColor(kBlue);
                hBackAbove->GetXaxis()->SetRangeUser(minTowerADC, maxTowerADC);
                hBackAbove->SetTitle(TString::Format("ZD101 Input, Back > threshold ? blue : red, %s;Back ADC Sum", (eastwest == west) ? "West" : "East"));
                hBackAbove->Draw("H COLZ");
	    }
    	    TH1D *hBackBelow = zdcdsm_ADCthresh->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthresh->GetName(), (eastwest == west) ? "west" : "east", "back_below"), (eastwest * 6) + (2 * 2) + 1 + 1, (eastwest * 6) + (2 * 2) + 1 + 1);
            if (hBackBelow) {
                hBackBelow->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hBackBelow);
                hBackBelow->SetStats(0);
                hBackBelow->SetLineColor(kRed);
                hBackBelow->GetXaxis()->SetRangeUser(minTowerADC, maxTowerADC);
                hBackBelow->SetTitle(TString::Format("ZD101 Input, Back > threshold ? blue : red, %s;Back ADC Sum", (eastwest == west) ? "West" : "East"));
                hBackBelow->Draw("H COLZ SAME");
		if (hBackAbove) {
            	    hBackBelow->Draw("H COLZ SAME");
		    hBackAbove->GetYaxis()->SetRangeUser(0.5/*TMath::Min(hBackAbove->GetMinimum(), hBackBelow->GetMinimum())*/, 0.5 + 2 * TMath::Max(hBackAbove->GetMaximum(), hBackBelow->GetMaximum()));
		} else {
            	    hBackBelow->Draw("H COLZ");
		}
	    }
    	    gPad->SetLogy();

	    c->cd((eastwest * 5) + 3 + 1);
    	    TH1D *hL2AboveRight = zdcdsm_ADCthreshL2->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthreshL2->GetName(), (eastwest == west) ? "west" : "east", "L1_eq_L2"), eastwest + 1, eastwest + 1);
            if (hL2AboveRight) {
                hL2AboveRight->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hL2AboveRight);
                hL2AboveRight->SetStats(0);
                hL2AboveRight->SetBinContent(1, 0);
                hL2AboveRight->SetBinError(1, 0);
                hL2AboveRight->SetBinContent(3, 0);
                hL2AboveRight->SetBinError(3, 0);
                hL2AboveRight->SetBinContent(5, 0);
                hL2AboveRight->SetBinError(5, 0);
                hL2AboveRight->SetTitle(TString::Format("ADC > threshold ZD101==VT201?, %s;0-1 = Sum, 2-3 = Front, 3-4 = Back", (eastwest == west) ? "West" : "East"));
                hL2AboveRight->Draw("H COLZ");
	    }
    	    TH1D *hL2AboveWrong = zdcdsm_ADCthreshL2->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthreshL2->GetName(), (eastwest == west) ? "west" : "east", "L1_neq_L2"), eastwest + 1, eastwest + 1);
            if (hL2AboveWrong) {
                hL2AboveWrong->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hL2AboveWrong);
                hL2AboveWrong->SetStats(0);
                hL2AboveWrong->SetBinContent(2, 0);
                hL2AboveWrong->SetBinError(2, 0);
                hL2AboveWrong->SetBinContent(4, 0);
                hL2AboveWrong->SetBinError(4, 0);
                hL2AboveWrong->SetBinContent(6, 0);
                hL2AboveWrong->SetBinError(6, 0);
                hL2AboveWrong->SetLineColor(kRed);
                hL2AboveWrong->SetFillColor(kRed);
                hL2AboveWrong->SetTitle(TString::Format("ADC > threshold ZD101==VT201?, %s;0-1 = Sum, 2-3 = Front, 3-4 = Back", (eastwest == west) ? "West" : "East"));
		if (hL2AboveRight) {
            	    hL2AboveWrong->Draw("H COLZ SAME");
		    hL2AboveRight->GetYaxis()->SetRangeUser(0.5/*TMath::Min(hL2AboveRight->GetMinimum(), hL2AboveWrong->GetMinimum())*/, 0.5 + 2 * TMath::Max(hL2AboveRight->GetMaximum(), hL2AboveWrong->GetMaximum()));
		} else {
            	    hL2AboveWrong->Draw("H COLZ");
		}
	    }
    	    gPad->SetLogy();

	    c->cd((eastwest * 5) + 4 + 1);
    	    TH1D *hL3AboveRight = zdcdsm_ADCthreshL3->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthreshL3->GetName(), (eastwest == west) ? "west" : "east", "L2_eq_L3"), eastwest + 1, eastwest + 1);
            if (hL3AboveRight) {
                hL3AboveRight->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hL3AboveRight);
                hL3AboveRight->SetStats(0);
                hL3AboveRight->SetBinContent(1, 0);
                hL3AboveRight->SetBinError(1, 0);
                hL3AboveRight->SetBinContent(3, 0);
                hL3AboveRight->SetBinError(3, 0);
                hL3AboveRight->SetBinContent(5, 0);
                hL3AboveRight->SetBinError(5, 0);
                hL3AboveRight->SetTitle(TString::Format("ADC > threshold VT201==LD301?, %s;0-1 = Sum, 2-3 = Front, 3-4 = Back", (eastwest == west) ? "West" : "East"));
                hL3AboveRight->Draw("H COLZ");
	    }
    	    TH1D *hL3AboveWrong = zdcdsm_ADCthreshL3->ProjectionY(TString::Format("%s_%s_%s", zdcdsm_ADCthreshL3->GetName(), (eastwest == west) ? "west" : "east", "L2_neq_L3"), eastwest + 1, eastwest + 1);
            if (hL3AboveWrong) {
                hL3AboveWrong->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(hL3AboveWrong);
                hL3AboveWrong->SetStats(0);
                hL3AboveWrong->SetBinContent(2, 0);
                hL3AboveWrong->SetBinError(2, 0);
                hL3AboveWrong->SetBinContent(4, 0);
                hL3AboveWrong->SetBinError(4, 0);
                hL3AboveWrong->SetBinContent(6, 0);
                hL3AboveWrong->SetBinError(6, 0);
                hL3AboveWrong->SetLineColor(kRed);
                hL3AboveWrong->SetFillColor(kRed);
                hL3AboveWrong->SetTitle(TString::Format("ADC > threshold VT201==LD301?, %s;0-1 = Sum, 2-3 = Front, 3-4 = Back", (eastwest == west) ? "West" : "East"));
		if (hL3AboveRight) {
            	    hL3AboveWrong->Draw("H COLZ SAME");
		    hL3AboveRight->GetYaxis()->SetRangeUser(0.5/*TMath::Min(hL3AboveRight->GetMinimum(), hL3AboveWrong->GetMinimum())*/, 0.5 + 2 * TMath::Max(hL3AboveRight->GetMaximum(), hL3AboveWrong->GetMaximum()));
		} else {
            	    hL3AboveWrong->Draw("H COLZ");
		}
	    }
    	    gPad->SetLogy();
        }
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCADCprepost(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    FIND_HISTO(TH2F, zdc_ADCprepost, file, zdc_ADCprepostName, isEmpty);
    FIND_HISTO(TH2F, zdcsmd_ADCprepost, file, zdcsmd_ADCprepostName, isEmpty);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 1, 0.001, 0.001);

    c->cd(1);
    if (zdc_ADCprepost) {
	//zdc_ADCprepost->GetYaxis()->SetRangeUser(minSMDADC, maxSMDADC);
	zdc_ADCprepost->Draw("H COLZ");
	gPad->SetLogz();
    }

    c->cd(2);
    if (zdcsmd_ADCprepost) {
	//zdcsmd_ADCprepost->GetYaxis()->SetRangeUser(minSMDADC, maxSMDADC);
	zdcsmd_ADCprepost->Draw("H COLZ");
	gPad->SetLogz();
    }
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayZDCADCprepostCorrelation(Bool_t towersmd, FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 2, 0.001, 0.001);

    Int_t ic = 1;
    for (Int_t p = 3;p < 7;p++) {
	c->cd(ic);
	FIND_HISTO(TH2F, zdc_ADCprepostCorrelation, file, TString::Format("%s_%i", towersmd ? zdcsmd_ADCprepostCorrelationName : zdc_ADCprepostCorrelationName, p), isEmpty);
	if (zdc_ADCprepostCorrelation) {
	    zdc_ADCprepostCorrelation->SetStats(kFALSE);
	    zdc_ADCprepostCorrelation->Draw("H COLZ");
	    gPad->SetLogz();
	}
	ic++;
    }
}

#undef FIND_HISTO

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayTab(Int_t tab, Int_t panel, FileType file, TPad *pad, Bool_t *isEmpty) {
    ZDCPlotsPresenter presenter;
    if (tab == 0) {
	if (panel == 0) {
	    presenter.displayTab(kZDC_General, file, pad, isEmpty);
	} else if (panel == 1) {
	    presenter.displayTab(kZDC_Tower, file, pad, isEmpty);
	}
    } else if (tab == 1) {
	if (panel == 0) {
	    presenter.displayTab(kZDCSMD_General, file, pad);
	}
    }
}

//-------------------------------------------------------------------
int ZDCPlotsPresenter::getMaxId() const {
    return sizeof(ZDCPlotsPresenterTitles) / sizeof(ZDCPlotsPresenterTitles[0]);
}

//-------------------------------------------------------------------
const Char_t *ZDCPlotsPresenter::getTitle(int id) const {
    const Char_t *title = 0;
    if ((id >= 0) && (id < getMaxId())) {
	title = ZDCPlotsPresenterTitles[id];
    }
    return title;
}

//-------------------------------------------------------------------
TCanvas *ZDCPlotsPresenter::displayFileTab(const Char_t *filename, Int_t id) const {
    TCanvas *c = 0;
    if (filename && filename[0]) {
        TFile *f = new TFile(filename, "READ");
        if (f && f->IsOpen()) {
            if ((id >= 0) && (id < this->getMaxId())) {
                c = new TCanvas(TString::Format("c_%i", id), this->getTitle(id));
		if (c) {
		    Bool_t isEmpty = false;
		    this->displayTab(id, f, c, &isEmpty);
		    if (isEmpty) {
			LOG_ERROR << "Not all necessary histograms for page " << id << " were found in file " << filename << endm;
		    }
		} else {
		    LOG_ERROR << "Cannot create canvas" << endm;
		}
            } else {
                LOG_ERROR << "Page " << id << " does not exist, valid range is between 0 and " << (this->getMaxId() - 1) << endm;
            }
            f->Close();
        } else {
            LOG_ERROR << "Cannot read file " << filename << endm;
        }
        if (f) delete f;
    }
    return c;
}

//-------------------------------------------------------------------
void ZDCPlotsPresenter::displayTab(int id, FileType file, TPad *pad, Bool_t *isEmpty) const {
	if (id == kZDC_General) {
	    displayZDC(file, pad, isEmpty);
	} else if (id == kZDC_Tower) {
	    displayZDCTower(false, file, pad, isEmpty);
	} else if (id == kZDC_Tower_scale) {
	    displayZDCTower_scale(false, file, pad, isEmpty);
	} else if (id == kZDC_TowerCorr) {
	    displayZDCTower(true, file, pad, isEmpty);
	} else if (id == kZDCEWadcSum) {
	  displayZDCEWadcSum(file, pad, isEmpty);
	} else if (id == kZDC_TDC) {
	  displayZDCTDC(file, pad, isEmpty);
	} else if (id == kZDC_TDCdiffE) {
	  displayZDCTDCdiff(east, file, pad, isEmpty);
	} else if (id == kZDC_TDCdiffW) {
	  displayZDCTDCdiff(west, file, pad, isEmpty);
	} else if (id == kZDC_ADCTDC) {
	    displayZDCADCTDC(file, pad, isEmpty);
	} else if (id == kZDC_FrontBackSum) {
	    displayZDCFrontBackSum(file, pad, isEmpty);
	} else if (id == kZDCSMD_General) {
	    displayZDCSMD(file, pad);
	} else if (id == kZDCSMD_ADCOccupancy) {
	    displayZDCSMDADCOccupancy(file, pad, isEmpty);
	} else if (id == kZDCSMD_MaxXY) {
	    displayZDCSMDMaxXY(file, pad, isEmpty);
	} else if (id == kZDCSMD_ADCEV) {
	    displayZDCSMDADC(east, 0, false, file, pad, isEmpty);
	} else if (id == kZDCSMD_ADCEH) {
	    displayZDCSMDADC(east, 1, false, file, pad, isEmpty);
	} else if (id == kZDCSMD_ADCWV) {
	    displayZDCSMDADC(west, 0, false, file, pad, isEmpty);
	} else if (id == kZDCSMD_ADCWH) {
	    displayZDCSMDADC(west, 1, false, file, pad, isEmpty);
	} else if (id == kZDCSMD_ADCEVCorr) {
	    displayZDCSMDADC(east, 0, true, file, pad, isEmpty);
	} else if (id == kZDCSMD_ADCEHCorr) {
	    displayZDCSMDADC(east, 1, true, file, pad, isEmpty);
	} else if (id == kZDCSMD_ADCWVCorr) {
	    displayZDCSMDADC(west, 0, true, file, pad, isEmpty);
	} else if (id == kZDCSMD_ADCWHCorr) {
	    displayZDCSMDADC(west, 1, true, file, pad, isEmpty);
	} else if (id == kZDCSMD_SmdSumTowerSumE) {
	    displayZDCSmdSumTowerSum(east, file, pad, isEmpty);
	} else if (id == kZDCSMD_SmdSumTowerSumW) {
	    displayZDCSmdSumTowerSum(west, file, pad, isEmpty);
	} else if (id == kZDCDSM_earliestTDC) {
	    displayZDCDSMearliestTDC(file, pad, isEmpty);
	} else if (id == kZDCDSM_ADCthresh) {
	    displayZDCDSMADCthresh(file, pad, isEmpty);
	} else if (id == kZDC_ADCprepost) {
	    displayZDCADCprepost(file, pad, isEmpty);
	} else if (id == kZDC_ADCprepostCorrelationTower) {
	    displayZDCADCprepostCorrelation(false, file, pad, isEmpty);
	} else if (id == kZDC_ADCprepostCorrelationSMD) {
	    displayZDCADCprepostCorrelation(true, file, pad, isEmpty);
	}
}
