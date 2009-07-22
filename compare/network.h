// -------------------------------------------------------------
//
//                          Variables
//
//  -------------------------------------------------------------

const char* fMethod;
Int_t fNTrain;

TTree* fTrainingTree;
Int_t fType;
Float_t fWeight; 
size_t fWFnum;
Double_t* fWFbin;
TString* fBranchNames;

TMultiLayerPerceptron *networkPSA;





// -------------------------------------------------------------
//
//                Neural Network Functions
//
//  -------------------------------------------------------------


void setupTrainingTree();

void buildNetwork();

void trainNetwork();

void displayResults();

void analyzeNN();



// -------------------------------------------------------------
//
//                  Small Helper Functions
//
// -------------------------------------------------------------

const char* GetMethod()
{
  return fMethod;
}



Int_t GetNTrain()
{
  return fNTrain;
}



// to create filenames containing learning method and number of iterations
const char* filenameCreator(const char* someString, const char* extension)
{
  TString *filename = new TString(someString);
  filename->Append(GetMethod());
  filename->Append((Form("_%d.",GetNTrain())));
  filename->Append(extension);  
  return filename->Data();
} 
