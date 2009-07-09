// select the DEP and BG events from files
// use like .x selectEventsfromData.C
// enter the filenames for the TChain manually

#include "TChain.h"

void selectEventsfromData()
{
// ---- setup the histograms and the canvas
   if (gDirectory->Get("DEPsingle")) delete DEPsingle;
   if (gDirectory->Get("BGsingle")) delete BGsingle;
   TCanvas *canvasDEP = new TCanvas("canvasDEP","DEP line",800,600);
     canvasDEP->SetFillColor(0);
     canvasDEP->SetBorderMode(0);
     canvasDEP->SetFrameBorderMode(0); 
   TCanvas *canvasBG = new TCanvas("canvasBG","background gamma line",800,600);
     canvasBG->SetFillColor(0);
     canvasBG->SetBorderMode(0);
     canvasBG->SetFrameBorderMode(0);

   TH1F *DEPsingleHisto = new TH1F("DEPsingleHisto","DEP events, single segment cut",100,1582,1602);
   TH1F *BGsingleHisto = new TH1F("BGsingleHisto","Bi 212, single segment cut",100,1610,1630);


// --- read in contents of the rootfile
   TChain chain("PSTree");
   chain.Add("calibrated.data_aug02_01.root");
   chain.Add("calibrated.data_aug02_02.root");
   chain.Add("calibrated.data_aug02_03.root");
   cout<<"TChain was set up."<<endl;
   
   Float_t Cha_Energy[20];
   chain.SetBranchAddress("Cha_Energy",Cha_Energy);

   Int_t Neve=chain.GetEntries();
   cout<<" total entries = "<<Neve<<endl;

   //Create output files + a clone of old tree in new files
   TFile *DEPfile = new TFile("/remote/pclg-09/vauth/tmp/singeSite.root","recreate");
   TTree *Singletree = chain.CloneTree(0);

   TFile *DEPfile = new TFile("singleDEP.root","recreate");
   TTree *DEPtree = chain.CloneTree(0);

   TFile *BGfile = new TFile("singleBi212.root","recreate");
   TTree *BGtree = chain.CloneTree(0);


// ---- loop over all events to find single segment events
   Int_t NsingleSeg=0;
   Int_t NDEPsingle=0;
   Int_t NBGsingle=0;

   for (Int_t i=0; i<Neve; i++) {
     if (i%10000==0) cout<<" now event "<<i<<endl;
     chain.GetEntry(i);

     Int_t Nseg=0;
     for (Int_t k=1; k<=18; k++) {
       if (Cha_Energy[k]>10) {
	 Nseg += 1;
       }
     }
     
     if (Nseg==1) {
       NsingleSeg++;
      
      Singletree->Fill();

       // fill histogram to find the FWHM of DEP peak
       if(Cha_Energy[0]>1582&&Cha_Energy[0]<1602){
	 DEPsingleHisto->Fill(Cha_Energy[0]);
       }

       // fill histogram to find the FWHM of BG peak
       if(Cha_Energy[0]>1610&&Cha_Energy[0]<1630){
	 BGsingleHisto->Fill(Cha_Energy[0]);
       }

     }
   }
   cout << "NsingleSeg is " << NsingleSeg << ", Singletree->GetEntries() gives " << Singletree->GetEntries() << endl;

   // draw the events close to DEP, fit peak
   canvasDEP->cd();
   DEPsingleHisto->Draw();
   DEPsingleHisto->GetXaxis()->SetTitle("Energy [keV]");
   DEPsingleHisto->GetYaxis()->SetTitle("Number of events");
   DEPsingleHisto->GetXaxis()->CenterTitle();
   DEPsingleHisto->GetYaxis()->CenterTitle();
   DEPsingleHisto->GetXaxis()->SetTitleFont(132);
   DEPsingleHisto->GetYaxis()->SetTitleFont(132);
   DEPsingleHisto->GetXaxis()->SetLabelFont(132);
   DEPsingleHisto->GetYaxis()->SetLabelFont(132);
   
   TF1 *fitDEP = new TF1("fitDEP","gaus",1587,1597);
   DEPsingleHisto->Fit("fitDEP","R");
   Float_t meanDEP = fitDEP->GetParameter(1);
   Float_t sigmaDEP = fitDEP->GetParameter(2);
   cout << "Making DEP cut for " << meanDEP - sigmaDEP << " <  Energy < " << meanDEP + sigmaDEP << endl;
   canvasDEP->SaveAs("DEPsingle.eps");

   // select the Bi212 line (1620.5 keV): fit peak
   canvasBG->cd();
   BGsingleHisto->Draw();
   BGsingleHisto->GetXaxis()->SetTitle("Energy [keV]");
   BGsingleHisto->GetYaxis()->SetTitle("Number of events");
   BGsingleHisto->GetXaxis()->CenterTitle();
   BGsingleHisto->GetYaxis()->CenterTitle();
   BGsingleHisto->GetXaxis()->SetTitleFont(132);
   BGsingleHisto->GetYaxis()->SetTitleFont(132);
   BGsingleHisto->GetXaxis()->SetLabelFont(132);
   BGsingleHisto->GetYaxis()->SetLabelFont(132);
   TF1 *fitBG = new TF1("fitBG","gaus",1615,1625);
   BGsingleHisto->Fit("fitBG","R");
   Float_t meanBG = fitBG->GetParameter(1);
   Float_t sigmaBG = fitBG->GetParameter(2);
   cout << "Making BG cut for " << meanBG - sigmaBG << " < Energy < " << meanBG + sigmaBG << endl;
   canvasBG->SaveAs("BGsingle.eps");
      

   // fill the trees
   for (Int_t i = 0; i < NsingleSeg ; i ++){
     Singletree -> GetEntry(i);

     if( Cha_Energy[0] >= (meanDEP - sigmaDEP)  && Cha_Energy[0] <= (meanDEP + sigmaDEP) ){
       DEPtree->Fill();
       NDEPsingle +=1;
     }

     if( Cha_Energy[0] >= (meanBG - sigmaBG)  && Cha_Energy[0] <= (meanBG + sigmaBG) ){
       BGtree->Fill();
       NBGsingle +=1;
     }
   }


   // DEPtree->Print();
   // BGtree->Print();
   cout<<"Number of single segment events: "<<NsingleSeg<<endl;
   cout<<"Number of single segment DEP events: "<<NDEPsingle<<endl;
   cout<<"Number of single segment Bi212 events: "<<NBGsingle<<endl;

   DEPtree->AutoSave();
   BGtree->AutoSave();
   
   //delete newfile;
   cout << endl;

}
