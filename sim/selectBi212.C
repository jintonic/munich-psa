// select the Bi212 events from a file written using the GEOutputCrystal I/O scheme
// use like .x selectBi212.C("Bi212_top10cm.root")


void selectBi212(const char* filename)
{
// ---- setup the histogram(s)
   Int_t    nbinsx = 2200;
   Double_t xlow   = 10;
   Double_t xup    = 2010;

   if (gDirectory->Get("singleSeg")) delete singleSeg;
   if (gDirectory->Get("Bi212single")) delete Bi212single;
   TH1F *singleSeg = new TH1F("singleSeg","single segment",nbinsx,xlow,xup);
   TH1F *Bi212single = new TH1F("Bi212single","Bi212 events, single segment cut",nbinsx,xlow,xup);

// --- read in contents of the rootfile
   TFile *_file0 = TFile::Open(filename);

   TTree *oldtree = (TTree*) _file0->Get("fTree");
   
   Float_t segEnergy[4][20];
   fTree->SetBranchAddress("segEnergy",segEnergy);

   Int_t Neve=fTree->GetEntries();
   cout<<" total entries = "<<Neve<<endl;

   //Create a new file + a clone of old tree in new file
   TFile *newfile = new TFile("singleBi212.root","recreate");
   TTree *newtree = oldtree->CloneTree(0);
   
// --- loop over all segments to find single segment events
   Int_t NsingleSeg=0;
   Int_t NBi212single=0;

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

     // select Bi212 events
     if (Nseg==1) {
       NsingleSeg++;     
       singleSeg->Fill(segEnergy[crystalID][0]);
       if(segEnergy[crystalID][0]>1616.5&&segEnergy[crystalID][0]<1624.5){
       newtree->Fill();
       Bi212single->Fill(segEnergy[crystalID][0]);
       NBi212single +=1;
       }
     }
   }

   Bi212single->Draw();
   cout<<"Number of single segment events: "<<NsingleSeg<<endl;
   cout<<"Number of single segment events with 1616.5 < core energy < 1624.5 keV: "<<NBi212single<<endl;

   newtree->Print();
   newtree->AutoSave();
   //   delete oldfile;
   delete newfile;

// --- save histogram with Bi212 events
    TFile* f1 = new TFile("single_histo.root","recreate");
     f1->cd();
     //    singleSeg->Write();
     Bi212single->Write();
     f1->Close();

}
