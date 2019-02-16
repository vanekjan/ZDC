#include <TMath.h>

#include <StMessMgr.h>

#include "StZDCGeom.h"

ClassImp(StZDCGeom)

StZDCGeom::StZDCGeom() {
    this->init();
}

StZDCGeom::~StZDCGeom() {
}

void StZDCGeom::init() {
    Float_t vertStripWidth = 1.5;
    Float_t horizStripWidth = 2.0;
    Float_t angle = TMath::Pi() / 4.0;
    Float_t s = TMath::Sin(angle);
    Float_t c = TMath::Cos(angle);
    Float_t beamEnterBottom = 5.6; // distance between nominal beam line and bottom edge = 56 mm
    Float_t towerDepth = 23.2; // tower depth = 232 mm
    Float_t smdDepth = 2.0; // SMD is 2 cm thick

    mStripDX[0] = vertStripWidth;
    mStripDX[1] = mStripDX[0] * kNstripsVert;
    mStripDY[1] = horizStripWidth * s;
    mStripDY[0] = mStripDY[1] * kNstripsHoriz;

    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	mSMDcenterX[eastwest] = 0;
	mSMDcenterY[eastwest] = (mStripDY[0] / 2.0) - beamEnterBottom; //
	mSMDcenterZ[eastwest] = (eastwest == 0 ? -1 : +1) * ((18.0 * 100.0) + (1.0 * towerDepth) + (0.5 * smdDepth)); // +/- 18 m from interaction point, behind one tower
	for (Int_t istrip = 0;istrip < kNstripsVert;istrip++) {
	    mStripsX[istrip + (eastwest == 0 ? 0 : 1) * kNstripsVert] = 
		((eastwest == 0 ? +1 : -1) * (Float_t(istrip) - (Float_t(kNstripsVert) / 2.0) + 0.5) * mStripDX[0]) + mSMDcenterX[eastwest];
	}
	for (Int_t istrip = 0;istrip < kNstripsHoriz;istrip++) {
	    mStripsY[istrip + (eastwest == 0 ? 0 : 1) * kNstripsHoriz] = ((Float_t(istrip) - (Float_t(kNstripsHoriz) / 2.0) + 0.5) * mStripDY[1]) + mSMDcenterY[eastwest];
	    mStripsZ[istrip + (eastwest == 0 ? 0 : 1) * kNstripsHoriz] = ((eastwest == 0 ? +1 : -1) * (Float_t(istrip) - (Float_t(kNstripsHoriz) / 2.0) + 0.5) * (horizStripWidth * c)) + mSMDcenterZ[eastwest];
	}
    }

    this->print();
}

Int_t StZDCGeom::getSMDCoord(StBeamDirection eastwest, Int_t horiz, Int_t strip, Float_t &x, Float_t &y, Float_t &z, Float_t &dx, Float_t &dy) const {
    Int_t result = 0;
    if (horiz == 0) {
        if ((strip >= 1) && (strip <= kNstripsVert)) {
    	    x = this->mStripsX[strip - 1 + (eastwest == east ? 0 : 1) * kNstripsVert];
	    y = this->mSMDcenterY[eastwest == east ? 0 : 1];
	    z = this->mSMDcenterZ[eastwest == east ? 0 : 1];
    	    dx = this->mStripDX[horiz];
	    dy = this->mStripDY[horiz];
	    result = 1;
	}
    } else if (horiz == 1) {
	if ((strip >= 1) && (strip <= kNstripsHoriz)) {
	    x = this->mSMDcenterX[eastwest == east ? 0 : 1];
	    y = this->mStripsY[strip - 1 + (eastwest == east ? 0 : 1) * kNstripsHoriz];
	    z = this->mStripsZ[strip - 1 + (eastwest == east ? 0 : 1) * kNstripsHoriz];
    	    dx = this->mStripDX[horiz];
	    dy = this->mStripDY[horiz];
	    result = 1;
	}
    }
    return result;
}

Int_t StZDCGeom::getSMDCenter(StBeamDirection eastwest, Float_t &x, Float_t &y, Float_t &z) const {
    x = this->mSMDcenterX[eastwest == east ? 0 : 1];
    y = this->mSMDcenterY[eastwest == east ? 0 : 1];
    z = this->mSMDcenterZ[eastwest == east ? 0 : 1];
    return 1;
}

void StZDCGeom::print() const {
    LOG_DEBUG << "Printing ZDC geometry..." << endm;
    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	for (Int_t horiz = 0;horiz < 2;horiz++) {
	    for (Int_t strip = 1;strip <= (horiz ? kNstripsHoriz : kNstripsVert);strip++) {
		Float_t x, y, z, dx, dy;
		if (getSMDCoord((StBeamDirection)eastwest, horiz, strip, x, y, z, dx, dy)) {
		    LOG_DEBUG << (eastwest ? "West" : "East") << " " << (horiz ? "Horiz" : "Vert") << " " << strip << ": "
			<< "(x,y,z) = (" << x << "," << y << "," << z << "), (dx,dy) = (" << dx << "," << dy << ")" << endm;
		}
	    }
	}
    }
    for (Int_t eastwest = 0;eastwest < 2;eastwest++) {
	Float_t x, y, z;
	if (getSMDCenter((StBeamDirection)eastwest, x, y, z)) {
	    LOG_DEBUG << (eastwest ? "West" : "East") << " ZDC center (x,y,z) = (" << x << "," << y << "," << z << ")" << endm;
	}
    }
}
