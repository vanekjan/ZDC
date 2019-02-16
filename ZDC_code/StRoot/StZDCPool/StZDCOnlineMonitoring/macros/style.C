#if !defined(__CINT__) || defined(__MAKECINT__) 

#include <TStyle.h> 
#include <TAttFill.h> 
#include <TColor.h> 

#endif 

void style() {
	Info("style", "Applying my style...");
        gStyle->SetPalette(1);
        //gStyle->SetOptStat(0);
        //gStyle->SetFillColor(kWhite);
        gStyle->SetCanvasColor(kWhite);
        gStyle->SetHistFillColor(kWhite);
        gStyle->SetTitleFillColor(kWhite);
        gStyle->SetHistLineColor(kBlack);
        gStyle->SetFrameFillColor(kWhite);
        gStyle->SetFrameLineColor(kBlack);
        gStyle->SetFrameBorderSize(1);
        //gStyle->SetFrameBorderMode(0);
        gStyle->SetPadColor(kWhite);
        gStyle->SetStatColor(kWhite);
        gStyle->SetTitleOffset(1.2, "y");

        gStyle->SetPadLeftMargin(0.13);
        gStyle->SetPadRightMargin(0.07);
        gStyle->SetPadTopMargin(0.07);
        gStyle->SetPadBottomMargin(0.13);

	//gStyle->SetEndErrorSize(5);
}
