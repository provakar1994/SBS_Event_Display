#include <TH2.h>
#include <TH2F.h>
#include <TChain.h>
#include <TCanvas.h>
#include <vector>
#include <iostream>
#include <TSystem.h>
#include "fadc_data.h"

const Int_t kNrows = 27;
const Int_t kNcols = 7;
const Int_t kNrowsPS = 26;
const Int_t kNcolsPS = 2;

const Int_t kCanvSize = 100;
std::string user_input;
Int_t gCurrentEntry = -1;

TChain *T = 0;
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

      for(Int_t i = 0; i < 1; i++) {
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
TH2F* hSH_int = new TH2F("sh_int","Shower ; Ncol ; Nrow",kNcols,1,kNcols+1,kNrows,1,kNrows+1);
TH2F* hSH_clus_e = new TH2F("sh_clus_e","Shower ; Ncol ; Nrow",kNcols,1,kNcols+1,kNrows,1,kNrows+1);
TH2F* hPS_int = new TH2F("ps_int","PreShower ; Ncol ; Nrow",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);
TH2F* hPS_clus_e = new TH2F("ps_clus_e","PreShower ; Ncol ; Nrow",kNcolsPS,1,kNcolsPS+1,kNrowsPS,1,kNrowsPS+1);

// TH1F* MakeHisto(Int_t row, Int_t col, Int_t bins, const char* suf="")
// {
//   TH1F *h = new TH1F(TString::Format("h%02d%02d%s",row,col, suf),
//       TString::Format("%d-%d",row+1,col+1),bins,DISP_MIN_SAMPLE,DISP_MAX_SAMPLE);
//   h->SetStats(0);
//   h->SetLineWidth(2);
//   h->GetYaxis()->SetLabelSize(0.1);
//   //h->GetYaxis()->SetLabelOffset(-0.17);
//   h->GetYaxis()->SetNdivisions(5);
//   return h;
// }


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

  Int_t r,c,idx,n,sub;
  // Clear old histograms, just in case modules are not in the tree
  hSH_int->Reset("ICES M");
  hPS_int->Reset("ICES M");
  //Clustering
  hSH_clus_e->Reset("ICES M");
  hPS_clus_e->Reset("ICES M");

  Float_t amp[kNrows][kNcols], ampPS[kNrowsPS][kNcolsPS];
  Float_t peak[kNrows][kNcols];
  Double_t adc[kNrows][kNcols], adcPS[kNrowsPS][kNcolsPS];
  Double_t tdc[kNrows][kNcols], tdcPS[kNrowsPS][kNcolsPS];
  //CLustering
  Double_t clus_e_SH[kNrows][kNcols];
  Double_t clus_e_PS[kNrows][kNcols];
  
  for(r  = 0; r < kNrows; r++) {
    for(c  = 0; c < kNcols; c++) {
      peak[r][c] = 0.0;
      adc[r][c] = 0.0;
      amp[r][c] = 0.0;
      tdc[r][c] = 0.0;
      adcPS[r][c] = 0.0;
      ampPS[r][c] = 0.0;
      tdcPS[r][c] = 0.0;
      //Clustering
      clus_e_SH[r][c] = 0.0;
      clus_e_PS[r][c] = 0.0;
    }
  }
  for(Int_t m = 0; m < fadc_datat::ndata; m++) {
    r = fadc_datat::row[m];//-1;
    c = fadc_datat::col[m];//-1;
    if(r < 0 || c < 0) {
      std::cerr << "Why is row negative? Or col?" << std::endl;
      continue;
    }
    if(r>= kNrows || c >= kNcols)
      continue;
    idx = fadc_datat::samps_idx[m];
    n = fadc_datat::nsamps[m];
    adc[r][c] = fadc_datat::a[m];
    tdc[r][c] = fadc_datat::tdc[m];
    amp[r][c] = fadc_datat::amp[m];

    //Clustering
    clus_e_SH[r][c] = fadc_datat::cl_e_SH[m];

    if( tdc[r][c]>0 ){
      //displayed = true;
      hSH_int->Fill( double(c+1),double(r+1),fadc_datat::a[m] );
      hSH_clus_e->Fill( double(c+1),double(r+1),fadc_datat::cl_e_PS[m] );
    }

  }

  for(Int_t m = 0; m < fadc_datat::ndataPS; m++) {
    r = fadc_datat::rowPS[m];//-1;
    c = fadc_datat::colPS[m];//-1;
    if(r < 0 || c < 0) {
      std::cerr << "Why is row negative? Or col?" << std::endl;
      continue;
    }
    if(r>= kNrowsPS || c >= kNcolsPS)
      continue;
    adcPS[r][c] = fadc_datat::aPS[m];
    tdcPS[r][c] = fadc_datat::tdcPS[m];
    ampPS[r][c] = fadc_datat::ampPS[m];

    //Clustering
    clus_e_PS[r][c] = fadc_datat::cl_e_PS[m];
    
    if( tdcPS[r][c]>0 ){
      hPS_int->Fill( double(c),double(r+1),fadc_datat::aPS[m] );
      hPS_clus_e->Fill( double(c),double(r+1),fadc_datat::cl_e_PS[m] );
    }
  }
    
  subCanv[0]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_int->SetStats(0);
  hSH_int->SetMaximum(50);
  hSH_int->SetMinimum(0); 
  hSH_int->GetYaxis()->SetNdivisions(kNrows);
  hSH_int->GetXaxis()->SetNdivisions(kNcols);
  hSH_int->Draw("text colz");
  gPad->Update();
  subCanv[0]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_int->SetStats(0);
  hPS_int->SetMaximum(50);
  hPS_int->SetMinimum(0); 
  hPS_int->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_int->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_int->Draw("text colz");
  gPad->Update();

  subCanv[1]->cd(1);
  gPad->SetGridx();
  gPad->SetGridy();
  hSH_clus_e->SetStats(0);
  hSH_clus_e->SetMaximum(50);
  hSH_clus_e->SetMinimum(0); 
  hSH_clus_e->GetYaxis()->SetNdivisions(kNrows);
  hSH_clus_e->GetXaxis()->SetNdivisions(kNcols);
  hSH_clus_e->Draw("text colz");
  gPad->Update();
  subCanv[1]->cd(2);
  gPad->SetGridx();
  gPad->SetGridy();
  hPS_clus_e->SetStats(0);
  hPS_clus_e->SetMaximum(50);
  hPS_clus_e->SetMinimum(0); 
  hPS_clus_e->GetYaxis()->SetNdivisions(kNrowsPS);
  hPS_clus_e->GetXaxis()->SetNdivisions(kNcolsPS);
  hPS_clus_e->Draw("text colz");
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


Int_t display(Int_t run = 290, Int_t event = -1)
{
  shgui::SetupGUI();
  gStyle->SetLabelSize(0.05,"XY");
  gStyle->SetTitleFontSize(0.08);
  if(!T) { 
    T = new TChain("T");
    TString dataDIR = gSystem->Getenv("OUT_DIR");
    TString filename = dataDIR + "/bbshower_"+run+"_500000.root";
    // TString filename = "../bbshower_434_30000.root";
    event = -1;
    T->Add(filename);
    T->SetBranchStatus("*",0);
    T->SetBranchStatus("bb.sh.*",1);
    T->SetBranchAddress("bb.sh.a_p",fadc_datat::a);
    T->SetBranchAddress("bb.sh.a_amp_p",fadc_datat::amp);
    T->SetBranchAddress("bb.sh.a_time",fadc_datat::tdc);
    T->SetBranchAddress("bb.sh.adcrow",fadc_datat::row);
    T->SetBranchAddress("bb.sh.adccol",fadc_datat::col);
    T->SetBranchStatus("Ndata.bb.sh.adcrow",1);
    T->SetBranchAddress("Ndata.bb.sh.adcrow",&fadc_datat::ndata);
    T->SetBranchStatus("bb.ps.*",1);
    T->SetBranchAddress("bb.ps.a_p",fadc_datat::aPS);
    T->SetBranchAddress("bb.ps.a_amp_p",fadc_datat::ampPS);
    T->SetBranchAddress("bb.ps.a_time",fadc_datat::tdcPS);
    T->SetBranchAddress("bb.ps.adcrow",fadc_datat::rowPS);
    T->SetBranchAddress("bb.ps.adccol",fadc_datat::colPS);
    T->SetBranchStatus("Ndata.bb.ps.adcrow",1);
    T->SetBranchAddress("Ndata.bb.ps.adcrow",&fadc_datat::ndataPS);
    // Clustering
    T->SetBranchAddress("bb.sh.clus.e_c",fadc_datat::cl_e_SH);
    T->SetBranchAddress("bb.sh.clus.e_c",fadc_datat::cl_e_PS);
    cout << "Opened up tree with nentries=" << T->GetEntries() << endl;
  }
  return 0;
  gCurrentEntry = event;
  while( user_input != "q" ) {
    if(is_number(user_input)) {
      gCurrentEntry = std::stoi(user_input);
    } else {
      gCurrentEntry++;
    }
    //dis1(event);
    displayEvent(gCurrentEntry,run);
    std::cout << "Display options: <enter> == next event, or q to stop." << std::endl;
    //std::cin >> user_input;
    getline(std::cin,user_input);
  }
  return 0;
}

