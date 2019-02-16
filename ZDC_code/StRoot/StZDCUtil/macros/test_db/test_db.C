//-----------------------------------

void loadLibs(bool useStDbMaker);

class StZDCDb;
class St_db_Maker;
bool testRun(const StZDCDb *db, St_db_Maker *dbMaker, int date, int time, const Char_t *validOutputDir, const Char_t *tmpOutputDir);

bool testDb(bool doTest, bool doTestStDbMaker);

int test_db(bool doTest = true);

//-----------------------------------

void loadLibs() {
    {
        Info(__FILE__, "Enabling Logger...");
        gROOT->LoadMacro("LoadLogger.C");
	LoadLogger();
    }

#define LOAD_LIB(NAME) { Info(__FILE__, "Loading %s", NAME); gSystem->Load(NAME); }
    LOAD_LIB("libStDb_Tables");
    LOAD_LIB("StChain");
    LOAD_LIB("StDbLib");
    LOAD_LIB("StDbBroker");
    LOAD_LIB("StUtilities");
    LOAD_LIB("St_db_Maker");

    LOAD_LIB("StZDCUtil");
#undef LOAD_LIB
}

class StZDCDb;
class St_db_Maker;
class StZDCDb;
bool testRun(const StZDCDb *db, St_db_Maker *dbMaker, int date, int time, const Char_t *validOutputDir, const Char_t *tmpOutputDir) {
    bool ok = false;

    Info("testRun", TString::Format("testing %08i.%06i", date, time));
    if (db && validOutputDir && tmpOutputDir && gSystem) {
	TString validOutFile = TString::Format("%s/test.%08i.%06i/zdcPedCal.txt", validOutputDir, date, time);
	TString tmpOutFile = TString::Format("%s/zdcPedCal.txt", tmpOutputDir, date, time);
	if (dbMaker) {
	    dbMaker->SetDateTime(date, time);
	    dbMaker->Make();
	} else {
	    db->setTextDb(validOutFile.Data());
	}
	db->exportTextDb(tmpOutFile.Data());
	Int_t ret = gSystem->Exec(TString::Format("diff %s %s", tmpOutFile.Data(), validOutFile.Data()));
	Info("testRun", TString::Format("test %08i.%06i: ret=%i", date, time, ret));
	ok = !ret;
    }
    if (ok) {
	Info("test_db", TString::Format("Test PASSED (%08i.%06i)", date, time));
    } else {
	Error("test_db", TString::Format("Test NOT passed! (%08i.%06i)", date, time));
    }

    return ok;
}

bool testDb(bool doTest, bool doTestStDbMaker) {
    bool ok = false;
    loadLibs();

    bool useStDbMaker = false;
    bool useTextDb = true;
    bool useChain = false;
    if (doTest) useStDbMaker = doTestStDbMaker;

    const Char_t *zdcPedCalFilename = "StRoot/StZDCUtil/macros/test_db/zdcPedCal.txt";
    const Char_t *zdcPedCalDir = "StRoot/StZDCUtil/macros/test_db/StarDb";
    const Char_t *zdcPedCalFilenameOut = "./zdcPedCal.txt";
    const Char_t *zdcPedCalDirOut = ".";
    const Char_t *testOutputRootDir = "StRoot/StZDCUtil/macros/test_db/";

    StChain *chain = 0;
    St_db_Maker *dbMaker = 0;

    if (useStDbMaker) {
	    if (useChain) {
		Info("test_db", "Creating StChain...");
		chain = new StChain("test_db");
		if (chain) {
		    Info("test_db", "Created StChain=0x%08x", chain);
		} else {
		    Error("test_db", "Cannot create StChain");
		}
	    }
	    Info("test_db", "Creating St_db_Maker...");
	    if (doTest) {
		dbMaker = new St_db_Maker("db", zdcPedCalDir, "MySQL:StarDb", "$STAR/StarDb");
	    } else {
		//dbMaker = new St_db_Maker("db", zdcPedCalDir, "MySQL:StarDb", "$STAR/StarDb");
		dbMaker = new St_db_Maker("db", "MySQL:StarDb", "$STAR/StarDb");
	    }
	    if (dbMaker) {
		Info("test_db", "Created St_db_Maker=0x%08x", dbMaker);
	    } else {
		Error("test_db", "Cannot create St_db_Maker");
	    }

	    if (chain) {
		Info("test_db", "Initializing chain...");
		chain->Init();
		Info("test_db", "Finished initializing chain.");
	    } else if (dbMaker) {
		Info("test_db", "Initializing dbMaker...");
		dbMaker->Init();
		Info("test_db", "Finished initializing dbMaker.");
	    }

	    if (!doTest) {
		if (dbMaker) {
		    dbMaker->SetDebug(1000);
		    dbMaker->SetDateTime(20100101, 000000);
		}

		if (chain) {
		    Info("test_db", "Starting chain->Make()...");
		    chain->Make();
		    Info("test_db", "Finished chain->Make().");
		} else if (dbMaker) {
		    Info("test_db", "Starting dbMaker->Make()...");
		    dbMaker->Make();
		    Info("test_db", "Finished dbMaker->Make().");
		}
	    }
    }

    Info("test_db", "Creating StZDCDb...");
    StZDCDb *db = new StZDCDb();
    if (db) {
	Info("test_db", "Created StZDCDb=0x%08x", db);
	ok = true;
	if (doTest) {
	    ok &= testRun(db, dbMaker, 20100101, 0, testOutputRootDir, zdcPedCalDirOut);
	    ok &= testRun(db, dbMaker, 20100101, 10, testOutputRootDir, zdcPedCalDirOut);
	} else {
	    if (useStDbMaker) {
	    } else if (useTextDb) {
		db->setTextDb(zdcPedCalFilename);
	    } else {
		db->SetDateTime(20100101, 000000);
	    }

	    Info("test_db", "Exporting text DB to file %s ...", zdcPedCalFilenameOut);
	    db->exportTextDb(zdcPedCalFilenameOut);
	    Info("test_db", "Finished exporting text DB.");

	    Info("test_db", "Exporting ROOT DB to directory %s ...", zdcPedCalDirOut);
	    db->exportROOTDb(zdcPedCalDirOut);
	    Info("test_db", "Finished exporting ROOT DB.");
	}
    } else {
	Error("test_db", "Cannot create StZDCDb");
    }

    if (false && chain) {
	Info("test_db", "Full chain listing...");
	chain->ls(0);
	Info("test_db", "Finished listing chain.");
    }

    return ok;
}

int test_db(bool doTest) {
    bool ok = true;
    ok &= testDb(doTest, true);
    if (doTest) ok &= testDb(doTest, false);
    if (doTest) if (ok) {
	cout << endl << "All tests PASSED" << endl << endl;
    } else {
	cerr << endl << "Tests NOT passed!" << endl << endl;
    }
    return ok;
}
