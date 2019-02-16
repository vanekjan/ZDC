#include "TFile.h"
#include "TTree.h"
#include "TH2.h"
#include "TProfile.h"
#include<iostream>

using namespace std;


void analyse_prepost(TString input = "run_19084043.histo.root"){
//    TString input = "run_19084043.histo.root";
    TFile* data = new TFile(input ,"r");
    TTree* zdcT = (TTree*)data -> Get("zdcTree");

    TString side[2] = {"West", "East"};
    TString varName2d;
    Int_t nAdcBins = 4000;
    const int nPreposts = 8;
    TString varName1d;
    TH2D* h[2][3];
    TH1D* hAdc[2][3][8];
    TH1D* towerAdcPrepost[2][3];

    TList *listOut2d = new TList();
    TList *listOutPrepost = new TList();
    TList *listOutAdc = new TList();

    Double_t totalAdcPrepost[nPreposts];

    for (int i = 0; i < 2; ++i) { // west and east
        for (int j = 1; j < 4; ++j) { // 3 PMTs/towers
            varName1d = "zdc_ADC_" + side[i] + Form(+"Tow%i", j);
            varName2d = varName1d + ":prepost_event";
            cout<<varName2d<<endl;
            h[i][j-1] = new TH2D(varName2d, varName2d, 7, -0.5, 6.5, nAdcBins, -0.5, nAdcBins-0.5);
            h[i][j-1] -> SetXTitle("prepost");
            h[i][j-1] -> SetYTitle("ADC");

            zdcT -> Project(varName2d, varName2d);
            listOut2d -> Add(h[i][j-1]);

            towerAdcPrepost[i][j-1] = new TH1D(varName1d, varName1d, nPreposts, -0.5, nPreposts-0.5);
            towerAdcPrepost[i][j-1] -> SetXTitle("prepost");
            towerAdcPrepost[i][j-1] -> SetYTitle("total ADC in run");

            for (Int_t p = 0; p < nPreposts; ++p) {
                totalAdcPrepost[p] = 0;
                hAdc[i][j-1][p] = new TH1D(varName1d + Form(" prepost_event=%i", p), varName1d + Form(" prepost_event=%i", p), nAdcBins, -0.5, nAdcBins-0.5);
                hAdc[i][j-1][p] -> SetXTitle("ADC");
                zdcT -> Project(varName1d + Form(" prepost_event=%i", p), varName1d, Form("prepost_event==%i", p));
                listOutAdc -> Add(hAdc[i][j-1][p]);
                for (int k = 0; k < nAdcBins; ++k) {
                    totalAdcPrepost[p] += (hAdc[i][j-1][p] -> GetBinContent(k)) * (hAdc[i][j-1][p] -> GetBinCenter(k));
                }
                towerAdcPrepost[i][j-1] -> SetBinContent(p, totalAdcPrepost[p]);
                listOutPrepost -> Add(towerAdcPrepost[i][j-1]);
            }
        }
    }

    TFile* dataOut = new TFile("prepost_analyse_"+input ,"recreate");
    listOutAdc -> Write("ADC",1, 0);
    listOut2d -> Write("ADC_vs_prepost", 1, 0);
    listOutPrepost -> Write("total_ADC_vs_prepost", 1, 0);

    dataOut -> Close();
    data -> Close();
}