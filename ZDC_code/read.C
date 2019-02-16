void loadLibs() {
    {
        Info(__FILE__, "Enabling Logger...");
        gROOT->LoadMacro("LoadLogger.C");
	LoadLogger();
    }

#define LOAD_LIB(NAME) { Info(__FILE__, "Loading %s", NAME); gSystem->Load(NAME); }
    LOAD_LIB("RTS");
    LOAD_LIB("StBichsel");
    LOAD_LIB("StEvent");
    LOAD_LIB("libStDb_Tables");
    LOAD_LIB("StChain");
    LOAD_LIB("StDbLib");
    LOAD_LIB("StZDCUtil");
    LOAD_LIB("StZDCPlots");
    LOAD_LIB("StZDCPolarimetryPlots");
    LOAD_LIB("StZDCOnlineMonitoring");
#undef LOAD_LIB
}

Bool_t checkStatusRUN(const Char_t *statusFilename) {
	Bool_t run = false;
        if (statusFilename && statusFilename[0]) {
	    TString status;
	    ifstream fstr(statusFilename);
	    if (fstr.good()) fstr >> status;
	    run = (status == "RUN");
	}
	return run;
}

Bool_t checkPIDFile(const Char_t *pidFilename) {
    Bool_t processExists = false;
    FileStat_t pidStat;
    if (pidFilename && pidFilename[0] && (gSystem->GetPathInfo(pidFilename, pidStat) == 0)) {
    	processExists = true;
    }
    return processExists;
}

void writePIDToFile(const Char_t *pidFilename) {
    if (pidFilename && pidFilename[0]) {
        ofstream fstr(pidFilename);
        if (fstr.good()) {
            fstr << gSystem->GetPid();
            fstr.close();
        } else {
            cerr << "Cannot write process ID to file " << pidFilename << endl;
        }
    }
}

const Char_t *emailNotification = 0;
const Char_t *publicDir = 0;
const Char_t *dirOut = 0;
const Char_t *filename = 0;
int nEvents = -1;
const Char_t *zdcPedCalFilename = 0;
const Char_t *seenRuns = 0;
const Char_t *statusFilename = 0;
Int_t sleepSeconds = 0;
const Char_t *dbconnect = 0;
const Char_t *cdevFilename = 0;
const Char_t *dbconnectCdev = 0;
Int_t dbLookBackSeconds = 0;
const Char_t *namesToProcess = 0;
const Char_t *namesToCalibrateSMDPed = 0;
Int_t minEventsToCalibrateSMDPed = 0;
const Char_t *namesToCalibrateSMDGain = 0;
Int_t minEventsToCalibrateSMDGain = 0;
const Char_t *namesToCalibrateTowerPed = 0;
Int_t minEventsToCalibrateTowerPed = 0;
const Char_t *namesToCalibrateTowerGain = 0;
Int_t minEventsToCalibrateTowerGain = 0;
const Char_t *namesToCalibrateSlewing = 0;
Int_t minEventsToCalibrateSlewing = 0;
Bool_t searchForOtherRunFiles = true;
Bool_t stopRunOnFirstError = true;// xuyifei
Bool_t deleteCanvasOnExit = false;
Bool_t updateInputTextDb = false;
Int_t prepost = 0;
Bool_t filterOutBadRuns = false;
const Char_t *namesToAddFill = 0;
Int_t minEventsToAddFill = 0;
const Char_t *outFormats = 0;
const Char_t *outFillFormats = 0;
TString tcuBitsToProcess = "";
const Char_t *pidFilename = "./read.pid";
const Char_t *stdoutFilename = "./read.out";
const Char_t *stdoutMode = "a";
Float_t normAnalyzingPower = 0;

void read(const Char_t *configFile = "config.C") {
    loadLibs();
    TSimpleSignalHandler stopSignal(kSigTermination, kFALSE);
    TSimpleSignalHandler reloadSignal(kSigUser1, kFALSE);
    TSimpleSignalHandler wakeupSignal(kSigUser2, kFALSE);
    reloadSignal.Add();
    wakeupSignal.Add();
    stopSignal.Add();

    TString status;
    Int_t iLoop = 0;
    Bool_t firstConfigLoaded = false;
    Bool_t anotherInstanceRunning = false;
    do {
	//cout << "Starting next loop... (" << iLoop << ")" << endl;
        if (!firstConfigLoaded || reloadSignal.isTriggered()) {
	    TString stdoutFilenameOld = stdoutFilename;
	    TString pidFilenameOld = pidFilename;
	    Int_t configFileError;
	    cout << "Reading configuration file " << configFile << endl;
	    gROOT->Macro(configFile, &configFileError);
	    if (configFileError != 0) {
		cerr << "Error code " << configFileError << endl;
	    } else {
		if (!firstConfigLoaded || (stdoutFilenameOld != stdoutFilename)) {
		    if (stdoutFilename && stdoutFilename[0]) {
			cout << "Redirecting output to file " << stdoutFilename << endl;
			if (gSystem->RedirectOutput(stdoutFilename, stdoutMode) != 0) {
			    cerr << "Cannot redirect output to file " << stdoutFilename << endl;
			}
		    }
		}
		if (!firstConfigLoaded || (pidFilenameOld != pidFilename)) {
		    if (checkPIDFile(pidFilename)) {
    			cerr << "Another instance is running: found PID file " << pidFilename << endl;
			anotherInstanceRunning = true;
			stopSignal.set(true);
		    } else {
			writePIDToFile(pidFilename);
			if (firstConfigLoaded && (pidFilenameOld != "")) gSystem->Unlink(pidFilenameOld);
	    	    }
		}
		firstConfigLoaded = true;
		reloadSignal.reset();
	    }
	}
	gROOT->ProcessLine("");
	TString filenameToUse = filename;
	const Char_t *dbFileList = "./dbquery.list";
	if (!stopSignal.isTriggered() && dbconnect && filename) {
	    filenameToUse = dbFileList;
    	    gSystem->Unlink(dbFileList);
	    TDatime nowTime;
	    nowTime.Set(nowTime.Convert(kTRUE));
	    Int_t endBackSeconds = 0*60;
	    TDatime beginTime(nowTime.Convert() - dbLookBackSeconds - endBackSeconds);
	    TDatime endTime(nowTime.Convert() - endBackSeconds);
	    writeQueryRunList(dbconnect, filename, beginTime, endTime, filterOutBadRuns, dbFileList);
	}
	if (!stopSignal.isTriggered()) {
	    StReadTrg::readTrg(filenameToUse, nEvents, 
		configFile,
		zdcPedCalFilename, 
		seenRuns, 
		dirOut, 
		publicDir, 
		emailNotification, 
		namesToProcess,
		namesToCalibrateSMDPed,
		minEventsToCalibrateSMDPed,
		namesToCalibrateSMDGain,
		minEventsToCalibrateSMDGain,
		namesToCalibrateTowerPed,
		minEventsToCalibrateTowerPed,
		namesToCalibrateTowerGain,
		minEventsToCalibrateTowerGain,
		namesToCalibrateSlewing,
		minEventsToCalibrateSlewing,
		searchForOtherRunFiles, 
		stopRunOnFirstError,
		deleteCanvasOnExit,
		updateInputTextDb,
		sleepSeconds,
		prepost,
		dbconnectCdev,
		cdevFilename,
		namesToAddFill,
		minEventsToAddFill,
		outFormats,
		outFillFormats,
		tcuBitsToProcess,
		normAnalyzingPower
	    );
	    iLoop++;
	}
	if (sleepSeconds > 0) {
	    //cout << "Sleeping for " << sleepSeconds << " seconds..." << endl;
	    wakeupSignal.reset();
	    Int_t sleepSecAtATime = 2;
	    for (Int_t i = 0;!stopSignal.isTriggered() && !wakeupSignal.isTriggered() && checkStatusRUN(statusFilename) && (i <= (sleepSeconds / sleepSecAtATime));i++) {
		gROOT->ProcessLine("");
		gSystem->Sleep((i ? sleepSecAtATime : (sleepSeconds % sleepSecAtATime)) * 1000);
	    }
	}
    } while (!stopSignal.isTriggered() && checkStatusRUN(statusFilename) && (sleepSeconds > 0));
    reloadSignal.Remove();
    stopSignal.Remove();
    if (!anotherInstanceRunning && pidFilename && pidFilename[0]) {
        gSystem->Unlink(pidFilename);
    }
}
