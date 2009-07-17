// use like findDataNoiseLevel filename segnumber samples

// To find the noise level in the data:   (for core as well as for a segment)
// look at first (~50) samples & fill into histo, repeat for enough events to get at least 1000 histo entries
// draw histo, fit gauss.

// #include "/home/pclg-09/vauth/work/MAGE/2009_07_08_newCheckout/MGDO/Root/MGTWaveform.cc"

#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TClonesArray.h>
#include <TF1.h>
#include <TH1D.h>
#include <TLegend.h>
#include <iostream>
#include <map>
#include <TString.h>
#include <TStyle.h>
#include <sstream>

#include "MGTWaveform.hh"
#include "MGCrystalData.hh"

using namespace std;

int main()
{
  const char* filename = "/remote/pclg-09/vauth/tmp/DEPfromData_BLcorrectedWFs.root";
  Int_t samples = 30;      // how many of the first samples are used
  Int_t segnumber = 1;     // just select some random segment
  Int_t events = 40;       // loop over that many events to get enough histo entries

  // read in the input file
  TFile *dataFile = TFile::Open(filename);
  TTree *dataTree = (TTree *) dataFile->Get("wfTree");
  //  dataTree->Print();
  Int_t Nevents = dataTree->GetEntries();
  cout << " Opened " << filename << ". Number of entries: " << Nevents << endl;
  
  TCanvas *canvasNoise = new TCanvas("canvasNoise","First samples",800,1200);
  canvasNoise->SetFillColor(0);
  canvasNoise->SetBorderMode(0);
  canvasNoise->SetFrameBorderMode(0);
  canvasNoise->Divide(1,2);
  
  TH1D *histoCoreNoise = new TH1D("histoCoreNoise","First samples from core",200,-250,250);
  histoCoreNoise->SetLineColor(2);
  TH1D *histoSegNoise = new TH1D("histoSegNoise","First samples from segment",200,-250,250);
  histoSegNoise->SetLineColor(4);

  gStyle->SetOptFit(0001);

  // get waveform info from the file
  TClonesArray* dataPSArray = new TClonesArray("MGTWaveform");
  std::map<Int_t, std::map<size_t, TH1D*> > dataPSHists;
  string dataBranchName = dataTree->GetListOfBranches()->At(0)->GetName(); 
  dataTree->SetBranchAddress(dataBranchName.c_str(), &dataPSArray);
  
  cout << "Looping over the first " << events << " events, " << endl;
  cout << "Using the first " << samples << " samples. " << endl;

  for (Int_t i = 0; i < events; i++) {
    dataTree->GetEntry(i);
    //   if (i%5==0) cout<<" now event "<<i<<endl;    

  // for the core;
    Int_t iWaveform=0;
    MGTWaveform* waveform = (MGTWaveform*) dataPSArray->At(iWaveform);
    size_t iContact = MGCrystalData::GetIContact(waveform->GetID());
    size_t segIDdat[19]={0,1,2,3,7,8,9,13,14,15,16,17,18,10,11,12,4,5,6};
    dataPSHists[0][segIDdat[iContact]] = waveform->GimmeHist();  // 0 = crystalID
    for (Int_t j =1; j <= samples; j ++){
      Double_t wfBinCore = dataPSHists[0][0]->GetBinContent(j);
      histoCoreNoise->Fill(wfBinCore);
    }
    
  // for a segment
    iWaveform = segnumber;
    waveform = (MGTWaveform*) dataPSArray->At(iWaveform);
    iContact = MGCrystalData::GetIContact(waveform->GetID());
    dataPSHists[0][segIDdat[iContact]] = waveform->GimmeHist();  // 0 = crystalID
    for (Int_t j =1; j <= samples; j ++){
      Double_t wfBinSeg = dataPSHists[0][1]->GetBinContent(j);
      histoSegNoise->Fill(wfBinSeg);
    }
  }

  canvasNoise->cd(1);
  histoCoreNoise->Draw();
  histoCoreNoise->GetXaxis()->SetTitle("Level");
  histoCoreNoise->GetYaxis()->SetTitle("Number of enties");
  histoCoreNoise->GetXaxis()->CenterTitle();
  histoCoreNoise->GetYaxis()->CenterTitle();
  histoCoreNoise->GetXaxis()->SetTitleFont(132);
  histoCoreNoise->GetYaxis()->SetTitleFont(132);
  histoCoreNoise->GetXaxis()->SetLabelFont(132);
  histoCoreNoise->GetYaxis()->SetLabelFont(132);

  histoCoreNoise->Fit("gaus");
  TF1 *fitCore = histoCoreNoise->GetFunction("gaus");
  Float_t meanCore = fitCore->GetParameter(1);
  Float_t sigmaCore = fitCore->GetParameter(2);
  cout << "Made fit for the core histo with mean " << meanCore << " and sigma " << sigmaCore << endl;

  canvasNoise->cd(2);
  histoSegNoise->Draw(); 
  histoSegNoise->GetXaxis()->SetTitle("Level");
  histoSegNoise->GetYaxis()->SetTitle("Number of enties");
  histoSegNoise->GetXaxis()->CenterTitle();
  histoSegNoise->GetYaxis()->CenterTitle();
  histoSegNoise->GetXaxis()->SetTitleFont(132);
  histoSegNoise->GetYaxis()->SetTitleFont(132);
  histoSegNoise->GetXaxis()->SetLabelFont(132);
  histoSegNoise->GetYaxis()->SetLabelFont(132);

  TF1 *fitSeg = new TF1("fitSeg","gaus",-150,150);
  histoSegNoise->Fit("fitSeg");
  Float_t meanSeg = fitSeg->GetParameter(1);
  Float_t sigmaSeg = fitSeg->GetParameter(2);
  cout << "Made fit for the segment histo with mean " << meanSeg << " and sigma " << sigmaSeg << endl;

  canvasNoise->SaveAs("findDataNoiseLevel.eps");

}  // end of int main
