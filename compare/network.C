// Build and train the NN for the PSA
// Use like .x network ("trainingfile.root", 100) (for 100 training iterations)

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

  //a smaller test network
  Int_t testnum = 50; Int_t testfrom = 111;  TString *testLayout = new TString();   
  for (Int_t b = testfrom; b < testfrom + testnum ; b++) {    testLayout->Append(branchName[b]);     
    if (b<testfrom + testnum -1 ) testLayout->Append(", ");   else testLayout->Append(":50:type");     }      const char* testChar = testLayout->Data(); 
  //TMultiLayerPerceptron *networkPSA = new TMultiLayerPerceptron(testChar, "Weight", trainingTree,"Entry$%2","(Entry$+1)%2");  
  //cout << " (Using test network, bins from " << testfrom-1 << " to " << testfrom + testnum -1 << ") ";

  cout << "Done." << endl;

  const char* method;
  // set learning method 
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kStochastic);  method = " kStochstic";              //works sometimes (?), very slow
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kBatch);   method = " kBatch";                      //seems to work
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kSteepestDescent);   method = " kDescent";          //seems to work
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kRibierePolak); method = " kRibierePolak";           //seems to work
    networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kFletcherReeves);  method = " kFletcherReeves";     //seems to work, kind of slow
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kBFGS); method = " kBFGS";                          //does NOT work (only sometimes..)
  cout << endl << "--- Using learning method" << method << ". ---" << endl << endl;


  networkPSA->Train(ntrain, "text,graph,update=10"); //full output every 10 epochs
  networkPSA->DumpWeights("weights.txt"); 


  // ----- Display the results

  // try the direct way
  networkPSA->DrawResult(0,"test");  


  // Using Evaluate, get the second entries from tree (should give the test dataset) 
  Double_t params[300];
  TCanvas* canvas_NN = new TCanvas("canvas_NN","Histo for ->Evaluate", 150, 150, 800, 600); 
  TH1D *hist_NN = new TH1D("hist_NN","NN output", 200, 0.0, 2.);
  Double_t NNoutput;  
  // loop over events 
  for (Int_t ientries = 0; ientries < nentries/2; ientries++)
    {
      trainingTree -> GetEntry(2*ientries+1); 
      // evaluate NN 
      for (b = 0; b < 300 ; b++){
	params[b] = wfBin[b];
	if(b%100==50 && ientries%250==0) cout << "entry " << 2*ientries+1 << ": params[" << b<< "] = " << params[b] << endl;
      }
      NNoutput = networkPSA -> Evaluate(0, params); 
      if (ientries%250==0) cout << "NNOutput from Evalutate: "<<NNoutput << " Network Result(0): " << networkPSA->Result(ientries,0) <<  endl;
      hist_NN -> Fill(NNoutput);
    }
  canvas_NN -> cd(); 
  hist_NN -> SetStats(kFALSE); 
  hist_NN -> Draw(); 
  

  // other try to display the results
  TCanvas* canvas_tree = new TCanvas("canvas_tree","Read from tree, evaluate", 300, 300, 800, 600); 
  canvas_tree->cd();
  TH1D *bg = new TH1D("bg","NN output", 200, 0.0, 2.0);
  TH1D *sig = new TH1D("sig","NN output", 200, 0.0, 2.0);
  for (i = 0; i < trainingTree->GetEntries(); i++) {
    trainingTree->GetEntry(i);
    if (type==0){
      for (b = 0; b < 300 ; b++){
   	params[b] = wfBin[b];
      }
      bg->Fill(networkPSA->Evaluate(0, params));
    }
    else {
      for (b = 0; b < 300 ; b++){
   	params[b] = wfBin[b];
      }
         sig->Fill(networkPSA->Evaluate(0,params));
    }
  }  
  bg->SetLineColor(kBlue);  bg->SetFillStyle(3008);   bg->SetFillColor(kBlue);
  sig->SetLineColor(kRed);  sig->SetFillStyle(3003); sig->SetFillColor(kRed);
  bg->SetStats(0);    bg->Draw();
  sig->SetStats(0);   sig->Draw("same");
  TLegend *legend = new TLegend(.75, .80, .95, .95);
  legend->AddEntry(bg, "Background");    legend->AddEntry(sig, "Signal");
  legend->Draw();


  delete trainingFile;
  delete friendFile;
} // end of "void network"

