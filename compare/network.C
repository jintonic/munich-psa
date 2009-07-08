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

  // set learning method 
  const char* method;
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kStochastic);  method = " kStochstic";              //works sometimes (?), very slow
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kBatch);   method = " kBatch";                      //seems to work
    networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kSteepestDescent);   method = " kDescent";          //seems to work
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kRibierePolak); method = " kRibierePolak";           //seems to work
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kFletcherReeves);  method = " kFletcherReeves";     //seems to work, slower than the 3 above
  //  networkPSA ->  SetLearningMethod(TMultiLayerPerceptron::kBFGS); method = " kBFGS";                          //does NOT work (only sometimes..)
  cout << endl << "--- Using learning method" << method << ". ---" << endl << endl;

  // create a canvas that will collect info about the NN
  TCanvas* NNcanvas = new TCanvas("NNcanvas","Neural network",20,20,1200,900);
  NNcanvas->Divide(2,2);
  // shows the network structure
  NNcanvas->cd(1); 
  networkPSA->Draw();
  // train the NN, plot the training progress
  NNcanvas->cd(2);
  networkPSA->Train(ntrain, "current,text,graph,update=5"); //full output every 5 epochs, in the current canvas and as simple text 
  networkPSA->DumpWeights("weights.txt");    //  weights can be saved to a file (DumpWeights) and then reloaded (LoadWeights) to a new compatible network




// ----- Display the results
  
  // Draw the network result for the test sample the "standard" way
  // will produce an histogram with the output for the two datasets. Arguments are index of the output neuron (currently there is only one (type)) 
  // and a string telling which dataset to use (train or test) and whether a X-Y comparison plot should be drawn (comp).
  // To draw the histogramm in the active canvas instead of creating a new one, use nocanv.
  NNcanvas->cd(3);
  networkPSA->DrawResult(0,"test,nocanv");   
  
   // draws the resulting network (signal and background) like in the mlp tutorial example from root.cern.ch
  NNcanvas->cd(4);
  TH1D *bg = new TH1D("bg","NN output", 200, -10.0, 10.0);
  TH1D *sig = new TH1D("sig","NN output", 200, -10.0, 10.0);
  Double_t params[300];
  for (i = 0; i < trainingTree->GetEntries(); i++) {
    trainingTree->GetEntry(i);
    if (type==0){
      for (b = 0; b < 300 ; b++){
   	params[b] = wfBin[b];	
	// if(b%100==50 && ientries%500==0) cout << "entry " << i << ": params[" << b<< "] = " << params[b] << endl;
      }
      bg->Fill(networkPSA->Evaluate(0, params));
      // if (i%500==0) cout << "At tree entry "<< i <<". bg has now " << bg->GetEntries() << " entries. " <<  endl;
    }
    else {
      for (b = 0; b < 300 ; b++){
   	params[b] = wfBin[b];
	// if(b%100==50 && ientries%500==0) cout << "entry " << i << ": params[" << b<< "] = " << params[b] << endl;
      }
         sig->Fill(networkPSA->Evaluate(0,params));
	 // if (i%500==0) cout << "At tree entry "<< i <<". sig has now " << sig->GetEntries() << " entries. " <<  endl;
    }
  }  
  cout << "After the for loop, sig has " << sig->GetEntries() << " entries and bg has "<< bg->GetEntries() << " ." << endl;

  bg->SetLineColor(kBlue);  bg->SetFillStyle(3008);   bg->SetFillColor(kBlue);
  sig->SetLineColor(kRed);  sig->SetFillStyle(3003); sig->SetFillColor(kRed);
  bg->SetStats(0);    bg->Draw();
  sig->SetStats(0);   sig->Draw("same");
  TLegend *legend = new TLegend(.75, .80, .95, .95);
  legend->AddEntry(bg, "Background");    legend->AddEntry(sig, "Signal");
  legend->Draw();

  NNcanvas->Print("NNcanvas.eps","Neural Network: structure, training and results");



// Maybe use TMLPAnalyzer to show more about the network ?

//    TCanvas* ana_canvas = new TCanvas("ana_canvas","Network analysis",100,100,800,800);
//    ana_canvas->Divide(1,2);
//    TMLPAnalyzer ana(networkPSA);

//    // Initialisation
//    ana.GatherInformations();

//    // Gives some information about the network in the terminal.
//    ana.CheckNetwork();

//    // Draws the distribution (on the test sample) of the impact on the network output of a small variation of each input
//    ana_canvas->cd(1);
//    ana.DrawDInputs();

//    //  Draw the distribution of the neural network (using ith neuron),  Parameters are (Int_t neuron, const char* signal, const char* bg)
//    //  Two distributions are drawn, for events passing respectively the "signal"  and "background" cuts. Only the test sample is used.
//    ana_canvas->cd(2);
//    ana.DrawNetwork(0,"type==1","type==0");   // (Int_t neuron, const char* signal, const char* bg)


   cout << endl;
} // end of "void network"

