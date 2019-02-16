#ifndef ZDCPolarimetryPlotsUtil_H
#define ZDCPolarimetryPlotsUtil_H

#include <TObject.h>

void appendRunsAsymmetry(const Char_t *inputFilename, const Char_t *prefix, const Char_t *outputFilename);

void plotsAsymmetrySummary(const Char_t *inputFilename, const Char_t *runsStr, Int_t nRuns, Float_t normAnalyzingPower, const Char_t *outputFilenameGif, const Char_t *outputFilenameC);

#endif
