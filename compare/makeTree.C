// To merge 2 PS trees (signal and background) into one, 
// in a format that can be used to train a neural network

// use like makeTree("Signal.root" , "Background.root")


void makeTree(const char* signalFname, const char* backgrFname)
{

  // read in the input files
  TFile *sigFile = TFile::Open(signalFname);
  TFile *bgFile = TFile::Open(backgrFname);

  TTree *signal = (TTree *) sigFile->Get("wfTree");
  TLeaf *fsigData = signal->GetLeaf("wfcollection.fData");
  // signal->GetLeaf("wfcollection.fData")->ls();

    //TBranch *sigbranch = signal->SetBranchAddress("wfcollection", &wfcollection);
  TTree *background = (TTree *) bgFile->Get("wfTree");
  
  // create the new tree, 
  TFile *newdatafile = new TFile("DataSample.root","recreate");
  TTree *datasam = new TTree("DataSample", "Data Sample for Neural Network");
   
  // Merge the 2 trees into one, and add a "type" branch, 
  // equal to 1 for the signal and 0 for the background 

  // not like this: leaves!!
  //   signal->SetBranchAdress("fUniqueID", &wfcollection.fUniqueID);	      
  //   signal->SetBranchAdress("fBits", &wfcollection.fBits);	      
  //   signal->SetBranchAdress("fName", &wfcollection.fName);	      
  //   signal->SetBranchAdress("fTitle", &wfcollection.fTitle);
  //   signal->SetBranchAdress("fSampFreq", &wfcollection.fSampFreq); 
  //   signal->SetBranchAdress("fTOffset", &wfcollection.fTOffset);	      
  //   signal->SetBranchAdress("fWFType", &wfcollection.fWFType);	      
  //   signal->SetBranchAdress("fData", &wfcollection.fData);	      
  //   signal->SetBranchAdress("fID", &wfcollection.fID);	      
  //   signal->SetBranchAdress("fWaveformFunction", &wfcollection.fWaveformFunction);

  Int_t type;
  datasam->Branch("type",   &type,   "type/I");
  type = 1;

  Int_t i;
  for (i = 0; i < signal->GetEntries(); i++) {
    signal->GetEntry(i);
    datasam->Fill();
  }
  type = 0;
  for (i = 0; i < background->GetEntries(); i++) {
    background->GetEntry(i);
    datasam->Fill();
  }
  
  // save output
   datasam->Print();
   datasam->AutoSave();
   delete newdatafile;

}
