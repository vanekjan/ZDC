#ifndef StZDCMonUtil_h
#define StZDCMonUtil_h

#include <TDatime.h>
#include <TString.h>
#include <TH2.h>
#include <TF1.h>
#include <TObjArray.h>

//-------------------------------------
#include <TSysEvtHandler.h>
class TSimpleSignalHandler : public TSignalHandler {
public:
    TSimpleSignalHandler(ESignals sig, Bool_t sync = kTRUE) : TSignalHandler(sig, sync), mTriggered(false) {}
    virtual Bool_t Notify() {this->set(true); return this->TSignalHandler::Notify();}
    Bool_t isTriggered() const {return this->mTriggered;}
    void set(Bool_t trig) {this->mTriggered = trig;}
    void reset() {this->set(false);}
private:
    Bool_t mTriggered;
    ClassDef(TSimpleSignalHandler, 1);
};
ClassImp(TSimpleSignalHandler)
//-------------------------------------

void querySqlRuns(const Char_t *connectString, 
    int beginDate, int beginTime,
    int endDate, int endTime,
    Bool_t filterBad,
    TString &outputList
    );

void querySqlCdev(const Char_t *connectString, 
    int beginDate, int beginTime,
    TString &outputList
    );

void writeQueryRunList(const Char_t *dbconnect, const Char_t *filenameFmt, const TDatime &beginTime, const TDatime &endTime, Bool_t filterBad, const Char_t *dbFileList);

void writeQueryCdev(const Char_t *connectString, int date, int time, const Char_t *outputCdevXmlFilename);

void getCdevFileTimestamp(const Char_t *cdevXmlFile, Int_t &date, Int_t &time);

#endif
