#include "psb.hh"

#include <vector>

#include <TGClient.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TGFileDialog.h>

#include "MGTWaveform.hh"
#include "MGCrystalData.hh"

using namespace std;

enum CommandIdentifiers 
{
   M_FILE_OPEN,
   M_FILE_SAVE,
   M_FILE_SAVEAS,
   M_FILE_PRINT,
   M_FILE_EXIT,

   M_HELP_CONTENTS,
   M_HELP_SEARCH,
   M_HELP_ABOUT,
};

enum NumberEntryIdentifiers 
{
   N_ENTERED_EVENT,
};


PSbMainFrame::PSbMainFrame(const TGWindow *p,UInt_t width,UInt_t height)
:TGMainFrame(p,width,height) 
{
   //top components
   fMenuDock = new TGDockableFrame(this);
   fMenuDock->SetWindowName("Main Menu");

   fToolBarDock = new TGDockableFrame(this);
   fToolBarDock->SetWindowName("Main Tool Bar");

   fCanvasTab = new TGTab(this);//,800,450);

   fStatusBar = new TGStatusBar(this,800,50);
   Int_t parts[] = {50,10,10,10,20};
   fStatusBar->SetParts(parts,5);

   //menu
   fMenuFile = new TGPopupMenu(fClient->GetRoot());
   fMenuFile->AddEntry("&Open...", M_FILE_OPEN);
   fMenuFile->AddEntry("&Save", M_FILE_SAVE);
   fMenuFile->AddEntry("S&ave as...", M_FILE_SAVEAS);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("&Print", M_FILE_PRINT);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("E&xit", M_FILE_EXIT);
   fMenuFile->DisableEntry(M_FILE_SAVE);
   fMenuFile->DisableEntry(M_FILE_SAVEAS);
   fMenuFile->DisableEntry(M_FILE_PRINT);
   //fMenuFile->HideEntry(M_FILE_PRINT);
   fMenuFile->Associate(this);

   fMenuHelp = new TGPopupMenu(fClient->GetRoot());
   fMenuHelp->AddEntry("&Contents", M_HELP_CONTENTS);
   fMenuHelp->AddEntry("&Search...", M_HELP_SEARCH);
   fMenuHelp->AddSeparator();
   fMenuHelp->AddEntry("&About", M_HELP_ABOUT);
   fMenuHelp->Associate(this);

   fMenuBar = new TGMenuBar(fMenuDock,800,50);
   fMenuBar->SetMinWidth(800);
   fMenuBar->AddPopup("F&ile", fMenuFile, 
	 new TGLayoutHints(kLHintsTop|kLHintsLeft));
   fMenuBar->AddPopup("&Help", fMenuHelp, 
	 new TGLayoutHints(kLHintsTop|kLHintsRight));
   fMenuDock->AddFrame(fMenuBar, 
	 new TGLayoutHints(kLHintsLeft|kLHintsExpandX));

   // tool bar
   fToolBar=new TGHorizontalFrame(fToolBarDock,800,50);
   fToolBar->SetMinWidth(800);
   
   fButtonFirst = new TGTextButton(fToolBar,"&First");
   fButtonFirst->Connect("Clicked()","PSbMainFrame",this,"Go2FirstEvent()");
   fToolBar->AddFrame(fButtonFirst, new TGLayoutHints(kLHintsExpandX,0,5,0,0));

   fButtonPrevious = new TGTextButton(fToolBar,"&Previous");
   fButtonPrevious->Connect("Clicked()","PSbMainFrame",this,"Go2PreviousEvent()");
   fToolBar->AddFrame(fButtonPrevious, new TGLayoutHints(kLHintsExpandX,0,5,0,0));
   
   fEnteredEvent = new TGNumberEntry(fToolBar, 0, 9, N_ENTERED_EVENT,
	 TGNumberFormat::kNESInteger,
	 TGNumberFormat::kNEANonNegative,
	 TGNumberFormat::kNELLimitMinMax,0,1);
   fToolBar->AddFrame(fEnteredEvent, new TGLayoutHints(kLHintsExpandX));

   fLabelTotal = new TGLabel(fToolBar," ");
   fLabelTotal->ChangeOptions(kSunkenFrame);
   fLabelTotal->SetTextJustify(kTextLeft);
   fToolBar->AddFrame(fLabelTotal, 
	 new TGLayoutHints(kLHintsExpandX|kLHintsExpandY));

   fButtonNext= new TGTextButton(fToolBar,"&Next");
   fButtonNext->Connect("Clicked()","PSbMainFrame",this,"Go2NextEvent()");
   fToolBar->AddFrame(fButtonNext, new TGLayoutHints(kLHintsExpandX,5,0,0,0));

   fButtonLast = new TGTextButton(fToolBar,"&Last");
   fButtonLast->Connect("Clicked()","PSbMainFrame",this,"Go2LastEvent()");
   fToolBar->AddFrame(fButtonLast, new TGLayoutHints(kLHintsExpandX,5,0,0,0));

   fToolBarDock->AddFrame(fToolBar,
	 new TGLayoutHints(kLHintsLeft|kLHintsExpandX));

   // tabbed canvas
   TGCompositeFrame *taf = fCanvasTab->AddTab("Overview");
   fOverviewCanvas = new TRootEmbeddedCanvas("Overview Canvas",taf,790,600);
   taf->AddFrame(fOverviewCanvas, 
	 new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));

   taf = fCanvasTab->AddTab("Analysis");
   taf->ChangeOptions(kHorizontalFrame);

   fAnalysisCanvas = new TRootEmbeddedCanvas("Analysis Canvas",taf,450,600);
   taf->AddFrame(fAnalysisCanvas, 
	 new TGLayoutHints(kLHintsLeft|kLHintsExpandX|kLHintsExpandY,1,1,1,1));

   fAnalysisPanel = new TGVerticalFrame(taf);
   //fAnalysisPanel->SetWidth(340);
   //fAnalysisPanel->ChangeOptions(kFixedWidth);
   taf->AddFrame(fAnalysisPanel, 
	 new TGLayoutHints(kLHintsRight|kLHintsExpandY));

    fChannelList = new TGListBox(fAnalysisPanel);
    fChannelList->Resize(200,150);
    fChannelList->SetMultipleSelections(kTRUE);
    fAnalysisPanel->AddFrame(fChannelList,
	  new TGLayoutHints(kLHintsTop|kLHintsExpandX|kLHintsExpandY,5,5,5,5));

   // draw windows
   SetWindowName("Waveform Browser");
   SetWMSizeHints(640, 480, 1600, 1200, 10, 10);

   AddFrame(fMenuDock, 
	 new TGLayoutHints(kLHintsExpandX));
   AddFrame(fToolBarDock, 
	 new TGLayoutHints(kLHintsExpandX));
   AddFrame(fCanvasTab, 
	 new TGLayoutHints(kLHintsExpandX|kLHintsExpandY));
   AddFrame(fStatusBar,
	 new TGLayoutHints(kLHintsBottom|kLHintsLeft|kLHintsExpandX));

   MapSubwindows();

   Resize();
   MapWindow();

   // data
   fPSTree = new TChain("wfTree", "wfTree");
   fPSArray = new TClonesArray("MGTWaveform");
}
PSbMainFrame::~PSbMainFrame()
{
   // window
   Cleanup();
   // data
   delete fPSTree;
   delete fPSArray;
}

const char *filetypes[] = { 
   "ROOT files",    "*.root",
   "All files",     "*",                         
   0,               0 };

Bool_t PSbMainFrame::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   // Handle messages send to the PSbMainFrame. 
   // E.g. all menu button messages.
   switch (GET_MSG(msg)) {

      case kC_COMMAND:
	 switch (GET_SUBMSG(msg)) {

	    case kCM_MENUSELECT:
	       //printf("Pointer over menu entry, id=%ld\n", parm1);
	       break;

	    case kCM_MENU:
	       switch (parm1) {

		  case M_FILE_OPEN:
		     {
			static TString dir(".");
                        TGFileInfo fi;
                        fi.fFileTypes = filetypes;
                        fi.fIniDir    = StrDup(dir);
                        new TGFileDialog(fClient->GetRoot(), 
			      this, 
			      kFDOpen, 
			      &fi);
                        dir = fi.fIniDir;
			fInputFile=fi.fFilename;

			OpenFile(fInputFile);
                     }
                     break;

                  case M_FILE_SAVE:
                     printf("M_FILE_SAVE\n");
                     break;

                  case M_FILE_PRINT:
                     printf("M_FILE_PRINT\n");
                     fMenuFile->HideEntry(M_FILE_PRINT);
                     break;

                  case M_FILE_EXIT:
                     CloseWindow();   // this also terminates theApp
                     break;

                  default:
                     break;
               }
	    case kCM_LISTBOX:
	       for (size_t i=0; i<fPSHists[0].size(); i++) {
		  if (parm1==(Long_t)i) printf("contact %d selected",i);
	       }
	    default:
	       break;
	 }
      default:
	 break;
   }

   return kTRUE;
}

void PSbMainFrame::OpenFile(char *input)
{
   fPSTree->AddFile(input);
   string wfBranchName = fPSTree->GetListOfBranches()->At(0)->GetName();
   fPSTree->SetBranchAddress(wfBranchName.c_str(), &fPSArray);
   fTotalEvent = fPSTree->GetEntries();
   fCurrentEvent = 0;

   TString wn="Waveform Browser: ";
   SetWindowName(wn+input);

   fLabelTotal->SetText(Form(" of %d events",fTotalEvent));

   ShowEvent(fCurrentEvent);
}

void PSbMainFrame::ShowEvent(Int_t event)
{
   if (event<0) fCurrentEvent=0;
   else if (event>=fTotalEvent) fCurrentEvent=fTotalEvent-1;
   else fCurrentEvent=event;

   fEnteredEvent->SetNumber(fCurrentEvent+1);

   FillPSHist(fCurrentEvent);

   TCanvas *ocan = fOverviewCanvas->GetCanvas();
   TCanvas *acan = fAnalysisCanvas->GetCanvas();

   ocan->Clear();
   ocan->Divide(6,4,0.0001,0.0001);
   ocan->cd(1);
   fPSHists[0][0]->Draw();
   for (size_t i=1; i<fPSHists[0].size(); i++) {
      ocan->cd(i+6);
      fPSHists[0][i]->Draw();
   }
   ocan->Update();

   for (size_t i=0; i<fPSHists[0].size(); i++) {
      fChannelList->AddEntry(Form("Contact %d",i),i);
   }
   fChannelList->MapSubwindows();
   fChannelList->Layout();
   //   fChannelList->AddEntry(Form("Contact %d",0),0);
   //   fChannelList->AddEntry(Form("Contact %d",1),1);

   acan->Clear();
   acan->cd();
   fPSHists[0][0]->Draw();
   acan->Update();
   //fChannelList->Select(0, 1);
}

void PSbMainFrame::FillPSHist(Int_t event)
{  
   fPSTree->GetEntry(event);

   Int_t Nwfs = fPSArray->GetEntries();
   if(Nwfs == 0) {
      fStatusBar->SetText("Error: no waveform found!",0);
   }

   // Pull out the histograms for each waveform in each crystal. While
   // we're at it, calculate the number of crystals
   vector<Int_t> crystalIDs;
   for(Int_t iWaveform=0; iWaveform<Nwfs; iWaveform++) {
      MGTWaveform* waveform = (MGTWaveform*) fPSArray->At(iWaveform);
      //(*waveform)*=-1;
      if(waveform == NULL) {
	 fStatusBar->SetText("Error: Got NULL waveform",0);
	 break;
      }

      Int_t crystalID = MGCrystalData::GetCrystalID(waveform->GetID());
      size_t iID = 0;
      while(iID<crystalIDs.size() && crystalIDs[iID]!=crystalID) iID++;
      if(iID == crystalIDs.size()) {
	 crystalIDs.push_back(crystalID);
      }

      size_t iContact = MGCrystalData::GetIContact(waveform->GetID());

      size_t segIDdat[19]={0,1,2,3,7,8,9,13,14,15,16,17,18,10,11,12,4,5,6};
      size_t segIDsim[19]={0,1,7,13,2,8,14,3,9,15,4,10,16,5,11,17,6,12,18};

      fPSHists[crystalID][segIDdat[iContact]] = waveform->GimmeHist();
      //fPSHists[crystalID][segIDsim[iContact]] = waveform->GimmeHist();
   }
}

int main(int argc, char **argv) {
   TApplication theApp("App",&argc,argv);
   PSbMainFrame theMainFrame(gClient->GetRoot());
   theApp.Run();
   return 0;
}

