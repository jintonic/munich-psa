class MyPSNNA
{

 public:

  // constructor
  
  MyPSNNA();

  // destructor
  
  ~MyPSNNA();

  // methods

  void readFile(const char* filename)
  {
    TFile *trainingFile = TFile::Open(filename);
    fTrainingTree = (TTree *) trainingFile->Get("DataSample");
  }

  void setupTrainingTree();

  void buildNetwork();
  
  void trainNetwork();
  
  void displayResults();
  
  void analyzeNN();

  void SetMethod(const char* method)
  {
    fMethod = method;
  }
  
  void SetNTrain(Int_t ntrain)
  {
    fNTrain = ntrain;
  }

  const char* GetMethod()
    {
      return fMethod;
    }
  
  Int_t GetNTrain()
  {
    return fNTrain;
  }
    
  // to create filenames containing learning method and number of iterations
  const char* filenameCreator(const char* someString, const char* extension);
  

  // private:

  const char* fMethod;
  Int_t fNTrain;
  
  TTree* fTrainingTree;
  Int_t fType;
  Float_t fWeight; 
  size_t fWFnum;
  Double_t* fWFbin;
  TString* fBranchNames;
  
  TMultiLayerPerceptron* networkPSA;

};


// ---------------------------------------------------------------------- 

