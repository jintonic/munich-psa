// select the events inside a peak of some energy 
// (e.g. 1578 keV for DEP for simulation with Egamma=2600 keV) 
// from a file written using the GEOutputCrystal I/O scheme
// use like .x selectEventsfromSim.C("Tl208_top10cm.root",1578)


void selectEventsfromSim(const char* filename, Int_t ECenter)
{
// ---- setup the histogram and the canvas

   TCanvas *canvasSIM = new TCanvas("canvasSIM","Peak from Simulation",800,600);
     canvasSIM->SetFillColor(0);
     canvasSIM->SetBorderMode(0);
     canvasSIM->SetFrameBorderMode(0); 

   TH1F *SingleHisto = new TH1F("SingleHisto","Selected events, single segment cut",10,ECenter-1,ECenter+1);


// --- read in contents of the rootfile
   TFile *_file0 = TFile::Open(filename);

   TTree *oldtree = (TTree*) _file0->Get("fTree");
   
   Float_t segEnergy[4][20];
   fTree->SetBranchAddress("segEnergy",segEnergy);

   Int_t Neve=fTree->GetEntries();
   cout<<" total entries = "<<Neve<<endl;

   //Create the output file + a clone of old tree in new file
   TFile *SingleFile = new TFile("/remote/pclg-09/vauth/tmp/singleSite.root","recreate");
   TTree *SingleTree = oldtree->CloneTree(0);
   TString *outfile = new TString();
   outfile->Append(Form("single%d",ECenter));
   outfile->Append("fromSIM.root");
   const char* outfileChar = outfile->Data();
   cout << "Results are written to " << outfileChar << endl; 
   TFile *OutputFile = new TFile(outfileChar,"recreate");
   TTree *OutputTree = oldtree->CloneTree(0);
   

// --- loop over all segments to find single segment events
   Int_t NsingleSeg=0;
   Int_t NSelectedSingle=0;

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

     if (Nseg==1) {
       NsingleSeg++;
       SingleTree->Fill();
       
       // fill histogram that is used to find the FWHM of peak
       if(segEnergy[crystalID][0]>(ECenter - 1) && segEnergy[crystalID][0]<(ECenter + 1) ){
	 SingleHisto->Fill(segEnergy[crystalID][0]);
       }
     } 
   }
   cout << "NsingleSeg is " << NsingleSeg << ", SingleTree->GetEntries() gives " << SingleTree->GetEntries() << endl;

  // draw
   SingleHisto->Draw();
   SingleHisto->GetXaxis()->SetTitle("Energy [keV]");
   SingleHisto->GetYaxis()->SetTitle("Number of events");
   SingleHisto->GetXaxis()->CenterTitle();
   SingleHisto->GetYaxis()->CenterTitle();
   SingleHisto->GetXaxis()->SetTitleFont(132);
   SingleHisto->GetYaxis()->SetTitleFont(132);
   SingleHisto->GetXaxis()->SetLabelFont(132);
   SingleHisto->GetYaxis()->SetLabelFont(132);

//    "exact" lines from sim, don't need to fit a peak like with data...
//    TF1 *fitSelected = new TF1("fitSelected","gaus",ECenter-0.3,ECenter+0.3);
//    SingleHisto->Fit("fitSelected","R");
//    Float_t meanSelected = fitSelected->GetParameter(1);
//    Float_t sigmaSelected = fitSelected->GetParameter(2);
//    cout << "Making cut for " << meanSelected - sigmaSelected << " <  Energy < " << meanSelected + sigmaSelected << endl;
   TString *outEPS = new TString();
   outEPS->Append(Form("single%d",ECenter));
   outEPS->Append("fromSIM.eps");
   const char* outEPSChar = outEPS->Data();
   canvasSIM->SaveAs(outEPSChar);

   //   SingleHisto->SetFillColor(45);
   //   SingleHisto->DrawCopy();
 

// fill the trees
   for (Int_t i = 0; i < NsingleSeg ; i ++){
     SingleTree -> GetEntry(i);
     if( segEnergy[crystalID][0] >= (ECenter - 0.2)  && segEnergy[crystalID][0] <= (ECenter + 0.2)  ){
       OutputTree->Fill();
       NSelectedSingle +=1;
     }
   }

   //   OutputTree->Print();
   cout<<"Number of single segment events: "<<NsingleSeg<<endl;
   cout<<"Number of selected events: "<<NSelectedSingle <<endl;
   OutputTree->AutoSave();

   //   delete OutputFile;
   cout << endl;
}
