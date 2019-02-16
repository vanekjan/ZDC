//-----------------------------------------------------------------
//
// This is a configuration file for the ZDC Online Monitoring job
// This file should be used to look at the raw trigger .dat files
// This is a stripped down version of config.C which has more options
//
//-----------------------------------------------------------------

{
    // Style of graphics
    gROOT->Macro("style.C");

    // Write output to this directory...
    dirOut = "/afs/rhic.bnl.gov/star/doc_protected/www/spin/ogrebeny/zdc2009";

    // ... and assume it is visible at this URL
    publicDir = "http://www.star.bnl.gov/protected/spin/ogrebeny/zdc2009";

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
	filename = "/trg/trgdata/run10056210.*.dat";

    // Number of events to process per run, <=0 means all
    nEvents = -1;

    // Minimum number of events in a run required for calibration
    minEventsToCalibrateSMDPed = 1000;
    minEventsToCalibrateSMDGain = 10000;
    minEventsToCalibrateTowerPed = 1000;
    minEventsToCalibrateTowerGain = 10000;
    minEventsToCalibrateSlewing = 10000;

    // Local text database dump to use
    // Set to NULL or empty string to access STAR DB instead
    zdcPedCalFilename = "./zdcPedCal.txt";
}
