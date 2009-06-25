// select the K40 line from data files (single segment events)
// use like .x selectBi212fromData.C
// enter the filenames for the TChain manually in this file

#include "TChain.h"

void selectBi212fromData()
{

// --- read in contents of the rootfiles
   TChain chain("PSTree");
   chain.Add("calibrated.data_aug02_01.root");
   chain.Add("calibrated.data_aug02_02.root");
   chain.Add("calibrated.data_aug02_03.root");
   cout<<"TChain was set up."<<endl;
   
   Float_t Cha_Energy[20];
   chain.SetBranchAddress("Cha_Energy",Cha_Energy);

   Int_t Neve=chain.GetEntries();
   cout<<" total entries = "<<Neve<<endl;

   //Create a new file + a clone of old tree in new file
   TFile *newfile = new TFile("singleBi212.root","recreate");
   TTree *newtree = chain.CloneTree(0);

// ---- loop over all events
   Int_t NsingleSeg=0;
   Int_t NBi212=0;

   for (Int_t i=0; i<Neve; i++) {
     if (i%10000==0) cout<<" now event "<<i<<endl;
     chain.GetEntry(i);
     
     Int_t Nseg=0;
     for (Int_t k=1; k<=18; k++) {
       if (Cha_Energy[k]>10) {
      	 Nseg += 1;
       }
     }
     
// select the Bi212 line (1620.5 keV)
     if (Nseg==1) {
       NsingleSeg++;
       if(Cha_Energy[0]>1616.5 && Cha_Energy[0]<1624.5){
	 newtree->Fill();
	 NBi212 +=1;
       }
     }
   }

   newtree->Print();
   cout<<"Number of single segment events: "<<NsingleSeg<<endl;
   cout<<"Number of single segment events with 1616.5 < Cha_Energy[0] < 1624.5: "<<NBi212<<endl;

   newtree->AutoSave();
   delete newfile;

}
