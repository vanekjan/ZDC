#ifndef StReadTrg_h
#define StReadTrg_h

#include <TObject.h>
#include <TString.h>

class TCanvas;
class StTriggerData;
class ZDCPlots;

TString getRunMask(TString filename, Int_t *prun = 0);

class StReadTrg : public TObject {
private:
    static void readTrgData(ZDCPlots *plots, const StTriggerData *trgData, const UInt_t *runTriggersToProcessMask, Int_t date = 0, Int_t time = 0, Int_t prepost = 0, Int_t *nPreFound = 0, Int_t *nPostFound = 0, Bool_t *eventAccepted = 0);
    static void readDaq(ZDCPlots *plots, const Char_t *filename, Int_t nEvents, const UInt_t *runTriggersToProcessMask, Int_t *run = 0, Int_t *date = 0, Int_t *time = 0, Int_t *evts = 0, Int_t *evtsAccepted = 0, Int_t prepost = 0, Int_t *nPreFound = 0, Int_t *nPostFound = 0);
    static void readDat(ZDCPlots *plots, const Char_t *filename, Int_t nEvents, const UInt_t *runTriggersToProcessMask, Int_t *run = 0, Int_t *date = 0, Int_t *time = 0, Int_t *evts = 0, Int_t *evtsAccepted = 0, Int_t prepost = 0, Int_t *nPreFound = 0, Int_t *nPostFound = 0);
public:
    static void readTrg(const Char_t *filename, Int_t nEvents = -1
			, const Char_t *configFile = 0
			, const Char_t *zdcPedCal = 0
			, const Char_t *seenRuns = 0
			, const Char_t *dirOut = 0
			, const Char_t *publicDir = 0
			, const Char_t *emailNotification = 0
			, const Char_t *namesToProcess = 0
			, const Char_t *namesToCalibratePed = 0
			, Int_t minEventsToCalibratePed = 0
			, const Char_t *namesToCalibrateGain = 0
			, Int_t minEventsToCalibrateGain = 0
			, const Char_t *namesToCalibrateTowerPed = 0
			, Int_t minEventsToCalibrateTowerPed = 0
			, const Char_t *namesToCalibrateTowerCalib = 0
			, Int_t minEventsToCalibrateTowerCalib = 0
			, const Char_t *namesToCalibrateSlewing = 0
			, Int_t minEventsToCalibrateSlewing = 0
			, Bool_t searchForOtherRunFiles = true
			, Bool_t stopRunOnFirstError = true
			, Bool_t deleteCanvasOnExit = false
			, Bool_t updateInputTextDb = false
			, Int_t refreshRateSeconds = 0
			, Int_t prepost = 0
			, const Char_t *dbconnectCdev = 0
			, const Char_t *cdevFilename = 0
			, const Char_t *namesToAddFill = 0
			, Int_t minEventsToAddFill = 0
			, const Char_t *outFormats = 0
			, const Char_t *outFillFormats = 0
			, const Char_t *tcuBitsToProcess = 0
			, Float_t normAnalyzingPower = 0
			);

    ClassDef(StReadTrg, 1)
};

#endif
