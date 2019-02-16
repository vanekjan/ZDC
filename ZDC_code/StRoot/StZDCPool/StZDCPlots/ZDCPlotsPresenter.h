#ifndef ZDCPlotsPresenter_H
#define ZDCPlotsPresenter_H

#include <StEnumerations.h>
#include <GenericFile.h>

class TH1;
class TPad;
class TObjArray;
class TCanvas;

class GenericFile;
typedef GenericFile FileType;

class ZDCPlotsPresenter {
public:
    // this will be called from PPlots
    static void displayTab(int tab, int panel, FileType file, TPad *pad, Bool_t *isEmpty = 0);

    ZDCPlotsPresenter();
    virtual ~ZDCPlotsPresenter();

    void clear() const;
    TObjArray *getCleanupArray() const {return mCleanup;}

    virtual Int_t getMaxId() const;
    virtual const Char_t *getTitle(Int_t id) const;
    enum {
	kZDC_General = 0,
	kZDC_Tower = 1,
	kZDC_Tower_scale = 2,
	kZDC_TowerCorr = 3,
	kZDCEWadcSum = 4,
	kZDC_TDC = 5,
	kZDC_TDCdiffE = 6,
	kZDC_TDCdiffW = 7,
	kZDC_ADCTDC = 8,
	kZDC_FrontBackSum = 9,
	kZDCSMD_General = 10,
	kZDCSMD_ADCOccupancy = 11,
	kZDCSMD_MaxXY = 12,
	kZDCSMD_ADCEV = 13,
	kZDCSMD_ADCEH = 14,
	kZDCSMD_ADCWV = 15,
	kZDCSMD_ADCWH = 16,
	kZDCSMD_ADCEVCorr = 17,
	kZDCSMD_ADCEHCorr = 18,
	kZDCSMD_ADCWVCorr = 19,
	kZDCSMD_ADCWHCorr = 20,
	kZDCSMD_SmdSumTowerSumE = 21,
	kZDCSMD_SmdSumTowerSumW = 22,
	kZDCDSM_earliestTDC = 23,
	kZDCDSM_ADCthresh = 24,
	kZDC_ADCprepost = 25,
	kZDC_ADCprepostCorrelationTower = 26,
	kZDC_ADCprepostCorrelationSMD = 27,
	kZDC_last = 27
    };
    virtual void displayTab(int id, FileType file, TPad *pad, Bool_t *isEmpty = 0) const;

    TCanvas *displayFileTab(const Char_t *filename, Int_t id) const;

    ClassDef(ZDCPlotsPresenter, 1)
protected:
    mutable TObjArray *mCleanup;

    TH1 *GetHisto(FileType &fd, const char *name) const;

    virtual void displayZDC(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCTower(Bool_t corrected, FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCTower_scale(Bool_t corrected, FileType file, TPad *pad, Bool_t *isEmpty = 0) const;//CJ
    virtual void displayZDCEWadcSum(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCSMD(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCTDC(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCTDCdiff(StBeamDirection eastwest, FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCADCTDC(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCFrontBackSum(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCSMDADCOccupancy(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCSMDMaxXY(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCSMDADC(StBeamDirection eastwest, int horiz, bool corrected, FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCSmdSumTowerSum(StBeamDirection eastwest, FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCDSMearliestTDC(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCDSMADCthresh(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCADCprepost(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCADCprepostCorrelation(Bool_t towersmd, FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
};

#define FIND_HISTO(TYPE, NAME, FILE, TITLE, ISEMPTY) \
TYPE *NAME = (TYPE*)GetHisto((FILE), (TITLE)); \
if (NAME) { \
    if (ISEMPTY) *(ISEMPTY) = false; \
} else { \
    LOG_ERROR << "Histogram " << #NAME << " not found: " << (TITLE) << endm; \
}

#endif
