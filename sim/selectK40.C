// select the K40 events from a file written using the GEOutputCrystal I/O scheme
// use like .x selectK40.C("Tl208_top10cm.root")


void selectK40(const char* filename)
{
// ---- setup the histogram(s)
   Int_t    nbinsx = 2200;
   Double_t xlow   = 10;
   Double_t xup    = 2010;

   if (gDirectory->Get("singleSeg")) delete singleSeg;
   if (gDirectory->Get("K40single")) delete K40single;
   TH1F *singleSeg = new TH1F("singleSeg","single segment",nbinsx,xlow,xup);
   TH1F *K40single = new TH1F("K40single","K40 events, single segment cut",nbinsx,xlow,xup);

// --- read in contents of the rootfile
   TFile *_file0 = TFile::Open(filename);

   TTree *oldtree = (TTree*) _file0->Get("fTree");
   
   Float_t segEnergy[4][20];
   fTree->SetBranchAddress("segEnergy",segEnergy);

   Int_t Neve=fTree->GetEntries();
   cout<<" total entries = "<<Neve<<endl;

   //Create a new file + a clone of old tree in new file
   TFile *newfile = new TFile("singleK40.root","recreate");
   TTree *newtree = oldtree->CloneTree(0);
   
// --- loop over all segments to find single segment events
   Int_t NsingleSeg=0;
   Int_t NK40single=0;

   for (Int_t i=0; i<Neve; i++) {
     if (i%10000==0) cout<<" now event "<<i<<endl;
     oldtree->GetEntry(i);
     
     Int_t Nseg=0;
     Int_t crystalID=1;  // (only one crystal simulated)
     for (Int_t k=1; k<=18; k++) {
       if (segEnergy[crystalID][k]>20) {
	 Nseg += 1;
       }
     }

     // select K40 events
     if (Nseg==1) {
       NsingleSeg++;     
       singleSeg->Fill(segEnergy[crystalID][0]);
       if(segEnergy[crystalID][0]>1456.8&&segEnergy[crystalID][0]<1464.8){
       newtree->Fill();
       K40single->Fill(segEnergy[crystalID][0]);
       NK40single +=1;
       }
     }
   }

   K40single->Draw();
   cout<<"Number of single segment events: "<<NsingleSeg<<endl;
   cout<<"Number of single segment 1461 keV events: "<<NK40single<<endl;

   newtree->Print();
   newtree->AutoSave();
   //   delete oldfile;
   delete newfile;

// --- save histogram with K40 events
    TFile* f1 = new TFile("single_histo.root","recreate");
     f1->cd();
     //    singleSeg->Write();
     K40single->Write();
     f1->Close();

}
