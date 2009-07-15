// Prepare the waveforms for analysis:
// Baseline correction, resampling (for simulated), chosing range, normalisation (for simulated)
// Set switch to chose if data or simulation

#include "/home/pclg-09/vauth/work/PSA/sim/prepareWF.h"

int main(int argc, char *argv[])
{    
                      // --------------------------------------------------
  int myswitch = 0;   // Set to 0 for simulation, 1 for data input files
                      // --------------------------------------------------

   MGLogger::SetSeverity(MGLogger::routine);
  //MGLogger::SetSeverity(MGLogger::trace);

   if(argc==1 || argc>3) {
      cout << "Usage: prepareWF inputWFfile [outputWFfile]" << endl;
      return 1;
   }
  const char* filename = argv[1];   // inputWFfile

  // set up output file (for the __final__ result)
  MGWaveformWriterROOT outputFile;
  if (argc==2) {
    outputFile.OpenOutputFile("preparedWFs");
  }
  else { //argc==3
    outputFile.OpenOutputFile(argv[2]);
  }

  if (myswitch == 0){
    // Resample sim    (creates resampledWFs.root)
    cout << "Resampling... " ;
    resampleWF(filename);
    filename = "resampledWFs.root";
    cout << "Done." << endl << endl;
  }

  // Baseline correction   (creates BLcorrectedWFs.root)
  cout << "Removing the baseline... " ;
  baselineCorrect(filename);
  filename = "BLcorrectedWFs.root";
  cout << "... Done." << endl << endl;


  if (myswitch == 0){
    // Normalize
    normalizeSim(filename);
    // filename = "normalisedWFs.root";
  }

  choseRegion(filename);

  outputFile.CloseOutputFile();

   return 0;
} // end of int main





// ---------------------------------------
//     Resample
// ---------------------------------------

void resampleWF(const char *inputFile)
{      
  // output 
  MGWaveformWriterROOT resampledFile;
  resampledFile.OpenOutputFile("resampledWFs");

  //input
  TChain wfTree("wfTree", "wfTree");
  wfTree.AddFile(inputFile);
  string wfBranchName = wfTree.GetListOfBranches()->At(0)->GetName();
  TClonesArray wfClonesArray("MGTWaveform");
  TClonesArray* pa = &wfClonesArray;
  wfTree.SetBranchAddress(wfBranchName.c_str(), &pa);


  MGWFResampler resampler;
  MGWFShiftSamples shift;
  MGWFAddNoise addNoise;
  MGWFRCDifferentiation decay;
  MGWFRCIntegration bandwidth;
  MGWFDerivativeFourthOrder current;
  
  shift.SetNumberOfShift(-200);
  addNoise.SetNoiseAmplitude(10); // 5 keV / 2.95 eV (pair production energy)
  decay.SetTimeConstant(50000*ns);
  bandwidth.SetTimeConstant(16*ns); // bandwidth ~ 40 MHz
  int Nentries = wfTree.GetEntries();
  //cout<<"Nentries = "<<Nentries<<endl;
  MGWaveform *outputwf=NULL;
  MGWaveform tmpwf;
  //for(int iEntry=0; iEntry<1; iEntry++) {
  for(int iEntry=0; iEntry<Nentries; iEntry++) {
    wfTree.GetEntry(iEntry);
    size_t nWaveforms = wfClonesArray.GetEntries();
    
    MGWaveformCollection outputWFs 
      = *(resampledFile.GimmeWaveformCollection(
					     0, nWaveforms, wfBranchName.c_str()));
    
    //for(size_t iWaveform = 0; iWaveform < 1; iWaveform++) {
    for(size_t iWaveform = 0; iWaveform < nWaveforms; iWaveform++) {
      MGTWaveform& waveform = *((MGTWaveform*) wfClonesArray.At(iWaveform));
      outputwf = outputWFs.GetWaveform(iWaveform);
      outputwf->SetID(waveform.GetID());
      size_t newLen=(size_t) floor(waveform.GetLength()*75*MHz/waveform.GetSamplingFrequency());
      //cout<<"newLen = "<<newLen<<endl;
      outputwf->SetLength(newLen);
      outputwf->SetSamplingFrequency(75*MHz); 
      outputwf->SetTOffset(waveform.GetTOffset());
      outputwf->SetWFType(waveform.GetWFType());
      //waveform.DumpWF();
      //current.TransformOutOfPlace(waveform, tmpwf);
      shift.TransformOutOfPlace(waveform, *outputwf);
      //resampler.TransformOutOfPlace(tmpwf, *outputwf);
      //addNoise.TransformInPlace(*outputwf);
      //bandwidth.TransformInPlace(*outputwf);
      //decay.TransformInPlace(*outputwf);
      //outputwf->DumpWF();
    }
    resampledFile.WriteEvent();
  }  
  resampledFile.CloseOutputFile();
} // end of function resampleWF




// ---------------------------------------
//     Baseline correction
// ---------------------------------------

void baselineCorrect(const char *inputFile)
{
   //input
   TChain wfTree("wfTree", "wfTree");
   wfTree.AddFile(inputFile);
   TClonesArray *wfArray = new TClonesArray("MGTWaveform");
   string wfBranchName = wfTree.GetListOfBranches()->At(0)->GetName();
   cout<<"wf branch name: "<<wfBranchName<<endl;
   wfTree.SetBranchAddress(wfBranchName.c_str(), &wfArray);

   //output
   TFile* BLCoutput = new TFile("BLcorrectedWFs.root","recreate");
   TTree *BLCtree = wfTree.CloneTree(0);


   MGWFBaselineRemover baseline;
   baseline.SetBaselineSamples(100);

   int Nentries = wfTree.GetEntries();
   for(int iEntry=0; iEntry<Nentries; iEntry++) {
      if (iEntry%100==0) cout<<" now entry: "<<iEntry<<endl;
      wfTree.GetEntry(iEntry);

      size_t nWaveforms = wfArray->GetEntries();

      for(size_t iWaveform = 0; iWaveform < nWaveforms; iWaveform++) {
         MGTWaveform *waveform = (MGTWaveform*) wfArray->At(iWaveform);
         baseline.TransformInPlace(*waveform);
      }
      BLCtree->Fill();
   }
   BLCtree->Write();
   BLCoutput->Close();
}





// ---------------------------------------
//     Normalize the Simulation
// ---------------------------------------


void normalizeSim(const char *inputFile)
{
  
  
}





// ---------------------------------------
//     Chose the right region
// ---------------------------------------


void choseRegion(const char *inputFile)
{
  
  
}
