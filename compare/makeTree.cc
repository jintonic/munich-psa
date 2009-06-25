// To convert PS trees into a format that can be used to train a neural network
// To use, enter the filename manually

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

int main()
{

  const char* sigFilename = "../dat/WF_DEP_fromData.root";
  const char* bgFilename1 = "../dat/WF_K40_fromData.root";
  const char* bgFilename2 = "../dat/WF_Bi212_fromData.root";

  // read in the trees from the input files
  TFile *sigFile = TFile::Open(sigFilename);
  TTree *sigTree = (TTree *) sigFile->Get("wfTree");
  //   cout << endl << "The signal wfTree: " << endl;
  //   sigTree->Print();

  TChain *bgTree = new TChain("wfTree", "wfTree");
  bgTree->AddFile(bgFilename1);
  bgTree->AddFile(bgFilename2);

   //  TFile *bgFile = TFile::Open(bgFilename);
   //  TTree *bgTree = (TTree *) bgFile->Get("wfTree");
  
  // create the new tree, 
  TFile *newdatafile = new TFile("DataSample.root","recreate");
  TTree *dataSample = new TTree("DataSample", "Data Sample for Neural Network");
  Int_t type;   // 1 for data, 0 for background

  // Set up the branches for the new tree
  dataSample->Branch("type",   &type,   "type/I");

  Double_t wfBin[300] = {};
  TString branchName[300] = {};
  TString branchName2[300] = {};

  for (Int_t b = 1; b <= 300 ; b++){
    if (b < 10) branchName[b-1] = (Form("wfBin_00%d",b));
    if (b >= 10 && b < 100) branchName[b-1] = (Form("wfBin_0%d",b));
    if (b >= 100) branchName[b-1] = (Form("wfBin_%d",b));
    branchName2[b-1] = branchName[b-1];
    branchName2[b-1].Append("/D");
    dataSample->Branch(branchName[b-1] ,  &wfBin[b-1], branchName2[b-1]);
  }


  // --- read the waveforms from the signal input file
  type = 1;   // 1 for data, 0 for background
  TClonesArray* sigPSArray = new TClonesArray("MGTWaveform");
  std::map<Int_t, std::map<size_t, TH1D*> > sigPSHists;
  string sigBranchName = sigTree->GetListOfBranches()->At(0)->GetName();
  sigTree->SetBranchAddress(sigBranchName.c_str(), &sigPSArray);
  
  Int_t sigEntries = sigTree->GetEntries();
  cout<<"Signal tree: total number of events = "<<sigEntries<<endl;
  for (Int_t i=0; i<sigEntries;i++) {
    sigTree->GetEntry(i);
    if (i%100==0) cout<<" now event "<<i<<endl;    
    Int_t Nwfs = sigPSArray->GetEntries();
      if(Nwfs == 0) {
        cout<<"Error: no waveform found!"<<endl;
      }
  
    vector<Int_t> crystalIDs;
    for(Int_t iWaveform=0; iWaveform<Nwfs; iWaveform++) {
      MGTWaveform* waveform = (MGTWaveform*) sigPSArray->At(iWaveform);
      if(waveform == NULL) {
	cout <<"Error: Got NULL waveform" << endl;
	break;
      }   
      Int_t crystalID = 0;
      size_t iContact = MGCrystalData::GetIContact(waveform->GetID());

      size_t segIDdat[19]={0,1,2,3,7,8,9,13,14,15,16,17,18,10,11,12,4,5,6};
      size_t segIDsim[19]={0,1,7,13,2,8,14,3,9,15,4,10,16,5,11,17,6,12,18};
      
      sigPSHists[crystalID][segIDdat[iContact]] = waveform->GimmeHist();
      //sigPSHists[crystalID][segIDsim[iContact]] = waveform->GimmeHist();

      Int_t nHistoBins = sigPSHists[0][0]->GetNbinsX();
      //      cout << "Core Histo: The number of bins is (could be something like 300 " << nHistoBins << endl;
      for (Int_t j =1; j <= nHistoBins; j ++){
	wfBin[j-1] = sigPSHists[0][0]->GetBinContent(j);
      }
    }
    dataSample->Fill();
  }



  // --- read the waveforms from the background input file
  type = 0;   // 1 for data, 0 for background
  TClonesArray* bgPSArray = new TClonesArray("MGTWaveform");
  std::map<Int_t, std::map<size_t, TH1D*> > bgPSHists;
  string bgBranchName = bgTree->GetListOfBranches()->At(0)->GetName();
  bgTree->SetBranchAddress(bgBranchName.c_str(), &bgPSArray);
  
  Int_t bgEntries = bgTree->GetEntries();
  cout<<"Background tree: total number of events = "<<bgEntries<<endl;
  for (Int_t i=0; i<bgEntries;i++) {
    bgTree->GetEntry(i);
    if (i%100==0) cout<<" now event "<<i<<endl;    
    Int_t Nwfs = bgPSArray->GetEntries();
      if(Nwfs == 0) {
        cout<<"Error: no waveform found!"<<endl;
      }
  
    vector<Int_t> crystalIDs;
    for(Int_t iWaveform=0; iWaveform<Nwfs; iWaveform++) {
      MGTWaveform* waveform = (MGTWaveform*) bgPSArray->At(iWaveform);
      if(waveform == NULL) {
	cout <<"Error: Got NULL waveform" << endl;
	break;
      }   
      Int_t crystalID = 0;
      size_t iContact = MGCrystalData::GetIContact(waveform->GetID());

      size_t segIDdat[19]={0,1,2,3,7,8,9,13,14,15,16,17,18,10,11,12,4,5,6};
      size_t segIDsim[19]={0,1,7,13,2,8,14,3,9,15,4,10,16,5,11,17,6,12,18};
      
      bgPSHists[crystalID][segIDdat[iContact]] = waveform->GimmeHist();
      //bgPSHists[crystalID][segIDsim[iContact]] = waveform->GimmeHist();

      Int_t nHistoBins = bgPSHists[0][0]->GetNbinsX();
      //      cout << "Core Histo: The number of bins is (could be something like 300 " << nHistoBins << endl;
      for (Int_t j =1; j <= nHistoBins; j ++){
	wfBin[j-1] = bgPSHists[0][0]->GetBinContent(j);
      }
    }
    dataSample->Fill();
  }


  // save output
    //   cout<<endl<<"The output Tree: "<<endl;
    //   dataSample->Print();
    cout << endl;
    dataSample->AutoSave();
    delete newdatafile;
 
    return 0;
}
