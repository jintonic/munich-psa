// To convert PS trees into a format that can be used to train a neural network
// Use like:   makeTree  Prepared_WFs_DEPfromData.root Prepared_WFs_BGfromData.root 

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TClonesArray.h>
#include <TH1D.h>
#include <iostream>
#include <map>
#include <TString.h>
#include <sstream>

#include "MGTWaveform.hh"
#include "MGCrystalData.hh"

using namespace std;


void FillMyTree(TTree *inputTree, TClonesArray* inputPSArray, const Int_t isSignal, TTree* dataSample, Double_t* wfBin);


int main(int argc, char *argv[])
{
  if(argc!=3) {
    cout << endl << "Wrong number of arguments. Use like: makeTree DEPFilename BackgroundFilename " << endl << endl;
    return 1;
  }
 
  // read in the trees from the input files
  const char* sigFilename = argv[1];
  const char* bgFilename = argv[2];

  TFile *sigFile = TFile::Open(sigFilename);
  TTree *sigTree = (TTree *) sigFile->Get("wfTree");
  TClonesArray* sigPSArray = new TClonesArray("MGTWaveform");
  // std::map<Int_t, std::map<size_t, TH1D*> > sigPSHists;
  string sigBranchName = sigTree->GetListOfBranches()->At(0)->GetName();
  sigTree->SetBranchAddress(sigBranchName.c_str(), &sigPSArray);

  TFile *bgFile = TFile::Open(bgFilename);
  TTree *bgTree = (TTree *) bgFile->Get("wfTree");
  TClonesArray* bgPSArray = new TClonesArray("MGTWaveform");
  //  std::map<Int_t, std::map<size_t, TH1D*> > bgPSHists;
  string bgBranchName = bgTree->GetListOfBranches()->At(0)->GetName();
  bgTree->SetBranchAddress(bgBranchName.c_str(), &bgPSArray);


  // create the new tree, 
  TFile *newdatafile = new TFile("DataSampleForNN.root","recreate");
  TTree *dataSample = new TTree("DataSample", "Data Sample for Neural Network");
  Int_t type;   // 1 for data, 0 for background

  // Set up the branches for the new tree (--> check how many samples the WF of the input files contain)
  dataSample->Branch("type",   &type,   "type/I");

  sigTree->GetEntry(1);
  MGTWaveform* sigCheckWF = (MGTWaveform*) sigPSArray->At(1);

  bgTree->GetEntry(1);  
  MGTWaveform* bgCheckWF = (MGTWaveform*) bgPSArray->At(1);

  if ( (sigCheckWF->GetLength()) != (bgCheckWF->GetLength())  ){
    cout << "Error! The number of PS samples in the DEP and Background file are not the same!" << endl << endl;
    return 1;
  }
  size_t length = sigCheckWF->GetLength(); 
  cout << "Using " << length << " sample points. " << endl;

  Double_t* wfBin = new Double_t[length];
  TString* branchName = new TString[length];
  TString* branchName2 = new TString[length];
  for (size_t b = 1; b <= length ; b++){
    if (b < 10) branchName[b-1] = (Form("wfBin_00%d",b));
    if (b >= 10 && b < 100) branchName[b-1] = (Form("wfBin_0%d",b));
    if (b >= 100) branchName[b-1] = (Form("wfBin_%d",b));
    branchName2[b-1] = branchName[b-1];
    branchName2[b-1].Append("/D");
    dataSample->Branch(branchName[b-1] ,  &wfBin[b-1], branchName2[b-1]);
  }


// --- read the waveforms and write to new tree
  type = 1;
  FillMyTree(sigTree, sigPSArray, type, dataSample, wfBin);  
  type = 0;
  FillMyTree(bgTree, bgPSArray, 0, dataSample, wfBin);  

  dataSample->AutoSave();
  delete newdatafile; 
  return 0;
} 


// --------------------------------------------------------------


void FillMyTree(TTree *inputTree, TClonesArray* inputPSArray, const Int_t type, TTree* dataSample, Double_t* wfBin)
{

  std::map<Int_t, std::map<size_t, TH1D*> > inputPSHists;
  Int_t inputEntries = inputTree->GetEntries();
  if (type == 0)   {
  cout<<"Background tree: total number of events = "<<inputEntries<<endl;
  }
  else {
    cout<<"Signal tree: total number of events = "<<inputEntries<<endl;
  }

  for (Int_t i=0; i<inputEntries;i++) {
    inputTree->GetEntry(i);
    if (i%100==0) cout << " now event " << i << "." << endl;    
    Int_t Nwfs = inputPSArray->GetEntries();
      if(Nwfs == 0) {
        cout<<"Error: no waveform found!"<<endl;
      }
  
    vector<Int_t> crystalIDs;
    for(Int_t iWaveform=0; iWaveform<Nwfs; iWaveform++) {
      MGTWaveform* waveform = (MGTWaveform*) inputPSArray->At(iWaveform);
      if(waveform == NULL) {
	cout <<"Error: Got NULL waveform" << endl;
	break;
      }   
      Int_t crystalID = 0;
      size_t iContact = MGCrystalData::GetIContact(waveform->GetID());

      size_t segIDdat[19]={0,1,2,3,7,8,9,13,14,15,16,17,18,10,11,12,4,5,6};
      size_t segIDsim[19]={0,1,7,13,2,8,14,3,9,15,4,10,16,5,11,17,6,12,18};
      
      inputPSHists[crystalID][segIDdat[iContact]] = waveform->GimmeHist();
      //inputPSHists[crystalID][segIDsim[iContact]] = waveform->GimmeHist();

      Int_t nHistoBins = inputPSHists[0][0]->GetNbinsX();
      //      cout << "Core Histo: The number of bins is (could be something like 300 " << nHistoBins << endl;
      for (Int_t j =1; j <= nHistoBins; j ++){
	wfBin[j-1] = inputPSHists[0][0]->GetBinContent(j);
      }
    }
    dataSample->Fill();
  }
}
