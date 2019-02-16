#ifndef ZDCPolarimetryPlots_H
#define ZDCPolarimetryPlots_H

#include <StZDCPool/StZDCPlots/ZDCPlots.h>

#include "ZDCPolarimetryPlotsNames.h"

class ZDCPolarimetryPlots: public ZDCPlots {

public:
    ZDCPolarimetryPlots(const Char_t *name = "ZDCPolarimetryPlots");
    virtual ~ZDCPolarimetryPlots();

    const Char_t *getCdevXmlFile() const {return mCdevXmlFile;}
    void setCdevXmlFile(const Char_t *filename) {mCdevXmlFile = filename;}

    virtual void init();
    virtual void initRun(Int_t date, Int_t time);
    virtual void getHisto(TObjArray *list, Bool_t inherited = true) const;
    virtual void processEvent(const StTriggerData *td, Int_t date, Int_t time, Int_t prepost = 0);
    virtual void clear();

    Bool_t getPolarimetryPlots() const {return mPolarimetryPlots;}
    void setPolarimetryPlots(Bool_t use) {mPolarimetryPlots = use;}

    ClassDef(ZDCPolarimetryPlots, 1)

protected:
    Bool_t mPolarimetryPlots;

    const Char_t *mCdevXmlFile;

    Int_t mSpinPattern[kNbunch];
    Int_t mSpinPatternY[kNbunch];
    Int_t mSpinPatternB[kNbunch];
    Int_t mBx7Offset;
    Int_t mBx48Offset;
    Float_t mYellowCNIAsym;
    Float_t mYellowCNIAsymOld;
    Float_t mYellowPol;
    Float_t mYellowPolErr;
    Float_t mBlueCNIAsym;
    Float_t mBlueCNIAsymOld;
    Float_t mBluePol;
    Float_t mBluePolErr;

    TH2F *m_bx; // bunch crossing population [bx7,bx48][raw,corrected]
    TH1F *m_spin; // distribution of spin combinations
    TH2F *m_bxspin; 
    TH2F *m_bxspinY;
    TH2F *m_bxspinB;

    TH1F *m_yellowPol;
    TH1F *m_yellowPolW;
    TH1F *m_bluePol;
    TH1F *m_bluePolW;
   
    TH2F *m_SmdHitsSpin[2][kNspin]; // 2-d hit distribution in X-Y for each spin combination, for east and west separately
    TH2F *m_SmdHitsBx[2][kNbunch]; // 2-d hit distribution in X-Y for each bunch crossing, for east and west separately
    TH2F *m_SmdStripsSpin[kNspin]; // Strip distribution for each spin combination, y=ADCcorr vs. x=[east,west][vert,horiz][1...8]
    TH2F *m_SmdStripsBx[kNbunch]; // Strip distribution for each bunch crossing, y=ADCcorr vs. x=[east,west][vert,horiz][1...8]

};

#endif
