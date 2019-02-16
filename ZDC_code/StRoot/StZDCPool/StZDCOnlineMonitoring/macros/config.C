//-----------------------------------------------------------------
//
// This is a configuration file for the ZDC Online Monitoring job
//
// To start the job:
// > ./read &
//
// If you make changes in this file, send a signal to the running 
// instance to reload configuration:
// > kill -USR1 `cat ./read.pid`
//
// To stop the running job:
// > kill `cat ./read.pid`
// or
// > echo STOP >| ./status.txt
// It will exit after finishing the current cycle, if any
//
// If it still does not exit, kill it:
// > kill -9 `cat ./read.pid`
// Then delete ./read.pid, otherwise next time it will 
// complain that another instance is running
//
// Oleksandr Grebenyuk
// ogrebenyuk@lbl.gov
// Feb 2009
//
//-----------------------------------------------------------------

{
  // Style of graphics
  gROOT->Macro("style.C");

    // Space separated email addresses to be notified when new results appear
    // Set to NULL or empty string to turn off
    emailNotification = "kramaluk@fjfi.cvut.cz";

    // Write output to this directory...
    //dirOut = "/afs/rhic.bnl.gov/star/doc_protected/www/spin/ogrebeny/zdc2009";
    ////dirOut = "/onlineweb/www/zdc";
    //dirOut = "/onlineweb/www/test/ogrebeny/zdc2009";
    dirOut = "/afs/rhic.bnl.gov/star/doc_protected/www/spin/ogrebeny/zdc";
    
    // ... and assume it is visible at this URL
    //publicDir = "http://www.star.bnl.gov/protected/spin/ogrebeny/zdc2009";
    //publicDir = "http://online.star.bnl.gov/zdc";
    publicDir = "http://www.star.bnl.gov/protected/spin/ogrebeny/zdc/";


    ////Bool_t queryDb = true;
    Bool_t queryDb = false;//CJ

    if (queryDb) {
	// Query database to get list of runs and their respective names and timestamps
        dbconnect = "mysql://onldb.starp.bnl.gov:3501/RunLog";
	// Look for runs in the last N seconds
	dbLookBackSeconds = 2*60*60;
	// Reject runs marked as bad "RTS" or "ShiftLeader"?
	filterOutBadRuns = false;
	// Use this mask to create file list for processing
	// RUNID is a placeholder for run number, 
	// NAME for the configuration name (pedestal, pedAsPhys, laser, etc.),
	// DATE.TIME for the timestamp of the run
        filename = "/trg/trgdata/runRUNID.*.dat NAME DATE.TIME";
    } else {
	// Process these files
	// Recognized formats are:
	// 		/path/to/run11999888.1.dat
	// 		/path/to/run11999888.*.dat
	// 		/path/to/*.dat
	// 		/path/to/file.list
	// and the same for .daq
	// Lists can be nested
	// After the file name, the run configuration name and timestamp may be added
	// If no timestamp specified, it will be taken from the earliest event in the daq file,
	// or from the current time in case of a dat file
	//filename = "/trg/trgdata/run10065061.*.dat";
	////filename = "./run10065061.list";

      filename = "./trg/trgdata/run10087018.*.dat";
      searchForOtherRunFiles = true;
    }

    // File with the latest CDEV record to use
    // If NULL or empty, connect to the DB
    //cdevFilename = "/onlineweb/www/cdev/outCdev.eve";
    // Connection string for CDEV query
    dbconnectCdev = "mysql://onldb.starp.bnl.gov:3502/Conditions_rhic";

    // Number of events to process per run, <=0 means all
    nEvents = -1;

    // Which runs to process (regular expression)
    // Set to NULL or empty string to take all requested
    //namesToProcess = "pedestal|pedAsPhys|ZDC|zdc|BBC";
    //namesToProcess = "ZDC|zdc|BBC";
    //namesToProcess = "zdc_pol";

    ////namesToProcess = "zdc_e|zdc_w|zdcE|zdcW|zdc_smd";
    namesToProcess = "";//CJ

    // Which triggers (TCU bit combinations) to process
    Int_t BBCTAC = (1 << 0);
    Int_t BBCE = (1 << 1);
    Int_t BBCW = (1 << 2);
    Int_t ZDCTAC = (1 << 3);
    Int_t ZDCES = (1 << 4);
    Int_t ZDCWS = (1 << 5);
    Int_t ZDCEF = (1 << 6);
    Int_t ZDCEB = (1 << 7);
    Int_t ZDCWF = (1 << 8);
    Int_t ZDCWB = (1 << 9);
    Int_t ZDCMB = (1 << 9);
    Int_t ZDCpol = (1 << 9);
    //tcuBitsToProcess = TString::Format("%i %i", ZDCMB, ZDCpol);
    //tcuBitsToProcess = TString::Format("%i", 
    /*
    tcuBitsToProcess = TString::Format("%i %i %i %i %i %i %i %i %i %i", 
	BBCE|ZDCEF|ZDCEB, 
	BBCE|ZDCWF|ZDCWB, 
	BBCW|ZDCEF|ZDCEB, 
	BBCW|ZDCWF|ZDCWB, 
	BBCE|BBCW|ZDCEF|ZDCEB, 
	BBCE|BBCW|ZDCWF|ZDCWB, 
	BBCE|BBCW|ZDCES|ZDCEF|ZDCEB, 
	BBCE|BBCW|ZDCWS|ZDCWF|ZDCWB,
	ZDCES|ZDCEF|ZDCEB,
	ZDCWS|ZDCWF|ZDCWB
    );
    */
    tcuBitsToProcess ="";//CJ


    // Which runs to use for calibration
    // Set to NULL or empty string to take all processed
    namesToCalibrateSMDPed = "ZDC|zdc|BBC";
    namesToCalibrateSMDGain = "ZDC|zdc";
    namesToCalibrateTowerPed = "none";
    namesToCalibrateTowerGain = "none";
    namesToCalibrateSlewing = "none";
    // Minimum number of events in a run required for calibration
    minEventsToCalibrateSMDPed = 10000;
    minEventsToCalibrateSMDGain = 100000;
    minEventsToCalibrateTowerPed = 10000;
    minEventsToCalibrateTowerGain = 100000;
    minEventsToCalibrateSlewing = 100000;

    // Local text database dump to use
    // Set to NULL or empty string to access STAR DB instead
    zdcPedCalFilename = "./zdcPedCal.txt";
    // Update this text database from each calibration run
    updateInputTextDb = true;

    // Local list of runs already processed, not to be repeated
    // This is a veto file, to re-process a run delete it from this list
    //    seenRuns = "./seenRuns.txt";

    // Read this file to see if the monitoring should continue
    // If the first word in the file is "RUN", the job goes to sleep, then wakes up and repeats
    statusFilename = "./status.txt";

    // Sleep this long between invocations
    // If <=0, the job will process input once and exit
    sleepSeconds = 0;
    //sleepSeconds = 1*60;

    // When finished, close all canvases
    // Set to false when running interactively
    deleteCanvasOnExit = true;

    // show pre- and post-trigger bunch crossings
    // prepost = 0 by default, which means reading out the triggered bunch crossing
    // prepost = 1 will generate plots for -1, 0, and +1
    // prepost = 2 will generate plots for -2, -1, 0, +1, and +2, and so on
    prepost = 0;

    // Which runs to accumulate per fill
    //namesToAddFill = "zdc_pol";
    namesToAddFill = "zdc_e|zdc_w|zdcE|zdcW|zdc_smd";
    minEventsToAddFill = 1000;

    outFormats = "pdf";
    outFillFormats = "pdf root";

    //pidFilename = "./read.pid"; // default is ./read.pid
    //stdoutFilename = "./read.out"; // NULL or empty string means output to terminal, default is ./read.out
    //stdoutMode = "w"; // "a" = append (default), "w" = overwrite

    //filename = "/star/u/cjena/lbl/run10/zdcMonitoring/code_check/trg/trgdata/*.dat";

    // Analyzing power which will be used to calculate the fractional transverse component
    normAnalyzingPower = 0.075; // p+p @ 500 GeV
    //normAnalyzingPower = 0.0375; // estimate for p+p @ 200 GeV
}
