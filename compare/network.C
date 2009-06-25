// Build and train the NN for the PSA
// Use like .x network ("trainingfile.root", 100) (for 100 training iterations)

#include <TString.h>

void network(const char* filename, Int_t ntrain){

  // ----- Read in the input file 
  TFile *trainingFile = TFile::Open(filename);
  TTree *trainingTree = (TTree *) trainingFile->Get("DataSample");
  cout << " Opened the input file." << endl;

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
  TFile *friendFile = new TFile("treepfriend.root","recreate");
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

  //TEST: to check if the kinf of network layout desciption and tree work
  TMultiLayerPerceptron *networkPSA = new TMultiLayerPerceptron("wfBin_001, wfBin_002, wfBin_003, wfBin_004, wfBin_005:10:type", "Weight", trainingTree, "Entry$%2","(Entry$+1)%2");    

  // use a for loop to construct the layout string containing the names of all the branches
  // "input layer (expression for each neuron, seperated by commas) : hidden layer (number of neurons) : output layer"
  TString *netLayout = new TString();
  for (Int_t b = 0; b < 300 ; b++){
    netLayout->Append(branchName[b]);
    if (b<299) netLayout->Append(", ");
    if (b==299) netLayout->Append(":300:type");
  }
  cout << endl << "Lenght of string after the loop with the branch names: "<< endl << netLayout->Sizeof() << endl ;
  // Does not work like this, use sstream instead??!


  /// TMultiLayerPerceptron *networkPSA = new TMultiLayerPerceptron(layoutChar, "Weight", trainingTree,"Entry$%2","(Entry$+1)%2");
                                                                 // Use half of the events as training and half as test dataset

  cout << "Done." << endl;

  // set learning method 
      //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kStochastic);  
      //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kBatch); 
      //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kSteepestDescent);
      //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kRibierePolak);
      //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kFletcherReeves); 
  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kBFGS); // the default learning method

  //  networkPSA->Train(ntrain, "text,graph,update=10"); //full output every ten epochs

  delete trainingFile;
  delete friendFile;
} // end of "void network"

