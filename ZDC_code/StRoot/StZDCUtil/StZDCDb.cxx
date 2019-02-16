
#include <fstream>
#include <sstream>
using namespace std;

#include <TUnixTime.h>
#include <TFile.h>

#include <StMessMgr.h>
#include <StMaker.h>
#include <StDbLib/StDbManager.hh>
#include <StDbLib/StDbConfigNode.hh>
#include <StDbLib/StDbTable.h>

#include <tables/St_ZdcCalPars_Table.h>
#include <tables/St_zdcsmdPed_Table.h>
#include <tables/St_zdcsmdGain_Table.h>
#include <tables/St_zdcsmdBeamCenter_Table.h>
#include <tables/St_zdcCalib_Table.h>

#include "StZDCDb.h"

ClassImp(StZDCDb);

const Char_t *trgDbName = "Calibrations/trg";
const Char_t *zdcDbName = "Calibrations/zdc";

const Char_t *ZdcCalParsName = "ZdcCalPars";
const Char_t *zdcsmdPedName = "zdcsmdPed";
const Char_t *zdcsmdGainName = "zdcsmdGain";
const Char_t *zdcsmdBeamCenterName = "zdcsmdBeamCenter";
const Char_t *zdcCalibName = "zdcCalib";

ClassImp(StZDCDb)
    
StZDCDb::StZDCDb(const Char_t *name)
    : TDataSet(name)
    , mInitialized(false)

    , mTextDbFilename(0)

    , mZdcCalParsTTable(NULL)
    , mZdcCalParsValidity(-2)
    , mZdcCalParsTable(NULL)
    , mZdcCalParsDirty(true)

    , mZdcSmdPedTTable(NULL)
    , mZdcSmdPedValidity(-2)
    , mZdcSmdPedTable(NULL)
    , mZdcSmdPedDirty(true)

    , mZdcSmdGainTTable(NULL)
    , mZdcSmdGainValidity(-2)
    , mZdcSmdGainTable(NULL)
    , mZdcSmdGainDirty(true)

    , mZdcSmdBeamCenterTTable(NULL)
    , mZdcSmdBeamCenterValidity(-2)
    , mZdcSmdBeamCenterTable(NULL)
    , mZdcSmdBeamCenterDirty(true)

    , mZdcCalibTTable(NULL)
    , mZdcCalibValidity(-2)
    , mZdcCalibTable(NULL)
    , mZdcCalibDirty(true)
{
}

StZDCDb::~StZDCDb() {
    if (mZdcCalParsTTable) delete mZdcCalParsTTable;
    if (mZdcSmdPedTTable) delete mZdcSmdPedTTable;
    if (mZdcSmdGainTTable) delete mZdcSmdGainTTable;
    if (mZdcSmdBeamCenterTTable) delete mZdcSmdBeamCenterTTable;
    if (mZdcCalibTTable) delete mZdcCalibTTable;
}

void StZDCDb::init() const {
    mChain = StMaker::GetChain();

    mZdcCalParsTTable = 0;
    mZdcCalParsValidity = -2;
    mZdcCalParsTable = 0;
    mZdcCalParsDirty = true;

    mZdcSmdPedTTable = 0;
    mZdcSmdPedValidity = -2;
    mZdcSmdPedTable = 0;
    mZdcSmdPedDirty = true;

    mZdcSmdGainTTable = 0;
    mZdcSmdGainValidity = -2;
    mZdcSmdGainTable = 0;
    mZdcSmdGainDirty = true;

    mZdcSmdBeamCenterTTable = 0;
    mZdcSmdBeamCenterValidity = -2;
    mZdcSmdBeamCenterTable = 0;
    mZdcSmdBeamCenterDirty = true;

    mZdcCalibTTable = 0;
    mZdcCalibValidity = -2;
    mZdcCalibTable = 0;
    mZdcCalibDirty = true;

    if (mTextDbFilename) {
        mZdcCalParsTTable = new St_ZdcCalPars(ZdcCalParsName, 1);
	mZdcCalParsTTable->Adopt(1, new ZdcCalPars_st);

        mZdcSmdPedTTable = new St_zdcsmdPed(zdcsmdPedName, 1);
	mZdcSmdPedTTable->Adopt(1, new zdcsmdPed_st);

        mZdcSmdGainTTable = new St_zdcsmdGain(zdcsmdGainName, 1);
	mZdcSmdGainTTable->Adopt(1, new zdcsmdGain_st);

        mZdcSmdBeamCenterTTable = new St_zdcsmdBeamCenter(zdcsmdBeamCenterName, 1);
	mZdcSmdBeamCenterTTable->Adopt(1, new zdcsmdBeamCenter_st);

        mZdcCalibTTable = new St_zdcCalib(zdcCalibName, 1);
	mZdcCalibTTable->Adopt(1, new zdcCalib_st);
    } else if (mChain) {
    } else {
        StDbManager *mgr = StDbManager::Instance();
	mgr->setVerbose(false);

	TString trgDbNameStr = trgDbName;
	trgDbNameStr.ReplaceAll(TString("/"), TString("_"));
        StDbConfigNode *dbTrg = mgr->initConfig(trgDbNameStr.Data());

        mZdcCalParsTable = dbTrg->addDbTable(ZdcCalParsName);
	LOG_DEBUG << "Created mZdcCalParsTable name: " << mZdcCalParsTable->getName() << ", " <<  mZdcCalParsTable->getMyName() << ", " << mZdcCalParsTable->getDbName() << ", " << mZdcCalParsTable->getCstructName() << endm;
        mZdcCalParsTTable = new St_ZdcCalPars(ZdcCalParsName, 1);
        mZdcCalParsTTable->Adopt(1, mZdcCalParsTable->GetTable());
	mZdcCalParsTTable->SetBit(TTable::kIsNotOwn);

	TString zdcDbNameStr = zdcDbName;
	zdcDbNameStr.ReplaceAll(TString("/"), TString("_"));
        StDbConfigNode *dbZdc = mgr->initConfig(zdcDbNameStr.Data());

        mZdcSmdPedTable = dbZdc->addDbTable(zdcsmdPedName);
	LOG_DEBUG << "Created mZdcSmdPedTable name: " << mZdcSmdPedTable->getName() << ", " << mZdcSmdPedTable->getMyName() << ", " << mZdcSmdPedTable->getDbName() << ", " << mZdcSmdPedTable->getCstructName() << endm;
        mZdcSmdPedTTable = new St_zdcsmdPed(zdcsmdPedName, 1);
        mZdcSmdPedTTable->Adopt(1, mZdcSmdPedTable->GetTable());
	mZdcSmdPedTTable->SetBit(TTable::kIsNotOwn);

        mZdcSmdGainTable = dbZdc->addDbTable(zdcsmdGainName);
	LOG_DEBUG << "Created mZdcSmdGainTable name: " << mZdcSmdGainTable->getName() << ", " << mZdcSmdGainTable->getMyName() << ", " << mZdcSmdGainTable->getDbName() << ", " << mZdcSmdGainTable->getCstructName() << endm;
        mZdcSmdGainTTable = new St_zdcsmdGain(zdcsmdGainName, 1);
        mZdcSmdGainTTable->Adopt(1, mZdcSmdGainTable->GetTable());
	mZdcSmdGainTTable->SetBit(TTable::kIsNotOwn);

        mZdcSmdBeamCenterTable = dbZdc->addDbTable(zdcsmdBeamCenterName);
	LOG_DEBUG << "Created mZdcSmdBeamCenterTable name: " << mZdcSmdBeamCenterTable->getName() << ", " << mZdcSmdBeamCenterTable->getMyName() << ", " << mZdcSmdBeamCenterTable->getDbName() << ", " << mZdcSmdBeamCenterTable->getCstructName() << endm;
        mZdcSmdBeamCenterTTable = new St_zdcsmdBeamCenter(zdcsmdBeamCenterName, 1);
        mZdcSmdBeamCenterTTable->Adopt(1, mZdcSmdBeamCenterTable->GetTable());
	mZdcSmdBeamCenterTTable->SetBit(TTable::kIsNotOwn);

        mZdcCalibTable = dbZdc->addDbTable(zdcCalibName);
	LOG_DEBUG << "Created mZdcCalibTable name: " << mZdcCalibTable->getName() << ", " << mZdcCalibTable->getMyName() << ", " << mZdcCalibTable->getDbName() << ", " << mZdcCalibTable->getCstructName() << endm;
        mZdcCalibTTable = new St_zdcCalib(zdcCalibName, 1);
        mZdcCalibTTable->Adopt(1, mZdcCalibTable->GetTable());
	mZdcCalibTTable->SetBit(TTable::kIsNotOwn);
    }
    mInitialized = true;
    if (mTextDbFilename) {
	reloadTextDb();
    }
}

void StZDCDb::setTextDb(const Char_t *filename) {
    mTextDbFilename = filename;
    mZdcCalParsDirty = true;
    mZdcSmdPedDirty = true;
    mZdcSmdGainDirty = true;
    mZdcSmdBeamCenterDirty = true;
    mZdcCalibDirty = true;
    if (!mInitialized) init();
    if (mZdcCalParsDirty || mZdcSmdPedDirty || mZdcSmdGainDirty || mZdcSmdBeamCenterDirty || mZdcCalibDirty) reloadTextDb();
}

#define PROCESS_CONTENT \
PROCESS(token, fstr, pBeginTime, "BeginTime", date) \
PROCESS(token, fstr, pBeginTime, "BeginTime", time) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, EAP0) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, EAP1) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, EAP2) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, EAP3) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, WAP0) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, WAP1) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, WAP2) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, WAP3) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, VPAR) \
PROCESS(token, fstr, t_slew, ZdcCalParsName, OFF) \
PROCESS(token, fstr, t_smdPed, zdcsmdPedName, RunID) \
PROCESS_ARRAY(token, fstr, t_smdPed, zdcsmdPedName, ZdcsmdPedestal) \
PROCESS(token, fstr, t_smdGain, zdcsmdGainName, RunID) \
PROCESS_ARRAY(token, fstr, t_smdGain, zdcsmdGainName, ZdcsmdGain) \
PROCESS_ARRAY(token, fstr, t_smdGain, zdcsmdGainName, ZdcsmdCalib) \
PROCESS(token, fstr, t_smdBeamCenter, zdcsmdBeamCenterName, RunID) \
PROCESS_ARRAY(token, fstr, t_smdBeamCenter, zdcsmdBeamCenterName, ZdcsmdBeamCenter) \
PROCESS(token, fstr, t_calib, zdcCalibName, RunID) \
PROCESS_ARRAY(token, fstr, t_calib, zdcCalibName, ZdcPedestal) \
PROCESS_ARRAY(token, fstr, t_calib, zdcCalibName, ZdcCalib) \

struct datetime {
    int date;
    int time;
};

void StZDCDb::reloadTextDb() const {
    if (!mInitialized) init();
    if (mTextDbFilename) {
	LOG_INFO << "Loading text DB from file " << mTextDbFilename << endm;
	ifstream fstr(mTextDbFilename);
	if (fstr.good()) {
	    ZdcCalPars_st *t_slew = mZdcCalParsTTable ? mZdcCalParsTTable->GetTable() : 0;
	    zdcsmdPed_st *t_smdPed = mZdcSmdPedTTable ? mZdcSmdPedTTable->GetTable() : 0;
	    zdcsmdGain_st *t_smdGain = mZdcSmdGainTTable ? mZdcSmdGainTTable->GetTable() : 0;
	    zdcsmdBeamCenter_st *t_smdBeamCenter = mZdcSmdBeamCenterTTable ? mZdcSmdBeamCenterTTable->GetTable() : 0;
	    zdcCalib_st *t_calib = mZdcCalibTTable ? mZdcCalibTTable->GetTable() : 0;
	    string token;
	    while (fstr.good()) {
		fstr >> token;
		if ((token != "=") && (token != ";")) {
		    datetime beginTime;
		    datetime *pBeginTime = &beginTime;
#define READ(TOKEN, IFSTR, TBL, STRUCTNAME, MEMBERNAME) \
{ \
    TString memberStr = TString::Format("%s.%s", STRUCTNAME, #MEMBERNAME); \
    if (TOKEN == memberStr.Data()) { \
	string tmp; \
	if (TBL) { \
	    IFSTR >> tmp >> TBL->MEMBERNAME >> tmp; \
	    ostringstream vtmp; vtmp << TBL->MEMBERNAME; \
	    LOG_DEBUG << "reading " << memberStr << ", value=" << vtmp.str() << endm; \
	} else { \
	    LOG_WARN << TString::Format("Zero pointer %s (reading %s)", #TBL, memberStr.Data()) << endm; \
	    IFSTR >> tmp >> tmp >> tmp; \
	} \
    } \
}

#define READ_ARRAY(TOKEN, IFSTR, TBL, STRUCTNAME, MEMBERNAME) \
{ \
    for (unsigned int i = 0;i < sizeof(TBL->MEMBERNAME)/sizeof(TBL->MEMBERNAME[0]);i++) { \
	TString memberStr = TString::Format("%s.%s[%i]", STRUCTNAME, #MEMBERNAME, i); \
	if (TOKEN == memberStr.Data()) { \
	    string tmp; \
	    if (TBL) { \
		IFSTR >> tmp >> TBL->MEMBERNAME[i] >> tmp; \
		ostringstream vtmp; vtmp << TBL->MEMBERNAME[i]; \
		LOG_DEBUG << "reading " << memberStr << ", value=" << vtmp.str() << endm; \
	    } else { \
		LOG_WARN << TString::Format("Zero pointer %s (reading %s)", #TBL, memberStr.Data()) << endm; \
		IFSTR >> tmp >> tmp >> tmp; \
	    } \
	} \
    } \
}

#define PROCESS(TOKEN, FSTR, TBL, STRUCTNAME, MEMBERNAME) \
READ(TOKEN, FSTR, TBL, STRUCTNAME, MEMBERNAME)

#define PROCESS_ARRAY(TOKEN, FSTR, TBL, STRUCTNAME, MEMBERNAME) \
READ_ARRAY(TOKEN, FSTR, TBL, STRUCTNAME, MEMBERNAME)

PROCESS_CONTENT

#undef PROCESS
#undef PROCESS_ARRAY

#undef READ
#undef READ_ARRAY

		    mBeginTime.Set(beginTime.date, beginTime.time);
		}
	    }
            mZdcCalParsDirty = false;
            mZdcSmdPedDirty = false;
            mZdcSmdGainDirty = false;
            mZdcSmdBeamCenterDirty = false;
            mZdcCalibDirty = false;
	} else {
	    LOG_ERROR << "Cannot read file " << mTextDbFilename << endm;
	}
    }
}

void StZDCDb::exportTextDb(const Char_t *filename) const {
    if (!mInitialized) init();
    if (filename) {
	LOG_INFO << "Exporting text DB to file " << filename << endm;
	ofstream fstr(filename);
	if (fstr.good()) {
	    datetime beginTime;
	    datetime *pBeginTime = &beginTime;
	    if (mTextDbFilename) {
		beginTime.date = mBeginTime.GetDate();
		beginTime.time = mBeginTime.GetTime();
	    } else if (mChain) {
		beginTime.date = mChain->GetDateTime().GetDate();
		beginTime.time = mChain->GetDateTime().GetTime();
	    } else {
		beginTime.date = mBeginTime.GetDate();
		beginTime.time = mBeginTime.GetTime();
	    }
	    LOG_DEBUG << "Start getting tables for export..." << endm;
	    LOG_DEBUG << "getting for export: zdcsmdBeamCenter" << endm;
	    const zdcsmdBeamCenter_st *t_smdBeamCenter = this->getZdcSmdBeamCenter();
	    LOG_DEBUG << "getting for export: zdcsmdPed" << endm;
	    const zdcsmdPed_st *t_smdPed = this->getZdcSmdPed();
	    LOG_DEBUG << "getting for export: zdcsmdGain" << endm;
	    const zdcsmdGain_st *t_smdGain = this->getZdcSmdGain();
	    LOG_DEBUG << "getting for export: ZdcCalPars" << endm;
	    const ZdcCalPars_st *t_slew = this->getZdcCalPars();
	    LOG_DEBUG << "getting for export: zdcCalib" << endm;
	    const zdcCalib_st *t_calib = this->getZdcCalib();
	    LOG_DEBUG << "Finished getting tables for export." << endm;

#define WRITE(OFSTR, TBL, STRUCTNAME, MEMBERNAME) \
{ \
    TString memberStr = TString::Format("%s.%s", STRUCTNAME, #MEMBERNAME); \
    if (TBL) { \
	OFSTR << memberStr.Data() << " = " << TBL->MEMBERNAME << ";\n"; \
    } else { \
	LOG_WARN << "Zero pointer " << #TBL << " (writing " << memberStr << ")" << endm; \
    } \
}

#define WRITE_ARRAY(OFSTR, TBL, STRUCTNAME, MEMBERNAME) \
{ \
    for (unsigned int i = 0;i < sizeof(TBL->MEMBERNAME)/sizeof(TBL->MEMBERNAME[0]);i++) { \
	TString memberStr = TString::Format("%s.%s[%i]", STRUCTNAME, #MEMBERNAME, i); \
	if (TBL) { \
	    OFSTR << memberStr.Data() << " = " << TBL->MEMBERNAME[i] << ";\n"; \
	} else { \
	    LOG_WARN << "Zero pointer " << #TBL << " (writing " << memberStr << ")" << endm; \
	} \
    } \
}

#define PROCESS(TOKEN, FSTR, TBL, STRUCTNAME, MEMBERNAME) \
WRITE(FSTR, TBL, STRUCTNAME, MEMBERNAME)

#define PROCESS_ARRAY(TOKEN, FSTR, TBL, STRUCTNAME, MEMBERNAME) \
WRITE_ARRAY(FSTR, TBL, STRUCTNAME, MEMBERNAME)

PROCESS_CONTENT

#undef PROCESS
#undef PROCESS_ARRAY

#undef WRITE
#undef WRITE_ARRAY

	} else {
	    LOG_ERROR << "Cannot write to file " << filename << endm;
	}
    }
}

#undef PROCESS_CONTENT

void StZDCDb::writeROOTDbTable(const Char_t *dir, const Char_t *tableName, const TTable *ttable, const StDbTable *dbtable) const {
    if (dir && tableName && ttable) {
	int date = 0;
	int time = 0;
	if (mTextDbFilename) {
	    date = mBeginTime.GetDate();
	    time = mBeginTime.GetTime();
	} else if (mChain) {
	    TDatime validity[2];
//	    mChain->GetValidity(ttable, validity);
	    date = validity[0].GetDate();
	    time = validity[0].GetTime();
	} else {
	    if (dbtable) {
		TDatime beginTime(dbtable->getBeginTime());
		date = beginTime.GetDate();
		time = beginTime.GetTime();
	    }
	}
	TString filename = TString::Format("%s/%s.%08i.%06i.root", dir, tableName, date, time);
	LOG_INFO << "Writing table " << tableName << " to file " << filename << endm;
	TFile *f = new TFile(filename, "RECREATE");
	if (f && f->IsOpen()) {
	    ttable->Write(tableName, TObject::kOverwrite);
	    f->Close();
	    delete f;
	    f = 0;
	} else {
	    LOG_ERROR << "Cannot write to file " << filename << endm;
	}
    }
}

void StZDCDb::exportROOTDb(const Char_t *dir, UChar_t saveTables) const {
    if (!mInitialized) init();
    if (dir) {
	LOG_DEBUG << "Exporting ROOT DB files to directory " << dir << endm;
	if (saveTables & ZDCCALPARS_TABLE) this->writeROOTDbTable(dir, ZdcCalParsName, mZdcCalParsTTable, mZdcCalParsTable);
	if (saveTables & ZDCSMDPED_TABLE) this->writeROOTDbTable(dir, zdcsmdPedName, mZdcSmdPedTTable, mZdcSmdPedTable);
	if (saveTables & ZDCSMDGAIN_TABLE) this->writeROOTDbTable(dir, zdcsmdGainName, mZdcSmdGainTTable, mZdcSmdGainTable);
	if (saveTables & ZDCSMDBEAMCENTER_TABLE) this->writeROOTDbTable(dir, zdcsmdBeamCenterName, mZdcSmdBeamCenterTTable, mZdcSmdBeamCenterTable);
	if (saveTables & ZDCCALIB_TABLE) this->writeROOTDbTable(dir, zdcCalibName, mZdcCalibTTable, mZdcCalibTable);
	LOG_DEBUG << "Finished exporting ROOT DB files to directory " << dir << endm;
    }
}

void StZDCDb::SetDateTime(int date, int time) {
    if (!mInitialized) init();
    if (mTextDbFilename) {
        mBeginTime.Set(date, time);
    } else if (mChain) {
    } else {
        mBeginTime.Set(date, time);
        unsigned unix = TUnixTime::Convert(mBeginTime, true);
        if ( !((mZdcCalParsTable->getBeginTime() < unix) && (unix < mZdcCalParsTable->getEndTime())) ) {
            mZdcCalParsDirty = true;
        }
        if ( !((mZdcSmdPedTable->getBeginTime() < unix) && (unix < mZdcSmdPedTable->getEndTime())) ) {
            mZdcSmdPedDirty = true;
        }
        if ( !((mZdcSmdGainTable->getBeginTime() < unix) && (unix < mZdcSmdGainTable->getEndTime())) ) {
            mZdcSmdGainDirty = true;
        }
        if ( !((mZdcSmdBeamCenterTable->getBeginTime() < unix) && (unix < mZdcSmdBeamCenterTable->getEndTime())) ) {
            mZdcSmdBeamCenterDirty = true;
        }
        if ( !((mZdcCalibTable->getBeginTime() < unix) && (unix < mZdcCalibTable->getEndTime())) ) {
            mZdcCalibDirty = true;
        }
    }
}

void StZDCDb::SetFlavor(const char *flavor, const char *tablename) {
    if (!mInitialized) init();
    if (mTextDbFilename) {
    } else if (mChain) {
    } else {
        if (!tablename || !strcmp(tablename, ZdcCalParsName))  {
            if(strcmp(mZdcCalParsTable->getFlavor(), flavor)) {
                mZdcCalParsTable->setFlavor(flavor);
                mZdcCalParsDirty = true;
            }
        }
        if (!tablename || !strcmp(tablename, zdcsmdPedName))  {
            if(strcmp(mZdcSmdPedTable->getFlavor(), flavor)) {
                mZdcSmdPedTable->setFlavor(flavor);
                mZdcSmdPedDirty = true;
            }
        }
        if (!tablename || !strcmp(tablename, zdcsmdGainName))  {
            if(strcmp(mZdcSmdGainTable->getFlavor(), flavor)) {
                mZdcSmdGainTable->setFlavor(flavor);
                mZdcSmdGainDirty = true;
            }
        }
        if (!tablename || !strcmp(tablename, zdcsmdBeamCenterName))  {
            if(strcmp(mZdcSmdBeamCenterTable->getFlavor(), flavor)) {
                mZdcSmdBeamCenterTable->setFlavor(flavor);
                mZdcSmdBeamCenterDirty = true;
            }
        }
        if (!tablename || !strcmp(tablename, zdcCalibName))  {
            if(strcmp(mZdcCalibTable->getFlavor(), flavor)) {
                mZdcCalibTable->setFlavor(flavor);
                mZdcCalibDirty = true;
            }
        }
    }
}

void StZDCDb::SetMaxEntryTime(int date, int time) {
    if (!mInitialized) init();
    if (mTextDbFilename) {
    } else if (mChain) {
    } else {
        unsigned unixMax = TUnixTime::Convert(TDatime(date,time), true);
        if (mZdcCalParsTable->getProdTime() != unixMax) {
            mZdcCalParsTable->setProdTime(unixMax);
            mZdcCalParsDirty = true;
        }
        if (mZdcSmdPedTable->getProdTime() != unixMax) {
            mZdcSmdPedTable->setProdTime(unixMax);
            mZdcSmdPedDirty = true;
        }
        if (mZdcSmdGainTable->getProdTime() != unixMax) {
            mZdcSmdGainTable->setProdTime(unixMax);
            mZdcSmdGainDirty = true;
        }
        if (mZdcSmdBeamCenterTable->getProdTime() != unixMax) {
            mZdcSmdBeamCenterTable->setProdTime(unixMax);
            mZdcSmdBeamCenterDirty = true;
        }
        if (mZdcCalibTable->getProdTime() != unixMax) {
            mZdcCalibTable->setProdTime(unixMax);
            mZdcCalibDirty = true;
        }
    }
}

const ZdcCalPars_st* StZDCDb::getZdcCalPars() const {
    if (!mInitialized) init();
    maybe_reload_ZdcCalPars();
    return mZdcCalParsTTable ? mZdcCalParsTTable->GetTable() : 0;
}
ZdcCalPars_st* StZDCDb::getZdcCalPars() {
    if (!mInitialized) init();
    maybe_reload_ZdcCalPars();
    return mZdcCalParsTTable ? mZdcCalParsTTable->GetTable() : 0;
}

const zdcsmdPed_st* StZDCDb::getZdcSmdPed() const {
    if (!mInitialized) init();
    maybe_reload_ZdcSmdPed();
    return mZdcSmdPedTTable ? mZdcSmdPedTTable->GetTable() : 0;
}
zdcsmdPed_st* StZDCDb::getZdcSmdPed() {
    if (!mInitialized) init();
    maybe_reload_ZdcSmdPed();
    return mZdcSmdPedTTable ? mZdcSmdPedTTable->GetTable() : 0;
}

const zdcsmdGain_st* StZDCDb::getZdcSmdGain() const {
    if (!mInitialized) init();
    maybe_reload_ZdcSmdGain();
    return mZdcSmdGainTTable ? mZdcSmdGainTTable->GetTable() : 0;
}
zdcsmdGain_st* StZDCDb::getZdcSmdGain() {
    if (!mInitialized) init();
    maybe_reload_ZdcSmdGain();
    return mZdcSmdGainTTable ? mZdcSmdGainTTable->GetTable() : 0;
}

const zdcsmdBeamCenter_st* StZDCDb::getZdcSmdBeamCenter() const {
    if (!mInitialized) init();
    maybe_reload_ZdcSmdBeamCenter();
    return mZdcSmdBeamCenterTTable ? mZdcSmdBeamCenterTTable->GetTable() : 0;
}
zdcsmdBeamCenter_st* StZDCDb::getZdcSmdBeamCenter() {
    if (!mInitialized) init();
    maybe_reload_ZdcSmdBeamCenter();
    return mZdcSmdBeamCenterTTable ? mZdcSmdBeamCenterTTable->GetTable() : 0;
}

const zdcCalib_st* StZDCDb::getZdcCalib() const {
    if (!mInitialized) init();
    maybe_reload_ZdcCalib();
    return mZdcCalibTTable ? mZdcCalibTTable->GetTable() : 0;
}
zdcCalib_st* StZDCDb::getZdcCalib() {
    if (!mInitialized) init();
    maybe_reload_ZdcCalib();
    return mZdcCalibTTable ? mZdcCalibTTable->GetTable() : 0;
}

/* Private methods used for caching SQL query results */
void StZDCDb::maybe_reload_ZdcCalPars() const {
        if (mTextDbFilename) {
            if (mZdcCalParsDirty) reloadTextDb();
            mZdcCalParsDirty = false;
        } else if (mChain) {
            if (!mZdcCalParsTTable) {
		LOG_DEBUG << "getting DB " << trgDbName << endm;
                TDataSet *DB = mChain->GetInputDB(trgDbName);
                if (DB) {
		    LOG_DEBUG << TString::Format("got DB=0x%08x", DB) << endm;
		    LOG_DEBUG << "getting table " << ZdcCalParsName << " in database " << trgDbName << endm;
		    mZdcCalParsTTable = static_cast<St_ZdcCalPars*>(DB->Find(ZdcCalParsName));
		    if (mZdcCalParsTTable) {
			LOG_DEBUG << TString::Format("got mZdcCalParsTTable=0x%08x", mZdcCalParsTTable) << endm;
		    } else {
			LOG_ERROR << "Cannot get table " << ZdcCalParsName << " in database " << trgDbName << endm;
		    }
		} else {
		    LOG_ERROR << "Cannot get database " << trgDbName << endm;
		}
            }
	} else {
            if (mZdcCalParsDirty) reload_dbtable(mZdcCalParsTable);
            mZdcCalParsDirty = false;
        }
}

void StZDCDb::maybe_reload_ZdcSmdPed() const {
        if (mTextDbFilename) {
            if (mZdcSmdPedDirty) reloadTextDb();
            mZdcSmdPedDirty = false;
        } else if (mChain) {
            if (!mZdcSmdPedTTable) {
		LOG_DEBUG << "getting DB " << zdcDbName << endm;
                TDataSet *DB = mChain->GetInputDB(zdcDbName);
                if (DB) {
		    LOG_DEBUG << TString::Format("got DB=0x%08x", DB) << endm;
		    LOG_DEBUG << "getting table " << zdcsmdPedName << " in database " << zdcDbName << endm;
		    mZdcSmdPedTTable = static_cast<St_zdcsmdPed*>(DB->Find(zdcsmdPedName));
		    if (mZdcSmdPedTTable) {
			LOG_DEBUG << TString::Format("got mZdcSmdPedTTable=0x%08x", mZdcSmdPedTTable) << endm;
		    } else {
			LOG_ERROR << "Cannot get table " << zdcsmdPedName << " in database " << zdcDbName << endm;
		    }
		} else{
		    LOG_ERROR << "Cannot get database " << zdcDbName << endm;
		}
            }
        } else {
            if (mZdcSmdPedDirty) reload_dbtable(mZdcSmdPedTable);
            mZdcSmdPedDirty = false;
        }
}

void StZDCDb::maybe_reload_ZdcSmdGain() const {
        if (mTextDbFilename) {
            if (mZdcSmdGainDirty) reloadTextDb();
            mZdcSmdGainDirty = false;
        } else if (mChain) {
            if (!mZdcSmdGainTTable) {
		LOG_DEBUG << "getting DB " << zdcDbName << endm;
                TDataSet *DB = mChain->GetInputDB(zdcDbName);
                if (DB) {
		    LOG_DEBUG << TString::Format("got DB=0x%08x", DB) << endm;
		    LOG_DEBUG << "getting table " << zdcsmdGainName << " in database " << zdcDbName << endm;
		    mZdcSmdGainTTable = static_cast<St_zdcsmdGain*>(DB->Find(zdcsmdGainName));
		    if (mZdcSmdGainTTable) {
			LOG_DEBUG << TString::Format("got mZdcSmdGainTTable=0x%08x", mZdcSmdGainTTable) << endm;
		    } else {
			LOG_ERROR << "Cannot get table " << zdcsmdGainName << " in database " << zdcDbName << endm;
		    }
		} else {
		    LOG_ERROR << "Cannot get database " << zdcDbName << endm;
		}
            }
        } else {
            if (mZdcSmdGainDirty) reload_dbtable(mZdcSmdGainTable);
            mZdcSmdGainDirty = false;
        }
}

void StZDCDb::maybe_reload_ZdcSmdBeamCenter() const {
        if (mTextDbFilename) {
            if (mZdcSmdBeamCenterDirty) reloadTextDb();
            mZdcSmdBeamCenterDirty = false;
        } else if (mChain) {
            if (!mZdcSmdBeamCenterTTable) {
		LOG_DEBUG << "getting DB " << zdcDbName << endm;
                TDataSet *DB = mChain->GetInputDB(zdcDbName);
                if (DB) {
		    LOG_DEBUG << TString::Format("got DB=0x%08x", DB) << endm;
		    LOG_DEBUG << "getting table " << zdcsmdBeamCenterName << " in database " << zdcDbName << endm;
		    mZdcSmdBeamCenterTTable = static_cast<St_zdcsmdBeamCenter*>(DB->Find(zdcsmdBeamCenterName));
		    if (mZdcSmdBeamCenterTTable) {
			LOG_DEBUG << TString::Format("got mZdcSmdBeamCenterTTable=0x%08x", mZdcSmdBeamCenterTTable) << endm;
		    } else {
			LOG_ERROR << "Cannot get table " << zdcsmdBeamCenterName << " in database " << zdcDbName << endm;
		    }
		} else {
		    LOG_ERROR << "Cannot get database " << zdcDbName << endm;
		}
            }
        } else {
            if (mZdcSmdBeamCenterDirty) reload_dbtable(mZdcSmdBeamCenterTable);
            mZdcSmdBeamCenterDirty = false;
        }
}

void StZDCDb::maybe_reload_ZdcCalib() const {
        if (mTextDbFilename) {
            if (mZdcCalibDirty) reloadTextDb();
            mZdcCalibDirty = false;
        } else if (mChain) {
            if (!mZdcCalibTTable) {
		LOG_DEBUG << "getting DB " << zdcDbName << endm;
                TDataSet *DB = mChain->GetInputDB(zdcDbName);
                if (DB) {
		    LOG_DEBUG << TString::Format("got DB=0x%08x", DB) << endm;
		    LOG_DEBUG << "getting table " << zdcCalibName << " in database " << zdcDbName << endm;
		    mZdcCalibTTable = static_cast<St_zdcCalib*>(DB->Find(zdcCalibName));
		    if (mZdcCalibTTable) {
			LOG_DEBUG << TString::Format("got mZdcCalibTTable=0x%08x", mZdcCalibTTable) << endm;
		    } else {
			LOG_ERROR << "Cannot get table " << zdcCalibName << " in database " << zdcDbName << endm;
		    }
		} else {
		    LOG_ERROR << "Cannot get database " << zdcDbName << endm;
		}
            }
        } else {
            if (mZdcCalibDirty) reload_dbtable(mZdcCalibTable);
            mZdcCalibDirty = false;
        }
}

void StZDCDb::reload_dbtable(StDbTable* table) const {
    LOG_DEBUG << "reload_dbtable using StDbManager, table name: " << table->getName() << ", " << table->getMyName() << ", " << table->getDbName() << ", " << table->getCstructName() << endm;
    StDbManager *mgr = StDbManager::Instance();
    mgr->setVerbose(false);
    mgr->setRequestTime(mBeginTime.AsSQLString());
    mgr->fetchDbTable(table);
}

Float_t StZDCDb::getSlewingCoeff(StBeamDirection eastwest, Int_t order) const {
    Float_t val = 0;
    if ((order >= 0) && (order <= 3)) {
	const ZdcCalPars_st *t = this->getZdcCalPars();
	if (t) {
	if (eastwest == east) {
	    if (order == 0) {
		val = t->EAP0;
	    } else if (order == 1) {
		val = t->EAP1;
	    } else if (order == 2) {
		val = t->EAP2;
	    } else if (order == 3) {
		val = t->EAP3;
	    }
	} else if (eastwest == west)  {
	    if (order == 0) {
		val = t->WAP0;
	    } else if (order == 1) {
		val = t->WAP1;
	    } else if (order == 2) {
		val = t->WAP2;
	    } else if (order == 3) {
		val = t->WAP3;
	    }
	}
	}
    }
    return val;
}

Float_t StZDCDb::getVertexVPAR() const {
    Float_t val = 0;
    const ZdcCalPars_st *t = this->getZdcCalPars();
    if (t) val = t->VPAR;
    return val;
}

Float_t StZDCDb::getVertexOFF() const {
    Float_t val = 0;
    const ZdcCalPars_st *t = this->getZdcCalPars();
    if (t) val = t->OFF;
    return val;
}
	
Float_t StZDCDb::getZdcSmdPed(StBeamDirection eastwest, Int_t horiz, Int_t strip) const {
    Float_t val = 0;
    if (((eastwest == east) || (eastwest == west)) && (horiz >= 0) && (horiz < 2) && (strip >= 0) && (strip < 8)) {
	const zdcsmdPed_st *t = this->getZdcSmdPed();
	if (t) val = t->ZdcsmdPedestal[((eastwest == east ? 0 : 1) * 2 * 8) + (horiz * 8) + strip];
    }
    return val;
}

Float_t StZDCDb::getZdcSmdGain(StBeamDirection eastwest, Int_t horiz, Int_t strip) const {
    Float_t val = 0;
    if (((eastwest == east) || (eastwest == west)) && (horiz >= 0) && (horiz < 2) && (strip >= 0) && (strip < 8)) {
	const zdcsmdGain_st *t = this->getZdcSmdGain();
	if (t) val = t->ZdcsmdGain[((eastwest == east ? 0 : 1) * 2 * 8) + (horiz * 8) + strip];
    }
    return val;
}

Float_t StZDCDb::getZdcSmdCalib(StBeamDirection eastwest, Int_t horiz) const {
    Float_t val = 0;
    if (((eastwest == east) || (eastwest == west)) && (horiz >= 0) && (horiz < 2)) {
	const zdcsmdGain_st *t = this->getZdcSmdGain();
	if (t) val = t->ZdcsmdCalib[((eastwest == east ? 0 : 1) * 2) + horiz];
    }
    return val;
}

Float_t StZDCDb::getZdcPedestal(StBeamDirection eastwest, Int_t tower) const {
    Float_t val = 0;
    if (((eastwest == east) || (eastwest == west)) && (tower >= 0) && (tower < 4)) {
	const zdcCalib_st *t = this->getZdcCalib();
	if (t) val = t->ZdcPedestal[((eastwest == east ? 0 : 1) * 4) + tower];
    }
    return val;
}

Float_t StZDCDb::getZdcCalib(StBeamDirection eastwest, Int_t tower) const {
    Float_t val = 0;
    if (((eastwest == east) || (eastwest == west)) && (tower >= 0) && (tower < 4)) {
	const zdcCalib_st *t = this->getZdcCalib();
	if (t) val = t->ZdcCalib[((eastwest == east ? 0 : 1) * 4) + tower];
    }
    return val;
}
