#ifndef STAR_StZDCDb
#define STAR_StZDCDb

#include <TObject.h>
#include <TDatime.h>
#include <TDataSet.h>

#include <StEnumerations.h>

class St_ZdcCalPars;
class ZdcCalPars_st;
class St_zdcsmdPed;
class zdcsmdPed_st;
class St_zdcsmdGain;
class zdcsmdGain_st;
class St_zdcsmdBeamCenter;
class zdcsmdBeamCenter_st;
class St_zdcCalib;
class zdcCalib_st;

class TTable;
class StDbTable;
class StMaker;

#define ZDCCALPARS_TABLE         (UChar_t(1 << 0))
#define ZDCSMDPED_TABLE          (UChar_t(1 << 1))
#define ZDCSMDGAIN_TABLE         (UChar_t(1 << 2))
#define ZDCSMDBEAMCENTER_TABLE   (UChar_t(1 << 3))
#define ZDCCALIB_TABLE           (UChar_t(1 << 4))

#define ZDC_ALL_TABLES           (UChar_t(0xFF))

class StZDCDb : public TDataSet
{
public:
    StZDCDb(const Char_t *name = "StZDCDb");
    virtual ~StZDCDb();
    
    void setTextDb(const Char_t *filename);
    const Char_t *getTextDb() const {return mTextDbFilename;}
    void reloadTextDb() const;

    StMaker *getChain() const {return mChain;}

    void exportTextDb(const Char_t *filename) const;
    void exportROOTDb(const Char_t *dir, UChar_t saveTables = ZDC_ALL_TABLES) const;

    /// St_db_Maker-compatible interface
    void SetDateTime(int date, int time);
    void SetFlavor(const Char_t *flavor, const Char_t *tablename=NULL);
    void SetMaxEntryTime(int date, int time);
    
    const ZdcCalPars_st* getZdcCalPars() const;
    ZdcCalPars_st* getZdcCalPars();
    Float_t getSlewingCoeff(StBeamDirection eastwest, Int_t order) const;
    Float_t getVertexVPAR() const;
    Float_t getVertexOFF() const;

    const zdcsmdPed_st* getZdcSmdPed() const;
    zdcsmdPed_st* getZdcSmdPed();
    Float_t getZdcSmdPed(StBeamDirection eastwest, Int_t horiz, Int_t strip) const;

    const zdcsmdGain_st* getZdcSmdGain() const;
    zdcsmdGain_st* getZdcSmdGain();
    Float_t getZdcSmdGain(StBeamDirection eastwest, Int_t horiz, Int_t strip) const;
    Float_t getZdcSmdCalib(StBeamDirection eastwest, Int_t horiz) const;

    const zdcsmdBeamCenter_st* getZdcSmdBeamCenter() const;
    zdcsmdBeamCenter_st* getZdcSmdBeamCenter();

    const zdcCalib_st* getZdcCalib() const;
    zdcCalib_st* getZdcCalib();
    Float_t getZdcPedestal(StBeamDirection eastwest, Int_t tower) const; // [east,west][sum,tower1,tower2,tower3]
    Float_t getZdcCalib(StBeamDirection eastwest, Int_t tower) const; // [east,west][sum,tower1,tower2,tower3]

private:
    mutable bool mInitialized;
    void init() const;

    const Char_t *mTextDbFilename;

    void writeROOTDbTable(const Char_t *dir, const Char_t *tableName, const TTable *ttable, const StDbTable *dbtable) const;
    
    // DB tables provided by St_db_Maker -- prefer to use these
    mutable St_ZdcCalPars *mZdcCalParsTTable;
    // version info from StMaker::GetValidity -- used to expire caches
    mutable Int_t mZdcCalParsValidity;
    // DB tables provided by StDbManager in standalone mode
    mutable StDbTable *mZdcCalParsTable;
    // ensure caches expire if beginTime, flavor, or maxEntryTime changes
    mutable bool mZdcCalParsDirty;
    void maybe_reload_ZdcCalPars() const;
    
    mutable St_zdcsmdPed *mZdcSmdPedTTable;
    mutable Int_t mZdcSmdPedValidity;
    mutable StDbTable *mZdcSmdPedTable;
    mutable bool mZdcSmdPedDirty;
    void maybe_reload_ZdcSmdPed() const;
    
    mutable St_zdcsmdGain *mZdcSmdGainTTable;
    mutable Int_t mZdcSmdGainValidity;
    mutable StDbTable *mZdcSmdGainTable;
    mutable bool mZdcSmdGainDirty;
    void maybe_reload_ZdcSmdGain() const;
    
    mutable St_zdcsmdBeamCenter *mZdcSmdBeamCenterTTable;
    mutable Int_t mZdcSmdBeamCenterValidity;
    mutable StDbTable *mZdcSmdBeamCenterTable;
    mutable bool mZdcSmdBeamCenterDirty;
    void maybe_reload_ZdcSmdBeamCenter() const;
    
    mutable St_zdcCalib *mZdcCalibTTable;
    mutable Int_t mZdcCalibValidity;
    mutable StDbTable *mZdcCalibTable;
    mutable bool mZdcCalibDirty;
    void maybe_reload_ZdcCalib() const;
    
    mutable StMaker *mChain;
    
    mutable TDatime mBeginTime;
    void reload_dbtable(StDbTable*) const;
    
    ClassDef(StZDCDb, 1)
};

#endif
