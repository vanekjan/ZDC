#ifndef ZDCPlots_H
#define ZDCPlots_H

#include <TDataSet.h>
#include <stdlib.h>
class TH1F;
class TH2F;
class TFile;
class TTree;
class TObjArray;

class StTriggerData;

class StZDCDb;
class StZDCGeom;

class ZDCPlots: public TDataSet {

public:
    ZDCPlots(const Char_t *name = "ZDCPlots");
    virtual ~ZDCPlots();

    const Char_t *getTextDb() const {return mZdcPedCal;}
    void setTextDb(const Char_t *filename) {mZdcPedCal = filename;}

    virtual void getHisto(TObjArray *list, Bool_t inherited = true) const;
    virtual void init();
    virtual void initRun(int date, int time);
    virtual void processEvent(const StTriggerData *td, int date, int time, int prepost = 0);
    virtual void saveHistograms(TFile *hfile);    
    virtual void clear();

    Bool_t getShiftCrewPlots() const {return mShiftCrewPlots;}
    void setShiftCrewPlots(Bool_t use) {mShiftCrewPlots = use;}

    Bool_t getExpertPlots() const {return mExpertPlots;}
    void setExpertPlots(Bool_t use) {mExpertPlots = use;}

    // These are called from Pplots
    static void initHisto(TObjArray *list = 0, const Char_t *zdcPedCal = 0, Bool_t useShiftCrewPlots = true, Bool_t useExpertPlots = false);
    static void resetHisto(const Char_t *zdcPedCal = 0, int date = 0, int time = 0);
    static void saveHisto(TFile *hfile);    
    static void fillHisto(const StTriggerData *td, int date = 0, int time = 0, int prepost = 0);

    ClassDef(ZDCPlots, 1)

protected:
    
    //========================
    TTree* mTree;
    
    //Float_t zdc_ADC[2][4];
    //Float_t zdc_TDC[2][4];

	int prepost_event;

    unsigned short zdc_ADC_EastSum_Attenuated;
    unsigned short zdc_ADC_EastSum;
    unsigned short zdc_ADC_EastTow1;
    unsigned short zdc_ADC_EastTow2;
    unsigned short zdc_ADC_EastTow3;

    unsigned short zdc_ADC_WestSum_Attenuated;
    unsigned short zdc_ADC_WestSum;
    unsigned short zdc_ADC_WestTow1;
    unsigned short zdc_ADC_WestTow2;
    unsigned short zdc_ADC_WestTow3;

    unsigned short zdc_TDC_EastSum;
    unsigned short zdc_TDC_EastTow1;
    unsigned short zdc_TDC_EastTow2;
    unsigned short zdc_TDC_EastTow3;

    unsigned short zdc_TDC_WestSum;
    unsigned short zdc_TDC_WestTow1;
    unsigned short zdc_TDC_WestTow2;
    unsigned short zdc_TDC_WestTow3;

    unsigned short zdc_ADC_EWSum;

    unsigned short tof_multiplicity;
   
    unsigned short bbc_ADC_EastSum_SmallTile;
    unsigned short bbc_ADC_EastSum_LargeTile;

    unsigned short bbc_ADC_WestSum_SmallTile;
    unsigned short bbc_ADC_WestSum_LargeTile;

	short zdc_smd_east_ver[8];// yifei Xu 
	short zdc_smd_east_hor[8];// yifei Xu 
	short zdc_smd_west_ver[8];// yifei Xu 
	short zdc_smd_west_hor[8];// yifei Xu 
    
    unsigned int trgToken;
    unsigned int token;
    unsigned int actionWord;
    unsigned long long l2sum;

    unsigned int zdc_e_count;
    //unsigned short bbc_ADC_EastSum;
    //unsigned short bbc_ADC_WestSum;
    
    //=========================
    

    Bool_t mShiftCrewPlots;
    Bool_t mExpertPlots;

    StZDCDb *mDb;
    const Char_t *mZdcPedCal;

    StZDCGeom *mGeom;

    TH1F *m_zdcTotEvents;

    TH1F *m_h76_zdc_time_east;
    TH1F *m_h77_zdc_time_west;
    TH1F *m_h78_zdc_timediff_east_west;
    TH1F *m_h146_zdc_Vertex_cm;
    TH2F *m_zdc_ADC;
    TH2F *m_zdc_ADCCorr;
    TH2F *m_zdc_TDC;
    TH2F *m_zdc_TDCdiff; // [east,west][sum-t1,sum-t2,sum-t3,t1-t2,t1-t3,t2-t3]
    
    TH1F *m_hADC_EastWestSum;//CJ
    TH2F *m_hADC_EWSumZDCvsBBC;//CJ
    TH2F *m_hADC_EWSumZDCvsTOFMult;//yhzhu

    TH1F *m_zdcsmd_N[2][2];
    TH1F *m_zdcsmd_A[2][2];

    TH2F *m_zdcsmd_ADC;
    TH1F *m_zdcsmd_Occupancy;
    TH2F *m_zdcsmd_ADCCorr;
    TH1F *m_zdcsmd_OccupancyCorr;
    TH2F *m_zdcsmd_MaxXYCorr[2];
    TH1F *m_zdcsmd_MaxXYCorrRatio[2];

    TH2F *m_zdc_ADCTDC[2][4];

    TH2F *m_zdc_FrontBackSum[2];

    TH2F *m_zdcsmd_SmdSumTowerSum[2][2];
    TH1F *m_zdcsmd_SmdSumTowerSumRatio[2][2];

    TH2F *m_zdcdsm_earliestTDC;
    TH2F *m_zdcdsm_earliestTDCMaxTDC[2];
    TH2F *m_zdcdsm_TDCdiff;
    TH2F *m_zdcdsm_TDCdiffL1L2;
    TH2F *m_zdcdsm_TDCdiffL3;
    TH2F *m_zdcdsm_ADCthresh;
    TH2F *m_zdcdsm_ADCthreshL2;
    TH2F *m_zdcdsm_ADCthreshL3;

    TH2F *m_zdc_ADCprepost;
    TH2F *m_zdcsmd_ADCprepost;
    TH2F *m_zdc_ADCprepostCorrelation[10];
    TH2F *m_zdcsmd_ADCprepostCorrelation[10];

    TH2F *m_zdc_test;
};

#endif

