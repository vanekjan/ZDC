#ifndef StZDCCalibration_h
#define StZDCCalibration_h

#include <TObject.h>

class TCanvas;
class TObjArray;

//class StTriggerData;

//class ZDCPlots;

void histFitSlicesY(TH2 *hist, TF1 *f1, Int_t firstxbin, Int_t lastxbin, Int_t cut, Option_t *option, TObjArray* arr);

void calibrateZDC(const Char_t *filenamePlotsIn
			, Bool_t &calibratePed, Bool_t &calibrateGain
			, Bool_t &calibrateTowerPed, Bool_t &calibrateTowerCalib, Bool_t &calibrateSlewing
			, const Char_t *zdcPedCalIn
			, Int_t run, Int_t date, Int_t time
			, const Char_t *zdcPedCalOut
			, const Char_t *dirTablesOut
			, TObjArray *cleanup = 0
			, TCanvas* canvasOut = 0, TCanvas* canvasOutTower = 0, TCanvas* canvasOutSlewing = 0
			);

#endif
