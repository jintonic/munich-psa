// Build and train the NN for the PSA
// Use like .x network("DataSample.root",100)        (for 100 training iterations)

#include "network.h"


void network(const char* filename, Int_t ntrain)
{

  fNTrain = ntrain;

  // ----- Read in the input file 
  TFile *trainingFile = TFile::Open(filename);
  fTrainingTree = (TTree *) trainingFile->Get("DataSample");

  // set branch adresses and add a weight branch
  setupTrainingTree();   
  

  // define the network layouy and build the NN
  buildNetwork();


  // --- set learning method 
  //  fMethod = "kStochstic";  //doesn't work ("Stop"=learning would lead to non real numbers..?)  
  fMethod = "kBatch";         
  //  fMethod = "kDescent";       
  //  fMethod = "kRibierePolak";  
  //  fMethod = "kFletcherReeves"; 
  //  fMethod = "kBFGS";                                                                             
  const char* method = GetMethod();


  // --- train the NN, plot the training progress
  trainNetwork(); 


 // --- Draw the network result in different ways
  displayResults();   

} // end of "void network"








// -------------------------------------------------------------
//
//                Neural Network Functions
//
//  -------------------------------------------------------------


void setupTrainingTree()
{
  cout << "Setting up the branches of the training Tree... ";

  fTrainingTree->SetBranchAddress("type",   &fType);

  fWFnum = (fTrainingTree->GetNbranches() )-1;   // one branch is type, all the others are wfBin
  fWFbin = new Double_t[fWFnum];
  fBranchNames = new TString[fWFnum];
  for (size_t b = 1; b <= fWFnum ; b++){
    if (b < 10) fBranchNames[b-1] = (Form("wfBin_00%d",b));
    if (b >= 10 && b < 100) fBranchNames[b-1] = (Form("wfBin_0%d",b));
    if (b >= 100) fBranchNames[b-1] = (Form("wfBin_%d",b));
    fTrainingTree->SetBranchAddress(fBranchNames[b-1] ,  &fWFbin[b-1]);
  }
  cout << "Done." << endl;

  // Add weight branch to the training tree
  Int_t nentries = fTrainingTree -> GetEntries(); 
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
  fTrainingTree->AddFriend(treefriendWeight);
}


//-----------------------------------------------------------------------------


void buildNetwork()
{
  cout << endl << "Setting up the neural network... " ;

  // use a for-loop to construct the layout string containing the names of all the branches
  // "input layer (expression for each neuron, seperated by commas) : hidden layer (number of neurons) : output layer"
  TString *netLayout = new TString();
  for (size_t b = 0; b < fWFnum ; b++) 
    {    netLayout->Append(fBranchNames[b]);     
      if (b<fWFnum -1 ) {
	netLayout->Append(", ");
      }
      else { 
	netLayout->Append(":50");        // Enter number of hidden neurons HERE
	netLayout->Append(":type");      // Output neuron
      }
    }
  const char* layoutChar = netLayout->Data();
  //  cout << endl << "(This is how the layout string for the NN looks: "<< endl << layoutChar << ".)" << endl ;  
  
  networkPSA = new TMultiLayerPerceptron(layoutChar, "Weight", fTrainingTree,"Entry$%2","(Entry$+1)%2");
                                                                 // Use half of the events as training and half as test dataset
  cout << "Done." << endl;
}


//-----------------------------------------------------------------------------


void trainNetwork()
{

  TMultiLayerPerceptron::ELearningMethod learnMethod = fMethod;

  networkPSA ->  SetLearningMethod(learnMethod);            
  cout << endl << "--- Using learning method " << fMethod << ". ---" << endl << endl;

  TCanvas* NNtraining = new TCanvas("NNtraining","Neural network: training process",20,20,800,600);

  networkPSA->Train(fNTrain, "current,text,graph,update=5"); //full output every 5 epochs, in the current canvas and as simple text 
  
  networkPSA->DumpWeights(filenameCreator("weights_","txt"));    //  save weights to a file (can be reloaded with LoadWeights)
  NNtraining->Print(filenameCreator("NNtraining_","eps"),"Neural Network: training process");
} //end of trainNetwork


//-----------------------------------------------------------------------------


void displayResults()
{
  // Draw the network result for the test sample the "standard" way
  TCanvas* NNresult = new TCanvas("NNresult","Neural network: Result",220,220,800,600);     
  networkPSA->DrawResult(0,"test,nocanv");   // Parameters: index of output neuron, dataset, "comp" = draw X-Y comparison plot, "nocanv" = draw in active canvas
  NNresult->Print(filenameCreator("NNresult_","eps"),"Neural Network: Result");
  
   // Draw the resulting network (signal and background) like in the mlp tutorial example from root.cern.ch
  TCanvas* NNmlp = new TCanvas("NNmlp","Neural network: Signal and Background",220,220,800,600);
  Int_t mlpMin= -5; 
  Int_t mlpMax = 5; 
  Int_t mlpBins = 200;
  TH1D *bg = new TH1D("bg","NN output", mlpBins, mlpMin, mlpMax);
  TH1D *sig = new TH1D("sig","NN output", mlpBins, mlpMin, mlpMax);
  Double_t *params = new Double_t[fWFnum];
  Int_t parindex;
  for (size_t i = 0; i < fTrainingTree->GetEntries(); i++) {
    fTrainingTree->GetEntry(i);	
    parindex = 0;
     if (fType==0){
       for (size_t b = 0; b < fWFnum ; b++){
	 params[parindex] = fWFbin[b];	
	 parindex++;
       }
       bg->Fill(networkPSA->Evaluate(0, params));
     }
     else {
       for (size_t b = 0; b < fWFnum ; b++){
	 params[parindex] = fWFbin[b];	
	 parindex++;
       }
       sig->Fill(networkPSA->Evaluate(0,params));
     }
  }  
  // cout << "After the for-loop, sig has " << sig->GetEntries() << " entries and bg has "<< bg->GetEntries() << " ." << endl;

  bg->SetLineColor(kBlue);  bg->SetFillStyle(3008);   bg->SetFillColor(kBlue);
  sig->SetLineColor(kRed);  sig->SetFillStyle(3003); sig->SetFillColor(kRed);
  bg->SetStats(0);    bg->Draw();
  sig->SetStats(0);   sig->Draw("same");
  TLegend *legend = new TLegend(.75, .80, .95, .95);
  legend->AddEntry(bg, "Background");    legend->AddEntry(sig, "Signal");
  legend->Draw();
  NNmlp->Print(filenameCreator("NNmlp_","eps"),"Neural Network: Result (signal and background)");
} // end of displayResults



// Maybe use TMLPAnalyzer to show more about the network ?
//  void analyzeNN()
//  {
//  TCanvas* ana_canvas = new TCanvas("ana_canvas","Network analysis",100,100,800,800);
//  ana_canvas->Divide(1,2);
//  TMLPAnalyzer ana(networkPSA);
   
//    // Initialisation
//  cout << "Next command is ana.GatherInformations() ... " << endl;
//  ana.GatherInformations();   // seems to cause the program to freeze?!
//  cout << "Done. "<<endl;
//  // Gives some information about the network in the terminal.
//  cout << "Next command is ana.CheckNetwork() ... " << endl;
//  ana.CheckNetwork();
//  cout << "Done. "<<endl;
    
//  // Draws the distribution (on the test sample) of the impact on the network output of a small variation of each input
//  ana_canvas->cd(1);
//  ana.DrawDInputs();
      
//  //  Draw the distribution of the neural network (using ith neuron),  
//  //  Two distributions are drawn, for events passing respectively the "signal"  and "background" cuts. Only the test sample is used.
//  ana_canvas->cd(2);
//  ana.DrawNetwork(0,"fType==1","fType==0");   // Parameters of DrawNetwork (are Int_t neuron, const char* signal, const char* bg)
//  } // end of analyzeNN




