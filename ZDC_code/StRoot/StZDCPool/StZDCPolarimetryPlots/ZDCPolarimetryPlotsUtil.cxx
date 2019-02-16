#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
using namespace std;

#include <TMath.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TH1F.h>
#include <TGaxis.h>
#include <TLine.h>
#include <TLatex.h>

#include <StMessMgr.h>

#include "ZDCPolarimetryPlotsUtil.h"

void appendRunsAsymmetry(const Char_t *inputFilename, const Char_t *prefix, const Char_t *outputFilename) {
    ifstream ifstr(inputFilename);
    if (ifstr.good()) {
        ofstream ofstr(outputFilename, ios_base::app);
        if (ofstr.good()) {
            ofstr << prefix << " ";
            while (ifstr.good()) {
                TString token;
                ifstr >> token;
                ofstr << token << " ";
            }
	    ofstr << endl;
            ofstr.close();
        } else {
            LOG_ERROR << "Cannot add to file " << outputFilename << endm;
        }
        ifstr.close();
    } else {
        LOG_ERROR << "Cannot read file " << inputFilename << endm;
    }
}

void plotsAsymmetrySummary(const Char_t *inputFilename, const Char_t *runsStr, Int_t nRuns, Float_t normAnalyzingPower, const Char_t *outputFilenameGif, const Char_t *outputFilenameC) {
    if (inputFilename && inputFilename[0]) {
	ifstream ifstr(inputFilename);
	if (ifstr.good()) {
	    const Int_t nTokens = 45 + (4*3);
	    const Int_t nTokensRow = 14;
	    typedef map<string, vector<string> > run_map_type;
	    run_map_type runs;
	    while (ifstr.good()) {
		vector<string> tokens;
		for (Int_t i = 0;i < nTokens;i++) {
		    string token;
		    ifstr >> token;
		    tokens.push_back(token);
		}
		if ((ifstr.good() || ifstr.eof()) && 
		    (tokens[1 + 2 + 0*nTokensRow] == "asymmetry") && (tokens[1 + 5 + 0*nTokensRow] == "polarization") && (tokens[1 + 8 + 0*nTokensRow] == "analyzingpower") && (tokens[1 + 11 + 0*nTokensRow] == "anglevertical") && 
		    (tokens[1 + 2 + 1*nTokensRow] == "asymmetry") && (tokens[1 + 5 + 1*nTokensRow] == "polarization") && (tokens[1 + 8 + 1*nTokensRow] == "analyzingpower") && (tokens[1 + 11 + 1*nTokensRow] == "anglevertical") && 
		    (tokens[1 + 2 + 2*nTokensRow] == "asymmetry") && (tokens[1 + 5 + 2*nTokensRow] == "polarization") && (tokens[1 + 8 + 2*nTokensRow] == "analyzingpower") && (tokens[1 + 11 + 2*nTokensRow] == "anglevertical") && 
		    (tokens[1 + 2 + 3*nTokensRow] == "asymmetry") && (tokens[1 + 5 + 3*nTokensRow] == "polarization") && (tokens[1 + 8 + 3*nTokensRow] == "analyzingpower") && (tokens[1 + 11 + 3*nTokensRow] == "anglevertical")
		) {
		    runs[tokens[0]] = tokens;
		} else {
		    if (tokens[0] != "") {
			LOG_WARN << "Malformed line in file " << inputFilename << ": read tokens ";
			for (Int_t i = 0;i < nTokens;i++) LOG_WARN << tokens[i] << " ";
		        LOG_WARN << endm;
		    }
		}
		tokens.clear();
	    }
	    //sort(runs.begin(), runs.end());
	    run_map_type::const_iterator runsEnd = runs.end();
	    run_map_type::const_iterator runsBegin = runs.end();
	    if (nRuns > 0) {
		for (Int_t i = 0;i < nRuns;i++) if (runsBegin != runs.begin()) --runsBegin;
	    } else {
		runsBegin = runs.begin();
	    }
	    Int_t nBins = distance(runsBegin, runsEnd);
	    TH1F *hY = new TH1F("h_asymmetry_yellow", "Yellow beam asymmetry", nBins, 0 - 0.55, nBins - 0.55);
	    TH1F *hB = new TH1F("h_asymmetry_blue", "Blue beam asymmetry", nBins, 0 - 0.45, nBins - 0.45);
	    TLegend *leg = 0;
	    TGaxis *axisTrans = 0;
	    TLine *lineZero = 0;
	    TLine *lineUnity = 0;
	    TH1F *hPhiY = new TH1F("h_phi_yellow", "Yellow beam polarization direction", nBins, 0 - 0.55, nBins - 0.55);
	    TH1F *hPhiB = new TH1F("h_phi_blue", "Blue beam polarization direction", nBins, 0 - 0.45, nBins - 0.45);
	    TLegend *legPhi = 0;
	    TLine *linePhiUp = 0;
	    TLine *linePhiNorth = 0;
	    TLine *linePhiSouth = 0;
	    TLine *linePhiDown = 0;
	    TLatex *labelPhiUp = 0;
	    TLatex *labelPhiNorth = 0;
	    TLatex *labelPhiSouth = 0;
	    TLatex *labelPhiDown1 = 0;
	    TLatex *labelPhiDown2 = 0;
	    Int_t runIndex = 1;
	    for (run_map_type::const_iterator runsIter = runsBegin;runsIter != runsEnd;++runsIter) {
		const string &runStr = (*runsIter).first;
		Float_t asymY = atof((*runsIter).second[1 + 2 + 0*nTokensRow + 1].c_str());
		Float_t asymYerr = atof((*runsIter).second[1 + 2 + 0*nTokensRow + 2].c_str());
		//Float_t polY = atof((*runsIter).second[1 + 2 + 0*nTokensRow + 4].c_str());
		//Float_t polYerr = atof((*runsIter).second[1 + 2 + 0*nTokensRow + 5].c_str());
		Float_t apY = atof((*runsIter).second[1 + 2 + 0*nTokensRow + 7].c_str());
		Float_t apYerr = atof((*runsIter).second[1 + 2 + 0*nTokensRow + 8].c_str());
		Float_t phiY = atof((*runsIter).second[1 + 2 + 0*nTokensRow + 10].c_str());
		Float_t phiYerr = atof((*runsIter).second[1 + 2 + 0*nTokensRow + 11].c_str());
		Float_t asymB = atof((*runsIter).second[1 + 2 + 2*nTokensRow + 1].c_str());
		Float_t asymBerr = atof((*runsIter).second[1 + 2 + 2*nTokensRow + 2].c_str());
		//Float_t polB = atof((*runsIter).second[1 + 2 + 2*nTokensRow + 4].c_str());
		//Float_t polBerr = atof((*runsIter).second[1 + 2 + 2*nTokensRow + 5].c_str());
		Float_t apB = atof((*runsIter).second[1 + 2 + 2*nTokensRow + 7].c_str());
		Float_t apBerr = atof((*runsIter).second[1 + 2 + 2*nTokensRow + 8].c_str());
		Float_t phiB = atof((*runsIter).second[1 + 2 + 2*nTokensRow + 10].c_str());
		Float_t phiBerr = atof((*runsIter).second[1 + 2 + 2*nTokensRow + 11].c_str());
		asymY = TMath::Abs(asymY);
		asymYerr = TMath::Abs(asymYerr);
		apY = TMath::Abs(apY);
		apYerr = TMath::Abs(apYerr);
		asymB = TMath::Abs(asymB);
		asymBerr = TMath::Abs(asymBerr);
		apB = TMath::Abs(apB);
		apBerr = TMath::Abs(apBerr);
		/*Float_t apY = (polY > 0) ? (asymY / polY) : 0;
		Float_t apYerr = (polY > 0) ? (asymYerr / polY) : 0;
		Float_t apB = (polB > 0) ? (asymB / polB) : 0;
		Float_t apBerr = (polB > 0) ? (asymBerr / polB) : 0;*/
		if (hY) {
		    if (normAnalyzingPower > 0) {
			hY->SetBinContent(runIndex, apY);
			hY->SetBinError(runIndex, apYerr);
		    } else {
			hY->SetBinContent(runIndex, asymY);
			hY->SetBinError(runIndex, asymYerr);
		    }
		    hY->GetXaxis()->SetBinLabel(runIndex, runStr.c_str());
		}
		if (hB) {
		    if (normAnalyzingPower > 0) {
		        hB->SetBinContent(runIndex, apB);
			hB->SetBinError(runIndex, apBerr);
		    } else {
		        hB->SetBinContent(runIndex, asymB);
			hB->SetBinError(runIndex, asymBerr);
		    }
		    hB->GetXaxis()->SetBinLabel(runIndex, runStr.c_str());
		}
		if (hPhiY) {
		    hPhiY->SetBinContent(runIndex, phiY);
		    hPhiY->SetBinError(runIndex, phiYerr);
		    hPhiY->GetXaxis()->SetBinLabel(runIndex, runStr.c_str());
		}
		if (hPhiB) {
		    hPhiB->SetBinContent(runIndex, phiB);
		    hPhiB->SetBinError(runIndex, phiBerr);
		    hPhiB->GetXaxis()->SetBinLabel(runIndex, runStr.c_str());
		}
		runIndex++;
	    }
	    Bool_t alwaysShowPhi = true;
	    Int_t width = TMath::Min(TMath::Max(nBins * 20, 700), 1000000);
	    Int_t height = 500;
	    if ((normAnalyzingPower > 0) || alwaysShowPhi) height *= 2;
	    TCanvas *c = new TCanvas("c_asymmetry", "Asymmetry", width, height);
	    if (c) {
		if ((normAnalyzingPower > 0) || alwaysShowPhi) {
		    c->Divide(1, 2);
		}

		c->cd();
		if ((normAnalyzingPower > 0) || alwaysShowPhi) c->cd(1);
		gPad->SetBottomMargin(0.22);
		gPad->SetRightMargin(gPad->GetLeftMargin());		
		leg = new TLegend(0.7, 0.8, 0.94, 0.94);
		Float_t minY = +1;
		Float_t maxY = -1;
		if (hY) {
		    for (Int_t i = 1;i <= hY->GetXaxis()->GetNbins();i++) {
			minY = TMath::Min((Float_t)minY, (Float_t)(hY->GetBinContent(i) - hY->GetBinError(i)));
			maxY = TMath::Max((Float_t)maxY, (Float_t)(hY->GetBinContent(i) + hY->GetBinError(i)));
		    }
		}
		if (hB) {
		    for (Int_t i = 1;i <= hB->GetXaxis()->GetNbins();i++) {
			minY = TMath::Min((Float_t)minY, (Float_t)(hB->GetBinContent(i) - hB->GetBinError(i)));
			maxY = TMath::Max((Float_t)maxY, (Float_t)(hB->GetBinContent(i) + hB->GetBinError(i)));
		    }
		}
		minY = TMath::Max((Float_t)minY, (Float_t)-2.0);
		maxY = TMath::Min((Float_t)maxY, (Float_t)+2.0);
		minY = 0;
		maxY = +0.1;
		if (normAnalyzingPower > 0) maxY = normAnalyzingPower * 1.2;
		Float_t minYaxis = minY - 0.10*(maxY - minY);
		Float_t maxYaxis = maxY + 0.25*(maxY - minY);
//cout << "minY=" << minY << ", maxY=" << maxY << ", minYaxis=" << minYaxis << ", maxYaxis=" << maxYaxis << endl;
		Float_t markerSize = 1.5 * ((width > 0) ? (700.0 / width) : 1.0);
		if (hY) {
		    hY->SetTitle(TString::Format(";%s;%s", runsStr, (normAnalyzingPower > 0) ? "A_{N}" : "|#epsilon_{phys}|"));
		    hY->SetMarkerStyle(kFullTriangleUp);
		    hY->SetMarkerColor(kRed);
		    hY->SetMarkerSize(markerSize);
		    hY->SetLineColor(kRed);
		    hY->SetLineWidth(2);
		    hY->GetYaxis()->SetRangeUser(minYaxis, maxYaxis);
		    hY->GetXaxis()->LabelsOption("v");
		    hY->GetXaxis()->SetLabelSize(0.06);
		    hY->GetXaxis()->SetTitleOffset(3.0);
		    hY->SetStats(kFALSE);
		    hY->Draw("P E");
		    if (leg) leg->AddEntry(hY, "Yellow", "PL");
		}
		if (hB) {
		    hB->SetMarkerStyle(kFullTriangleDown);
		    hB->SetMarkerColor(kBlue);
		    hB->SetMarkerSize(markerSize);
		    hB->SetLineColor(kBlue);
		    hB->SetLineWidth(2);
		    hB->Draw("P E SAME");
		    if (leg) leg->AddEntry(hB, "Blue", "PL");
		}
		if (leg) {
            	    leg->SetFillColor(0);
            	    leg->SetFillStyle(0);
            	    leg->SetLineColor(0);
            	    leg->Draw();
		}
		lineZero = new TLine(-0.5, 0, nBins - 0.5, 0);
		if (lineZero) {
		    lineZero->SetLineColor(kBlack);
		    lineZero->SetLineWidth(1);
		    lineZero->Draw();
		}
		if (normAnalyzingPower > 0) {
		    axisTrans = new TGaxis(nBins - 0.5, minYaxis, nBins - 0.5, maxYaxis, minYaxis / normAnalyzingPower, maxYaxis / normAnalyzingPower, 510, "+L");
		    axisTrans->SetTitle(TString::Format("Fractional transverse component (A_{N} = %.1f%%)", normAnalyzingPower * 100.0));
		    axisTrans->SetTitleOffset(1.2);
		    axisTrans->Draw();
		    lineUnity = new TLine(-0.5, normAnalyzingPower, nBins - 0.5, normAnalyzingPower);
		    if (lineUnity) {
			lineUnity->SetLineColor(kBlack);
			lineUnity->SetLineWidth(1);
			lineUnity->Draw();
		    }
		}

		if ((normAnalyzingPower > 0) || alwaysShowPhi) {
		c->cd(2);
		gPad->SetBottomMargin(0.22);
		gPad->SetRightMargin(gPad->GetLeftMargin());		
		legPhi = new TLegend(0.7, 0.8, 0.94, 0.94);
		Float_t markerSize = 1.5 * ((width > 0) ? (700.0 / width) : 1.0);
		if (hPhiY) {
		    hPhiY->SetTitle(TString::Format(";%s;%s", runsStr, "Azimuthal angle between spin and vertical"));
		    hPhiY->SetMarkerStyle(kFullTriangleUp);
		    hPhiY->SetMarkerColor(kRed);
		    hPhiY->SetMarkerSize(markerSize);
		    hPhiY->SetLineColor(kRed);
		    hPhiY->SetLineWidth(2);
		    //hPhiY->GetYaxis()->SetRangeUser(-TMath::Pi(), +TMath::Pi());
		    hPhiY->GetYaxis()->SetRangeUser(-TMath::Pi(), +5);
		    //hPhiY->GetYaxis()->SetRangeUser(-2.0, +2.0);
		    hPhiY->GetXaxis()->LabelsOption("v");
		    hPhiY->GetXaxis()->SetLabelSize(0.06);
		    hPhiY->GetXaxis()->SetTitleOffset(3.0);
		    hPhiY->SetStats(kFALSE);
		    hPhiY->Draw("P E");
		    if (legPhi) legPhi->AddEntry(hPhiY, "Yellow", "PL");
		}
		if (hPhiB) {
		    hPhiB->SetMarkerStyle(kFullTriangleDown);
		    hPhiB->SetMarkerColor(kBlue);
		    hPhiB->SetMarkerSize(markerSize);
		    hPhiB->SetLineColor(kBlue);
		    hPhiB->SetLineWidth(2);
		    hPhiB->Draw("P E SAME");
		    if (legPhi) legPhi->AddEntry(hPhiB, "Blue", "PL");
		}
		if (legPhi) {
            	    legPhi->SetFillColor(0);
            	    legPhi->SetFillStyle(0);
            	    legPhi->SetLineColor(0);
            	    legPhi->Draw();
		}
		linePhiUp = new TLine(-0.5, 0, nBins - 0.5, 0);
		if (linePhiUp) {
		    linePhiUp->SetLineColor(kBlack);
		    linePhiUp->SetLineWidth(1);
		    linePhiUp->Draw();
		}
		linePhiNorth = new TLine(-0.5, +TMath::Pi()/2.0, nBins - 0.5, +TMath::Pi()/2.0);
		if (linePhiNorth) {
		    linePhiNorth->SetLineColor(kBlack);
		    linePhiNorth->SetLineWidth(1);
		    linePhiNorth->Draw();
		}
		linePhiSouth = new TLine(-0.5, -TMath::Pi()/2.0, nBins - 0.5, -TMath::Pi()/2.0);
		if (linePhiSouth) {
		    linePhiSouth->SetLineColor(kBlack);
		    linePhiSouth->SetLineWidth(1);
		    linePhiSouth->Draw();
		}
		linePhiDown = new TLine(-0.5, +TMath::Pi(), nBins - 0.5, +TMath::Pi());
		if (linePhiDown) {
		    linePhiDown->SetLineColor(kBlack);
		    linePhiDown->SetLineWidth(1);
		    linePhiDown->Draw();
		}
		labelPhiUp = new TLatex(0, 0 + 0.05, "Up = 0");
		if (labelPhiUp) {
		    //labelPhiUp->SetTextAlign(21);
		    labelPhiUp->Draw();
		}
		labelPhiNorth = new TLatex(0, (+TMath::Pi()/2.0) + 0.05, "North = +#pi/2");
		if (labelPhiNorth) {
		    //labelPhiNorth->SetTextAlign(21);
		    labelPhiNorth->Draw();
		}
		labelPhiSouth = new TLatex(0, (-TMath::Pi()/2.0) + 0.05, "South = -#pi/2");
		if (labelPhiSouth) {
		    //labelPhiSouth->SetTextAlign(21);
		    labelPhiSouth->Draw();
		}
		labelPhiDown1 = new TLatex(0, (+TMath::Pi()) + 0.05, "Down = +#pi");
		if (labelPhiDown1) {
		    //labelPhiDown1->SetTextAlign(21);
		    labelPhiDown1->Draw();
		}
		labelPhiDown2 = new TLatex(0, (-TMath::Pi()) + 0.05, "Down = -#pi");
		if (labelPhiDown2) {
		    //labelPhiDown2->SetTextAlign(21);
		    labelPhiDown2->Draw();
		}
		}
		if (outputFilenameGif && outputFilenameGif[0]) c->SaveAs(outputFilenameGif);
		if (outputFilenameC && outputFilenameC[0]) c->SaveAs(outputFilenameC);
	    }
	    if (hY) delete hY;
	    if (hB) delete hB;
	    if (leg) delete leg;
	    if (axisTrans) delete axisTrans;
	    if (lineZero) delete lineZero;
	    if (lineUnity) delete lineUnity;
	    if (hPhiY) delete hPhiY;
	    if (hPhiB) delete hPhiB;
	    if (legPhi) delete legPhi;
	    if (linePhiUp) delete linePhiUp;
	    if (linePhiNorth) delete linePhiNorth;
	    if (linePhiSouth) delete linePhiSouth;
	    if (linePhiDown) delete linePhiDown;
	    if (labelPhiUp) delete labelPhiUp;
	    if (labelPhiNorth) delete labelPhiNorth;
	    if (labelPhiSouth) delete labelPhiSouth;
	    if (labelPhiDown1) delete labelPhiDown1;
	    if (labelPhiDown2) delete labelPhiDown2;
	    if (c) delete c;
	} else {
	    LOG_ERROR << "Cannot read file " << inputFilename << endm;
	}
    }
}
