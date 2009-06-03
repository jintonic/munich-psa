#include <cmath>

#include "io/MGLogger.hh"
#include "io/MGCrystalHits.hh"

#include "waveform/MGWaveformManager.hh"
#include "waveform/MGWFGenFromDrift.hh"
#include "waveform/MGSORCrystalFields.hh"

#include "MGCrystalData.hh"
#include "MGMultiWaveformTransformer.hh"
#include "MGWFAddNoise.hh"
#include "MGWFRCDifferentiation.hh"
#include "MGWFRCIntegration.hh"

#include "TMath.h"
#include "TChain.h"

using namespace std;
using namespace CLHEP;

int main(int argc, char *argv[]) 
{
   cout<<"TChain will be allocated"<<endl;
   TChain* chain = new TChain("fTree");
   for (int i = 1; i<argc; i++) {
      chain->Add(argv[i]);
      cout<<argv[i]<<" added into chain."<<endl;
   }

   const int maxNhits = 1000;
   int   Nhits;  
   int   hitSegmentID[maxNhits];
   float hitX[maxNhits],hitY[maxNhits],hitZ[maxNhits],hitT[maxNhits],hitE[maxNhits];
   float segEnergy[4][20];

   chain->SetBranchStatus("*",1);
   chain->SetBranchAddress("Nhits",&Nhits);
   chain->SetBranchAddress("hitX",hitX);
   chain->SetBranchAddress("hitY",hitY);
   chain->SetBranchAddress("hitZ",hitZ);
   chain->SetBranchAddress("hitT",hitT);
   chain->SetBranchAddress("hitE",hitE);
   chain->SetBranchAddress("hitSegmentID",hitSegmentID);
   chain->SetBranchAddress("segEnergy",segEnergy);
   cout<<"TChain properly setup - DONE"<<endl;
 
   MGLogger::SetSeverity(MGLogger::routine);
   //MGLogger::SetSeverity(MGLogger::debugging);

   cout<<"\n----- Here comes the Waveform part -----\n"<<endl;
   
   cout<<"Setup Waveform Manager"<<endl;
   MGWaveformManager wfM;
   wfM.SetCoincidenceTime(1*ns);
   wfM.SetCoincidenceLength(1*mm);
   cout<<"Setup Waveform Manager - DONE\n"<<endl;

   cout<<"Setup Crystal Data"<<endl;
   int id = 0;
   double ri = 5.0*mm, ro = 37.5*mm, hi = 70.0*mm;
   MGCrystalData* crystal = new MGCrystalData(
         id, 		// crystal ID
         MGCrystalData::kNType,
         MGCrystalData::kCoaxial,
         ri,  		// inner radius
         ro, 		// outer radius
         hi, 		// height
         hi, 		// well depth
         0.0, 		// global x
         0.0, 		// global y
         0.0, 		// global z
         3,   		// Nseg in z
         6);  		// Nseg in phi
   crystal->SetCrystalAxesAngle(0.0*CLHEP::degree);
   crystal->SetSegmentationAngle(0.0*CLHEP::degree);
   cout<<"Setup Crystal Data - DONE\n"<<endl;

   cout<<"Setup Crystal Fields"<<endl;
   MGSORCrystalFields* fields = new MGSORCrystalFields(crystal);
   fields->SetFileName("/home/pclg-09/vauth/work/PSA/sim/3000VFieldSI"); //created with TestSOREField (with "SetGrid(60,180,70)")
   fields->LoadFields();
   cout<<"Setup Crystal Fields - DONE\n"<<endl;

   cout<<"Setup Waveform Generator"<<endl;
   MGWFGenFromDrift* wfGen 
      = new MGWFGenFromDrift(crystal,fields,1000,
            1*CLHEP::GHz,100*ns,2.95*eV, MGWaveform::kCharge);
   wfGen->SetPrecision(2);
   cout<<"Setup Waveform Generator - DONE\n"<<endl;
   
   cout<<"Adding Waveform Generator to Manager"<<endl;
   wfM.AddWaveformGenerator(wfGen);
   cout<<"Adding Waveform Generator to Manager - DONE\n"<<endl;

   cout<<"Adding Transformations to Manager"<<endl;
   MGWFAddNoise addNoise;
   addNoise.SetNoiseAmplitude(600); // 5 keV / 2.95 eV (pair production energy)
   MGWFRCDifferentiation decay;
   decay.SetTimeConstant(5*CLHEP::us);
   MGWFRCIntegration bandwidth;
   bandwidth.SetTimeConstant(30*ns); // bandwidth ~ 10 MHz
   MGMultiWaveformTransformer trans;
   trans.AddWFTransformer(&decay);
   trans.AddWFTransformer(&bandwidth);
   trans.AddWFTransformer(&addNoise);
   wfM.SetElectronicsModel(&trans);
   cout<<"Adding Transformations to Manager - DONE\n"<<endl;

   cout<<"Setup Crystal Hits"<<endl;
   MGCrystalHits CHits;
   cout<<"Setup Crystal Hits - DONE\n"<<endl;

   cout<<"Generate WF in loop:"<<endl;
   int Nchain = chain->GetEntries();
   cout<<"\tNo. of expected loops = "<<Nchain<<endl<<endl;
   
   for (int n=0; n<Nchain; n++) {
   //for (int n=0; n<3; n++) {
      if (n%100==0) cout<<"Processing event "<<n<<endl; 
      chain->GetEntry(n);
      
      //cout<<"\tNhits = "<<Nhits<<endl;
      CHits.Reset();
      for (int i=0; i<Nhits; i++) {
	//if (hitSegmentID[i]==8)
            CHits.AddCrystalHit(
                  hitX[i]*CLHEP::cm,
                  hitY[i]*CLHEP::cm,
                  hitZ[i]*CLHEP::cm,
                  hitT[i]*CLHEP::ns,
                  hitE[i]*CLHEP::keV,
                  id);
      }
      //CHits.Print();
      wfM.GenerateWaveforms(&CHits);    
   }
   cout<<"Generate WF in loop - DONE"<<endl;

   wfM.EndOfRunAction();

   return 0;
}

