#include <iostream>
#include <fstream>
using namespace std;
#include <math.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include "TTree.h"
#include <TCanvas.h>
#include <TLine.h>
#include <TArrow.h>
#include <TLatex.h>
#include <TString.h>
#include <TBox.h>
#include <TObjArray.h>
#include <TDatime.h>

#include <StMessMgr.h>
#include <StTriggerData.h>

#include <StZDCUtil/StZDCDb.h>
#include <StZDCUtil/StZDCGeom.h>

#include "ZDCPlotsNames.h"
#include "ZDCPlots.h"

ZDCPlots *ZDCPlotsInstance = 0;

ClassImp(ZDCPlots)

float rawThresh = 100.0; // ADC counts
float pedThresh = 5.0; // ADC counts above pedestal
float smdSumThresh = 50; // ADC counts above pedestal
float towerThresh = 12; // ADC counts above pedestal
float towerSumThresh = 40; // ADC counts above pedestal

int goodHitADCthreshold = 25;
int goodHitMinTDC = 40;
int goodHitMaxTDC = 4000;

//-------------------------------------------------------------------
void ZDCPlots::initHisto(TObjArray *list, const Char_t *zdcPedCal, Bool_t useShiftCrewPlots, Bool_t useExpertPlots) {
	LOG_DEBUG << "initHisto begin" << endm;
	if (ZDCPlotsInstance) delete ZDCPlotsInstance; ZDCPlotsInstance = 0;
	ZDCPlotsInstance = new ZDCPlots();
	LOG_DEBUG << "instance created " << ZDCPlotsInstance << endm;
	if (ZDCPlotsInstance) {
		ZDCPlotsInstance->setShiftCrewPlots(useShiftCrewPlots);
		ZDCPlotsInstance->setExpertPlots(useExpertPlots);
		LOG_DEBUG << "initializing..." << endm;
		ZDCPlotsInstance->init();
		ZDCPlotsInstance->getHisto(list);
		LOG_DEBUG << "instance initialized" << endm;
	}
	LOG_DEBUG << "initHisto end" << endm;
}
//-------------------------------------------------------------------
void ZDCPlots::resetHisto(const Char_t *zdcPedCal, int date, int time) {
	if (ZDCPlotsInstance) {
		ZDCPlotsInstance->clear();
		ZDCPlotsInstance->setTextDb(zdcPedCal);
		ZDCPlotsInstance->initRun(date, time);
	}
}
//-------------------------------------------------------------------
void ZDCPlots::saveHisto(TFile *hfile) {
	if (ZDCPlotsInstance) {
		ZDCPlotsInstance->saveHistograms(hfile);
	}
}
//-------------------------------------------------------------------
void ZDCPlots::fillHisto(const StTriggerData *td, int date, int time, int prepost) {
	if (ZDCPlotsInstance) {
		ZDCPlotsInstance->processEvent(td, date, time, prepost);
	}
}
//-------------------------------------------------------------------
ZDCPlots::ZDCPlots(const Char_t *name)
		: TDataSet(name)
		, mShiftCrewPlots(true)
		, mExpertPlots(false)
		, mDb(new StZDCDb())
		, mZdcPedCal(0)
		, mGeom(new StZDCGeom())
		, m_h76_zdc_time_east(0)
		, m_h77_zdc_time_west(0)
		, m_h78_zdc_timediff_east_west(0)
		, m_h146_zdc_Vertex_cm(0)
		, m_zdc_ADC(0)
		, m_zdc_ADCCorr(0)
		, m_zdc_TDC(0)
		, m_zdc_TDCdiff(0)
		, m_hADC_EastWestSum(0)
		, m_hADC_EWSumZDCvsBBC(0)
		, m_hADC_EWSumZDCvsTOFMult(0)
		, m_zdcsmd_ADC(0)
		, m_zdcsmd_Occupancy(0)
		, m_zdcsmd_ADCCorr(0)
		, m_zdcsmd_OccupancyCorr(0)
		, m_zdcdsm_earliestTDC(0)
		, m_zdcdsm_TDCdiff(0)
		, m_zdcdsm_TDCdiffL1L2(0)
		, m_zdcdsm_TDCdiffL3(0)
		, m_zdcdsm_ADCthresh(0)
		, m_zdcdsm_ADCthreshL2(0)
		, m_zdcdsm_ADCthreshL3(0)
		, m_zdc_ADCprepost(0)
		, m_zdcsmd_ADCprepost(0)
		, m_zdc_test(0)
{
	memset(this->m_zdcsmd_N, 0, sizeof(this->m_zdcsmd_N));
	memset(this->m_zdcsmd_A, 0, sizeof(this->m_zdcsmd_A));
	memset(this->m_zdcsmd_MaxXYCorr, 0, sizeof(this->m_zdcsmd_MaxXYCorr));
	memset(this->m_zdcsmd_MaxXYCorrRatio, 0, sizeof(this->m_zdcsmd_MaxXYCorrRatio));
	memset(this->m_zdc_ADCTDC, 0, sizeof(this->m_zdc_ADCTDC));
	memset(this->m_zdc_FrontBackSum, 0, sizeof(this->m_zdc_FrontBackSum));
	memset(this->m_zdcsmd_SmdSumTowerSum, 0, sizeof(this->m_zdcsmd_SmdSumTowerSum));
	memset(this->m_zdcsmd_SmdSumTowerSumRatio, 0, sizeof(this->m_zdcsmd_SmdSumTowerSumRatio));
	memset(this->m_zdcdsm_earliestTDCMaxTDC, 0, sizeof(this->m_zdcdsm_earliestTDCMaxTDC));
	memset(this->m_zdc_ADCprepostCorrelation, 0, sizeof(this->m_zdc_ADCprepostCorrelation));
	memset(this->m_zdcsmd_ADCprepostCorrelation, 0, sizeof(this->m_zdcsmd_ADCprepostCorrelation));
}
//-------------------------------------------------------------------
ZDCPlots::~ZDCPlots() {
	this->setShiftCrewPlots(true);
	this->setExpertPlots(true);
	TObjArray list;
	this->getHisto(&list, false);
	for (Int_t i = 0;i < list.GetEntries();i++) {
		TH1 *h = static_cast<TH1*>(list.At(i));
		if (h) delete h;
	}
	if (this->mGeom) delete this->mGeom;
	if (this->mDb) delete this->mDb;

	delete mTree;
}
//-------------------------------------------------------------------
void ZDCPlots::init() {
	//Added by Chitta
	zdc_e_count = 0;

	mTree = new TTree("zdcTree", "ROOT Tree for ZDC monitoring");


	/*
    for (int eastwest = 0; eastwest < 2;eastwest++)
      for (int tower = 0;tower < 4;tower++) {
        TString towerName = (tower == 0) ? "Sum" : TString::Format("Tower %i", tower);
        TString zdc_ADC_BrName = TString::Format("%s_%s_%i", zdc_ADC, eastwest ? "west" : "east", tower);
        TString zdc_TDC_BrName = TString::Format("%s_%s_%i", zdc_TDC, eastwest ? "west" : "east", tower);

        //this->m_zdc_ADCTDC[eastwest][tower] = new TH2F(TString::Format("%s_%s_%i", zdc_ADCTDCName, eastwest ? "west" : "east", tower), TString::Format("ZDC TAC vs. ADC, %s %s;ADC;TDC", eastwest ? "West" : "East", towerName.Data()), 256, 0, 4096, 256, 0, 4096);

        mTree->Branch(zdc_ADC_BrName,&zdc_ADC_BrName,zdc_ADC_BrName/F);
        mTree->Branch(zdc_TDC_BrName,&zdc_TDC_BrName,zdc_TDC_BrName/F);
        }
    */

	mTree->Branch("zdc_ADC_EastSum_Attenuated",&zdc_ADC_EastSum_Attenuated,"zdc_ADC_EastSum_Attenuated/s");
	mTree->Branch("zdc_ADC_EastSum",&zdc_ADC_EastSum,"zdc_ADC_EastSum/s");
	mTree->Branch("zdc_ADC_EastTow1",&zdc_ADC_EastTow1,"zdc_ADC_EastTow1/s");
	mTree->Branch("zdc_ADC_EastTow2",&zdc_ADC_EastTow2,"zdc_ADC_EastTow2/s");
	mTree->Branch("zdc_ADC_EastTow3",&zdc_ADC_EastTow3,"zdc_ADC_EastTow3/s");

	mTree->Branch("zdc_ADC_WestSum_Attenuated",&zdc_ADC_WestSum_Attenuated,"zdc_ADC_WestSum_Attenuated/s");
	mTree->Branch("zdc_ADC_WestSum",&zdc_ADC_WestSum,"zdc_ADC_WestSum/s");
	mTree->Branch("zdc_ADC_WestTow1",&zdc_ADC_WestTow1,"zdc_ADC_WestTow1/s");
	mTree->Branch("zdc_ADC_WestTow2",&zdc_ADC_WestTow2,"zdc_ADC_WestTow2/s");
	mTree->Branch("zdc_ADC_WestTow3",&zdc_ADC_WestTow3,"zdc_ADC_WestTow3/s");

	mTree->Branch("zdc_TDC_EastSum",&zdc_TDC_EastSum,"zdc_TDC_EastSum/s");
	mTree->Branch("zdc_TDC_EastTow1",&zdc_TDC_EastTow1,"zdc_TDC_EastTow1/s");
	mTree->Branch("zdc_TDC_EastTow2",&zdc_TDC_EastTow2,"zdc_TDC_EastTow2/s");
	mTree->Branch("zdc_TDC_EastTow3",&zdc_TDC_EastTow3,"zdc_TDC_EastTow3/s");

	mTree->Branch("zdc_TDC_WestSum",&zdc_TDC_WestSum,"zdc_TDC_WestSum/s");
	mTree->Branch("zdc_TDC_WestTow1",&zdc_TDC_WestTow1,"zdc_TDC_WestTow1/s");
	mTree->Branch("zdc_TDC_WestTow2",&zdc_TDC_WestTow2,"zdc_TDC_WestTow2/s");
	mTree->Branch("zdc_TDC_WestTow3",&zdc_TDC_WestTow3,"zdc_TDC_WestTow3/s");

	mTree->Branch("zdc_ADC_EWSum",&zdc_ADC_EWSum,"zdc_ADC_EWSum/s");

	mTree->Branch("tof_multiplicity",&tof_multiplicity,"tof_multiplicity/s");

	mTree->Branch("bbc_ADC_EastSum_SmallTile",&bbc_ADC_EastSum_SmallTile,"bbc_ADC_EastSum_SmallTile/s");
	mTree->Branch("bbc_ADC_EastSum_LargeTile",&bbc_ADC_EastSum_LargeTile,"bbc_ADC_EastSum_LargeTile/s");

	mTree->Branch("bbc_ADC_WestSum_SmallTile",&bbc_ADC_WestSum_SmallTile,"bbc_ADC_WestSum_SmallTile/s");
	mTree->Branch("bbc_ADC_WestSum_LargeTile",&bbc_ADC_WestSum_LargeTile,"bbc_ADC_WestSum_LargeTile/s");

	mTree->Branch("zdc_smd_east_ver", zdc_smd_east_ver, "zdc_smd_east_ver[8]/S");
	mTree->Branch("zdc_smd_east_hor", zdc_smd_east_hor, "zdc_smd_east_hor[8]/S");
	mTree->Branch("zdc_smd_west_ver", zdc_smd_west_ver, "zdc_smd_west_ver[8]/S");
	mTree->Branch("zdc_smd_west_hor", zdc_smd_west_hor, "zdc_smd_west_hor[8]/S");

	mTree->Branch("trgToken", &trgToken, "trgToken/s");
	mTree->Branch("token", &token, "token/s");
	mTree->Branch("actionWord", &actionWord, "actionWord/s");
	mTree->Branch("l2sum", &l2sum, "l2sum/s");
	mTree->Branch("prepost_event", &prepost_event, "prepost_event/s");

	//mTree->Branch("bbc_ADC_EastSum",&bbc_ADC_EastSum,"bbc_ADC_EastSum/s");
	//mTree->Branch("bbc_ADC_WestSum",&bbc_ADC_WestSum,"bbc_ADC_WestSum/s");

	//==================================



	this->m_zdcTotEvents = new TH1F(zdcTotEventsName, "Total number of events with ZDC", 1, 0, 1);
	if (this->getShiftCrewPlots()) {
		this->m_h76_zdc_time_east = new TH1F(h76_zdc_time_eastName, "ZDC Time East", 256, 0., 4096.);
		this->m_h77_zdc_time_west = new TH1F(h77_zdc_time_westName, "ZDC Time West", 256, 0., 4096.);
		this->m_h78_zdc_timediff_east_west = new TH1F(h78_zdc_timediff_east_westName, "ZDC Time (West - East)", 256, -2048., +2048.);
		this->m_h146_zdc_Vertex_cm = new TH1F(h146_zdc_Vertex_cmName, "Vertex Position from ZDC (cm);z, cm", 50, -100., +100.);
		this->m_zdc_ADC = new TH2F(zdc_ADCName, "ZDC Raw Tower ADC;Tower;ADC", 8, 0, 8, 4096, 0, 4096);
		this->m_zdc_ADCCorr = new TH2F(zdc_ADCCorrName, "ZDC Corrected Tower ADC;Tower;(ADC - PED) * Calib", 8, 0, 8, 1100, -100, 1000);
//	this->m_hADC_EastWestSum = new TH1F(hADC_EastWestSumName, "ZDC East+West ADC Sum", 300, 0., 3000.);
//	this->m_hADC_EWSumZDCvsBBC = new TH2F(hADC_EWSumZDCvsBBCName, "ZDC E+W ADC Sum vs. BBC E+W ADC Sum", 600, 0., 60000., 300, 0., 3000.);

		this->m_hADC_EastWestSum = new TH1F(hADC_EastWestSumName, "ZDC East+West ADC Sum", 200, 0.,2000.);//yhzhu
		this->m_hADC_EWSumZDCvsBBC = new TH2F(hADC_EWSumZDCvsBBCName, "ZDC E+W ADC Sum vs. BBC E+W ADC Sum", 800, 0., 80000., 200, 0., 2000.);//yhzhu
		this->m_hADC_EWSumZDCvsTOFMult = new TH2F(hADC_EWSumZDCvsTOFMultName, "ZDC East+West ADC Sum vs TOF multiplicity", 200, 0.,2000., 200, 0.,2000.);//yhzhu

		for (int eastwest = 0;eastwest < 2;eastwest++)
			for (int horiz = 0;horiz < 2;horiz++) {
				this->m_zdcsmd_N[eastwest][horiz] = new TH1F(TString::Format(zdcsmd_NName "_%s_%s", eastwest ? "west" : "east", horiz ? "horiz" : "vert"), TString::Format("ZDC SMD Occupancy, %s %s (ADC > PED + %i)", eastwest ? "west" : "east", horiz ? "horiz" : "vert", Int_t(pedThresh)), 8, 0.5, 8.5);
				this->m_zdcsmd_A[eastwest][horiz] = new TH1F(TString::Format(zdcsmd_AName "_%s_%s", eastwest ? "west" : "east", horiz ? "horiz" : "vert"), TString::Format("ZDC SMD Amplitude, %s %s (ADC > PED + %i)", eastwest ? "west" : "east", horiz ? "horiz" : "vert", Int_t(pedThresh)), 8, 0.5, 8.5);
			}
	}
	if (this->getExpertPlots()) {

		//this->m_zdcEastWest_ADCSum = new TH1F(zdcsmd_ADCName, "ZDC East+West ADC Sum",  4096, 0, 4096);//CJena
		//this->m_zdcEWvsbbcEW_ADCSum = new TH1F(zdcsmd_ADCName, "ZDC EW ADC Sum vs. BBC EW ADC Sum",  40000, 0, 40000, 4096, 0, 4096);//CJena

		this->m_zdcsmd_ADC = new TH2F(zdcsmd_ADCName, "ZDC SMD Raw ADC;Slat(Left-East,Right-West);ADC", 32, 0 + 0.5, 32 + 0.5, 4096, 0, 4096);
		this->m_zdcsmd_Occupancy = new TH1F(zdcsmd_OccupancyName, TString::Format("ZDC SMD Occupancy (ADC > %i);Slat;Hits", Int_t(rawThresh)), 32, 0 + 0.5, 32 + 0.5);
		this->m_zdcsmd_ADCCorr = new TH2F(zdcsmd_ADCCorrName, "ZDC SMD Corrected ADC;Slat;(ADC - Ped) / Gain", 32, 0 + 0.5, 32 + 0.5, 4096 + 20, -20, 4096);
		this->m_zdcsmd_OccupancyCorr = new TH1F(zdcsmd_OccupancyCorrName, TString::Format("ZDC SMD Corrected Occupancy (ADC > PED + %i);Slat;Hits", Int_t(pedThresh)), 32, 0 + 0.5, 32 + 0.5);
		for (int eastwest = 0;eastwest < 2;eastwest++) {
			this->m_zdcsmd_MaxXYCorr[eastwest] = new TH2F(TString::Format(zdcsmd_MaxXYCorrName "_%s", eastwest ? "west" : "east"), TString::Format("Max X vs. Y Corrected ADC (%s);Max Corrected ADC (vert);Max Corrected ADC (horiz)", eastwest ? "west" : "east"), 70, -10, 200, 70, -10, 200);
			this->m_zdcsmd_MaxXYCorrRatio[eastwest] = new TH1F(TString::Format(zdcsmd_MaxXYCorrRatioName "_%s", eastwest ? "west" : "east"), TString::Format("Max X / Max Y Corrected ADC %s (ADC > PED + %i);Max vert corrected ADC / Max horiz corrected ADC", eastwest ? "West" : "East", Int_t(pedThresh)), 150, 0, 3);
		}
		this->m_zdc_TDC = new TH2F(zdc_TDCName, "ZDC Tower TDC;Tower;TDC", 8, 0, 8, 4096, 0, 4096);
		this->m_zdc_TDCdiff = new TH2F(zdc_TDCdiffName, "ZDC Tower #DeltaTDC;[E,W][S-1,S-2,S-3,1-2,1-3,2-3];#DeltaTDC", 12, 0, 12, 4096, -2048, 2048);
		for (int eastwest = 0;eastwest < 2;eastwest++)
			for (int tower = 0;tower < 4;tower++) {
				TString towerName = (tower == 0) ? "Sum" : TString::Format("Tower %i", tower);
				this->m_zdc_ADCTDC[eastwest][tower] = new TH2F(TString::Format("%s_%s_%i", zdc_ADCTDCName, eastwest ? "west" : "east", tower), TString::Format("ZDC TAC vs. ADC, %s %s;ADC;TDC", eastwest ? "West" : "East", towerName.Data()), 256, 0, 4096, 256, 0, 4096);
			}
		for (int eastwest = 0;eastwest < 2;eastwest++) {
			this->m_zdc_FrontBackSum[eastwest] = new TH2F(TString::Format(zdc_FrontBackSumName "_%s", eastwest ? "west" : "east"), TString::Format("ZDC Front vs. Back ADC Sum, %s;Front Sum;Back Sum", eastwest ? "West" : "East"), 200, 0, 1000, 200, 0, 1000);
		}
		for (int eastwest = 0;eastwest < 2;eastwest++)
			for (int horiz = 0;horiz < 2;horiz++) {
				this->m_zdcsmd_SmdSumTowerSum[eastwest][horiz] = new TH2F(TString::Format(zdcsmd_SmdSumTowerSumName "_%s_%s", eastwest ? "west" : "east", horiz ? "horiz" : "vert"), TString::Format("ZDC Corrected SMD Sum vs. Corrected Tower Sum, %s %s;Tower ADC Sum;SMD ADC Sum", eastwest ? "West" : "East", horiz ? "Horiz" : "Vert"), 100, -200, 2000, 100, -300, 3000);
				this->m_zdcsmd_SmdSumTowerSumRatio[eastwest][horiz] = new TH1F(TString::Format(zdcsmd_SmdSumTowerSumRatioName "_%s_%s", eastwest ? "west" : "east", horiz ? "horiz" : "vert"), TString::Format("ZDC Corrected SMD Sum / Corrected Tower Sum, %s %s (SMD Sum > %i and Tower Sum > %i);SMD Sum / Tower Sum", eastwest ? "West" : "East", horiz ? "Horiz" : "Vert", Int_t(smdSumThresh), Int_t(towerSumThresh)), 200, 0, 2);
			}
		this->m_zdcdsm_earliestTDC = new TH2F(zdcdsm_earliestTDCName, "ZDC DSM Earliest TDC;East,West;ZD101 Input: Max TAC", 2, 0, 2, 4096, 0, 4096);
		this->m_zdcdsm_TDCdiff = new TH2F(zdcdsm_TDCdiffName, "ZDC Vertex DSM  #DeltaTDC vs Tower TDC East - West;Tower Max TDC East - West;VT201 Input:  #DeltaTDC", 255, -2048, +2048, 256, 0, 1024);
		this->m_zdcdsm_TDCdiffL1L2 = new TH2F(zdcdsm_TDCdiffL1L2Name, "ZDC Vertex DSM  #DeltaTDC vs Earliest TDC East - West;ZD101 Input Max TAC East - West;VT201 Input:  #DeltaTDC", 255, -2048, +2048, 256, 0, 1024);
		this->m_zdcdsm_TDCdiffL3 = new TH2F(zdcdsm_TDCdiffL3Name, "ZDC Last DSM  #DeltaTDC in window?;Tower Max TDC East - West;LD301 Input:  #DeltaTDC in window", 255, -2048, +2048, 2, 0, 2);
		for (int eastwest = 0;eastwest < 2;eastwest++) {
			this->m_zdcdsm_earliestTDCMaxTDC[eastwest] = new TH2F(TString::Format("%s_%s", zdcdsm_earliestTDCName, eastwest ? "west" : "east"), TString::Format("ZDC DSM Earliest TDC vs. Max Tower TDC, %s;Max Tower TDC;ZD101 Input: Max TAC", eastwest ? "West" : "East"), 256, 0, 4096, 256, 0, 4096);
		}
		this->m_zdcdsm_ADCthresh = new TH2F(zdcdsm_ADCthreshName, "ZDC DSM ADC <> threshold;[East,West][Sum,Front,Back][<,>];ADC", 12, 0, 12, 4096, 0, 4096);
		this->m_zdcdsm_ADCthreshL2 = new TH2F(zdcdsm_ADCthreshL2Name, "ZDC DSM ADC <> threshold, L1 != L2;[East,West];[Sum,Front,Back][==,!=]", 2, 0, 2, 6, 0, 6);
		this->m_zdcdsm_ADCthreshL3 = new TH2F(zdcdsm_ADCthreshL3Name, "ZDC DSM ADC <> threshold, L2 != L3;[East,West];[Sum,Front,Back][==,!=]", 2, 0, 2, 6, 0, 6);

		this->m_zdc_ADCprepost = new TH2F(zdc_ADCprepostName, "ZDC Tower ADC Corr Sum vs. Pre-Post;Pre-Post;Tower (ADC-PED)*Calib Sum East + West", 11, -5.5, +5.5, 250, 0, 1500);
		this->m_zdcsmd_ADCprepost = new TH2F(zdcsmd_ADCprepostName, "ZDC SMD ADC Corr Sum vs. Pre-Post;Pre-Post;SMD (ADC-PED)*Calib/Gain Sum East + West", 11, -5.5, +5.5, 250, 0, 1500);
		for (int p = 0;p < 10;p++) {
			this->m_zdc_ADCprepostCorrelation[p] = new TH2F(TString::Format("%s_%i", zdc_ADCprepostCorrelationName, p), TString::Format("ZDC Tower PrePost %+i vs %+i;Tower prepost=%+i;Tower prepost=%+i", p-4, p-5, p-5, p-4), 200, -50, 1000, 200, -50, 200);
			this->m_zdcsmd_ADCprepostCorrelation[p] = new TH2F(TString::Format("%s_%i", zdcsmd_ADCprepostCorrelationName, p), TString::Format("ZDC SMD PrePost %+i vs %+i;SMD prepost=%+i;SMD prepost=%+i", p-4, p-5, p-5, p-4), 200, -50, 1500, 200, -50, 200);
		}

		this->m_zdc_test = new TH2F(zdc_testName, "Test;SMD Pre-0 (> 50);strip", 200, 0, 200, 32, 0, 32);
	}
}
//-------------------------------------------------------------------
void ZDCPlots::getHisto(TObjArray *list, Bool_t inherited) const
{
#define ADDHIST(hist) if ((list) && (hist)) (list)->Add(hist);
	ADDHIST(this->m_zdcTotEvents);
	if (this->getShiftCrewPlots())
	{
		ADDHIST(this->m_h76_zdc_time_east);
		ADDHIST(this->m_h77_zdc_time_west);
		ADDHIST(this->m_h78_zdc_timediff_east_west);
		ADDHIST(this->m_h146_zdc_Vertex_cm);
		ADDHIST(this->m_zdc_ADC);
		ADDHIST(this->m_zdc_ADCCorr);

		ADDHIST(this->m_hADC_EastWestSum);
		ADDHIST(this->m_hADC_EWSumZDCvsBBC);
		ADDHIST(this->m_hADC_EWSumZDCvsTOFMult);

		for (int eastwest = 0;eastwest < 2;eastwest++)
			for (int horiz = 0;horiz < 2;horiz++)
			{
				ADDHIST(this->m_zdcsmd_N[eastwest][horiz]);
				ADDHIST(this->m_zdcsmd_A[eastwest][horiz]);
			}
	}
	if (this->getExpertPlots()) {
		ADDHIST(this->m_zdcsmd_ADC);
		ADDHIST(this->m_zdcsmd_Occupancy);
		ADDHIST(this->m_zdcsmd_ADCCorr);
		ADDHIST(this->m_zdcsmd_OccupancyCorr);
		for (int eastwest = 0;eastwest < 2;eastwest++) {
			ADDHIST(this->m_zdcsmd_MaxXYCorr[eastwest]);
			ADDHIST(this->m_zdcsmd_MaxXYCorrRatio[eastwest]);
		}
		ADDHIST(this->m_zdc_TDC);
		ADDHIST(this->m_zdc_TDCdiff);
		for (int eastwest = 0;eastwest < 2;eastwest++)
			for (int tower = 0;tower < 4;tower++) {
				ADDHIST(this->m_zdc_ADCTDC[eastwest][tower]);
			}
		for (int eastwest = 0;eastwest < 2;eastwest++) {
			ADDHIST(this->m_zdc_FrontBackSum[eastwest]);
		}
		for (int eastwest = 0;eastwest < 2;eastwest++)
			for (int horiz = 0;horiz < 2;horiz++) {
				ADDHIST(this->m_zdcsmd_SmdSumTowerSum[eastwest][horiz]);
				ADDHIST(this->m_zdcsmd_SmdSumTowerSumRatio[eastwest][horiz]);
			}
		ADDHIST(this->m_zdcdsm_earliestTDC);
		ADDHIST(this->m_zdcdsm_TDCdiff);
		ADDHIST(this->m_zdcdsm_TDCdiffL1L2);
		ADDHIST(this->m_zdcdsm_TDCdiffL3);
		for (int eastwest = 0;eastwest < 2;eastwest++) {
			ADDHIST(this->m_zdcdsm_earliestTDCMaxTDC[eastwest]);
		}
		ADDHIST(this->m_zdcdsm_ADCthresh);
		ADDHIST(this->m_zdcdsm_ADCthreshL2);
		ADDHIST(this->m_zdcdsm_ADCthreshL3);

		ADDHIST(this->m_zdc_ADCprepost);
		ADDHIST(this->m_zdcsmd_ADCprepost);
		for (int p = 0;p < 10;p++) {
			ADDHIST(this->m_zdc_ADCprepostCorrelation[p]);
			ADDHIST(this->m_zdcsmd_ADCprepostCorrelation[p]);
		}

		ADDHIST(this->m_zdc_test);
	}
#undef ADDHIST
}
//-------------------------------------------------------------------
void ZDCPlots::initRun(int date, int time) {
	if (mDb) {
		LOG_DEBUG << "setting text db to " << TString::Format("%s", this->getTextDb()) << endm;
		mDb->setTextDb(this->getTextDb());
		LOG_DEBUG << "finished setting text db" << endm;
		mDb->SetDateTime(date, time);
	}
}
//-------------------------------------------------------------------
void ZDCPlots::clear() {
	TObjArray list;
	this->getHisto(&list);
	for (Int_t i = 0;i < list.GetEntries();i++) {
		TH1 *h = static_cast<TH1*>(list.At(i));
		if (h) h->Reset();
	}
	mTree->Reset();
}
//-------------------------------------------------------------------
void ZDCPlots::saveHistograms(TFile *hfile) {
	if (hfile) {
		hfile->cd();
		TObjArray list;
		this->getHisto(&list);
		for (Int_t i = 0;i < list.GetEntries();i++) {
			TH1 *h = static_cast<TH1*>(list.At(i));
			if (h) h->Write();
		}

		mTree->Write();
	}
	cout << "====================================================" << endl;
	cout << "Number of ZDC_E events: " << zdc_e_count << endl;
	cout << "====================================================" << endl;
}
//-------------------------------------------------------------------
void ZDCPlots::processEvent(const StTriggerData *td, int date, int time, int prepost) {
	if (mDb) mDb->SetDateTime(date, time);

	if (td && td->zdcPresent()) {

		for (int eastwest = 0; eastwest < 2; eastwest++) {
			for (int tower = 0; tower < 4; tower++) {
				unsigned short adc = (tower == 0) ? td->zdcUnAttenuated((StBeamDirection) eastwest, prepost) : td->zdcADC((StBeamDirection) eastwest, tower, prepost);
				unsigned short tdc = (tower == 0) ? td->zdcTDC((StBeamDirection) eastwest, prepost) : td->zdcPmtTDC((StBeamDirection) eastwest, tower, prepost);
				//if (this->m_zdc_TDC) this->m_zdc_TDC->Fill((eastwest * 4) + tower, tdc);
				//if (this->m_zdc_ADCTDC[eastwest][tower]) this->m_zdc_ADCTDC[eastwest][tower]->Fill(adc, tdc);
				//zdc_TDC_WestTow1 = tdc;
				//chiyang
				// cout<<"test chi="<<td->vpdEarliestTDCHighThr((StBeamDirection)eastwest, prepost)<<endl;

			}
		}

		/*
      // selecting only ZDC_E trigger
      const unsigned long long ZDC_E_mask = 0x800000;
        l2sum = td->l2sum();
        cout << "L2 sum mask: " << (l2sum & ZDC_E_mask) << endl;

        if ((l2sum & ZDC_E_mask) != ZDC_E_mask )
      return;
        else
      cout << "Analyzing ZDC_E triggered event." << endl;
        */

		++zdc_e_count;

		int east = 0;
		int west = 1;


		for (int prepostSet = 0; prepostSet < 6; ++prepostSet) {
			prepost_event = prepostSet;
			zdc_ADC_EastSum_Attenuated = td->zdcAttenuated((StBeamDirection) east, prepostSet);
			zdc_ADC_EastSum = td->zdcUnAttenuated((StBeamDirection) east, prepostSet);
			zdc_ADC_EastTow1 = td->zdcADC((StBeamDirection) east, 1, prepostSet);
			zdc_ADC_EastTow2 = td->zdcADC((StBeamDirection) east, 2, prepostSet);
			zdc_ADC_EastTow3 = td->zdcADC((StBeamDirection) east, 3, prepostSet);

			zdc_ADC_WestSum_Attenuated = td->zdcAttenuated((StBeamDirection) west, prepostSet);
			zdc_ADC_WestSum = td->zdcUnAttenuated((StBeamDirection) west, prepostSet);
			zdc_ADC_WestTow1 = td->zdcADC((StBeamDirection) west, 1, prepostSet);
			zdc_ADC_WestTow2 = td->zdcADC((StBeamDirection) west, 2, prepostSet);
			zdc_ADC_WestTow3 = td->zdcADC((StBeamDirection) west, 3, prepostSet);

			zdc_TDC_EastSum = td->zdcTDC((StBeamDirection) east, prepostSet);
			zdc_TDC_EastTow1 = td->zdcPmtTDC((StBeamDirection) east, 1, prepostSet);
			zdc_TDC_EastTow2 = td->zdcPmtTDC((StBeamDirection) east, 2, prepostSet);
			zdc_TDC_EastTow3 = td->zdcPmtTDC((StBeamDirection) east, 3, prepostSet);

			zdc_TDC_WestSum = td->zdcTDC((StBeamDirection) west, prepostSet);
			zdc_TDC_WestTow1 = td->zdcPmtTDC((StBeamDirection) west, 1, prepostSet);
			zdc_TDC_WestTow2 = td->zdcPmtTDC((StBeamDirection) west, 2, prepostSet);
			zdc_TDC_WestTow3 = td->zdcPmtTDC((StBeamDirection) west, 3, prepostSet);

			trgToken = td->trgToken();
			token = td->token();
			actionWord = td->actionWord();


			for (int ini = 0; ini < 8; ini++) {
				zdc_smd_east_ver[ini] = -999;// yifei
				zdc_smd_east_hor[ini] = -999;// yifei
				zdc_smd_west_ver[ini] = -999;// yifei
				zdc_smd_west_hor[ini] = -999;// yifei
			}

			for (int eastwest = 0; eastwest < 2; eastwest++)
				for (int horiz = 0; horiz < 2; horiz++)
					for (int strip = 0; strip < 8; strip++) {
						// unsigned short adcx = td->zdcSMD((StBeamDirection)eastwest, horiz, strip + 1, prepost);
						int adcx = td->zdcSMD((StBeamDirection) eastwest, horiz, strip + 1);
						// float pedx = mDb ? mDb->getZdcSmdPed((StBeamDirection)eastwest, horiz, strip) : 0;
						// float gain = mDb ? mDb->getZdcSmdGain((StBeamDirection)eastwest, horiz, strip) : 0;
						// float calib = mDb ? mDb->getZdcSmdCalib((StBeamDirection)eastwest, horiz) : 0;
						// float adcCorr = (gain > 0) ? ((((float)adc - ped) / gain) * calib) : 0;
						// float adcCorrThresh = (gain > 0) ? (((pedThresh) / gain) * calib) : 0;
						// if (adcCorr > adcCorrThresh)
						// {
						// m_zdcsmd_A[eastwest][horiz]->Fill(strip + 1, adcCorr);
						// m_zdcsmd_N[eastwest][horiz]->Fill(strip + 1, 1);
						float zdc_smd_ped[2][2][8] =
								{
										{{85.58,  83.2125, 80.8655, 87.2975, 67.171,  85.1085, 80.6875, 78.2435},
												{80.57,  78.463,  85.4785, 91.8345, 85.742, 82.1725, 80.7425, 77.463}},
										{{87.932, 89.557,  90.718,  93.4085, 92.1285, 87.849,  74.11,   77.062},
												{82.569, 75.6175, 74.23,   75.9275, 73.002, 80.4215, 73.9905, 75.7385}}
								}; // run10363017
						adcx -= zdc_smd_ped[eastwest][horiz][strip];
						//	if( adcx - zdc_smd_ped[eastwest][horiz][strip] > 0)
						if (adcx > 0) {
							if (eastwest == 0 && horiz == 0) zdc_smd_east_ver[strip] = adcx;// - zdc_smd_ped[0][0][strip];
							if (eastwest == 0 && horiz == 1) zdc_smd_east_hor[strip] = adcx;// - zdc_smd_ped[0][1][strip];
							if (eastwest == 1 && horiz == 0) zdc_smd_west_ver[strip] = adcx;// - zdc_smd_ped[1][0][strip];
							if (eastwest == 1 && horiz == 1) zdc_smd_west_hor[strip] = adcx;// - zdc_smd_ped[1][1][strip];
						}
						//}
					}

//      zdc_ADC_EWSum = td->zdcHardwareSum(prepost);
			zdc_ADC_EWSum = zdc_ADC_WestSum_Attenuated + zdc_ADC_EastSum_Attenuated;

			tof_multiplicity = (int) td->tofMultiplicity(prepostSet);

			bbc_ADC_EastSum_SmallTile = td->bbcADCSum((StBeamDirection) east, prepostSet); //pmt: 1-16
			bbc_ADC_EastSum_LargeTile = td->bbcADCSumLargeTile((StBeamDirection) east, prepostSet);//pmt: 17-24

			bbc_ADC_WestSum_SmallTile = td->bbcADCSum((StBeamDirection) west, prepostSet); //pmt: 1-16
			bbc_ADC_WestSum_LargeTile = td->bbcADCSumLargeTile((StBeamDirection) west, prepostSet);//pmt: 17-24


			//bbc_ADC_EastSum = 0;
			//bbc_ADC_WestSum = 0;
			//for(int pmt=1; pmt<=24; pmt++) {
			//  bbc_ADC_EastSum += td->bbcADC((StBeamDirection)east, pmt, prepost);
			//  bbc_ADC_WestSum += td->bbcADC((StBeamDirection)west, pmt, prepost);
			//}


			/*
            //For ADC
            Int_t eastwest;
            Int_t tower;
            if(eastwest == 0){
          zdc_ADC_EastSum =  td->zdcUnAttenuated((StBeamDirection)eastwest, prepost);
          if(tower == 1)zdc_ADC_EastTow1 =  td->zdcADC((StBeamDirection)eastwest, tower, prepost);
          if(tower == 2)zdc_ADC_EastTow1 =  td->zdcADC((StBeamDirection)eastwest, tower, prepost);
          if(tower == 3)zdc_ADC_EastTow1 =  td->zdcADC((StBeamDirection)eastwest, tower, prepost);
            }


            if(eastwest == 1){
          zdc_ADC_WestSum = td->zdcUnAttenuated((StBeamDirection)eastwest, prepost);
          if(tower == 1)zdc_ADC_WestTow1 = td->zdcADC((StBeamDirection)eastwest, tower, prepost);
          if(tower == 2)zdc_ADC_WestTow1 = td->zdcADC((StBeamDirection)eastwest, tower, prepost);
          if(tower == 3)zdc_ADC_WestTow1 = td->zdcADC((StBeamDirection)eastwest, tower, prepost);
            }

            //For TDC
            if(eastwest == 0){
          zdc_TDC_EastSum = td->zdcTDC((StBeamDirection)eastwest, prepost);
          if(tower == 1)zdc_TDC_EastTow1 = td->zdcPmtTDC((StBeamDirection)eastwest, tower, prepost);
          if(tower == 2)zdc_TDC_EastTow1 = td->zdcPmtTDC((StBeamDirection)eastwest, tower, prepost);
          if(tower == 3)zdc_TDC_EastTow1 = td->zdcPmtTDC((StBeamDirection)eastwest, tower, prepost);
            }

            if(eastwest == 1){
          zdc_TDC_WestSum = td->zdcTDC((StBeamDirection)eastwest, prepost);
          if(tower == 1)zdc_TDC_WestTow1 = td->zdcPmtTDC((StBeamDirection)eastwest, tower, prepost);
          if(tower == 2)zdc_TDC_WestTow1 = td->zdcPmtTDC((StBeamDirection)eastwest, tower, prepost);
          if(tower == 3)zdc_TDC_WestTow1 = td->zdcPmtTDC((StBeamDirection)eastwest, tower, prepost);
            }

            */

			mTree->Fill();
		} //prepost loop
	}
	//=============Tree finished===================


	//float tdiffMult = 1.0/2*40.0/0.03;
	float tdiffMult = 1.0/2*0.02*30;

	if (td && td->zdcPresent()) {
		int te = td->zdcPmtTDC(east, 1, prepost);
		int tw = td->zdcPmtTDC(west, 1, prepost);
		int te2 = td->zdcPmtTDC(east, 2, prepost);//yhzhu
		int tw2 = td->zdcPmtTDC(west, 2, prepost);//yhzhu
		int te3 = td->zdcPmtTDC(east, 3, prepost);//yhzhu
		int tw3 = td->zdcPmtTDC(west, 3, prepost);//yhzhu


		if ((te != 0) || (tw != 0)) {
			//LOG_DEBUG << "te=" << te << ", tw=" << tw << endm;
			if (this->m_zdcTotEvents) {
				this->m_zdcTotEvents->Fill(0.5);
				//LOG_DEBUG << "1. te=" << te << ", tw=" << tw << ", tot=" << this->m_zdcTotEvents->Integral() << endm;
			}
		}
		int tdiff = tw - te;
		float vertexZ = 0;
		float adcE = td->zdcAttenuated(east, prepost);
		float adcW = td->zdcAttenuated(west, prepost);
//        UShort_t tofmult = (int)td->tofMultiplicity(prepost); //yhzhu added
		UShort_t tofmult = (int)td->tofMultiplicity(prepost); //yhzhu added
		cout<<"tofmult ="<<tofmult<<endl;
		cout<<"tof tray 40 mult ="<<(int)td->tofTrayMultiplicity(40,prepost)<<endl;

		//float tdcE = td->zdcTDC(east, prepost);
		//float tdcW = td->zdcTDC(west, prepost);
		if (mDb && false) {
			Float_t twCorr = tw;
			twCorr -= (mDb->getSlewingCoeff(west, 0) + (mDb->getSlewingCoeff(west, 1) * adcW) + (mDb->getSlewingCoeff(west, 2) * adcW * adcW) + (mDb->getSlewingCoeff(west, 3) * adcW * adcW * adcW));
			Float_t teCorr = te;
			teCorr -= (mDb->getSlewingCoeff(east, 0) + (mDb->getSlewingCoeff(east, 1) * adcE) + (mDb->getSlewingCoeff(east, 2) * adcE * adcE) + (mDb->getSlewingCoeff(east, 3) * adcE * adcE * adcE));
			vertexZ = ((twCorr - teCorr) * mDb->getVertexVPAR()) + mDb->getVertexOFF();
		} else {
			vertexZ = tdiff * tdiffMult;
		}

//        if((tofmult>=10)) return; //yhzhu added
		if (this->getShiftCrewPlots()) {
//	    if ((te > 500) && (te < 3000) ) { //yhzhu
			if ((te > 50) && (te < 3000) ) { //yhzhu
				if (this->m_h76_zdc_time_east) this->m_h76_zdc_time_east->Fill(te);
			}//yhzhu
//	    if ((tw > 500) && (tw < 3000)) { //yhzhu
			if ((tw > 50) && (tw < 3000) ) { //yhzhu
				if (this->m_h77_zdc_time_west) this->m_h77_zdc_time_west->Fill(tw);
			}//yhzhu
//	    if ((te > 500) && (te < 3000) && (tw > 500) && (tw < 3000)) { //yhzhu
			if ((te > 50) && (te < 3000) && (tw > 50) && (tw < 3000) ) { //yhzhu
				if (this->m_h78_zdc_timediff_east_west) this->m_h78_zdc_timediff_east_west->Fill(tdiff);
				if (this->m_h146_zdc_Vertex_cm) this->m_h146_zdc_Vertex_cm->Fill(vertexZ);
			}//yhzhu

			if (this->m_hADC_EastWestSum) this->m_hADC_EastWestSum->Fill(zdc_ADC_EWSum);
			if (this->m_hADC_EWSumZDCvsBBC) this->m_hADC_EWSumZDCvsBBC->Fill(bbc_ADC_EastSum_SmallTile+bbc_ADC_WestSum_SmallTile, zdc_ADC_EWSum);
			if (this->m_hADC_EWSumZDCvsTOFMult) this->m_hADC_EWSumZDCvsTOFMult->Fill(tofmult,zdc_ADC_EWSum);


			for (int eastwest = 0;eastwest < 2;eastwest++)
				for (int tower = 0;tower < 4;tower++) {
					unsigned short adc = (tower == 0) ? td->zdcUnAttenuated((StBeamDirection)eastwest, prepost) : td->zdcADC((StBeamDirection)eastwest, tower, prepost);
					unsigned short adc_opp = (tower == 0) ? td->zdcUnAttenuated((StBeamDirection)(abs(eastwest-1)), prepost) : td->zdcADC((StBeamDirection)(abs(eastwest-1)), tower, prepost);//yhzhu


					float ped = mDb ? mDb->getZdcPedestal((StBeamDirection)eastwest, tower) : 0;
					float calib = mDb ? mDb->getZdcCalib((StBeamDirection)eastwest, tower) : 0;
//                cout<<"ped="<<ped<<endl;
					float adcCorr = (adc - ped) * calib;
//		if (this->m_zdc_ADC) this->m_zdc_ADC->Fill((eastwest * 4) + tower, adc);
//		if (this->m_zdc_ADC && (te > 500) && (te < 2500) && (tw > 500) && (tw < 2500) && adc_opp > 50 && adc_opp < 800 ) this->m_zdc_ADC->Fill((eastwest * 4) + tower, adc);
					//       	if (this->m_zdc_ADC && (te > 500) && (te < 3000) && (tw > 500) && (tw < 3000) &&  (bbc_ADC_EastSum_SmallTile+bbc_ADC_WestSum_SmallTile)>25000 && (bbc_ADC_EastSum_SmallTile+bbc_ADC_WestSum_SmallTile)<30000) this->m_zdc_ADC->Fill((eastwest * 4) + tower, adc);//yhzhu
					if (this->m_zdc_ADC && (te > 500) && (te < 3000) && (tw > 500) && (tw < 3000)) this->m_zdc_ADC->Fill((eastwest * 4) + tower, adc);//yhzhu


//		if (this->m_zdc_ADCCorr) this->m_zdc_ADCCorr->Fill((eastwest * 4) + tower, adcCorr);
//		if (this->m_zdc_ADCCorr &&  (te > 500) && (te < 3000) && (tw > 500) && (tw < 3000) && (bbc_ADC_EastSum_SmallTile+bbc_ADC_WestSum_SmallTile)>25000 && (bbc_ADC_EastSum_SmallTile+bbc_ADC_WestSum_SmallTile)<30000) this->m_zdc_ADCCorr->Fill((eastwest * 4) + tower, adcCorr);//yhzhu
					if (this->m_zdc_ADCCorr &&  (te > 500) && (te < 3000) && (tw > 500) && (tw < 3000)) this->m_zdc_ADCCorr->Fill((eastwest * 4) + tower, adcCorr);//yhzhu

				}
			for (int eastwest = 0;eastwest < 2;eastwest++)
				for (int horiz = 0;horiz < 2;horiz++)
					for (int strip = 0;strip < 8;strip++) {
						unsigned short adc = td->zdcSMD((StBeamDirection)eastwest, horiz, strip + 1, prepost);
//LOG_DEBUG << "ZDC SMD: eastwest = " << eastwest << ", horiz = " << horiz << ", strip = " << strip << ": adc = " << adc << endm;
						float ped = mDb ? mDb->getZdcSmdPed((StBeamDirection)eastwest, horiz, strip) : 0;
						float gain = mDb ? mDb->getZdcSmdGain((StBeamDirection)eastwest, horiz, strip) : 0;
						float calib = mDb ? mDb->getZdcSmdCalib((StBeamDirection)eastwest, horiz) : 0;
						float adcCorr = (gain > 0) ? ((((float)adc - ped) / gain) * calib) : 0;
						float adcCorrThresh = (gain > 0) ? (((pedThresh) / gain) * calib) : 0;
						if (adcCorr > adcCorrThresh) {
							m_zdcsmd_A[eastwest][horiz]->Fill(strip + 1, adcCorr);
							m_zdcsmd_N[eastwest][horiz]->Fill(strip + 1, 1);
						}
					}
		}
		if (this->getExpertPlots()) {
			float maxAdcCorr[2][2];
			bool maxAdcCorrAboveThresh[2][2];
			for (int eastwest = 0;eastwest < 2;eastwest++) {
				unsigned short adcTowerSum = td->zdcUnAttenuated((StBeamDirection)eastwest, prepost);
				float pedTowerSum = mDb ? mDb->getZdcPedestal((StBeamDirection)eastwest, 0) : 0;
				float calibTowerSum = mDb ? mDb->getZdcCalib((StBeamDirection)eastwest, 0) : 0;
				float towerSum = (adcTowerSum - pedTowerSum) * calibTowerSum;
				for (int horiz = 0;horiz < 2;horiz++) {
					maxAdcCorr[eastwest][horiz] = -5000;
					maxAdcCorrAboveThresh[eastwest][horiz] = false;
					float smdSum = 0;
					for (int strip = 0;strip < 8;strip++) {
						unsigned short adc = td->zdcSMD((StBeamDirection)eastwest, horiz, strip + 1, prepost);
						float ped = mDb ? mDb->getZdcSmdPed((StBeamDirection)eastwest, horiz, strip) : 0;
						float gain = mDb ? mDb->getZdcSmdGain((StBeamDirection)eastwest, horiz, strip) : 0;
						float calib = mDb ? mDb->getZdcSmdCalib((StBeamDirection)eastwest, horiz) : 0;
						float adcCorr = (gain > 0) ? ((((float)adc - ped) / gain) * calib) : 0;
						float adcCorrThresh = (gain > 0) ? (((pedThresh) / gain) * calib) : 0;
						int i = (eastwest * 16) + (horiz * 8) + strip + 1;
						if (this->m_zdcsmd_ADC) this->m_zdcsmd_ADC->Fill(i, adc);
						if (this->m_zdcsmd_ADCCorr) this->m_zdcsmd_ADCCorr->Fill(i, adcCorr);
						if (adc > rawThresh) {
							if (this->m_zdcsmd_Occupancy) this->m_zdcsmd_Occupancy->Fill(i);
						}
						if (adcCorr > adcCorrThresh) {
							if (this->m_zdcsmd_OccupancyCorr) this->m_zdcsmd_OccupancyCorr->Fill(i);
						}
						//if (adcCorr > adcCorrThresh) {
						if (adcCorr > maxAdcCorr[eastwest][horiz]) {
							maxAdcCorr[eastwest][horiz] = adcCorr;
							if (adcCorr > adcCorrThresh) maxAdcCorrAboveThresh[eastwest][horiz] = true;
						}
						//}
						smdSum += adcCorr;
					}
					if (this->m_zdcsmd_SmdSumTowerSum[eastwest][horiz]) this->m_zdcsmd_SmdSumTowerSum[eastwest][horiz]->Fill(towerSum, smdSum);
					if ((towerSum > towerSumThresh) && (smdSum > smdSumThresh)) {
						if (this->m_zdcsmd_SmdSumTowerSumRatio[eastwest][horiz]) this->m_zdcsmd_SmdSumTowerSumRatio[eastwest][horiz]->Fill(smdSum / towerSum);
					}
				}
			}
			for (int eastwest = 0;eastwest < 2;eastwest++) {
				if (this->m_zdcsmd_MaxXYCorr[eastwest]) this->m_zdcsmd_MaxXYCorr[eastwest]->Fill(maxAdcCorr[eastwest][0], maxAdcCorr[eastwest][1]);
				if (maxAdcCorrAboveThresh[eastwest][0] && maxAdcCorrAboveThresh[eastwest][1]) {
					if (this->m_zdcsmd_MaxXYCorrRatio[eastwest]) this->m_zdcsmd_MaxXYCorrRatio[eastwest]->Fill(maxAdcCorr[eastwest][0] / maxAdcCorr[eastwest][1]);
				}
			}
			for (int eastwest = 0;eastwest < 2;eastwest++) {
				for (int tower = 0;tower < 4;tower++) {
					unsigned short adc = (tower == 0) ? td->zdcUnAttenuated((StBeamDirection)eastwest, prepost) : td->zdcADC((StBeamDirection)eastwest, tower, prepost);
					unsigned short tdc = (tower == 0) ? td->zdcTDC((StBeamDirection)eastwest, prepost) : td->zdcPmtTDC((StBeamDirection)eastwest, tower, prepost);
					if (this->m_zdc_TDC) this->m_zdc_TDC->Fill((eastwest * 4) + tower, tdc);
					if (this->m_zdc_ADCTDC[eastwest][tower]) this->m_zdc_ADCTDC[eastwest][tower]->Fill(adc, tdc);
				}
				if (this->m_zdc_TDCdiff) {
					this->m_zdc_TDCdiff->Fill((eastwest * 6) + 0, td->zdcTDC((StBeamDirection)eastwest, prepost) - td->zdcPmtTDC((StBeamDirection)eastwest, 1, prepost));
					this->m_zdc_TDCdiff->Fill((eastwest * 6) + 1, td->zdcTDC((StBeamDirection)eastwest, prepost) - td->zdcPmtTDC((StBeamDirection)eastwest, 2, prepost));
					this->m_zdc_TDCdiff->Fill((eastwest * 6) + 2, td->zdcTDC((StBeamDirection)eastwest, prepost) - td->zdcPmtTDC((StBeamDirection)eastwest, 3, prepost));
					this->m_zdc_TDCdiff->Fill((eastwest * 6) + 3, td->zdcPmtTDC((StBeamDirection)eastwest, 1, prepost) - td->zdcPmtTDC((StBeamDirection)eastwest, 2, prepost));
					this->m_zdc_TDCdiff->Fill((eastwest * 6) + 4, td->zdcPmtTDC((StBeamDirection)eastwest, 1, prepost) - td->zdcPmtTDC((StBeamDirection)eastwest, 3, prepost));
					this->m_zdc_TDCdiff->Fill((eastwest * 6) + 5, td->zdcPmtTDC((StBeamDirection)eastwest, 2, prepost) - td->zdcPmtTDC((StBeamDirection)eastwest, 3, prepost));
				}
			}
			unsigned short maxTDC[2] = {0};
			for (int eastwest = 0;eastwest < 2;eastwest++) {
				unsigned short earliestTDC = td->zdcEarliestTDC((StBeamDirection)eastwest, prepost);
				unsigned short adcSum = 0;
				unsigned short adcFront = 0;
				unsigned short adcBack = 0;
				if (this->m_zdcdsm_earliestTDC) this->m_zdcdsm_earliestTDC->Fill(eastwest, earliestTDC);
				for (int tower = 1;tower < 4;tower++) {
					unsigned short tdc = td->zdcPmtTDC((StBeamDirection)eastwest, tower, prepost);
					unsigned short adc = td->zdcADC((StBeamDirection)eastwest, tower, prepost);
					bool goodHit = (adc > goodHitADCthreshold) && (tdc > goodHitMinTDC) && (tdc < goodHitMaxTDC);
					if (goodHit) {
						if (tdc > maxTDC[eastwest]) maxTDC[eastwest] = tdc;
						if (tower == 1) {
							adcFront += adc;
						} else {
							adcBack += adc;
						}
						adcSum += adc;
					}
				}
				if (this->m_zdcdsm_earliestTDCMaxTDC[eastwest]) this->m_zdcdsm_earliestTDCMaxTDC[eastwest]->Fill(maxTDC[eastwest], earliestTDC);
				if (this->m_zdcdsm_ADCthresh) {
					if (td->zdcSumADCaboveThreshold((StBeamDirection)eastwest, prepost)) {
						this->m_zdcdsm_ADCthresh->Fill((eastwest * 6) + (0 * 2) + 0, adcSum);
					} else {
						this->m_zdcdsm_ADCthresh->Fill((eastwest * 6) + (0 * 2) + 1, adcSum);
					}
					if (td->zdcFrontADCaboveThreshold((StBeamDirection)eastwest, prepost)) {
						this->m_zdcdsm_ADCthresh->Fill((eastwest * 6) + (1 * 2) + 0, adcFront);
					} else {
						this->m_zdcdsm_ADCthresh->Fill((eastwest * 6) + (1 * 2) + 1, adcFront);
					}
					if (td->zdcBackADCaboveThreshold((StBeamDirection)eastwest, prepost)) {
						this->m_zdcdsm_ADCthresh->Fill((eastwest * 6) + (2 * 2) + 0, adcBack);
					} else {
						this->m_zdcdsm_ADCthresh->Fill((eastwest * 6) + (2 * 2) + 1, adcBack);
					}
				}
				if (this->m_zdc_FrontBackSum[eastwest]) this->m_zdc_FrontBackSum[eastwest]->Fill(adcFront, adcBack);
				if (this->m_zdcdsm_ADCthreshL2) {
					this->m_zdcdsm_ADCthreshL2->Fill(eastwest, 0 + ((td->zdcSumADCaboveThreshold((StBeamDirection)eastwest, prepost) == td->zdcSumADCaboveThresholdL2((StBeamDirection)eastwest)) ? 1 : 0));
					this->m_zdcdsm_ADCthreshL2->Fill(eastwest, 2 + ((td->zdcFrontADCaboveThreshold((StBeamDirection)eastwest, prepost) == td->zdcFrontADCaboveThresholdL2((StBeamDirection)eastwest)) ? 1 : 0));
					this->m_zdcdsm_ADCthreshL2->Fill(eastwest, 4 + ((td->zdcBackADCaboveThreshold((StBeamDirection)eastwest, prepost) == td->zdcBackADCaboveThresholdL2((StBeamDirection)eastwest)) ? 1 : 0));
				}
				if (this->m_zdcdsm_ADCthreshL3) {
					this->m_zdcdsm_ADCthreshL3->Fill(eastwest, 0 + ((td->zdcSumADCaboveThresholdL2((StBeamDirection)eastwest) == td->zdcSumADCaboveThresholdL3((StBeamDirection)eastwest)) ? 1 : 0));
					this->m_zdcdsm_ADCthreshL3->Fill(eastwest, 2 + ((td->zdcFrontADCaboveThresholdL2((StBeamDirection)eastwest) == td->zdcFrontADCaboveThresholdL3((StBeamDirection)eastwest)) ? 1 : 0));
					this->m_zdcdsm_ADCthreshL3->Fill(eastwest, 4 + ((td->zdcBackADCaboveThresholdL2((StBeamDirection)eastwest) == td->zdcBackADCaboveThresholdL3((StBeamDirection)eastwest)) ? 1 : 0));
				}
			}
			int maxTDCdiff = maxTDC[0] - maxTDC[1];
			int earliestTDCdiff = td->zdcEarliestTDC(east, prepost) - td->zdcEarliestTDC(west, prepost);
			if (this->m_zdcdsm_TDCdiff) this->m_zdcdsm_TDCdiff->Fill(maxTDCdiff, td->zdcTimeDifference());
			if (this->m_zdcdsm_TDCdiffL1L2) this->m_zdcdsm_TDCdiffL1L2->Fill(earliestTDCdiff, td->zdcTimeDifference());
			if (this->m_zdcdsm_TDCdiffL3) this->m_zdcdsm_TDCdiffL3->Fill(maxTDCdiff, td->zdcTimeDifferenceInWindow() ? 1 : 0);

			Float_t towerADCsum_prev = 0;
			Float_t smdADCsum_prev = 0;
			Float_t towerADC[11][2][3];
			Float_t smdADC[11][2][2][8];
			memset(towerADC, 0, sizeof(towerADC));
			memset(smdADC, 0, sizeof(smdADC));
			for (Int_t p = -1 * (Int_t)td->numberOfPreXing();p <= +1 * (Int_t)td->numberOfPostXing();p++) {
				Float_t towerADCsum = 0;
				Float_t smdADCsum = 0;
				for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
					for (Int_t tower = 1;tower <= 3;tower++) {
						float ped = mDb ? mDb->getZdcPedestal((StBeamDirection)eastwest, tower) : 0;
						float calib = mDb ? mDb->getZdcCalib((StBeamDirection)eastwest, tower) : 0;
						unsigned short adc = td->zdcADC((StBeamDirection)eastwest, tower, p);
						float adcCorr = (adc > 0) ? ((adc - ped) * calib) : 0;
						towerADC[p + 5][eastwest][tower - 1] = adcCorr;
						towerADCsum += adcCorr;
						if (p >= -4) {
							if (this->m_zdc_ADCprepostCorrelation[p + 4]) this->m_zdc_ADCprepostCorrelation[p + 4]->Fill(towerADC[p + 4][eastwest][tower - 1], towerADC[p + 5][eastwest][tower - 1]);
						}
					}
					for (Int_t horiz = 0;horiz < 2;horiz++) {
						for (Int_t strip = 0;strip < 8;strip++) {
							float ped = mDb ? mDb->getZdcSmdPed((StBeamDirection)eastwest, horiz, strip) : 0;
							float gain = mDb ? mDb->getZdcSmdGain((StBeamDirection)eastwest, horiz, strip) : 0;
							float calib = mDb ? mDb->getZdcSmdCalib((StBeamDirection)eastwest, horiz) : 0;
							unsigned short adc = td->zdcSMD((StBeamDirection)eastwest, horiz, strip + 1, p);
							float adcCorr = ((gain > 0) && (adc > 0)) ? ((((float)adc - ped) / gain) * calib) : 0;
							smdADC[p + 5][eastwest][horiz][strip] = adcCorr;
							smdADCsum += adcCorr;
							if (p >= -4) {
								if (this->m_zdcsmd_ADCprepostCorrelation[p + 4]) this->m_zdcsmd_ADCprepostCorrelation[p + 4]->Fill(smdADC[p + 4][eastwest][horiz][strip], smdADC[p + 5][eastwest][horiz][strip]);
							}
							//if (m_zdc_test && (p == 0) && (smdADC[p + 5][eastwest][horiz][strip] > 50)) m_zdc_test->Fill(smdADC[p + 5][eastwest][horiz][strip], eastwest*16 + horiz*8 + strip);
						}
					}
				}
				if (this->m_zdc_ADCprepost) this->m_zdc_ADCprepost->Fill(p, towerADCsum);
				if (this->m_zdcsmd_ADCprepost) this->m_zdcsmd_ADCprepost->Fill(p, smdADCsum);
				//if (p >= -4) {
				//    if (this->m_zdc_ADCprepostCorrelation[p + 4]) this->m_zdc_ADCprepostCorrelation[p + 4]->Fill(towerADCsum_prev, towerADCsum);
				//}
				//if (p >= -4) {
				//    if (this->m_zdcsmd_ADCprepostCorrelation[p + 4]) this->m_zdcsmd_ADCprepostCorrelation[p + 4]->Fill(smdADCsum_prev, smdADCsum);
				//}
				towerADCsum_prev = towerADCsum;
				smdADCsum_prev = smdADCsum;
			}
		}
	}
}
