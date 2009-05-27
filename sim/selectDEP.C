// select the DEP events from a file written using the GEOutputCrystal I/O scheme
// use like .x selectDEP.C("Tl208_top10cm.root")


void selectDEP(const char* filename)
{
// ---- setup the histogram(s)
   Int_t    nbinsx = 2200;
   Double_t xlow   = 10;
   Double_t xup    = 2010;

   if (gDirectory->Get("singleSeg")) delete singleSeg;
   if (gDirectory->Get("DEPsingle")) delete DEPsingle;
   TH1F *singleSeg = new TH1F("singleSeg","single segment",nbinsx,xlow,xup);
   TH1F *DEPsingle = new TH1F("DEPsingle","DEP events, single segment cut",nbinsx,xlow,xup);

// --- read in contents of the rootfile
   TFile *_file0 = TFile::Open(filename);
  
   Float_t segEnergy[4][20];
   fTree->SetBranchAddress("segEnergy",segEnergy);

   Int_t Neve=fTree->GetEntries();
   cout<<" total entries = "<<Neve<<endl;

   
// --- loop over all segments to find single segment events
   Int_t NsingleSeg=0;
   Int_t NDEPsingle=0;

   for (Int_t i=0; i<Neve; i++) {
     if (i%10000==0) cout<<" now event "<<i<<endl;
     fTree->GetEntry(i);
     
     Int_t Nseg=0;
     Int_t crystalID=1;  // (only one crystal simulated)
     for (Int_t k=1; k<=18; k++) {
       if (segEnergy[crystalID][k]>20) {
	 Nseg += 1;
       }
     }

     // select DEP events
     if (Nseg==1) {
       NsingleSeg++;     
       singleSeg->Fill(segEnergy[crystalID][0]);
       if(segEnergy[crystalID][0]>1577&&segEnergy[crystalID][0]<1579){
	 DEPsingle->Fill(segEnergy[crystalID][0]);
	 NDEPsingle +=1;
       }
     }
   }

   DEPsingle->Draw();
   cout<<"Number of single segment events: "<<NsingleSeg<<endl;
   cout<<"Number of DEP events: "<<NDEPsingle<<endl;


// --- save histogram with DEP events
   TFile* f1 = new TFile("sim_DEP.root","recreate");
    f1->cd();
    //    singleSeg->Write();
    DEPsingle->Write();
    f1->Close();

}
