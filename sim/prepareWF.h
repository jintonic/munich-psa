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


void resampleWF(const char *inputFile);

void baselineCorrect(const char *inputFile);

void normalizeSim(const char *inputFile); 

void choseRegion(const char *inputFile); 
