#ifndef StZDCGeom_h
#define StZDCGeom_h

#include <TObject.h>

#include <StEnumerations.h>

// ZDC geometry
// all coordinates are in cm

class StZDCGeom {
public:
    StZDCGeom();
    virtual ~StZDCGeom();
    
    enum {kNtowers = 3, kNstripsVert = 7, kNstripsHoriz = 8};

    Int_t getSMDCoord(StBeamDirection eastwest, Int_t horiz, Int_t strip, Float_t &x, Float_t &y, Float_t &z, Float_t &dx, Float_t &dy) const;
    
    Int_t getSMDCenter(StBeamDirection eastwest, Float_t &x, Float_t &y, Float_t &z) const;

    void print() const;

    ClassDef(StZDCGeom, 1);
private:
    Float_t mStripsX[2*kNstripsVert];
    Float_t mStripsY[2*kNstripsHoriz];
    Float_t mStripsZ[2*kNstripsHoriz];
    Float_t mStripDX[2];
    Float_t mStripDY[2];
    Float_t mSMDcenterX[2];
    Float_t mSMDcenterY[2];
    Float_t mSMDcenterZ[2];

    void init();
};

#endif
