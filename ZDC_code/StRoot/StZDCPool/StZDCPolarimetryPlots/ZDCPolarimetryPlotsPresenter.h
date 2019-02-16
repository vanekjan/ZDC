#ifndef ZDCPolarimetryPlotsPresenter_H
#define ZDCPolarimetryPlotsPresenter_H

#include <TString.h>

#include <StZDCPool/StZDCPlots/ZDCPlotsPresenter.h>

#include "ZDCPolarimetryPlotsUtil.h"

class TPad;
class GenericFile;
typedef GenericFile FileType;

class ZDCPolarimetryPlotsPresenter : public ZDCPlotsPresenter {
public:
    ZDCPolarimetryPlotsPresenter();
    virtual ~ZDCPolarimetryPlotsPresenter();

    virtual Int_t getMaxId() const;
    virtual const Char_t *getTitle(Int_t id) const;
    enum {
	kBx = ZDCPlotsPresenter::kZDC_last + 1,
	kZDCRawAsymBx = ZDCPlotsPresenter::kZDC_last + 2,
	kZDCRawAsymSpin = ZDCPlotsPresenter::kZDC_last + 3,
	kZDCSingleSpinAsymmetry = ZDCPlotsPresenter::kZDC_last + 4,
	kZDC_last = ZDCPlotsPresenter::kZDC_last + 4
    };
    virtual void displayTab(Int_t id, FileType file, TPad *pad, Bool_t *isEmpty = 0) const;

    const Char_t *getAsymmetryOutputFilename() const {return mAsymmetryOutputFilename.Data();}
    void setAsymmetryOutputFilename(const Char_t *filename) {mAsymmetryOutputFilename = filename ? filename : "";}

    ClassDef(ZDCPolarimetryPlotsPresenter, 1)
protected:
    virtual void displayBx(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displayZDCRawAsymBxSpin(Bool_t bxSpin, FileType file, TPad *pad, Bool_t *isEmpty = 0) const;
    virtual void displaySingleSpinAsymmetry(FileType file, TPad *pad, Bool_t *isEmpty = 0) const;

    TString mAsymmetryOutputFilename;
};

#endif
