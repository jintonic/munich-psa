#include <string>

#include "TFile.h"
#include "TChain.h"
#include "TClonesArray.h"

#include "MGTWaveform.hh"
#include "MGWFBaselineRemover.hh"
#include "MGWFResampler.hh"
#include "MGWFShiftSamples.hh"
#include "MGWFAddNoise.hh"
#include "MGWFRCDifferentiation.hh"
#include "MGWFRCIntegration.hh"
#include "MGWFDerivativeFourthOrder.hh"

#include "io/MGLogger.hh"
#include "waveform/MGWaveformWriterROOT.hh"

using namespace std;
using namespace CLHEP;


//subfunctions

void resampleWF(const char *inputFile, const char *resampledChar);

void baselineCorrect(const char *inputFile, const char *baselineChar, const int baselineSamples);

void normalizeSim(const char *inputFile, const char *normalizedChar); 

void choseRegion(const char *inputFile, const char *outfileChar, const int baselineSamples); 


// filename helpers
const char* filenameCreator(const char* someString);     // appends a string to a filename

const char* extractFilename (const char* inputFile);             // seperates filename from path

static const char* fInputChar;

const char* GetInputChar()   {  return fInputChar;  }
