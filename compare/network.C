// Build and train the NN for the PSA
// Use like .x network ("trainingfile.root", 10) (for 10 training iterations)

#include <TString.h>

void network(const char* filename, Int_t ntrain){

  // ----- Read in the input file 
  TFile *trainingFile = TFile::Open(filename);
  TTree *trainingTree = (TTree *) trainingFile->Get("DataSample");
  cout << "Opened the input file." << endl;

  // Set up the branches for the training tree
  Int_t type;
  Double_t wfBin[300];
  TString branchName[300];

  cout << "Setting up the branches of the training Tree... ";
  trainingTree->SetBranchAddress("type",   &type);
  for (Int_t b = 1; b <= 300 ; b++){
    if (b < 10) branchName[b-1] = (Form("wfBin_00%d",b));
    if (b >= 10 && b < 100) branchName[b-1] = (Form("wfBin_0%d",b));
    if (b >= 100) branchName[b-1] = (Form("wfBin_%d",b));
    trainingTree->SetBranchAddress(branchName[b-1] ,  &wfBin[b-1]);
  }
  cout << "Done." << endl;


  // ----- Add weight branch to the training tree
  Int_t nentries = trainingTree -> GetEntries(); 
  Float_t fWeight; 
  TFile *friendFile = new TFile("treefriend.root","recreate");
  TTree *treefriendWeight = new TTree("treefriendWeight", "A tree with a weight branch to add as friend to the training tree");
  treefriendWeight->Branch("Weight", &fWeight, "Weight/F"); 
  for (Int_t i = 0; i < nentries; i++)
    {
      treefriendWeight->GetEntry(i); 
      fWeight       = 1.0 / float(nentries);  // maybe?
      treefriendWeight->Fill();       
    }
  treefriendWeight->Write();
  trainingTree->AddFriend(treefriendWeight);


  // ----- Build and train the NN 
  cout << endl << "Setting up the neural network... " ;

  // use a for loop to construct the layout string containing the names of all the branches
  // "input layer (expression for each neuron, seperated by commas) : hidden layer (number of neurons) : output layer"
  TString *netLayout = new TString();
  for (Int_t b = 0; b < 300 ; b++){
    netLayout->Append(branchName[b]);
    if (b<299) netLayout->Append(", ");
    else netLayout->Append(":300:type");            // enter number of neurons in hidden layer HERE.
  }
  const char* layoutChar = netLayout->Data();
  // cout << endl << "(This is how the layout string for the NN looks: "<< endl << layoutChar << endl << ".)" << endl ;
  
  TMultiLayerPerceptron *networkPSA = new TMultiLayerPerceptron(layoutChar, "Weight", trainingTree,"Entry$%2","(Entry$+1)%2");
                                                                 // Use half of the events as training and half as test dataset

  //TEST: to check if the kind of network layout desciption and tree work
  // TMultiLayerPerceptron *networkPSA = new TMultiLayerPerceptron("wfBin_001, wfBin_002, wfBin_003, wfBin_004, wfBin_005:10:type", "Weight", trainingTree, "Entry$%2","(Entry$+1)%2");   cout << " (Using the test network with only 5 inputs) "; 

  //a larger test network
   Int_t testnum = 50; Int_t testfrom = 111;  TString *testLayout = new TString();   
   for (Int_t b = testfrom; b < testfrom + testnum ; b++) {    testLayout->Append(branchName[b]);     
     if (b<testfrom + testnum -1 ) testLayout->Append(", ");   else testLayout->Append(":50:type");     }      const char* testChar = testLayout->Data(); 
   //   TMultiLayerPerceptron *networkPSA = new TMultiLayerPerceptron(testChar, "Weight", trainingTree,"Entry$%2","(Entry$+1)%2");  
   // cout << " (Using test network, bins from " << testfrom-1 << " to " << testfrom + testnum -1 << ") ";

  cout << "Done." << endl;

  const char* method;
  // set learning method 
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kStochastic);  method = " kStochstic";              //works sometimes (?), extremly slow
  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kBatch);   method = " kBatch";                      //seems to work
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kSteepestDescent);   method = " kDescent";          //seems to work
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kRibierePolak); method = " RibierePolak";               //seems to work
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kFletcherReeves);  method = " kFletcherReeves";     //seems to work
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kBFGS); method = " kBFGS";                          //does not work 
  cout << endl << "--- Using learning method" << method << ". ---" << endl << endl;
  networkPSA->Train(ntrain, "text,graph,update=1"); //full output every epoch


  // ----- Display the results
  TCanvas* canvas_NN = new TCanvas("canvas_NN"); 
   TH1F *bg = new TH1F("bgh", "NN output", 150, -1.0, 2.0);
   TH1F *sig = new TH1F("sigh", "NN output", 150, -1.0, 2.0);
   bg->SetDirectory(0);
   sig->SetDirectory(0);
   Double_t paramsBG[300];
   Double_t paramsSig[300];
   for (i = 0; i < trainingTree->GetEntries(); i++) {
      trainingTree->GetEntry(i);
      if (type==0){
      for (b = 0; b < 300 ; b++){
	paramsBG[b] = branchName[b];
      }
      bg->Fill(networkPSA->Evaluate(0, paramsBG));
      }
      else {
      for (b = 0; b < 300 ; b++){
	paramsSig[b] = branchName[b];
      }
      sig->Fill(networkPSA->Evaluate(0,paramsSig));
      }
   }
   bg->SetLineColor(kBlue);
   bg->SetFillStyle(3008);   bg->SetFillColor(kBlue);
   sig->SetLineColor(kRed);
   sig->SetFillStyle(3003); sig->SetFillColor(kRed);
   bg->SetStats(0);
   sig->SetStats(0);
   bg->Draw();
   sig->Draw("same");
   TLegend *legend = new TLegend(.75, .80, .95, .95);
   legend->AddEntry(bg, "Background");
   legend->AddEntry(sig, "Signal");
   legend->Draw();
   mlpa_canvas->cd(0);



  delete trainingFile;
  delete friendFile;
} // end of "void network"

