#include <iostream>
#include <map>

#include <TGFrame.h>
#include <TApplication.h>
#include <TGDockableFrame.h>
#include <TGTab.h>
#include <TGStatusBar.h>
#include <TGMenu.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TGListBox.h>
#include <TChain.h>
#include <TClonesArray.h>
#include <TH1D.h>

class PSbMainFrame : public TGMainFrame 
{
public:
   PSbMainFrame(const TGWindow *p,UInt_t width=800,UInt_t height=800);
   virtual ~PSbMainFrame();
   
   virtual void CloseWindow(){gApplication->Terminate(0);}
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t);

   // action
   void OpenFile(char *input);
   void ShowEvent(Int_t event);
   void FillPSHist(Int_t event);
   void Go2FirstEvent(){ShowEvent(0);}
   void Go2LastEvent(){ShowEvent(999999);}
   void Go2PreviousEvent(){ShowEvent(fCurrentEvent-1);}
   void Go2NextEvent(){ShowEvent(fCurrentEvent+1);}

private:
   // top components
   TGDockableFrame     *fMenuDock;
   TGDockableFrame     *fToolBarDock;
   TGTab               *fCanvasTab;
   TGStatusBar         *fStatusBar;

   // menu
   TGMenuBar           *fMenuBar;
   TGPopupMenu         *fMenuFile, *fMenuHelp;

   // tool bar
   TGHorizontalFrame   *fToolBar;
   TGTextButton        *fButtonFirst, *fButtonLast, *fButtonPrevious, *fButtonNext;
   TGNumberEntry       *fEnteredEvent;
   TGLabel             *fLabelTotal;
   TGListBox           *fChannelList;

   // tabbed canvas
   TRootEmbeddedCanvas *fOverviewCanvas, *fAnalysisCanvas;
   TGVerticalFrame     *fAnalysisPanel;

   // data
   TChain *fPSTree;

   char  *fInputFile;
   TClonesArray* fPSArray;
   std::map<Int_t, std::map<size_t, TH1D*> > fPSHists;
 
   Int_t fCurrentEvent;
   Int_t fTotalEvent;

public:
   ClassDef(PSbMainFrame,0)
};

