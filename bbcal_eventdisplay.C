#include "Riostream.h"
#include <TMath.h>
#include <TFile.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraphErrors.h>
#include <TTree.h>
#include <TGaxis.h>
#include <TLine.h>
#include <TROOT.h>
#include <TLegend.h>
#include <string>
#include <TCanvas.h>
#include <TStyle.h>

//Display an array, set a minimum value to display.
void Draw(Double_t data[], vector<Double_t>* means, Double_t min, const char* title, Int_t rows = 27, Int_t cols = 7){

  //Create a 2D histogram to store the data.
  TH2I* display = new TH2I(Form("display_%d", rows),title,cols,0,cols,rows,0,rows);
  
  //Fill the bins with data.
  for(int i = 0; i < cols; i++){
    for(int j = 0; j < rows; j++){
      
      //int k = (j*cols)+i;
      //if(cols==2)
      int k = (i*rows)+j;
      //Only display if value(channel) > min.
      if(data[k] - means->at(k) > min){
	//Fill the bins
	display->SetBinContent(i+1,j+1,(Int_t)(data[k] - means->at(k)));
      } else {
	//Else display 0.
	display->SetBinContent(i+1,j+1,0);
      }
    }
  }
  display->GetYaxis()->SetNdivisions(rows);
  display->GetXaxis()->SetNdivisions(cols);
  
  //Have to completely replace axes to position axis labels where they are	
  display->GetYaxis()->SetLabelSize(0);
  display->GetXaxis()->SetLabelSize(0);	
  
  //Let's make a function to set the axis values
  TF1 *xfunc = new TF1("xfunc","x",1,cols+1);
  TF1 *yfunc = new TF1("yfunc","x",1,rows+1);
  
  //Set colour, box text size and remove the stats box.
  display->SetFillColor(kRed-9);
  display->SetStats(0);
  //display->SetMarkerSize(1.2+((7-cols)/1.7));
  display->SetMarkerSize(2.5);
  
  //Have to make the old axis labels invisible (unable to edit some axis properties directly).
  display->GetYaxis()->SetLabelSize(0);
  display->GetXaxis()->SetLabelSize(0);
  display->SetTitleSize(10.5);

  //Display the event.
  display->Draw("box,text");	

  //Create completely new axes to get label in the middle of the divisions)
  TGaxis *x = new TGaxis(0,0,cols,0,"xfunc",cols+1,"M");
  //x->SetLabelSize(0.25/cols);
  x->SetLabelSize(0.05);
  //x->SetLabelOffset(0.015*(cols-7));
  x->SetLabelOffset(0.001);
  x->Draw();

  TGaxis *y = new TGaxis(0,0,0,rows,"yfunc",rows+1,"M");
  //y->SetLabelSize(0.25/cols);
  y->SetLabelSize(0.05);
  y->Draw();

	
  //Vertical lines.
  for (int i = 1; i < cols; i++){
    TLine *line = new TLine(i,0,i,rows);
    line->SetLineStyle(kDotted);
    line->Draw();	
  }

  //Horizontal lines.
  //Vertical lines.
  for (int i = 1; i < rows; i++){
    TLine *line = new TLine(0,i,cols,i);
    line->SetLineStyle(kDotted);
    line->Draw();	
  }

  //Memory clean up.
  //delete gROOT->FindObject("display");
}

void eventDisplay(const char* filename, Int_t evt = -1){
  vector<Double_t>* meanValuesSH = new vector<double>;
  meanValuesSH->resize(189);
  vector<Double_t>* meanValuesPS = new vector<double>;
  meanValuesPS->resize(52);
  /*
  //Get the mean and RMS values.
  vector<Double_t>* rmsValues;
  vector<Double_t>* rmsValuesPS;
  TFile *calibration = TFile::Open("pedestalcalibrated.root");
  meanValues = (vector<Double_t>*)calibration->Get("Mean");
  rmsValues = (vector<Double_t>*)calibration->Get("RMS");
  meanValuesPS = (vector<Double_t>*)calibration->Get("MeanPS");
  rmsValuesPS = (vector<Double_t>*)calibration->Get("RMSPS");
  calibration->Close();
  */
  
  //Create a Canvas
  TCanvas* c1 = new TCanvas("c1","Event Display (ADC integral in pC)",600,800);
  TPad *shower = new TPad("shower","Shower",0.01,0.01,0.49,0.99);
  shower->Draw();
  TPad *preshower = new TPad("preshower","Pre-Shower",.51,.01,.99,.99);
  preshower->SetLeftMargin(0.15);
  preshower->Draw();

  TCanvas* c2 = new TCanvas("c2","Event Display (ADC time in ns)",600,800);
  TPad *tshower = new TPad("tshower","Shower",0.01,0.01,0.49,0.99);
  tshower->Draw();
  TPad *tpreshower = new TPad("tpreshower","Pre-Shower",.51,.01,.99,.99);
  tpreshower->SetLeftMargin(0.15);
  tpreshower->Draw();
  
  //Open the file.
  TFile *events = TFile::Open(filename);
  
  //Get the Tree.
  TTree* tree = 0;
  events->GetObject("T",tree);
  
  //Set the variable to hold the values.
  Double_t dataSH[189];
  Double_t dataPS[52];
  Double_t tdataSH[189];
  Double_t tdataPS[52];
  tree->SetBranchAddress("bb.sh.a_p",&dataSH);
  tree->SetBranchAddress("bb.ps.a_p",&dataPS);
  tree->SetBranchAddress("bb.sh.a_time",&tdataSH);
  tree->SetBranchAddress("bb.ps.a_time",&tdataPS);
  
  //Get the number of events.
  Int_t nEvents = TMath::Min((UInt_t) tree->GetEntries(), (UInt_t)evt+1);
  
  Int_t event = 0;
  Int_t cell = 0;

  if(evt>=0 && evt<nEvents){
    tree->GetEntry(evt);		
    
    //Create the histogram to draw this event.
    std::string title = "Event ";
    title += std::to_string(evt);
    
    //Display the event
    shower->cd();
    Draw(dataSH,meanValuesSH, 10, title.c_str(),27,7);
    preshower->cd();
    Draw(dataPS,meanValuesPS, 10, "" ,26,2);

    tshower->cd();
    Draw(tdataSH,meanValuesSH, 10, title.c_str(),27,7);
    tpreshower->cd();
    Draw(tdataPS,meanValuesPS, 10, "" ,26,2);
    
  }else{
    for(Int_t event = 0; event < nEvents; event++){
      //Read in that data.
      tree->GetEntry(event);		
      
      //Get the total calibrated ADC value
      Double_t sum = 0;
      for(int i = 0; i < 189; i++){
	sum += dataSH[i];//-meanValues->at(i);	
	meanValuesSH->at(i) = 0.;
      }
      for(int i = 0; i < 52; i++){
	sum += dataPS[i];//-meanValues->at(i);	
	meanValuesPS->at(i) = 0.;
      }
      
      //Don't display events with a total ADC value < 50
      if(sum < 2){
	continue;
      }
      
      //Create the histogram to draw this event.
      std::string title = "Event ";
      title += std::to_string(event);
      
      //Display the event
      //c1->cd(1);
      shower->cd();
      Draw(dataSH,meanValuesSH, 10, title.c_str(),27,7);
      //c1->cd(2);
      preshower->cd();
      Draw(dataPS,meanValuesPS, 10, "" ,26,2);
      gPad -> WaitPrimitive();
      
      //Memory clean up
      delete gROOT->FindObject("display");
    }
  }
}
