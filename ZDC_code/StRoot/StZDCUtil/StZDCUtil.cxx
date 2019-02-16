#include <TPRegexp.h>
#include <TObjArray.h>
#include <TObjString.h>

#include <StMessMgr.h>

#include "StZDCUtil.h"

void getPattern(const TString &cdevRecord, const Char_t *beam, const Char_t *sub, const Char_t *patternName, TString &patternOut, Float_t *numPatternOut, Int_t size) {
    TPRegexp fillPatternRE(TString::Format("<%s>.*<%s>.*<%s[^>]*>(.*)</%s>.*</%s>.*</%s>", beam, sub, patternName, patternName, sub, beam));
    TObjArray *fillPatternArray = fillPatternRE.MatchS(cdevRecord);
    if (fillPatternArray && (fillPatternArray->GetEntries() >= 2) && fillPatternArray->At(1)) patternOut = static_cast<TObjString*>(fillPatternArray->At(1))->GetString();
    if (fillPatternArray) delete fillPatternArray;
    LOG_DEBUG << "Read pattern \"" << beam << "\" \"" << patternName << "\": " << patternOut.Length() << " \"" << patternOut << "\"" << endm;
    if (numPatternOut && size) {
        TObjArray *arr = patternOut.Tokenize(" ");
        for (Int_t i = 0;(arr ? (i < arr->GetEntries()) : false) && (i < size);i++) if (arr->At(i)) numPatternOut[i] = (static_cast<TObjString*>(arr->At(i)))->GetString().Atof();
        if (arr) delete arr;
    }
}

