// Prepare the waveforms for analysis: (use like "prepareWF inputWFfile.root")
// Baseline correction, resampling (for simulated), normalization (for simulated), chosing the range
// Set switch to chose if data or simulation
// to compile: cd MaGe/sandbox , ln -s , ...

#include "/home/pclg-09/vauth/work/PSA/compare/prepareWF.h"

int main(int argc, char *argv[])
{    
  MGLogger::SetSeverity(MGLogger::routine);  
  const char* filename = argv[1];   // this is the path to the input WF file
  if(argc!=2) {
    cout << "Usage: prepareWF inputWFfile" << endl;
    return 1;
  }
  fInputChar = extractFilename(filename);  // just the name without the path

  // ---------- Change settings HERE ----------
     int myswitch = 1;                   // Set to 0 for simulation, 1 for data input files
     const int baselineSamples = 30;     // number of samples from the beginning that are used for baseline correction...
     // const size_t pulseSamples = 50;  // Set directly in choseRegion ...
  // ------------------------------------------


 // define the name of the output files
 const char* resampledChar = filenameCreator("Resampled_");
 const char* baselineChar = filenameCreator("BLcorrected_");
 const char* normalizedChar = filenameCreator("Normalized_");
 const char* outfileChar = filenameCreator("Prepared_");


 if (myswitch == 0){
 // Resample sim    (creates resampledWFs.root)
   cout << "Resampling... " << endl;
   resampleWF(filename, resampledChar);
   filename = resampledChar;
   cout << "Done." << endl;
   cout << filename << " has been created. " << endl << endl;
 }


 // Baseline correction   (creates BLcorrectedWFs.root)
 cout << "Removing the baseline... " << endl;
 baselineCorrect(filename, baselineChar, baselineSamples);         
 filename = baselineChar;
 cout << "... Done." << endl;
 cout << filename << " has been created. " << endl << endl;


 if (myswitch == 0){
   // Normalize (creates normalizedWFs.root)
   // cout << "Normalizing... " << endl;
   // normalizeSim(filename, normalizedChar);
   // filename = normalizedChar;
   // cout << "... Done." << endl;
   // cout << filename << " has been created. " << endl << endl;
 }


 // Select the interesting range (returns 
 cout << "Selecting the region around the pulse... " << endl;
 choseRegion(filename, outfileChar, baselineSamples);
 cout << "... Done." << endl;
 cout << "The results were written to " << outfileChar << endl << endl; 

  return 0;
} // end of int main







// --------------------------------------------------------------
//                    The sub-functions:
// --------------------------------------------------------------





// ---------------------------------------
//     Resample
// ---------------------------------------

void resampleWF(const char *inputFile, const char *resampledChar)
{      
  // output 
  MGWaveformWriterROOT resampledFile;
  resampledFile.OpenOutputFile(resampledChar);

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
  //  addNoise.SetNoiseAmplitude(10); // 5 keV / 2.95 eV (pair production energy)
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
      = *(resampledFile.GimmeWaveformCollection(0, nWaveforms, wfBranchName.c_str()));
    
    for(size_t iWaveform = 0; iWaveform < nWaveforms; iWaveform++) {
      MGTWaveform& waveform = *((MGTWaveform*) wfClonesArray.At(iWaveform));
      outputwf = outputWFs.GetWaveform(iWaveform);
      outputwf->SetID(waveform.GetID());
      size_t newLen=(size_t) floor(waveform.GetLength()*75*MHz/waveform.GetSamplingFrequency());
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

void baselineCorrect(const char *inputFile, const char *baselineChar, const int baselineSamples)
{
   //input
   TChain wfTree("wfTree", "wfTree");
   wfTree.AddFile(inputFile);
   TClonesArray *wfArray = new TClonesArray("MGTWaveform");
   string wfBranchName = wfTree.GetListOfBranches()->At(0)->GetName();
   cout<<"wf branch name: "<<wfBranchName<<endl;
   wfTree.SetBranchAddress(wfBranchName.c_str(), &wfArray);

   //output
   TFile* BLCoutput = new TFile(baselineChar,"recreate");
   TTree *BLCtree = wfTree.CloneTree(0);


   MGWFBaselineRemover baseline;
   baseline.SetBaselineSamples(baselineSamples);
   cout << "Using the first " << baselineSamples << " samples for baseline removal." << endl;

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

}  // end of function baselineCorrect





// ---------------------------------------
//     Normalize the Simulation
// ---------------------------------------


void normalizeSim(const char *inputFile, const char *normalizedChar)
{
  
  
}  // end of function normalizeSim





// ---------------------------------------
//     Chose the right region
// ---------------------------------------


void choseRegion(const char *inputFile, const char *outputFile,  const int baselineSamples)
{
   //input
   TChain wfTree("wfTree", "wfTree");
   wfTree.AddFile(inputFile);
   TClonesArray *wfArray = new TClonesArray("MGTWaveform");
   string wfBranchName = wfTree.GetListOfBranches()->At(0)->GetName();
   cout<<"wf branch name: "<<wfBranchName<<endl;
   wfTree.SetBranchAddress(wfBranchName.c_str(), &wfArray);
   int Nentries = wfTree.GetEntries();

   //output
   TFile* SelectedRangeOutput = new TFile(outputFile,"recreate");
   TTree* SRtree = wfTree.CloneTree(0);

   // loop over all events
   for(int iEntry=0; iEntry<Nentries; iEntry++) {
      wfTree.GetEntry(iEntry);
      MGTWaveform* waveform = (MGTWaveform*) wfArray->At(0);   // just look at the core for chosing the region
      
      //find the maximum amplitude of the wave , start at baselineSamples (max should not lie before that)
      int wave_imax=0;
      double wave_max = 0.0;
      for ( size_t i=baselineSamples; i<300; i++ ) {
	if (waveform->At(i)>wave_max){
	  wave_imax=i;
	  wave_max = waveform->At(i);
	}
      } 
    
      // find the middle of the pulse
      int wave_imiddle=-1;
      for (int i=baselineSamples; i<=300; i++) {
	if (waveform->At(i-1) < 0.5*wave_max  &&  waveform->At(i) >=0.5*wave_max  &&  wave_imiddle<0)  {
	 wave_imiddle=i;
	}
      }
      if(iEntry%100==0) cout<<" For entry " << iEntry << " the middle of the rising pulse is at " << wave_imiddle*13.33333 << " ns. " <<endl;
      if (wave_imiddle<0) {
	cout<<" Error: Pulse middle point not in range! "<<endl;
	abort();
      }	 
      
      //now fill the Tree with samples left and right of this middle point
      size_t nWaveforms = wfArray->GetEntries();         // should give 19, make new array with length = 60 and fill it for each of them
      for(size_t iWaveform = 0; iWaveform < nWaveforms; iWaveform++) {
         MGTWaveform *waveform = (MGTWaveform*) wfArray->At(iWaveform);
	 const size_t length = 50;      // number of samples written to output file (has to be EVEN number!)
	 double wfdata[length] = {0};
	 	 for (size_t i=0; i < length ; i++) {
	 wfdata[i]=waveform->At(wave_imiddle-(length/2)+i);
	 }
         new((*wfArray)[iWaveform])  MGTWaveform(wfdata, length, 75*CLHEP::MHz, 0.0*CLHEP::ns, MGWaveform::kCharge, iWaveform);
      }
      SRtree->Fill();
   }
   SRtree->Write();
   SelectedRangeOutput->Close();

}  // end of function choseRegion



// -------------------------------------------------



// small helper that appends strings to name of input file --> new filenames
const char* filenameCreator(const char* someString)
{
  const char *inputChar = GetInputChar();
  TString *filename = new TString(someString);
  filename->Append(inputChar);
  const char* fileChar = filename->Data();
  return fileChar;
}  // end of function filenameCreator



// 

const char* extractFilename (const char* inputFile)
{
  string source (inputFile);
  string baseName;
  size_t pos = source.rfind("/");    // position of last slash
  if (pos != source.size() )  baseName = source.substr (pos+1);   // get from after last slash to the end
  return baseName.c_str();
}
