#include <TH2.h>
#include <TH2F.h>
#include <TChain.h>
#include <TCanvas.h>
#include <vector>
#include <iostream>
#include <TSystem.h>
#include "fadc_data.h"
#include "gmn_tree.C"

const Int_t kNrows = 27;
const Int_t kNcols = 7;
const Int_t kNrowsPS = 26;
const Int_t kNcolsPS = 2;

const Int_t kCanvSize = 100;
std::string user_input;
Int_t gCurrentEntry = -1;

gmn_tree *T;
Int_t foundModules = 0;
TCanvas *canvas = 0;
TCanvas *subCanv[4];


void clicked_displayEntryButton();
void clicked_displayNextButton();
namespace shgui {
  TGMainFrame *main = 0;
  TGHorizontalFrame *frame1 = 0;
  TGTab *fTab;
  TGLayoutHints *fL3;
  TGCompositeFrame *tf;
  TGTextButton *exitButton;
  TGTextButton *displayEntryButton;
  TGTextButton *displayNextButton;
  TGNumberEntry *entryInput;
  TGLabel *ledLabel;

  TRootEmbeddedCanvas *canv[1];

  TGCompositeFrame* AddTabSub(Int_t sub) {
    tf = fTab->AddTab(Form("SH Sub%d",sub+1));

    TGCompositeFrame *fF5 = new TGCompositeFrame(tf, (12+1)*kCanvSize,(6+1)*kCanvSize , kHorizontalFrame);
    TGLayoutHints *fL4 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
					   kLHintsExpandY, 5, 5, 5, 5);
    TRootEmbeddedCanvas *fEc1 = new TRootEmbeddedCanvas(Form("shSubCanv%d",sub), fF5, 6*kCanvSize,8*kCanvSize);
    //TRootEmbeddedCanvas *fEc1 = new TRootEmbeddedCanvas(0, fF5, 600, 600);
    //Int_t wid = fEc1->GetCanvasWindowId();
    //subCanv[sub] = new TCanvas(Form("subCanv%d",sub),10,10,wid);
    //subCanv[sub]->Divide(12,6);
    //fEc1->AdoptCanvas(subCanv[sub]);
    canv[sub] = fEc1;
    fF5->AddFrame(fEc1,fL4);
    tf->AddFrame(fF5,fL4);
    return tf;
  }

  void SetupGUI() {
    if(!main) {
      main = new TGMainFrame(gClient->GetRoot(), 1000, 900);
      frame1 = new TGHorizontalFrame(main, 150, 20, kFixedWidth);
      ledLabel = new TGLabel(frame1,"Run #: ");
      displayEntryButton = new TGTextButton(frame1,"&Display Entry","clicked_displayEntryButton()");
      entryInput = new TGNumberEntry(frame1,0,5,-1,TGNumberFormat::kNESInteger);
      displayNextButton = new TGTextButton(frame1,"&Next Entry","clicked_displayNextButton()");
      exitButton = new TGTextButton(frame1, "&Exit", 
				    "gApplication->Terminate(0)");
      TGLayoutHints *frame1LH = new TGLayoutHints(kLHintsTop|kLHintsLeft|
						  kLHintsExpandX,2,2,2,2);
      frame1->AddFrame(ledLabel,frame1LH);
      frame1->AddFrame(displayEntryButton,frame1LH);
      frame1->AddFrame(entryInput,frame1LH);
      frame1->AddFrame(displayNextButton,frame1LH);
      frame1->AddFrame(exitButton,frame1LH);
      frame1->Resize(800, displayNextButton->GetDefaultHeight());
      main->AddFrame(frame1, new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1));

      // Create the tab widget
      fTab = new TGTab(main, 300, 300);
      fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

      // Create Tab1 (SH Sub1)
      for(Int_t i = 0; i < 4; i++) {
        tf = AddTabSub(i);
      }
      main->AddFrame(fTab, new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
					     kLHintsExpandY, 2, 2, 5, 1));
      main->MapSubwindows();
      main->Resize();   // resize to default size
      main->MapWindow();

      for(Int_t i = 0; i < 4; i++) {
        subCanv[i] = canv[i]->GetCanvas();
      	// if( kNrows<12 || kNcols<12) {
      	//   //subCanv[i]->Divide(kNrows,kNcols,0.001,0.001);
      	//   subCanv[i]->Divide(kNcols,kNrows,0.001,0.001);
        // } else {
      	//   subCanv[i]->Divide(12,6,0.001,0.001);
        // }
      	subCanv[i]->Divide(2,1,0.001,0.001);
      }
    }
  }
};

Double_t nhit = 0;
TH1F *histos[kNrows][kNcols];
TH2F* hSH_int = new TH2F("sh_int","Shower(a_p) ; Ncol ; Nrow",kNcols,1,kNcols+1,kNrows,1,kNrows+1);
TH2F* hSH_intEng = new TH2F("sh_intE","Shower(a_c) ; Ncol ; Nrow",kNcols,1,kNcols+1,kNrows,1,kNrows+1);
TH2F* hSH_clus_e = new TH2F("sh_clus_e","Shower(Cluster) ; Ncol ; Nrow",kNcols,1,kNcols+1,kNrows,1,kNrows+1);
TH2F* hPS_int = new TH2F("ps_int","PreShower(a_p) ; Ncol ; Nrow",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);
TH2F* hPS_intEng = new TH2F("ps_intE","PreShower(a_c) ; Ncol ; Nrow",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);
TH2F* hPS_clus_e = new TH2F("ps_clus_e","PreShower(Cluster) ; Ncol ; Nrow",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);

bool is_number(const std::string& mystring)
{
  std::string::const_iterator it = mystring.begin();
  while (it != mystring.end() && std::isdigit(*it)) ++it;
  return !mystring.empty() && it == mystring.end();
}

void displayEvent(Int_t entry = -1, Int_t run = 7 )
{
  if(entry == -1) {
    gCurrentEntry++;
  } else {
    gCurrentEntry = entry;
  }

  if(gCurrentEntry<0) {
    gCurrentEntry = 0;
  }

  T->GetEntry(gCurrentEntry);
  std::cout << "Displaying event " << gCurrentEntry << std::endl;
  shgui::ledLabel->SetText(TString::Format("Run #: %d",run));

  Int_t r,c;

  // Clear old histograms, just in case modules are not in the tree
  hSH_int->Reset("ICES M");
  hSH_intEng->Reset("ICES M");
  hPS_int->Reset("ICES M");
  hPS_intEng->Reset("ICES M");
  //Clustering
  hSH_clus_e->Reset("ICES M");
  hPS_clus_e->Reset("ICES M");

  // Shower
  for(Int_t m = 0; m < T->Ndata_bb_sh_adcrow; m++) {
    r = T->bb_sh_adcrow[m];
    c = T->bb_sh_adccol[m];
    if(r < 0 || c < 0) {
      std::cerr << "Why is row negative? Or col?" << std::endl;
      continue;
    }
    if(r>= kNrows || c >= kNcols)
      continue;

    if( T->bb_sh_a_time[m]>0 ){
     hSH_int->Fill( double(c+1),double(r+1), T->bb_sh_a_p[m] );
     hSH_intEng->Fill( double(c+1),double(r+1), T->bb_sh_a_c[m] );     
    } 

    // SH Clustering
    hSH_clus_e->Fill( double(c+1),double(r+1), 0. );
    int cid = T->bb_sh_clus_id[0];

    for( int b = 0; b < T->bb_sh_clus_nblk[0]; b++ ){
      int cblkid = T->bb_sh_clus_blk_id[b];
      if( m == cblkid ) 
	hSH_clus_e->Fill( double(c+1),double(r+1), 0.5 );
    
      if( m == cid )
	hSH_clus_e->Fill( double(c+1),double(r+1), 1.0 );
    }
  }

  // // Pre-Shower
  for(Int_t m = 0; m < T->Ndata_bb_ps_adcrow; m++) {
    r = T->bb_ps_adcrow[m];
    c = T->bb_ps_adccol[m];
    if(r < 0 || c < 0) {
      std::cerr << "Why is row negative? Or col?" << std::endl;
      continue;
    }
    if(r>= kNrows || c >= kNcols)
      continue;

    if( T->bb_ps_a_time[m]>0 ){
     hPS_int->Fill( double(c),double(r+1), T->bb_ps_a_p[m] ); 
     hPS_intEng->Fill( double(c),double(r+1), T->bb_ps_a_c[m] );     
    }

    // PS Clustering
    hPS_clus_e->Fill( double(c),double(r+1), 0. );
    int cid = T->bb_ps_clus_id[0];

    for( int b = 0; b < T->bb_ps_clus_nblk[0]; b++ ){
      int cblkid = T->bb_ps_clus_blk_id[b];
      if( m == cblkid ) 
	hPS_clus_e->Fill( double(c),double(r+1), 0.5 );
    
      if( m == cid )
	hPS_clus_e->Fill( double(c),double(r+1), 1.0 );
    }

  }
  
  subCanv[0]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_intEng->SetStats(0);
  hSH_intEng->SetMaximum(0.5);
  hSH_intEng->SetMinimum(0); 
  hSH_intEng->GetYaxis()->SetNdivisions(kNrows);
  hSH_intEng->GetXaxis()->SetNdivisions(kNcols);
  hSH_intEng->Draw("text colz");
  gPad->Update();
  subCanv[0]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_intEng->SetStats(0);
  hPS_intEng->SetMaximum(0.5);
  hPS_intEng->SetMinimum(0); 
  hPS_intEng->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_intEng->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_intEng->Draw("text colz");
  gPad->Update();

  subCanv[1]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_clus_e->SetStats(0);
  hSH_clus_e->SetMaximum(1.5);
  hSH_clus_e->SetMinimum(0); 
  hSH_clus_e->GetYaxis()->SetNdivisions(kNrows);
  hSH_clus_e->GetXaxis()->SetNdivisions(kNcols);
  hSH_clus_e->Draw("colz");
  gPad->Update();
  subCanv[1]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_clus_e->SetStats(0);
  hPS_clus_e->SetMaximum(1.5);
  hPS_clus_e->SetMinimum(0); 
  hPS_clus_e->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_clus_e->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_clus_e->Draw("colz");
  gPad->Update();

  subCanv[2]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_intEng->SetStats(0);
  hSH_intEng->SetMaximum(0.5);
  hSH_intEng->SetMinimum(0); 
  hSH_intEng->GetYaxis()->SetNdivisions(kNrows);
  hSH_intEng->GetXaxis()->SetNdivisions(kNcols);
  hSH_intEng->Draw("text colz");
  gPad->Update();
  subCanv[2]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_clus_e->SetStats(0);
  hSH_clus_e->SetMaximum(1.5);
  hSH_clus_e->SetMinimum(0); 
  hSH_clus_e->GetYaxis()->SetNdivisions(kNrows);
  hSH_clus_e->GetXaxis()->SetNdivisions(kNcols);
  hSH_clus_e->Draw("colz");
  gPad->Update();

  subCanv[3]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_intEng->SetStats(0);
  hPS_intEng->SetMaximum(0.5);
  hPS_intEng->SetMinimum(0); 
  hPS_intEng->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_intEng->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_intEng->Draw("text colz");
  gPad->Update();
  subCanv[3]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_clus_e->SetStats(0);
  hPS_clus_e->SetMaximum(1.5);
  hPS_clus_e->SetMinimum(0); 
  hPS_clus_e->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_clus_e->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_clus_e->Draw("colz");
  gPad->Update();

}

void clicked_displayNextButton()
{
  //if(gCurrentEntry>gMaxEntries);
  shgui::entryInput->SetIntNumber(++gCurrentEntry);
  displayEvent(gCurrentEntry);
}

void clicked_displayEntryButton()
{
  gCurrentEntry = shgui::entryInput->GetIntNumber();
  displayEvent(gCurrentEntry);
}


Int_t display(Int_t run = 290, Int_t event = 50000)
{
  shgui::SetupGUI();
  gStyle->SetLabelSize(0.05,"XY");
  gStyle->SetTitleFontSize(0.08);

  //TString filename = "../bbshower_434_30000.root";
  TString filename = Form("$OUT_DIR/bbshower_%d_%d.root",run,event);
  TFile *f = TFile::Open(filename); 
  TChain *C = (TChain*)f->Get("T");
  cout << "Opened up tree with nentries=" << C->GetEntries() << endl;

  T = new gmn_tree(C);
  event = -1;

  gCurrentEntry = event;
  while( user_input != "q" ) {
    if(is_number(user_input)) {
      gCurrentEntry = std::stoi(user_input);
    } else {
      gCurrentEntry++;
    }
    displayEvent(gCurrentEntry,run);
    std::cout << "Display options: <enter> == next event, or q to stop." << std::endl;
    //std::cin >> user_input;
    getline(std::cin,user_input);
  }
  return 0;
}

