#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <Rtypes.h>
#include <TCanvas.h>
#include <TNtupleD.h>
#include <TChain.h>
#include <TObjArray.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <map>

#include "Utility.h"
#include "Constants.h"

///< usage: ./getToTCalibrationValues -i inputfiles -o outputfile -c outputfileForCalibData -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed
///< Get calibration data for all fibers to pull the mean ToT to `pullTo` ns.
///< This macro should be used to get the calibration data for different PADIWA configs.
///< Sabrina's calibration where pions and protons were used only works for config 0!
///< Since there is only proton data for config 0 we can only use pion data for the other configs.

extern char* optarg;

static Int_t   totCut = 5;   // cut away all signals with ToT<totCut to get rid of possible bias for gaus fit
static Float_t pullTo = 15;  // calib value that is written to file is pullTo/gausMean

void getToTCalibrationValues(const TString inputFiles, const char *outputFile, const char *outputCalib, ULong_t procNr)
{
  TChain chain("Signals", "Signals");
  fileHandling::makeChain(chain, inputFiles);
  TObjArray* files = chain.GetListOfFiles();
  printf("%sFiles to be processed:%s\n", text::BOLD, text::RESET);
  for (int ifile=0; ifile<files->GetEntriesFast(); ++ifile){ printf("%s\n", files->At(ifile)->GetTitle()); }

  /* Define variables
  ==========================================================
  ==========================================================*/
  Double_t eventNr      = -1;
  Double_t timeStamp    = -1;
  Double_t ToT          = -1;
  Double_t chID         = -1;
  Double_t TDC          = -1;
  Double_t layer        = -1;
  Double_t x            = -1;
  Double_t y            = -1;
  Double_t signalNr     = -1;
  Double_t padiwaConfig = -1;
  Double_t refTime      = -1;

  Int_t fiberNr         = -1;
  Int_t layerIter       =  0;

  ULong_t nEvents       =  0;

  /* Define histograms and other useful containers
  ==========================================================
  ==========================================================*/
  std::vector<TH2D*> totLayerVec{};
  for(Int_t i = 0; i<8; i++) {
    totLayerVec.emplace_back(new TH2D(Form("hToTL%i",i+1),"ToT distribution vs fiber;fiber;ToT",33,0,33,500,0,50));
  }

  std::vector<TH2D*> totPadiwaVec{};
  for(auto& name : constants::padiwaNames) {
    totPadiwaVec.emplace_back(new TH2D(Form("hToTPadiwa%s",name.c_str()),"ToT distribution vs padiwa channel;channel;ToT",17,0,17,500,0,50));
  }

  std::vector<TH1D*> totLayerGausMean{};
  std::vector<std::vector<Float_t>> fitContentLayer{};

  std::vector<TH1D*> totPadiwaGausMean{};
  std::vector<std::vector<Float_t>> fitContentPadiwa{};

  std::vector<Int_t> layerMarker{};
  /*========================================================
  ==========================================================*/

  for (Int_t ifile=0; ifile<files->GetEntriesFast(); ++ifile){
    TFile *SignalFile = new TFile(Form("%s", files->At(ifile)->GetTitle()));
    TTree *signals = (TTree*)SignalFile->Get("Signals");

    signals->SetBranchAddress("EventNr", &eventNr);
    signals->SetBranchAddress("timeStamp", &timeStamp);
    signals->SetBranchAddress("ToT", &ToT);
    signals->SetBranchAddress("chID", &chID);
    signals->SetBranchAddress("TDC", &TDC);
    signals->SetBranchAddress("layer", &layer);
    signals->SetBranchAddress("x", &x);
    signals->SetBranchAddress("y", &y);
    signals->SetBranchAddress("signalNr", &signalNr);
    signals->SetBranchAddress("padiwaConfig", &padiwaConfig);
    signals->SetBranchAddress("refTime", &refTime);

    nEvents = procNr;
    if ((nEvents == -1) || (nEvents > signals->GetEntries())) { nEvents = signals->GetEntries(); }

    printf("events to process: %lu\t %.1f%% of file %s\n", nEvents, Float_t(100*nEvents)/Float_t(signals->GetEntries()), files->At(ifile)->GetTitle());

    for (ULong_t entry = 0; entry < nEvents; entry++) {
      /* Make some fancy terminal output */
      if ((((entry+1)%10000) == 0) || (entry == (nEvents-1))) {
        printf("\rprocessing event %lu...", entry+1);
        fflush(stdout);
        std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
      } /* End of terminal output*/

      signals->GetEntry(entry);

      ToT *= 1e9; // convert ToT value to ns
      if (ToT < totCut) { continue; }                             // do not use small ToT values to not have gaus fit biased by noise
      if (signalNr != 1) { continue; }
      fiberNr = mapping::getFiberNr(padiwaConfig, chID, TDC);

      totPadiwaVec.at(constants::padiwaPosMap.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))))->Fill(mapping::getPadiwaChannel(chID), ToT);
      totLayerVec.at(Int_t(layer)-1)->Fill(fiberNr, ToT);
    }// end of loop over file
  }// end of loop over files


  TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  Int_t histCounter = 0;
  // loop over TH2D from above, fit 1D ToT distribution for all fibers and extract the mean value
  // meanwhile only take the layers that actually have data and leave the rest ignored
  for(auto& hist : totLayerVec) {                                                             // loop over histos
    layerIter++;
    if(hist->GetEntries() != 0) {
      layerMarker.emplace_back(layerIter);                                                    // write down which layers are used
      fout->WriteObject(hist, hist->GetName());                                               // write ToT vs fiber to file
      hist->FitSlicesY(0,2,33);                                                               // fit 1D distributions with gaus
      totLayerGausMean.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));   // get fit mean values (written to 1D histos)
      fout->WriteObject(totLayerGausMean.back(), totLayerGausMean.back()->GetName());         // write fit results to file
      fitContentLayer.emplace_back(std::vector<Float_t>());                                        // prepare extraction of fit results
      for(Int_t i=0; i<totLayerGausMean.back()->GetSize(); i++) {
        fitContentLayer.back().emplace_back((*totLayerGausMean.back())[i]);                        // extract fit results
      }
      histCounter++;
    }
  }

  for(auto& hist : totPadiwaVec) {                                                             // loop over histos
    fout->WriteObject(hist, hist->GetName());                                               // write ToT vs fiber to file
    hist->FitSlicesY(0,2,17);                                                               // fit 1D distributions with gaus
    totPadiwaGausMean.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));   // get fit mean values (written to 1D histos)
    fout->WriteObject(totPadiwaGausMean.back(), totPadiwaGausMean.back()->GetName());         // write fit results to file
    fitContentPadiwa.emplace_back(std::vector<Float_t>());                                        // prepare extraction of fit results
    for(Int_t i=0; i<totPadiwaGausMean.back()->GetSize(); i++) {
      fitContentPadiwa.back().emplace_back((*totPadiwaGausMean.back())[i]);                        // extract fit results
    }
  }

  TCanvas *c1 = new TCanvas("cToTDistsLayers","cToTDistsLayers");
  c1->DivideSquare(histCounter);

  Int_t padIter = 1;
  for(auto& hist : totLayerVec) {
    if(hist->GetEntries() == 0) { continue; }
    c1->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c2 = new TCanvas("cToTDistsPadiwas","cToTDistsPadiwas");
  c2->DivideSquare(8);

  padIter = 1;
  for(auto& hist : totPadiwaVec) {
    if(hist->GetEntries() == 0) { continue; }
    c2->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  fout->WriteObject(c1, c1->GetName());
  fout->WriteObject(c2, c2->GetName());

  fout->Close();

  // write fit results to file
  std::ofstream calibOutput;
  calibOutput.open(outputCalib);
  calibOutput << "This file contains calibration corrections in Padiwa and Fiber (in layer) mapping.";
  calibOutput << "Multiply the value for each fiber with the uncalibrated ToT value to pull everything to 15 ns.\n";
  calibOutput << "DO NOT FORGET TO REMOVE THE LAST COMMA FOR EACH PADIWA/LAYER!!!\n";
  Int_t fitIter = 0;

  for (auto& content : fitContentPadiwa) {
    calibOutput << "Padiwa ";
    calibOutput << constants::padiwaNames.at(fitIter);
    calibOutput << "\n";
    for (Int_t i=2;i<content.size()-1;i++) {
      calibOutput << pullTo/content.at(i);
      calibOutput << ", ";
    }
    calibOutput << "\n";
    fitIter++;
  }
  calibOutput << "\n\n\n";
  fitIter = 0;
  for (auto& content : fitContentLayer) {
    calibOutput << "Layer ";
    calibOutput << layerMarker.at(fitIter);
    calibOutput << "\n";
    for (Int_t i=2;i<content.size()-1;i++) {
      calibOutput << pullTo/content.at(i);
      calibOutput << ", ";
    }
    calibOutput << "\n";
    fitIter++;
  }

  calibOutput.close();
}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="getToTCalibrationValues_output.root";
  ULong_t procNr=-1;
  char    outputCalib[512]="getToTCalibrationValues_output.txt";

  int argsforloop;
  while ((argsforloop = getopt(argc, argv, "hi:o:n:c:")) != -1) {
    switch (argsforloop) {
      case '?':
        ///TODO: write usage function
        exit(EXIT_FAILURE);
      case 'i':
        strncpy(inputFile, optarg, 512);
        break;
      case 'o':
        strncpy(outputFile, optarg, 512);
        break;
      case 'n':
        procNr = std::atoi(optarg);
        break;
      case 'c':
        strncpy(outputCalib, optarg, 512);
        break;
      default:
        printf("\n\n%s%sdefault case%s\n\n",text::BOLD,text::RED,text::RESET);
        exit(EXIT_FAILURE);
    }
  }

  printf("\n\n%sRunning getToTCalibrationValues%s\n\n",text::BOLD,text::RESET);
  
  getToTCalibrationValues(inputFile,outputFile,outputCalib,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}