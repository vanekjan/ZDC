#include <iostream>
#include <fstream>
using namespace std;

#include <TMath.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TArrow.h>
#include <TLatex.h>
#include <TString.h>
#include <TBox.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TDatime.h>
#include <TPRegexp.h>

#include <StMessMgr.h>
#include <StTriggerData.h>

#include <StZDCUtil/StZDCDb.h>
#include <StZDCUtil/StZDCGeom.h>
#include <StZDCUtil/StZDCUtil.h>

#include "ZDCPolarimetryPlotsNames.h"
#include "ZDCPolarimetryPlots.h"

ClassImp(ZDCPolarimetryPlots)

Float_t pedThresh = 5.0;

//-------------------------------------------------------------------
ZDCPolarimetryPlots::ZDCPolarimetryPlots(const Char_t *name)
    : ZDCPlots(name)
    , mPolarimetryPlots(true)
    , mBx7Offset(0)
    , mBx48Offset(0)
    , mYellowCNIAsym(0)
    , mYellowCNIAsymOld(0)
    , mYellowPol(0)
    , mYellowPolErr(0)
    , mBlueCNIAsym(0)
    , mBlueCNIAsymOld(0)
    , mBluePol(0)
    , mBluePolErr(0)
    , m_bx(0)
    , m_spin(0)
    , m_bxspin(0)
    , m_bxspinY(0)
    , m_bxspinB(0)
    , m_yellowPol(0)
    , m_yellowPolW(0)
    , m_bluePol(0)
    , m_bluePolW(0)
{
    memset(mSpinPattern, 0, sizeof(mSpinPattern));
    memset(mSpinPatternY, 0, sizeof(mSpinPatternY));
    memset(mSpinPatternB, 0, sizeof(mSpinPatternB));
    memset(m_SmdHitsSpin, 0, sizeof(m_SmdHitsSpin));
    memset(m_SmdHitsBx, 0, sizeof(m_SmdHitsBx));
    memset(m_SmdStripsSpin, 0, sizeof(m_SmdStripsSpin));
    memset(m_SmdStripsBx, 0, sizeof(m_SmdStripsBx));
}
//-------------------------------------------------------------------
ZDCPolarimetryPlots::~ZDCPolarimetryPlots() {
  this->setPolarimetryPlots(true);
  TObjArray list;
    this->getHisto(&list, false);
    for (Int_t i = 0;i < list.GetEntries();i++) {
        TH1 *h = static_cast<TH1*>(list.At(i));
        if (h) delete h;
    }
}
//-------------------------------------------------------------------
void ZDCPolarimetryPlots::init() {
    this->ZDCPlots::init();
    if (this->getPolarimetryPlots()) {
	this->m_bx = new TH2F(bxName, "Bunch crossing distribution;0=Bx7raw, 1=Bx7corrected, 2=Bx48raw, 3=Bx48corrected;Bx", 4, 0, 4, kNbunch, 0, kNbunch);
	this->m_spin = new TH1F(spinName, "Spin combination distribution;Spin combination", kNspin, 0, kNspin);
	this->m_bxspin = new TH2F(bxspinName, "Spin combination per bunch crossing;Bx7corrected;Spin combination", kNbunch, 0, kNbunch, kNspin, 0 - 0.5, kNspin - 0.5);
	this->m_bxspinY = new TH2F(bxspinYName, "Yellow Spin per bunch crossing;Bx7corrected;Yellow Spin", kNbunch, 0, kNbunch, 3, -1.5, +1.5);
	this->m_bxspinB = new TH2F(bxspinBName, "Blue Spin per bunch crossing;Bx7corrected;Blue Spin", kNbunch, 0, kNbunch, 3, -1.5, +1.5);
	this->m_yellowPol = new TH1F(yellowPolName, "Yellow beam polarization;P_{Y}", 1000, 0, 1);
	this->m_yellowPolW = new TH1F(yellowPolWName, "Yellow beam polarization / pol. error squared;P_{Y}/#siqma_{Y}^{2}, 1/#sigma_{Y}^{2}, N", 3, 0, 3);
	this->m_bluePol = new TH1F(bluePolName, "Blue beam polarization;P_{B}", 1000, 0, 1);
	this->m_bluePolW = new TH1F(bluePolWName, "Blue beam polarization / pol. error squared;P_{B}/#siqma_{B}^{2}, 1/#sigma_{B}^{2}, N", 3, 0, 3);
	for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	    Float_t minX = 0, maxX = StZDCGeom::kNstripsVert, minY = 0, maxY = StZDCGeom::kNstripsHoriz;
	    if (this->mGeom) {
		Float_t x, y, z, dx, dy;
		this->mGeom->getSMDCoord((StBeamDirection)eastwest, 0, 1, x, y, z, dx, dy);
		if (eastwest == 0) {
		    minX = x - (0.5 * dx);
		} else {
		    maxX = x + (0.5 * dx);
		}
		this->mGeom->getSMDCoord((StBeamDirection)eastwest, 0, StZDCGeom::kNstripsVert, x, y, z, dx, dy);
		if (eastwest == 0) {
		    maxX = x + (0.5 * dx);
		} else {
		    minX = x - (0.5 * dx);
		}
		this->mGeom->getSMDCoord((StBeamDirection)eastwest, 1, 1, x, y, z, dx, dy);
		minY = y - (0.5 * dy);
		this->mGeom->getSMDCoord((StBeamDirection)eastwest, 1, StZDCGeom::kNstripsHoriz, x, y, z, dx, dy);
		maxY = y + (0.5 * dy);
	    }
	
	    for (Int_t ispin = 0;ispin < kNspin;ispin++) {
	      m_SmdHitsSpin[eastwest][ispin] = new TH2F(TString::Format("%s_%s_%i", smdHitsSpinName, eastwest ? "west" : "east", ispin), TString::Format("ZDC SMD X vs Y, %s, spin combination %i;X, cm;Y, cm", eastwest ? "West" : "East", ispin), StZDCGeom::kNstripsVert, minX, maxX, StZDCGeom::kNstripsHoriz, minY, maxY);
	      m_SmdHitsSpin[eastwest][ispin]->Sumw2();
	    }
	    for (Int_t ibx = 0;ibx < kNbunch;ibx++) {
	      m_SmdHitsBx[eastwest][ibx] = new TH2F(TString::Format("%s_%s_%i", smdHitsBxName, eastwest ? "west" : "east", ibx), TString::Format("ZDC SMD X vs Y, %s, bunch crossing %i;X, cm;Y, cm", eastwest ? "West" : "East", ibx), StZDCGeom::kNstripsVert, minX, maxX, StZDCGeom::kNstripsHoriz, minY, maxY);
	    }
	    
	}
	
	for (Int_t ispin = 0;ispin < kNspin;ispin++) {
	  m_SmdStripsSpin[ispin] = new TH2F(TString::Format("%s_%i", smdStripsSpinName, ispin), TString::Format("ZDC SMD ADC vs Strip, spin combination %i;Strip;(ADC - PED)/Gain", ispin), 32, 0, 32, 500, 0, 1000);
	}
	for (Int_t ibx = 0;ibx < kNbunch;ibx++) {
	  m_SmdStripsBx[ibx] = new TH2F(TString::Format("%s_%i", smdStripsBxName, ibx), TString::Format("ZDC SMD SMD ADC vs Strip, bunch crossing %i;Strip;(ADC-PED)/Gain", ibx), 32, 0, 32, 500, 0, 1000);
	}
	
    }
}
//-------------------------------------------------------------------
void ZDCPolarimetryPlots::getHisto(TObjArray *list, Bool_t inherited) const {
    if (inherited) this->ZDCPlots::getHisto(list);
#define ADDHIST(hist) if ((list) && (hist)) (list)->Add(hist);
    if (this->getPolarimetryPlots()) {
	ADDHIST(m_bx);
	ADDHIST(m_spin);
	ADDHIST(m_bxspin);
	ADDHIST(m_bxspinY);
	ADDHIST(m_bxspinB);
	ADDHIST(m_yellowPol);
	ADDHIST(m_yellowPolW);
	ADDHIST(m_bluePol);
	ADDHIST(m_bluePolW);
	
	for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	  for (Int_t ispin = 0;ispin < kNspin;ispin++) {
	    ADDHIST(m_SmdHitsSpin[eastwest][ispin]);
	  }
	  for (Int_t ibx = 0;ibx < kNbunch;ibx++) {
	    ADDHIST(m_SmdHitsBx[eastwest][ibx]);
	  }
	}
	for (Int_t ispin = 0;ispin < kNspin;ispin++) {
	  ADDHIST(m_SmdStripsSpin[ispin]);
	}
	for (Int_t ibx = 0;ibx < kNbunch;ibx++) {
	  ADDHIST(m_SmdStripsBx[ibx]);
	}
    }
#undef ADDHIST
}

//-------------------------------------------------------------------
void ZDCPolarimetryPlots::initRun(Int_t date, Int_t time) {
    this->ZDCPlots::initRun(date, time);

    // Populate mBx7Offset, mBx48Offset, and mSpinPattern[kNbunch] from this->getCdevXmlFile().
    // Do not calculate offsets yourself, this would only be reliable if the abort gaps are 
    // visible and stable, like in the physics running.

    mBx7Offset = -40;
    mBx48Offset = -57;

    mYellowPol = 0;
    mYellowPolErr = 0;
    mBluePol = 0;
    mBluePolErr = 0;

    const Char_t *cdevFilename = this->getCdevXmlFile();
    if (cdevFilename && cdevFilename[0]) {
	TString cdevRecord;
	ifstream ifstr(cdevFilename);
	if (ifstr.good()) {
	    const Int_t buflen = 2048;
	    Char_t buf[buflen];
	    while (ifstr.good()) {
		ifstr.getline(buf, buflen);
		buf[buflen - 1] = 0;
		cdevRecord += buf;
	    }
	    ifstr.close();
	} else {
	    LOG_ERROR << "Cannot read CDEV record file " << cdevFilename << endm;
	}
	if (cdevRecord != "") {
	    LOG_DEBUG << "CDEV record: " << cdevRecord << endm;
	    TString fillPatternYStr;
	    TString fillPatternBStr;
	    TString polPatternYStr;
	    TString polPatternBStr;
	    Float_t fillPatternY[360];
	    Float_t fillPatternB[360];
	    Float_t polPatternY[360];
	    Float_t polPatternB[360];
	    TString coggingStr;
	    Float_t cogging = 0;
	    getPattern(cdevRecord, "Yell", "buckets", "intendedFillPatternS", fillPatternYStr, fillPatternY, 360);
	    getPattern(cdevRecord, "Blue", "buckets", "intendedFillPatternS", fillPatternBStr, fillPatternB, 360);
	    getPattern(cdevRecord, "Yell", "buckets", "polarizationFillPatternS", polPatternYStr, polPatternY, 360);
	    getPattern(cdevRecord, "Blue", "buckets", "polarizationFillPatternS", polPatternBStr, polPatternB, 360);
	    getPattern(cdevRecord, "Yell", "buckets", "patternRotationM", coggingStr, &cogging, 1);
	    LOG_DEBUG << "Read cogging " << cogging << endm;
	    TString asymYStr;
	    Float_t asymY = 0;
	    TString asymYerrStr;
	    Float_t asymYerr = 0;
	    TString apYStr;
	    Float_t apY = 0;
	    TString apYerrStr;
	    Float_t apYerr = 0;
	    TString asymBStr;
	    Float_t asymB = 0;
	    TString asymBerrStr;
	    Float_t asymBerr = 0;
	    TString apBStr;
	    Float_t apB = 0;
	    TString apBerrStr;
	    Float_t apBerr = 0;
	    getPattern(cdevRecord, "Yell", "rhic_pol", "avgAsymXS", asymYStr, &asymY, 1);
	    getPattern(cdevRecord, "Yell", "rhic_pol", "avgAsymErrorXS", asymYerrStr, &asymYerr, 1);
	    getPattern(cdevRecord, "Yell", "rhic_pol", "analyzingPowerS", apYStr, &apY, 1);
	    getPattern(cdevRecord, "Yell", "rhic_pol", "analyzingPowerErrorS", apYerrStr, &apYerr, 1);
	    getPattern(cdevRecord, "Blue", "rhic_pol", "avgAsymXS", asymBStr, &asymB, 1);
	    getPattern(cdevRecord, "Blue", "rhic_pol", "avgAsymErrorXS", asymBerrStr, &asymBerr, 1);
	    getPattern(cdevRecord, "Blue", "rhic_pol", "analyzingPowerS", apBStr, &apB, 1);
	    getPattern(cdevRecord, "Blue", "rhic_pol", "analyzingPowerErrorS", apBerrStr, &apBerr, 1);
	    LOG_DEBUG << "Read yellow asymmetry " << asymY << " +/- " << asymYerr << endm;
	    LOG_DEBUG << "Read blue asymmetry " << asymB << " +/- " << asymBerr << endm;
	    LOG_DEBUG << "Read yellow analyzing power " << apY << " +/- " << apYerr << endm;
	    LOG_DEBUG << "Read blue analyzing power " << apB << " +/- " << apBerr << endm;
	    mYellowCNIAsym = asymY;
	    mBlueCNIAsym = asymB;
	    if (apY != 0) {
		mYellowPol = asymY / apY;
		mYellowPolErr = mYellowPol * TMath::Sqrt(((asymYerr/asymY)*(asymYerr/asymY))/* + ((apYerr/apY)*(apYerr/apY))*/);
	    }
	    if (apB != 0) {
		mBluePol = asymB / apB;
		mBluePolErr = mBluePol * TMath::Sqrt(((asymBerr/asymB)*(asymBerr/asymB))/* + ((apBerr/apB)*(apBerr/apB))*/);
	    }
	    LOG_DEBUG << "Polarization: yellow = " << mYellowPol << " +/- " << mYellowPolErr << ", blue = " << mBluePol << " +/- " << mBluePolErr << endm;
	    for (Int_t ctrY = 0;ctrY < 360;ctrY += 3) {
		Int_t ctrB = ctrY + 240 - (Int_t(cogging) * 2);
    		while (ctrB < 0) ctrB += 360;
    		while (ctrB >= 360) ctrB -= 360;
		Int_t fY = Int_t(fillPatternY[ctrY]);
		Int_t fB = Int_t(fillPatternB[ctrB]);
		Int_t pY = Int_t(polPatternY[ctrY]);
		Int_t pB = Int_t(polPatternB[ctrB]);
		Int_t mY = fY * pY;
		Int_t mB = fB * pB;
		LOG_DEBUG << "fY fB pY pB mY mB = " << fY << " " << fB << " " << pY << " " << pB << " " << mY << " " << mB << endm;
    		Int_t spinWord = 0;
    		if ((mY > 0) && (mB > 0)) {
		    spinWord = 1;
    		} else if ((mY > 0) && (mB < 0)) {
		    spinWord = 2;
    		} else if ((mY < 0) && (mB > 0)) {
		    spinWord = 3;
    		} else if ((mY < 0) && (mB < 0)) {
		    spinWord = 4;
		} else {
		    spinWord = 0;
		}
/*
Here we adjust for timeshifts at STAR and move from base-360 to base-120
2004: Yellow bunch-0 coincides with bx7=0;
   ctr = ctrY/3;
2005: Yellow bunch-0 is at 114
   ctr = ctrY/3 - 6;
2006: up until March 6, 2006 14:50 EST:
   ctr = ctrY/3 - 5;
2008:
*/
    		Int_t ctr = ctrY / 3;
    		while (ctr >= 120) ctr -= 120;
    		while (ctr < 0) ctr += 120;
		mSpinPattern[ctr] = spinWord;
		mSpinPatternY[ctr] = mY;
		mSpinPatternB[ctr] = mB;
		LOG_DEBUG << "Spin pattern: " << ctr << " " << spinWord << "(" << mY << " , " << mB << ")" << endm;
	    }
	}
    }
}
//-------------------------------------------------------------------
void ZDCPolarimetryPlots::clear() {
    this->ZDCPlots::clear();
    mBx7Offset = 0;
    mBx48Offset = 0;
    mYellowPol = 0;
    mYellowPolErr = 0;
    mBluePol = 0;
    mBluePolErr = 0;
    memset(mSpinPattern, 0, sizeof(mSpinPattern));
    memset(mSpinPatternY, 0, sizeof(mSpinPatternY));
    memset(mSpinPatternB, 0, sizeof(mSpinPatternB));
}
//-------------------------------------------------------------------
void ZDCPolarimetryPlots::processEvent(const StTriggerData *td, Int_t date, Int_t time, Int_t prepost) {
    this->ZDCPlots::processEvent(td, date, time, prepost);
    if (td && td->zdcPresent() && td->zdcSMDPresent()) {
	if (this->getPolarimetryPlots()) {
	    UInt_t bx7raw = td->bunchId7Bit();
	    UInt_t bx48raw = td->bunchId48Bit();
	    Int_t bx7corrected = bx7raw + mBx7Offset;
	    while (bx7corrected >= kNbunch) bx7corrected -= kNbunch;
	    while (bx7corrected < 0) bx7corrected += kNbunch;
	    Int_t bx48corrected = bx48raw + mBx48Offset;
	    while (bx48corrected >= kNbunch) bx48corrected -= kNbunch;
	    while (bx48corrected < 0) bx48corrected += kNbunch;
	    if (m_bx) {
		m_bx->Fill(0.0, bx7raw);
		m_bx->Fill(1.0, bx7corrected);
		m_bx->Fill(2.0, bx48raw);
		m_bx->Fill(3.0, bx48corrected);
	    }
	    if (m_yellowPol) m_yellowPol->Fill(mYellowPol);
	    if ((mYellowPolErr > 0) && m_yellowPolW && (mYellowCNIAsym != mYellowCNIAsymOld)) {
		Float_t w = 1.0 / (mYellowPolErr * mYellowPolErr);
		if (m_yellowPolW) {
		    m_yellowPolW->Fill(0.0, mYellowPol * w);
		    m_yellowPolW->Fill(1.0, w);
		    m_yellowPolW->Fill(2.0, 1.0);
		}
		mYellowCNIAsymOld = mYellowCNIAsym;
	    }
	    if ((mBluePolErr > 0) && m_bluePolW && (mBlueCNIAsym != mBlueCNIAsymOld)) {
		Float_t w = 1.0 / (mBluePolErr * mBluePolErr);
		if (m_bluePolW) {
		    m_bluePolW->Fill(0.0, mBluePol * w);
		    m_bluePolW->Fill(1.0, w);
		    m_bluePolW->Fill(2.0, 1.0);
		}
		mBlueCNIAsymOld = mBlueCNIAsym;
	    }
	    if (m_bluePol) m_bluePol->Fill(mBluePol);
	    Int_t spinCombination = mSpinPattern[bx7corrected];
	    Int_t spinCombinationY = mSpinPatternY[bx7corrected];
	    Int_t spinCombinationB = mSpinPatternB[bx7corrected];
	    if (m_spin) m_spin->Fill(spinCombination);
	    if (m_bxspin) m_bxspin->Fill(bx7corrected, spinCombination);
	    if (m_bxspinY) m_bxspinY->Fill(bx7corrected, spinCombinationY);
	    if (m_bxspinB) m_bxspinB->Fill(bx7corrected, spinCombinationB);
	    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
		// Find (maxX,maxY,energy) of the highest hit in the SMD
        	Float_t maxAdcCorr[2];
        	Int_t maxAdcCorrStrip[2];
        	Bool_t maxAdcCorrAboveThresh[2];
                for (Int_t horiz = 0;horiz < 2;horiz++) {
                    maxAdcCorr[horiz] = -5000;
                    maxAdcCorrStrip[horiz] = -1;
                    maxAdcCorrAboveThresh[horiz] = kFALSE;
                    for (Int_t strip = 0;strip < 8;strip++) {
			Int_t istrip = (eastwest * 16) + (horiz * 8) + strip;
                        UShort_t adc = td->zdcSMD((StBeamDirection)eastwest, horiz, strip + 1, prepost);
                        Float_t ped = mDb ? mDb->getZdcSmdPed((StBeamDirection)eastwest, horiz, strip) : 0;
                        Float_t gain = mDb ? mDb->getZdcSmdGain((StBeamDirection)eastwest, horiz, strip) : 0;
                        Float_t calib = mDb ? mDb->getZdcSmdCalib((StBeamDirection)eastwest, horiz) : 0;
                        Float_t adcCorr = (gain > 0) ? ((((Float_t)adc - ped) / gain) * calib) : 0;
                        Float_t adcCorrThresh = (gain > 0) ? (((pedThresh) / gain) * calib) : 0;
                        if (adcCorr > maxAdcCorr[horiz]) {
                            maxAdcCorr[horiz] = adcCorr;
                            maxAdcCorrStrip[horiz] = strip;
                            if (adcCorr > adcCorrThresh) maxAdcCorrAboveThresh[horiz] = true;
                        }
			
			if (adcCorr > adcCorrThresh) {
			  if (m_SmdStripsSpin[spinCombination]) m_SmdStripsSpin[spinCombination]->Fill(istrip, adcCorr);
			  if (m_SmdStripsBx[bx7corrected]) m_SmdStripsBx[bx7corrected]->Fill(istrip, adcCorr);
			}
		    }
        	}
		if (maxAdcCorrAboveThresh[0] && maxAdcCorrAboveThresh[1]) {
		  //Float_t energy = (maxAdcCorr[0] + maxAdcCorr[1]) / 2.0;
		  Float_t x = maxAdcCorrStrip[0], y = maxAdcCorrStrip[1];
		    if (this->mGeom) {
			Float_t z, dx, dy, tmp;
			this->mGeom->getSMDCoord((StBeamDirection)eastwest, 0, maxAdcCorrStrip[0] + 1, x, tmp, z, dx, dy);
			this->mGeom->getSMDCoord((StBeamDirection)eastwest, 1, maxAdcCorrStrip[1] + 1, tmp, y, z, dx, dy);
		    }
		    if (m_SmdHitsSpin[eastwest][spinCombination]) m_SmdHitsSpin[eastwest][spinCombination]->Fill(x, y); // or (x, y, energy) - weighted?
		    if (m_SmdHitsBx[eastwest][bx7corrected]) m_SmdHitsBx[eastwest][bx7corrected]->Fill(x, y); // or (x, y, energy) - weighted?
		}
	    }
	}
    }
}
