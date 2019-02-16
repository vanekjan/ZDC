#include <fstream>
using namespace std;

#include <TSQLServer.h>
#include <TSQLRow.h>
#include <TSQLResult.h>
#include <TString.h>
#include <TObjString.h>
#include <TPRegexp.h>

#include <StMessMgr.h>

#include "StZDCMonUtil.h"

void querySqlRuns(const Char_t *connectString,
    int beginDate, int beginTime,
    int endDate, int endTime,
    Bool_t filterBad,
    TString &outputList
) {
    TString beginDateStr;
    {
	TDatime beginDateTime(beginDate, beginTime);
	beginDateStr = beginDateTime.AsSQLString();
    }
    TString endDateStr;
    {
	TDatime endDateTime(endDate, endTime);
	endDateStr = endDateTime.AsSQLString();
    }
    outputList = "";
    TSQLServer *db = 0;
    Int_t nTry = 0;
    Int_t maxTries = 5;
    while (!db && (nTry < maxTries)) {
	LOG_INFO << "Connecting to SQL server " << connectString << endm;
        db = TSQLServer::Connect(connectString, "", "");
        Int_t delayInSeconds = 10;
        time_t startTime, curTime;
        time(&startTime);
        if (!db) {
            LOG_ERROR << "Connection to db failed. Re-trying after " << delayInSeconds << " seconds..." << endm;
            do {
                time(&curTime);
            } while((curTime - startTime) < delayInSeconds);
        }
        nTry+=1;
    }
    if (!db) {
        {LOG_ERROR << "Could not connect to db server after " << nTry << " tries!" << endm;}
        return;
    }
    TString query;
    if (filterBad) {
	query= TString::Format("select runNumber,name,beginTime from l0TriggerSet where (beginTime > \"%s\") and (beginTime < \"%s\") \
and (runNumber in (select runNumber from runStatus where (beginTime > \"%s\") and (beginTime < \"%s\") and (rtsStatus=0) and (shiftLeaderStatus=-1)))", 
beginDateStr.Data(), endDateStr.Data(), beginDateStr.Data(), endDateStr.Data());
    } else {
	query = TString::Format("select runNumber,name,beginTime from l0TriggerSet where (beginTime > \"%s\") and (beginTime < \"%s\")",
beginDateStr.Data(), endDateStr.Data());
    }
    LOG_INFO << "Sending query:" << endl << query << endm;
    TSQLResult *result = db->Query(query);
    if (!result) {
        LOG_ERROR << "SQL query failed" << endm;
    } else {
	TSQLRow* row = 0;
	while ((row = result->Next())) {
	    TString runStr = row->GetField(0);
	    TString name = row->GetField(1);
	    TString timestamp = row->GetField(2);
	    TDatime ts(timestamp.Data());
	    Int_t run = runStr.Atoi();
	    Int_t date = ts.GetDate();
	    Int_t time = ts.GetTime();
	    LOG_DEBUG << "Run: " << runStr << "(" << run << ") " << name << " " << timestamp << "(" << date << "." << time << ")" << endm;
	    outputList += TString::Format(";%i,%s,%08i,%06i", run, name.Data(), date, time);
	}
    }
    db->Close();
    delete db;
    //LOG_DEBUG << "Output string " << outputList << endm;
}

void querySqlCdev(const Char_t *connectString,
    int beginDate, int beginTime,
    TString &outputList
) {
    TString beginDateStr;
    {
	TDatime beginDateTime(beginDate, beginTime);
	beginDateStr = beginDateTime.AsSQLString();
    }
    outputList = "";
    TSQLServer *db = 0;
    Int_t nTry = 0;
    Int_t maxTries = 5;
    while (!db && (nTry < maxTries)) {
	LOG_INFO << "Connecting to SQL server " << connectString << endm;
        db = TSQLServer::Connect(connectString, "", "");
        Int_t delayInSeconds = 10;
        time_t startTime, curTime;
        time(&startTime);
        if (!db) {
            LOG_ERROR << "Connection to db failed. Re-trying after " << delayInSeconds << " seconds..." << endm;
            do {
                time(&curTime);
            } while((curTime - startTime) < delayInSeconds);
        }
        nTry+=1;
    }
    if (!db) {
        {LOG_ERROR << "Could not connect to db server after " << nTry << " tries!" << endm;}
        return;
    }
    TString query;
    query = TString::Format("select dataS from kretDbBlobS where (nodeID = 9) and (beginTime <= \"%s\") and (endTime > \"%s\") order by beginTime desc limit 1", beginDateStr.Data(), beginDateStr.Data());
    LOG_INFO << "Sending query:" << endl << query << endm;
    TSQLResult *result = db->Query(query);
    if (!result) {
        LOG_ERROR << "SQL query failed" << endm;
    } else {
	TSQLRow* row = 0;
	while ((row = result->Next())) {
	    outputList = row->GetField(0);
	}
    }
    db->Close();
    delete db;
    //LOG_DEBUG << "Output string " << outputList << endm;
}

void writeQueryRunList(const Char_t *dbconnect, const Char_t *filenameFmt, const TDatime &beginTime, const TDatime &endTime, Bool_t filterBad, const Char_t *dbFileList) {
    if (dbconnect && dbFileList && filenameFmt) {
        ofstream fstr(dbFileList);
        if (fstr.good()) {
            TString runs;
            querySqlRuns(dbconnect, beginTime.GetDate(), beginTime.GetTime(), endTime.GetDate(), endTime.GetTime(), filterBad, runs);
            TObjArray *runsList = runs.Tokenize(TString(";"));
            if (runsList) {
                for (Int_t i = 0;i < runsList->GetEntries();i++) {
                    TObjString *runObjStr = static_cast<TObjString*>(runsList->At(i));
                    if (runObjStr) {
                        TString runStr = runObjStr->GetString();
                        TObjArray *runInfo = runStr.Tokenize(TString(","));
                        if (runInfo) {
                            if (runInfo->GetEntries() >= 4) {
                                TObjString *runIdObjStr = static_cast<TObjString*>(runInfo->At(0));
                                TObjString *nameObjStr = static_cast<TObjString*>(runInfo->At(1));
                                TObjString *dateObjStr = static_cast<TObjString*>(runInfo->At(2));
                                TObjString *timeObjStr = static_cast<TObjString*>(runInfo->At(3));
                                TString runIdStr;
                                TString nameStr;
                                TString dateStr;
                                TString timeStr;
                                if (runIdObjStr) runIdStr = runIdObjStr->GetString();
                                if (nameObjStr) nameStr = nameObjStr->GetString();
                                if (dateObjStr) dateStr = dateObjStr->GetString();
                                if (timeObjStr) timeStr = timeObjStr->GetString();
                                TString filenameStr = filenameFmt;
                                filenameStr.ReplaceAll("RUNID", runIdStr);
                                filenameStr.ReplaceAll("NAME", nameStr);
                                filenameStr.ReplaceAll("DATE", dateStr);
                                filenameStr.ReplaceAll("TIME", timeStr);
                                fstr << filenameStr << endl;
                                if (!fstr.good()) {
                                    cerr << "Cannot write a line to file " << dbFileList << endl;
                                }
                            }
                            delete runInfo;
                            runInfo = 0;
                        }
                    }
                }
                delete runsList;
                runsList = 0;
            }
            if (!fstr.good()) {
                cerr << "File " << dbFileList << " is likely corrupted" << endl;
            }
            fstr.close();
        } else {
            cerr << "Cannot write local file for DB query results " << dbFileList << endl;
        }
    }
}

void writeQueryCdev(const Char_t *dbconnect, int date, int time, const Char_t *outputCdevXmlFilename) {
    if (dbconnect && outputCdevXmlFilename) {
        ofstream fstr(outputCdevXmlFilename);
        if (fstr.good()) {
            TString cdev;
            querySqlCdev(dbconnect, date, time, cdev);
            fstr << cdev << endl;
            if (!fstr.good()) {
                cerr << "File " << cdev << " is likely corrupted" << endl;
            }
            fstr.close();
        } else {
            cerr << "Cannot write local file for DB query results " << outputCdevXmlFilename << endl;
        }
    }
}

void getCdevFileTimestamp(const Char_t *cdevXmlFile, Int_t &date, Int_t &time) {
    ifstream fstr(cdevXmlFile);
    if (fstr.good()) {
	date = 0;
	time = 0;
	const Int_t buflen = 2048;
	Char_t buf[buflen];
	TPRegexp unixTimeRE("<unixTime[^>]*>(.*)</unixTime>");
	do {
	    fstr.getline(buf, buflen);
	    buf[buflen - 1] = 0;
	    TString str = buf;
	    TObjArray *unixTimeArray = unixTimeRE.MatchS(str);
	    UInt_t unixTime = 0;
	    if (unixTimeArray && (unixTimeArray->GetEntries() >= 2) && unixTimeArray->At(1)) unixTime = static_cast<TObjString*>(unixTimeArray->At(1))->GetString().Atoi();
	    if (unixTimeArray) delete unixTimeArray;
	    if (unixTime) {
		TDatime d(unixTime);
		d.Set(d.Convert(kTRUE));
	        date = d.GetDate();
		time = d.GetTime();
	    }
	} while ((date == 0) && fstr.good());
    } else {
	LOG_ERROR << "Cannot read file " << cdevXmlFile << endm;
    }
}
