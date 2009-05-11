#include <cmath>
#include <iostream>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "MGTWaveform.hh"

using namespace std;

int main(int argc, char* argv[]) 
{
   if(argc < 2 || argc>3) {
      cout << "Usage: ./g2m input.root [output.root]" << endl;
      return 1;
   }

   //input:
   TChain* PSTree = new TChain("PSTree","root"); 
   PSTree->AddFile(argv[1]);

   Int_t PS_TotalNum;
   Int_t PS_Amp[8000];
   Int_t Cha_PSStart[20];
   Int_t Cha_PSEnd[20];
   Int_t Cha_NumPS[20];
   Int_t Cha_MCAEnergy[20];

   PSTree->SetBranchAddress("PS_TotalNum",&PS_TotalNum);
   PSTree->SetBranchAddress("PS_Amp",PS_Amp);
   PSTree->SetBranchAddress("Cha_PSStart",Cha_PSStart);
   PSTree->SetBranchAddress("Cha_PSEnd",Cha_PSEnd);
   PSTree->SetBranchAddress("Cha_NumPS",Cha_NumPS);
   PSTree->SetBranchAddress("Cha_MCAEnergy",Cha_MCAEnergy);

   //output:
   TFile *foutput = NULL;
   if (argc==2) 
      foutput=new TFile("wfcollection.root","recreate");
   else 
      foutput=new TFile(argv[2],"recreate");
   foutput->cd();

   //content:
   const Int_t Ncha=19;
   TClonesArray *wfClonesArray=new TClonesArray("MGTWaveform", Ncha);
   TTree *wfTree = new TTree("wfTree", "wfTree");
   wfTree->Branch("wfcollection", &wfClonesArray);
   const size_t length = 300;
   double wfdata[length]={0};
 
   Int_t Cha_Etotal;
   Int_t nentries = PSTree->GetEntries();
   cout<<" total number of events "<<nentries<<endl;
   for (Int_t i=0; i<nentries;i++) {
//   for (Int_t i=0; i<1000; i++) {
      PSTree->GetEntry(i);
      if (i%1000==0) cout<<" now event "<<i<<endl;
      
      Cha_Etotal=0;
      for (Int_t icha=0; icha<Ncha; icha++) {
         if (icha!=0) Cha_Etotal+=Cha_MCAEnergy[icha];
         Int_t k=0;
         for (Int_t j=Cha_PSStart[icha]; j<=Cha_PSEnd[icha]; j++) {
            wfdata[k]=(double)PS_Amp[j];
            k++;
         }
         new((*wfClonesArray)[icha]) 
            MGTWaveform(wfdata, length, 75*CLHEP::MHz, 0.0*CLHEP::ns, MGWaveform::kCharge, icha);
      }
      wfTree->Fill();
      if (i%1000==0) wfTree->Write("kOverwrite");
      wfClonesArray->Delete();
   }

   wfTree->Write("kOverwrite");
   foutput->Close();//wfTree also deleted.

   delete foutput;
   delete PSTree;
   delete wfClonesArray;

   return 0;
}

