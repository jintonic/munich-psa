// Build and train the NN for the PSA
// Use like .x network("DataSample.root",100)        (for 100 training iterations)

#include "MyPSNNA.cxx"


void network(const char* filename, Int_t ntrain)
{

  MyPSNNA* fPSNNA = new MyPSNNA();
  fPSNNA->SetNTrain(ntrain);

  // ----- Read in the input file 
  fPSNNA->readFile(filename);


  // set branch adresses and add a weight branch
  fPSNNA->setupTrainingTree();   
  

  // define the network layouy and build the NN
  fPSNNA->buildNetwork();


  // --- set learning method 
  //  fPSNNA->SetMethod("kStochstic");  //doesn't work ("Stop"=learning would lead to non real numbers..?)  
  fPSNNA->SetMethod("kBatch");         
  //  fPSNNA->SetMethod("kDescent");       
  //  fPSNNA->SetMethod("kRibierePolak");  
  //  fPSNNA->SetMethod("kFletcherReeves"); 
  //  fPSNNA->SetMethod("kBFGS");                                                                             
  const char* method = fPSNNA->GetMethod();


  // --- train the NN, plot the training progress
  fPSNNA->trainNetwork(); 


 // --- Draw the network result in different ways
  fPSNNA->displayResults();   

  // Maybe use TMLPAnalyzer to show more about the network ?
  //fPSNNA->analyzeNN();

} // end of "void network"

