
#include <fstream>
using namespace std;

#include <TMath.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TArrow.h>
#include <TLatex.h>
#include <TString.h>
#include <TBox.h>
#include <TLegend.h>

#include <StMessMgr.h>

#include "ZDCPolarimetryPlotsNames.h"
#include "ZDCPolarimetryPlotsPresenter.h"

const Char_t *ZDCPolarimetryPlotsPresenterTitles[] = {
"Bunch crossing distribution",
"ZDC Raw Asymmetry per Bunch Crossing",
"ZDC Raw Asymmetry per Spin Combination",
"ZDC Single Spin Asymmetry"
};

ClassImp(ZDCPolarimetryPlotsPresenter)

//-------------------------------------------------------------------
ZDCPolarimetryPlotsPresenter::ZDCPolarimetryPlotsPresenter()
{
}

//-------------------------------------------------------------------
ZDCPolarimetryPlotsPresenter::~ZDCPolarimetryPlotsPresenter() {
}

//-------------------------------------------------------------------
void ZDCPolarimetryPlotsPresenter::displayBx(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    FIND_HISTO(TH2F, bx, file, bxName, isEmpty);
    FIND_HISTO(TH2F, spin, file, spinName, isEmpty);
    FIND_HISTO(TH2F, bxspin, file, bxspinName, isEmpty);
    FIND_HISTO(TH2F, bxspinY, file, bxspinYName, isEmpty);
    FIND_HISTO(TH2F, bxspinB, file, bxspinBName, isEmpty);
    
    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(4, 2, 0.001, 0.001);

    if (bx) {
	for (Int_t i = 0;i < 4;i++) {
	    TH1D *h = bx->ProjectionY(TString::Format("%s_%s_%s", bx->GetName(), (i / 2) ? "bx48" : "bx7", (i % 2) ? "corrected" : "raw"), i + 1, i + 1);
    	    if (h) {
    	        h->SetDirectory(0);
		if (this->mCleanup) this->mCleanup->Add(h);
		h->SetTitle(TString::Format("%s %s", (i / 2) ? "bx48" : "bx7", (i % 2) ? "Corrected" : "Raw"));
		c->cd(((i / 2) * 4) + (i % 2) + 1);
		h->Draw("H COLZ");
	    }
	}
    } 
    if (spin) {
	c->cd(3);
	spin->Draw("H COLZ");
    }
    if (bxspin) {
	c->cd(7);
	bxspin->Draw("H COLZ");
	gPad->SetLogz();
    }
    if (bxspinY) {
	c->cd(4);
	bxspinY->Draw("H COLZ");
	gPad->SetLogz();
    }
    if (bxspinB) {
	c->cd(8);
	bxspinB->Draw("H COLZ");
	gPad->SetLogz();
    }
}

//-------------------------------------------------------------------
void ZDCPolarimetryPlotsPresenter::displayZDCRawAsymBxSpin(Bool_t bxSpin, FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);

    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 2, 0.001, 0.001);

    FIND_HISTO(TH2F, bxspin, file, bxspinName, isEmpty);
    Int_t nBin = bxSpin ? Int_t(kNspin) : Int_t(kNbunch);

    TH1F *h_strip[2][2];
    TH1F *h_stripSpin[2][2][kNspin];
    memset(h_strip, 0, sizeof(h_strip));
    memset(h_stripSpin, 0, sizeof(h_stripSpin));
    const Char_t *spinNames[] = {"unknown", "upup", "updn", "dnup", "dndn"};
    const Char_t *spinTitles[] = {"??", "#uparrow#uparrow", "#uparrow#downarrow", "#downarrow#uparrow", "#downarrow#downarrow"};
    Color_t spinColors[] = {kBlack, kRed, kGreen, kMagenta, kBlue};
    Style_t spinMarkers[] = {kFullStar, kFullCircle, kFullTriangleUp, kFullTriangleDown, kFullSquare};
   
    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	for (Int_t horiz = 0;horiz < 2;horiz++) {
	    h_strip[eastwest][horiz] = new TH1F(TString::Format("h_strip_%s_%s_%s", bxSpin ? "spin" : "bx", eastwest ? "west" : "east", horiz ? "updown" : "leftright"), 
		TString::Format("Raw asymmetry per %s, %s %s;%s;%s", bxSpin ? "spin combination" : "bunch crossing", eastwest ? "West" : "East", horiz ? "Up-Down" : "Left-Right", bxSpin ? "spin" : "bx7", horiz ? "(U - D)/(U + D)" : "(L - R)/(L + R)"), 
		nBin, 0, nBin);
	    if (h_strip[eastwest][horiz] && this->mCleanup) this->mCleanup->Add(h_strip[eastwest][horiz]);
	    for (Int_t ispin = 0;ispin < kNspin;ispin++) {
		h_stripSpin[eastwest][horiz][ispin] = new TH1F(TString(TString::Format("h_strip_%s_%s_%s_%s", spinNames[ispin], bxSpin ? "spin" : "bx", eastwest ? "west" : "east", horiz ? "updown" : "leftright")), 
		    TString(TString::Format("Raw %s asymmetry per %s, %s %s;%s;%s", spinTitles[ispin], bxSpin ? "spin combination" : "bunch crossing", eastwest ? "West" : "East", horiz ? "Up-Down" : "Left-Right", bxSpin ? "spin" : "bx7", horiz ? "(U - D)/(U + D)" : "(L - R)/(L + R)")), 
		    nBin, 0, nBin);
		if (h_stripSpin[eastwest][horiz][ispin] && this->mCleanup) this->mCleanup->Add(h_stripSpin[eastwest][horiz][ispin]);
	    }
	}
    }
    
    
    for (Int_t ibx = 0;ibx < nBin;ibx++) {
	FIND_HISTO(TH2F, smdStrips, file, TString::Format("%s_%i", bxSpin ? smdStripsSpinName : smdStripsBxName, ibx), isEmpty);
	if (smdStrips) {
	    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
		for (Int_t horiz = 0;horiz < 2;horiz++) {
		    if (h_strip[eastwest][horiz]) {
			Int_t istripAmask[32] = {0};
			Int_t istripBmask[32] = {0};
			istripAmask[(eastwest * 16) + (horiz * 8) + 0] = 1;
			istripAmask[(eastwest * 16) + (horiz * 8) + 1] = 1;
			istripBmask[(eastwest * 16) + (horiz * 8) + ((horiz ? 8 : 7) - 1) - 0] = 1;
			istripBmask[(eastwest * 16) + (horiz * 8) + ((horiz ? 8 : 7) - 1) - 1] = 1;
			Float_t sumA = 0;
			Float_t sumAerr2 = 0;
			Float_t sumB = 0;
			Float_t sumBerr2 = 0;
			for (Int_t istrip = 0;istrip < 32;istrip++)
			for (Int_t iadc = 1;iadc <= smdStrips->GetYaxis()->GetNbins() + 1;iadc++) {
			    Int_t ibin = smdStrips->GetBin(istrip + 1, iadc);
			    if (istripAmask[istrip]) {
				Float_t valA = smdStrips->GetBinContent(ibin);
				Float_t valAerr = smdStrips->GetBinError(ibin);
				sumA += valA;
				sumAerr2 += valAerr * valAerr;
			    }
			    if (istripBmask[istrip]) {
				Float_t valB = smdStrips->GetBinContent(ibin);
				Float_t valBerr = smdStrips->GetBinError(ibin);
				sumB += valB;
				sumBerr2 += valBerr * valBerr;
			    }
			}
			//Float_t sumAerr = TMath::Sqrt(sumAerr2);
			//Float_t sumBerr = TMath::Sqrt(sumBerr2);
			if ((sumA != 0) && (sumB != 0)) {
			    Float_t sumAB = sumA + sumB;
			    Float_t ratio = (sumA - sumB) / sumAB;
			    //Float_t ratioErr = (((TMath::Sqrt(((sumAerr/sumA)*(sumAerr/sumA))+((sumBerr/sumB)*(sumBerr/sumB)))*(sumA / sumB)) / ((sumA / sumB) + 1))) * (2.0 / ((sumA / sumB) + 1));
			    Float_t ratioErr = (2.0 / (sumAB*sumAB)) * TMath::Sqrt((sumB*sumB*sumAerr2) + (sumA*sumA*sumBerr2));
			    h_strip[eastwest][horiz]->SetBinContent(ibx + 1, ratio);
			    h_strip[eastwest][horiz]->SetBinError(ibx + 1, ratioErr);
			    Bool_t checkedSpinBxOk = false;
			    for (Int_t ispin = 0;ispin < kNspin;ispin++) {
				Bool_t checkedSpinOk = true;
				if (bxSpin) {
				    checkedSpinOk = (ispin == ibx);
				} else {
				    for (Int_t i = 0;i < kNspin;i++) {
					checkedSpinOk &= (i == ispin) ? (bxspin->GetBinContent(bxspin->GetBin(ibx + 1, i + 1)) > 0) : (bxspin->GetBinContent(bxspin->GetBin(ibx + 1, i + 1)) == 0);
					//checkedSpinOk &= (bxspin->GetBinContent(bxspin->GetBin(ibx + 1, 0 + 1)) == 0);
				    }
				}
    				checkedSpinBxOk |= checkedSpinOk;
				if (checkedSpinOk) {
				    LOG_INFO << "Raw asymmetry " << (bxSpin ? "spin combination " : "bx") << " " << ibx << ", spin " << spinNames[ispin] << ", " << (eastwest ? "West" : "East") << " " << (horiz ? "Up-Down" : "Left-Right") << ": " << ratio << " +/- " << ratioErr << endm;
				    if (h_stripSpin[eastwest][horiz][ispin]) {
					h_stripSpin[eastwest][horiz][ispin]->SetBinContent(ibx + 1, ratio);
				        h_stripSpin[eastwest][horiz][ispin]->SetBinError(ibx + 1, ratioErr);
				    }
				}
			    }
			    if (!checkedSpinBxOk) {
				LOG_ERROR << "Cannot determine spin combination for " << (bxSpin ? "spin" : "bx") << " " << ibx << endm;
			    }
			}
		    }
		}
	    }
	}
    }
 
    Float_t asym[2][2][kNspin];
    Float_t asymErr[2][2][kNspin];
    Float_t asymChi2[2][2][kNspin];
    Int_t asymNDF[2][2][kNspin];
    Float_t maxRange = 0;
    Float_t asymMean[2][2];
    Int_t asymMeanN[2][2];
 
    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	for (Int_t horiz = 0;horiz < 2;horiz++) {
	    asymMean[eastwest][horiz] = 0;
	    asymMeanN[eastwest][horiz] = 0;
	    for (Int_t ispin = 0;ispin < kNspin;ispin++) {
		asym[eastwest][horiz][ispin] = 0;
		asymErr[eastwest][horiz][ispin] = 0;
		asymChi2[eastwest][horiz][ispin] = 0;
		asymNDF[eastwest][horiz][ispin] = 0;
	    }
	    if (h_strip[eastwest][horiz]) {
		c->cd((eastwest * 2) + horiz + 1);
		h_strip[eastwest][horiz]->GetYaxis()->SetTitleOffset(1.5);
		h_strip[eastwest][horiz]->SetStats(false);
		h_strip[eastwest][horiz]->Draw("P");
		for (Int_t i = 1;i <= h_strip[eastwest][horiz]->GetXaxis()->GetNbins();i++) {
		    h_strip[eastwest][horiz]->SetBinContent(i, 0);
		    h_strip[eastwest][horiz]->SetBinError(i, 0);
		}
	    }
	    TLegend *leg = new TLegend(0.6, 0.73, 0.9, 0.9);
	    if (leg && this->mCleanup) this->mCleanup->Add(leg);
	    for (Int_t ispin = 0;ispin < kNspin;ispin++) {
		if ((ispin == 0) || (ispin == 2) || (ispin == 3)) continue;
		if (h_stripSpin[eastwest][horiz][ispin]) {
		    h_stripSpin[eastwest][horiz][ispin]->SetMarkerStyle(spinMarkers[ispin]);
		    h_stripSpin[eastwest][horiz][ispin]->SetMarkerColor(spinColors[ispin]);
		    h_stripSpin[eastwest][horiz][ispin]->SetLineColor(spinColors[ispin]);
		    h_stripSpin[eastwest][horiz][ispin]->SetLineWidth(1);
		    h_stripSpin[eastwest][horiz][ispin]->SetStats(false);
		    h_stripSpin[eastwest][horiz][ispin]->Draw("P E ][ SAME");
		    TF1 *f = new TF1(TString::Format("f_%s_%s_%s", spinNames[ispin], eastwest ? "west" : "east", horiz ? "vert" : "horiz"), "pol0");
		    if (f) {
			if (this->mCleanup) this->mCleanup->Add(f);
			if (h_stripSpin[eastwest][horiz][ispin]->GetEntries() > 0) {
			    h_stripSpin[eastwest][horiz][ispin]->Fit(f, "QN");
			}
		        asym[eastwest][horiz][ispin] = f->GetParameter(0);
		        asymErr[eastwest][horiz][ispin] = f->GetParError(0);
		        asymChi2[eastwest][horiz][ispin] = f->GetChisquare();
		        asymNDF[eastwest][horiz][ispin] = f->GetNDF();
		        f->SetLineColor(spinColors[ispin]);
		        f->SetLineWidth(1);
		        f->Draw("SAME");
		    }
		    if (leg) leg->AddEntry(h_stripSpin[eastwest][horiz][ispin], TString::Format("%s (#chi^{2}/dof = %.1f/%i)", spinTitles[ispin], asymChi2[eastwest][horiz][ispin], asymNDF[eastwest][horiz][ispin]), "PL");
		    Float_t minY = 100;
		    Float_t maxY = -100;
		    for (Int_t ibin = 1;ibin <= h_stripSpin[eastwest][horiz][ispin]->GetXaxis()->GetNbins();ibin++) {
			if (!((h_stripSpin[eastwest][horiz][ispin]->GetBinContent(ibin) == 0) && (h_stripSpin[eastwest][horiz][ispin]->GetBinError(ibin) == 0))) {
			    if (h_stripSpin[eastwest][horiz][ispin]->GetBinContent(ibin) + h_stripSpin[eastwest][horiz][ispin]->GetBinError(ibin) > maxY) maxY = h_stripSpin[eastwest][horiz][ispin]->GetBinContent(ibin) + h_stripSpin[eastwest][horiz][ispin]->GetBinError(ibin);
			    if (h_stripSpin[eastwest][horiz][ispin]->GetBinContent(ibin) - h_stripSpin[eastwest][horiz][ispin]->GetBinError(ibin) < minY) minY = h_stripSpin[eastwest][horiz][ispin]->GetBinContent(ibin) - h_stripSpin[eastwest][horiz][ispin]->GetBinError(ibin);
			}
		    }
		    Float_t range = maxY - minY;
		    if (range > maxRange) maxRange = range;
		    asymMean[eastwest][horiz] += asym[eastwest][horiz][ispin];
		    asymMeanN[eastwest][horiz] += 1;
		}
	    }
	    if (asymMeanN[eastwest][horiz] > 0) asymMean[eastwest][horiz] /= asymMeanN[eastwest][horiz];
	    if (leg) {
		leg->SetFillColor(0);
		leg->SetFillStyle(0);
		leg->SetLineColor(0);
		leg->Draw();
	    }
	    if (true) {
		Float_t upupdndnRatio = 100.0 * (asym[eastwest][horiz][1] - asym[eastwest][horiz][4]) / 2.0;
		Float_t upupdndnRatioErr = 100.0 * TMath::Sqrt((asymErr[eastwest][horiz][1]*asymErr[eastwest][horiz][1]) + (asymErr[eastwest][horiz][4]*asymErr[eastwest][horiz][4])) / 2.0;
		TString asymStr = TString::Format("%.3f +/- %.3f %%", upupdndnRatio, upupdndnRatioErr);
		LOG_INFO << (eastwest ? "West" : "East") << " " << (horiz ? "Up-Down" : "Left-Right") << " (\"++\" - \"--\")/2 = " << asymStr << endm;
		TString asymLabel = TString::Format("A^{%s}_{%s} (%s,%s) = %.3f #pm %.3f %%", eastwest ? "W" : "E", horiz ? "UD" : "LR", spinTitles[1], spinTitles[4], upupdndnRatio, upupdndnRatioErr);
		LOG_DEBUG << asymLabel << endm;
		TLatex *l = new TLatex(0.2, 0.8, asymLabel);
		if (l) {
		    if (this->mCleanup) this->mCleanup->Add(l);
		    l->SetNDC(true);
		    //l->SetTextColor(spinColors[1]);
		    l->Draw();
		}
	    }
	    if (false) {
		Float_t updndnupRatio = 100.0 * (asym[eastwest][horiz][2] - asym[eastwest][horiz][3]) / 2.0;
		Float_t updndnupRatioErr = 100.0 * TMath::Sqrt((asymErr[eastwest][horiz][2]*asymErr[eastwest][horiz][2]) + (asymErr[eastwest][horiz][3]*asymErr[eastwest][horiz][3])) / 2.0;
		TString asymStr = TString::Format("%.3f +/- %.3f %%", updndnupRatio, updndnupRatioErr);
		LOG_INFO << (eastwest ? "West" : "East") << " " << (horiz ? "Up-Down" : "Left-Right") << " (\"+-\" - \"-+\")/2 = " << asymStr << endm;
		TString asymLabel = TString::Format("A^{%s}_{%s} (%s,%s) = %.3f #pm %.3f %%", eastwest ? "W" : "E", horiz ? "UD" : "LR", spinTitles[2], spinTitles[3], updndnupRatio, updndnupRatioErr);
		LOG_DEBUG << asymLabel << endm;
		TLatex *l = new TLatex(0.2, 0.7, asymLabel);
		if (l) {
		    if (this->mCleanup) this->mCleanup->Add(l);
		    l->SetNDC(true);
		    //l->SetTextColor(spinColors[2]);
		    l->Draw();
		}
	    }
	}
    }
 
    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	for (Int_t horiz = 0;horiz < 2;horiz++) {
	    if (h_strip[eastwest][horiz]) {
		c->cd((eastwest * 2) + horiz + 1);
		h_strip[eastwest][horiz]->GetYaxis()->SetRangeUser(asymMean[eastwest][horiz] - (1.1 * (maxRange / 2.0)), asymMean[eastwest][horiz] + (2.0 * (maxRange / 2.0)));
	    }
	}
    }
}

void normAngle(Float_t &angle) {
    while (angle < -TMath::Pi()) angle += 2.0 * TMath::Pi();
    while (angle >= +TMath::Pi()) angle -= 2.0 * TMath::Pi();
}

Float_t getAngle(Float_t x1, Float_t y1, Float_t x2, Float_t y2) {
    Float_t angle = 0;
    Float_t dx = x2 - x1;
    Float_t dy = y2 - y1;
    if (dx == 0) {
	if (dy > 0) {
	    angle = +TMath::Pi() / 2.0;
	} else if (dy < 0) {
	    angle = -TMath::Pi() / 2.0;
	} else {
	    angle = 0;
	}
    } else {
	angle = TMath::ATan(TMath::Abs(dy / dx));
	if (dx > 0) {
	    if (dy > 0) {
		angle = +angle;
	    } else if (dy < 0) {
		angle = -angle;
	    }
	} else if (dx < 0) {
	    if (dy > 0) {
		angle = TMath::Pi() - angle;
	    } else if (dy < 0) {
		angle = -TMath::Pi() + angle;
	    }
	}
    }
    normAngle(angle);
    return angle;
}

void getSqrtAsym(
    Float_t n_p_l, 
    Float_t e_p_l, 
    Float_t n_p_r, 
    Float_t e_p_r, 
    Float_t n_m_l, 
    Float_t e_m_l, 
    Float_t n_m_r, 
    Float_t e_m_r, 
    Float_t &asym,
    Float_t &asymErr
) {
    asym = (TMath::Sqrt(n_p_l * n_m_r) - TMath::Sqrt(n_m_l * n_p_r)) / (TMath::Sqrt(n_p_l * n_m_r) + TMath::Sqrt(n_m_l * n_p_r));
    asymErr = 
	TMath::Sqrt(
	(n_m_r * n_m_l * n_p_r * e_p_l * e_p_l / n_p_l) +
	(n_p_l * n_m_l * n_p_r * e_m_r * e_m_r / n_m_r) + 
	(n_p_l * n_m_r * n_p_r * e_m_l * e_m_l / n_m_l) + 
	(n_p_l * n_m_r * n_m_l * e_p_r * e_p_r / n_p_r)
	) / ((TMath::Sqrt(n_p_l * n_m_r) + TMath::Sqrt(n_m_l * n_p_r)) * (TMath::Sqrt(n_p_l * n_m_r) + TMath::Sqrt(n_m_l * n_p_r)));
}

//-------------------------------------------------------------------
void ZDCPolarimetryPlotsPresenter::displaySingleSpinAsymmetry(FileType file, TPad *pad, Bool_t *isEmpty) const {
    if (isEmpty) *isEmpty = true;
    if (!pad) return;
    pad->Clear();
    pad->cd(0);


    FIND_HISTO(TH1F, yellowPol, file, yellowPolName, isEmpty);
    FIND_HISTO(TH1F, yellowPolW, file, yellowPolWName, isEmpty);
    FIND_HISTO(TH1F, bluePol, file, bluePolName, isEmpty);
    FIND_HISTO(TH1F, bluePolW, file, bluePolWName, isEmpty);
    
    TPad* c = new TPad("pad2", "apd2",0.0,0.1,1.,1.);
    c->Draw();
    c->cd(0);
    c->Divide(2, 2, 0.001, 0.001);

    Float_t yPol = yellowPol ? yellowPol->GetMean() : 0;
    Float_t yPolErr = yellowPol ? yellowPol->GetRMS() : 0;
    Float_t bPol = bluePol ? bluePol->GetMean() : 0;
    Float_t bPolErr = bluePol ? bluePol->GetRMS() : 0;

    LOG_DEBUG << "yPol = " << yPol << " +/- " << yPolErr << endm;
    LOG_DEBUG << "bPol = " << bPol << " +/- " << bPolErr << endm;

    if (yellowPolW) {
	Float_t polW = yellowPolW->GetBinContent(1), w = yellowPolW->GetBinContent(2);//, n = yellowPolW->GetBinContent(3);
	if (w > 0) {
	    polW /= w;
	    w = TMath::Sqrt(1.0 / w);
	    yPol = polW;
	    yPolErr = w;
	}
    }

    if (bluePolW) {
	Float_t polW = bluePolW->GetBinContent(1), w = bluePolW->GetBinContent(2);//, n = bluePolW->GetBinContent(3);
	if (w > 0) {
	    polW /= w;
	    w = TMath::Sqrt(1.0 / w);
	    bPol = polW;
	    bPolErr = w;
	}
    }

    LOG_DEBUG << "new yPol = " << yPol << " +/- " << yPolErr << endm;
    LOG_DEBUG << "new bPol = " << bPol << " +/- " << bPolErr << endm;
    TString outputAsymmetryStr;

    for (Int_t yellowblue = 0;yellowblue < 2;yellowblue++) {
	for (Int_t forwardbackward = 0;forwardbackward < 2;forwardbackward++) {
	    Int_t eastwest = (yellowblue ? !forwardbackward : forwardbackward);
	    FIND_HISTO(TH2F, hits_pp, file, TString::Format("%s_%s_%i", smdHitsSpinName, eastwest ? "west" : "east", 1), isEmpty);
	    FIND_HISTO(TH2F, hits_pm, file, TString::Format("%s_%s_%i", smdHitsSpinName, eastwest ? "west" : "east", 2), isEmpty);
	    FIND_HISTO(TH2F, hits_mp, file, TString::Format("%s_%s_%i", smdHitsSpinName, eastwest ? "west" : "east", 3), isEmpty);
	    FIND_HISTO(TH2F, hits_mm, file, TString::Format("%s_%s_%i", smdHitsSpinName, eastwest ? "west" : "east", 4), isEmpty);
	    Float_t beamX = 0, beamY = 0;
	    TH1F *hits_phi_p = new TH1F(TString::Format("hits_phi_p_%s_%s", yellowblue ? "blue" : "yell", forwardbackward ? "back" : "forw"), TString::Format("N^{#uparrow}(#phi) %s %s;#phi;Counts", yellowblue ? "Blue" : "Yellow", forwardbackward ? "Backward" : "Forward"), 16, -TMath::Pi(), +TMath::Pi());
	    TH1F *hits_phi_m = new TH1F(TString::Format("hits_phi_m_%s_%s", yellowblue ? "blue" : "yell", forwardbackward ? "back" : "forw"), TString::Format("N^{#downarrow}(#phi) %s %s;#phi;Counts", yellowblue ? "Blue" : "Yellow", forwardbackward ? "Backward" : "Forward"), 16, -TMath::Pi(), +TMath::Pi());
	    if (hits_phi_p && this->mCleanup) this->mCleanup->Add(hits_phi_p);
	    if (hits_phi_m && this->mCleanup) this->mCleanup->Add(hits_phi_m);
	    for (Int_t binX = 1;binX <= hits_pp->GetXaxis()->GetNbins();binX++) {
		Float_t x = hits_pp->GetXaxis()->GetBinCenter(binX);
		for (Int_t binY = 1;binY <= hits_pp->GetYaxis()->GetNbins();binY++) {
		    Float_t y = hits_pp->GetYaxis()->GetBinCenter(binY);
		    if ((TMath::Abs(x - beamX) < 2.25) && (TMath::Abs(y - beamY) < 4.0)) continue;
		    Float_t phi = getAngle(beamX, beamY, x, y);
//cout << "x=" << x << ", y=" << y << ", phi=" << phi << ", ";
		    {
			TH2F *h_pp = yellowblue ? hits_pp : hits_pp;
			TH2F *h_pm = yellowblue ? hits_mp : hits_pm;
			Float_t newVal = 0, newErr = 0;
			Int_t binPhi = hits_phi_p->GetXaxis()->FindBin(phi);
			if (hits_phi_p) {
			    newVal += hits_phi_p->GetBinContent(binPhi);
			    newErr += (hits_phi_p->GetBinError(binPhi) * hits_phi_p->GetBinError(binPhi));
			}
			if (hits_pp) {
			    newVal += h_pp->GetBinContent(binX, binY);
			    newErr += (h_pp->GetBinError(binX, binY) * h_pp->GetBinError(binX, binY));
			}
			if (hits_pm) {
			    newVal += h_pm->GetBinContent(binX, binY);
			    newErr += (h_pm->GetBinError(binX, binY) * h_pm->GetBinError(binX, binY));
			}
			if (hits_phi_p) {
			    hits_phi_p->SetBinContent(binPhi, newVal);
			    hits_phi_p->SetBinError(binPhi, TMath::Sqrt(newErr));
			}
//cout << "binPhi_p=" << binPhi << ", p=" << newVal << " +/- " << TMath::Sqrt(newErr) << ", ";
		    }
		    {
			TH2F *h_mp = yellowblue ? hits_pm : hits_mp;
			TH2F *h_mm = yellowblue ? hits_mm : hits_mm;
			Float_t newVal = 0, newErr = 0;
			Int_t binPhi = hits_phi_m->GetXaxis()->FindBin(phi);
			if (hits_phi_m) {
			    newVal += hits_phi_m->GetBinContent(binPhi);
			    newErr += (hits_phi_m->GetBinError(binPhi) * hits_phi_m->GetBinError(binPhi));
			}
			if (hits_mp) {
			    newVal += h_mp->GetBinContent(binX, binY);
			    newErr += (h_mp->GetBinError(binX, binY) * h_mp->GetBinError(binX, binY));
			}
			if (hits_mm) {
			    newVal += h_mm->GetBinContent(binX, binY);
			    newErr += (h_mm->GetBinError(binX, binY) * h_mm->GetBinError(binX, binY));
			}
			if (hits_phi_m) {
			    hits_phi_m->SetBinContent(binPhi, newVal);
			    hits_phi_m->SetBinError(binPhi, TMath::Sqrt(newErr));
			}
//cout << "binPhi_m=" << binPhi << ", m=" << newVal << " +/- " << TMath::Sqrt(newErr) << endl;
		    }
		    
		}
	    }
	    TH1F *hits_phi = new TH1F(TString::Format("hits_phi_%s_%s", yellowblue ? "blue" : "yell", forwardbackward ? "back" : "forw"), TString::Format("Square root asymmetry vs. #phi, %s %s (%s);#phi, rad;#epsilon_{phys}", yellowblue ? "Blue" : "Yellow", forwardbackward ? "Backward" : "Forward", eastwest ? "West" : "East"), 8, -TMath::Pi() / 2.0, +TMath::Pi() / 2.0);
	    if (hits_phi && this->mCleanup) this->mCleanup->Add(hits_phi);
	    TF1 *f_phi = new TF1(TString::Format("fit_phi_%s_%s", yellowblue ? "blue" : "yell", forwardbackward ? "back" : "forw"), "[0] * sin(x - [1])");
	    if (f_phi && this->mCleanup) this->mCleanup->Add(f_phi);
	    if (f_phi) {
		f_phi->SetParLimits(0, 0, +1);
		f_phi->SetParLimits(1, -TMath::Pi(), +TMath::Pi());
	    }
	    TF1 *f_const = new TF1(TString::Format("fit_cons_%s_%s", yellowblue ? "blue" : "yell", forwardbackward ? "back" : "forw"), "[0]");
	    if (f_const && this->mCleanup) this->mCleanup->Add(f_const);
	    Float_t asymmetry = 0, asymmetryErr = 0;
	    Float_t phi0 = 0, phi0Err = 0;
	    Float_t chi2 = 0;
	    Int_t ndf = 0;
	    Float_t fitConst = 0, fitConstErr = 0;
	    Float_t fitConstChi2 = 0;
	    Int_t fitConstNdf = 0;
	    if (hits_phi && hits_phi_p && hits_phi_m) {
	    for (Int_t binPhi = 1;binPhi <= hits_phi->GetXaxis()->GetNbins();binPhi++) {
		Float_t phi = hits_phi->GetXaxis()->GetBinCenter(binPhi);
		Float_t phiLeft = phi - ((yellowblue ? -1 : +1) * (TMath::Pi() / 2.0));
		Float_t phiRight = phi + ((yellowblue ? -1 : +1) * (TMath::Pi() / 2.0));
		normAngle(phiLeft);
		normAngle(phiRight);
		Int_t binLeft_p = hits_phi_p->GetXaxis()->FindBin(phiLeft);
		Int_t binRight_p = hits_phi_p->GetXaxis()->FindBin(phiRight);
		Int_t binLeft_m = hits_phi_m->GetXaxis()->FindBin(phiLeft);
		Int_t binRight_m = hits_phi_m->GetXaxis()->FindBin(phiRight);
		Float_t n_p_l = hits_phi_p->GetBinContent(binLeft_p);
		Float_t e_p_l = hits_phi_p->GetBinError(binLeft_p);
		Float_t n_p_r = hits_phi_p->GetBinContent(binRight_p);
		Float_t e_p_r = hits_phi_p->GetBinError(binRight_p);
		Float_t n_m_l = hits_phi_m->GetBinContent(binLeft_m);
		Float_t e_m_l = hits_phi_m->GetBinError(binLeft_m);
		Float_t n_m_r = hits_phi_m->GetBinContent(binRight_m);
		Float_t e_m_r = hits_phi_m->GetBinError(binRight_m);
		Float_t asym = 0, asymErr = 0;
		getSqrtAsym(n_p_l, e_p_l, n_p_r, e_p_r, n_m_l, e_m_l, n_m_r, e_m_r, asym, asymErr);
/*cout << "phi=" << phi << "(" << binPhi << "), phiLeft=" << phiLeft << "(" << binLeft_p << "," << binLeft_m << 
"), phiRight=" << phiRight << "(" << binRight_p << "," << binRight_m << ") ";
cout << "npl=" << n_p_l << " +/- " << e_p_l << ", ";
cout << "npr=" << n_p_r << " +/- " << e_p_r << ", ";
cout << "nml=" << n_m_l << " +/- " << e_m_l << ", ";
cout << "nmr=" << n_m_r << " +/- " << e_m_r << ", ";
cout << "asym=" << asym << " +/- " << asymErr << endl;*/
		hits_phi->SetBinContent(binPhi, asym);
		hits_phi->SetBinError(binPhi, asymErr);
	    }
		if (f_phi && (hits_phi->Integral() != 0)) {
		    hits_phi->Fit(f_phi, "QN I");
		    asymmetry = f_phi->GetParameter(0);
		    asymmetryErr = f_phi->GetParError(0);
		    if (TMath::IsNaN(asymmetry)) {
			asymmetry = 0;
			asymmetryErr = 0;
		    }
		    phi0 = f_phi->GetParameter(1);
		    phi0Err = f_phi->GetParError(1);
		    if (TMath::IsNaN(phi0)) {
			phi0 = 0;
			phi0Err = 0;
		    } else {
			phi0 += TMath::Pi();
			normAngle(phi0);
		    }
		    chi2 = f_phi->GetChisquare();
		    if (TMath::IsNaN(chi2)) {
			chi2 = 0;
		    }
		    ndf = f_phi->GetNDF();
		    if (TMath::IsNaN(ndf)) {
			ndf = 0;
		    }
		}
		if (f_const && (hits_phi->Integral() != 0)) {
		    hits_phi->Fit(f_const, "QN I");
		    fitConst = f_const->GetParameter(0);
		    fitConstErr = f_const->GetParError(0);
		    if (TMath::IsNaN(fitConst)) {
			fitConst = 0;
			fitConstErr = 0;
		    }
		    fitConstChi2 = f_const->GetChisquare();
		    if (TMath::IsNaN(fitConstChi2)) {
			fitConstChi2 = 0;
		    }
		    fitConstNdf = f_const->GetNDF();
		    if (TMath::IsNaN(fitConstNdf)) {
			fitConstNdf = 0;
		    }
		}
	    }
	    Float_t aN = 0, aNErr = 0;
	    Float_t pol = yellowblue ? bPol : yPol;
	    Float_t polErr = yellowblue ? bPolErr : yPolErr;
	    if ((pol > 0) && (asymmetry > 0)) {
		aN = asymmetry / pol;
		aNErr = aN * TMath::Sqrt(((asymmetryErr/asymmetry)*(asymmetryErr/asymmetry)) + ((polErr/pol)*(polErr/pol)));
	    }
	    outputAsymmetryStr += TString::Format("%s %s asymmetry %f %f polarization %f %f analyzingpower %f %f anglevertical %f %f\n", (yellowblue ? "blue" : "yellow"), (forwardbackward ? "backward" : "forward"), asymmetry, asymmetryErr, pol, polErr, aN, aNErr, phi0, phi0Err);
	    if (hits_phi && (hits_phi->Integral() != 0)) {
		c->cd((yellowblue * 2) + forwardbackward + 1);
		hits_phi->SetMarkerStyle(kFullCircle);
		hits_phi->SetStats(kFALSE);
		hits_phi->Draw("P E ][");
		if (f_phi) {
		    f_phi->SetLineWidth(2);
		    f_phi->Draw("same");
		}
		if (f_const) {
		    f_const->SetLineWidth(2);
		    f_const->SetLineColor(kBlue);
		    f_const->Draw("same");
		}
		TString asymLabel = 
		    TString::Format("#splitline{#splitline{#splitline{#splitline{#splitline{|#epsilon_{phys}|sin(#phi-#phi_{0}) fit:}"
			"{|#epsilon_{phys}| = %.2f #pm %.2f %%}}{#phi_{0} + #pi = %.3f #pm %.3f}}{#chi^{2}/dof = %.1f/%i}}{P_{%s} = %.1f #pm %.1f %%}}"
			"{A_{N}(%s, x_{F}%s0) = %.2f #pm %.2f %%}"
		    , 100.0 * asymmetry, 100.0 * asymmetryErr, phi0, phi0Err, chi2, ndf, yellowblue ? "B" : "Y", 100.0 * (yellowblue ? bPol : yPol), 100.0 * (yellowblue ? bPolErr : yPolErr)
		    , yellowblue ? "B" : "Y", forwardbackward ? "<" : ">", 100.0 * aN, 100.0 * aNErr);
		LOG_DEBUG << asymLabel << endm;
                TLatex *l = new TLatex(0.2, 0.55, asymLabel);
                if (l) {
		    if (this->mCleanup) this->mCleanup->Add(l);
                    l->SetNDC(true);
                    l->Draw();
                }
		TString constLabel = TString::Format("#splitline{const = %.2f #pm %.2f %%}{#chi^{2}/dof = %.1f/%i}", 100.0 * fitConst, 100.0 * fitConstErr, fitConstChi2, fitConstNdf);
		LOG_DEBUG << constLabel << endm;
                TLatex *lConst = new TLatex(0.6, 0.2, constLabel);
                if (lConst) {
		    if (this->mCleanup) this->mCleanup->Add(lConst);
                    lConst->SetNDC(true);
                    lConst->SetTextColor(kBlue);
                    lConst->Draw();
                }
	    }
	}
    }
    const Char_t *filename = this->getAsymmetryOutputFilename();
    if (filename && filename[0]) {
	ofstream ofstr(filename);
	if (ofstr.good()) {
	    ofstr << outputAsymmetryStr;
	    ofstr.close(); 
	} else {
	    LOG_ERROR << "Cannot add to file " << filename << endm;
	}
    }
}

//-------------------------------------------------------------------
Int_t ZDCPolarimetryPlotsPresenter::getMaxId() const {
    return ZDCPlotsPresenter::getMaxId() + (sizeof(ZDCPolarimetryPlotsPresenterTitles) / sizeof(ZDCPolarimetryPlotsPresenterTitles[0]));
}

//-------------------------------------------------------------------
const Char_t *ZDCPolarimetryPlotsPresenter::getTitle(Int_t id) const {
    const Char_t *title = 0;
    if ((id >= 0) && (id < getMaxId())) {
	if (id < ZDCPlotsPresenter::getMaxId()) {
	    title = ZDCPlotsPresenter::getTitle(id);
	} else {
	    title = ZDCPolarimetryPlotsPresenterTitles[id - ZDCPlotsPresenter::getMaxId()];
	}
    }
    return title;
}

//-------------------------------------------------------------------
void ZDCPolarimetryPlotsPresenter::displayTab(Int_t id, FileType file, TPad *pad, Bool_t *isEmpty) const {
	if (id < ZDCPlotsPresenter::getMaxId()) {
	    ZDCPlotsPresenter::displayTab(id, file, pad, isEmpty);
	} else if (id == kBx) {
	    displayBx(file, pad, isEmpty);
	} else if (id == kZDCRawAsymBx) {
	    displayZDCRawAsymBxSpin(false, file, pad, isEmpty);
	} else if (id == kZDCRawAsymSpin) {
	    displayZDCRawAsymBxSpin(true, file, pad, isEmpty);
	} else if (id == kZDCSingleSpinAsymmetry) {
	    displaySingleSpinAsymmetry(file, pad, isEmpty);
	}
}
