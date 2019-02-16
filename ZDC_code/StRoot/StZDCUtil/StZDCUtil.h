#ifndef StZDCUtil_h
#define StZDCUtil_h

#include <TString.h>

void getPattern(const TString &cdevRecord, const Char_t *beam, const Char_t *sub, const Char_t *patternName, TString &patternOut, Float_t *numPatternOut = 0, Int_t size = 0);

#endif
