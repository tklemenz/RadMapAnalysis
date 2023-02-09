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

///< usage: ./getLinPadiwaCalib -i inputfile_pion -j inputfile_proton -o outputfile -c outputfileForCalibData -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed
///< Get calibration data for all fibers to pull the mean ToT to `pullTo` ns.
///< This macro should be used to get the calibration data for different PADIWA configs.
///< Sabrina's calibration where pions and protons were used only works for config 0!
///< Since there is only proton data for config 0 we can only use pion data for the other configs.

extern char* optarg;

static Int_t   totCut = 5;   // cut away all signals with ToT<totCut to get rid of possible bias for gaus fit
static Float_t pullTo = 0;   // calib value that is written to file is pullTo/gausMean
                             // will be recalculated later

void getLinPadiwaCalib(const TString inputFilesPi, const TString inputFilesPro, const char *outputFile, const char *outputCalib, ULong_t procNr)
{
  /// Prepare pion like signal file
  //=====================================
  TChain chainPi("Signals", "Signals");
  fileHandling::makeChain(chainPi, inputFilesPi);
  TObjArray* filesPi = chainPi.GetListOfFiles();
  printf("%sPion like files to be processed:%s\n", text::BOLD, text::RESET);
  for (int ifile=0; ifile<filesPi->GetEntriesFast(); ++ifile){ printf("%s\n", filesPi->At(ifile)->GetTitle()); }

  /// Prepare proton like signal file
  //=====================================
  TChain chainPro("Signals", "Signals");
  fileHandling::makeChain(chainPro, inputFilesPro);
  TObjArray* filesPro = chainPro.GetListOfFiles();
  printf("%sProton like files to be processed:%s\n", text::BOLD, text::RESET);
  for (int ifile=0; ifile<filesPro->GetEntriesFast(); ++ifile){ printf("%s\n", filesPro->At(ifile)->GetTitle()); }

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

  std::vector<std::vector<float>> fitSlope{ {},{},{},{},{},{},{},{} };
  std::vector<std::vector<float>> fitOffset{ {},{},{},{},{},{},{},{} };

  /* Define histograms and other useful containers
  ==========================================================
  ==========================================================*/

  std::vector<TH2D*> totPadiwaVecPion{};
  for(auto& name : constants::padiwaNames) {
    totPadiwaVecPion.emplace_back(new TH2D(Form("hToTPadiwa%s_Pion",name.c_str()),"ToT distribution vs padiwa channel;channel;ToT",17,0,17,300,5,35));
  }

  std::vector<TH1D*> totPadiwaGausMeanPion{};
  std::vector<std::vector<Float_t>> fitContentPadiwaPion{};

  std::vector<TH2D*> totPadiwaVecProton{};
  for(auto& name : constants::padiwaNames) {
    totPadiwaVecProton.emplace_back(new TH2D(Form("hToTPadiwa%s_Proton",name.c_str()),"ToT distribution vs padiwa channel;channel;ToT",17,0,17,300,5,35));
  }

  std::vector<TH1D*> totPadiwaGausMeanProton{};
  std::vector<std::vector<Float_t>> fitContentPadiwaProton{};

  /*========================================================
  ==========================================================*/

   TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  for (Int_t ifile=0; ifile<filesPi->GetEntriesFast(); ++ifile){
    TFile *SignalFile = new TFile(Form("%s", filesPi->At(ifile)->GetTitle()));
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

    printf("events to process: %lu\t %.1f%% of file %s\n", nEvents, Float_t(100*nEvents)/Float_t(signals->GetEntries()), filesPi->At(ifile)->GetTitle());

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
      if (layer == 4 && (fiberNr == 23 || fiberNr == 27)) { continue; }

      totPadiwaVecPion.at(constants::padiwaPosMap.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))))->Fill(mapping::getPadiwaChannel(chID), ToT);
    }// end of loop over file
  }// end of loop over files

  for(auto& hist : totPadiwaVecPion) {                                                          // loop over histos
    fout->WriteObject(hist, hist->GetName());                                                   // write ToT vs fiber to file
    hist->FitSlicesY(0,2,17);                                                                   // fit 1D distributions with gaus
    totPadiwaGausMeanPion.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));  // get fit mean values (written to 1D histos)
    fout->WriteObject(totPadiwaGausMeanPion.back(), totPadiwaGausMeanPion.back()->GetName());   // write fit results to file
    fitContentPadiwaPion.emplace_back(std::vector<Float_t>());                                  // prepare extraction of fit results
    for(Int_t i=0; i<totPadiwaGausMeanPion.back()->GetSize(); i++) {
      fitContentPadiwaPion.back().emplace_back((*totPadiwaGausMeanPion.back())[i]);             // extract fit results
    }
  }

  // ==================================   Proton like file   =============================================================
  for (Int_t ifile=0; ifile<filesPro->GetEntriesFast(); ++ifile){
    TFile *SignalFile = new TFile(Form("%s", filesPro->At(ifile)->GetTitle()));
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

    printf("events to process: %lu\t %.1f%% of file %s\n", nEvents, Float_t(100*nEvents)/Float_t(signals->GetEntries()), filesPro->At(ifile)->GetTitle());

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

      totPadiwaVecProton.at(constants::padiwaPosMap.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))))->Fill(mapping::getPadiwaChannel(chID), ToT);
    }// end of loop over file
  }// end of loop over files

  for(auto& hist : totPadiwaVecProton) {                                                          // loop over histos
    fout->WriteObject(hist, hist->GetName());                                               // write ToT vs fiber to file
    hist->FitSlicesY(0,2,17);                                                               // fit 1D distributions with gaus
    totPadiwaGausMeanProton.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));  // get fit mean values (written to 1D histos)
    fout->WriteObject(totPadiwaGausMeanProton.back(), totPadiwaGausMeanProton.back()->GetName());       // write fit results to file
    fitContentPadiwaProton.emplace_back(std::vector<Float_t>());                                  // prepare extraction of fit results
    for(Int_t i=0; i<totPadiwaGausMeanProton.back()->GetSize(); i++) {
      fitContentPadiwaProton.back().emplace_back((*totPadiwaGausMeanProton.back())[i]);                 // extract fit results
    }
  }
  // ==================================   End Proton like file   =============================================================

  TCanvas *c1 = new TCanvas("cToTDistsPadiwas_Pion","cToTDistsPadiwas_Pion");
  c1->DivideSquare(8);

  Int_t padIter = 1;
  for(auto& hist : totPadiwaVecPion) {
    if(hist->GetEntries() == 0) { continue; }
    c1->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c2 = new TCanvas("cToTDistsPadiwas_Proton","cToTDistsPadiwas_Proton");
  c2->DivideSquare(8);

  padIter = 1;
  for(auto& hist : totPadiwaVecProton) {
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
  calibOutput << "This file contains slope and offset of linear calibration for padiwa gain.\n";
  calibOutput << "The slope of the corresponding fit is\n";

  for (Int_t i = 0; i<8; i++) {
    calibOutput << "Padiwa ";
    calibOutput << constants::padiwaNames[i];
    calibOutput << "\n";
    Padiwa padiwa = constants::padiwaPosMapInv.at(i);
    for (Int_t j = 2; j<fitContentPadiwaPion.at(i).size()-1; j++) {
      float y2 = constants::padiwaGainCorrProton.at(padiwa).at(j-1);
      float y1 = constants::padiwaGainCorr.at(padiwa).at(j-1);
      float x2 = fitContentPadiwaProton.at(i).at(j);
      float x1 = fitContentPadiwaPion.at(i).at(j);
      float slope = (y2-y1)/(x2-x1);
      //float slope = (constants::padiwaGainCorrProton.at(padiwa).at(j-1)-constants::padiwaGainCorr.at(padiwa).at(j-1))/(fitContentPadiwaProton.at(i).at(j)-fitContentPadiwaPion.at(i).at(j));
      //printf("slope = %g = (%g-%g)/(%g-%g)\n", slope, y2, y1, x2, x1);
      calibOutput << slope;
      calibOutput << ", ";
      fitSlope.at(i).emplace_back(slope);
    }
    calibOutput << "\n";
  }

  calibOutput << "The offset of the corresponding fit is\n";
  for (Int_t i = 0; i<8; i++) {
    calibOutput << "Padiwa ";
    calibOutput << constants::padiwaNames[i];
    calibOutput << "\n";
    Padiwa padiwa = constants::padiwaPosMapInv.at(i);
    for (Int_t j = 2; j<fitContentPadiwaPion.at(i).size()-1; j++) {
      float y = constants::padiwaGainCorr.at(padiwa).at(j-1);
      float m = fitSlope.at(i).at(j-2);
      float x = fitContentPadiwaPion.at(i).at(j);
      //float offset = constants::padiwaGainCorr.at(padiwa).at(j-1)-(fitSlope.at(i).at(j-2)*fitContentPadiwaPion.at(i).at(j));
      float offset = y -m*x;
      //printf("offset = %g = %g-%g*%g\n", offset, y, m, x);
      calibOutput << offset;
      calibOutput << ", ";
      fitOffset.at(i).emplace_back(offset);
    }
    calibOutput << "\n";
  }

  calibOutput.close();
}

int main(int argc, char** argv)
{
  char    inputFile_pion[512]="";
  char    inputFile_proton[512]="";
  char    outputFile[512]="getLinPadiwaCalib_output.root";
  ULong_t procNr=-1;
  char    outputCalib[512]="getLinPadiwaCalib_output.txt";

  int argsforloop;
  while ((argsforloop = getopt(argc, argv, "hi:j:o:n:c:")) != -1) {
    switch (argsforloop) {
      case '?':
        ///TODO: write usage function
        exit(EXIT_FAILURE);
      case 'i':
        strncpy(inputFile_pion, optarg, 512);
        break;
      case 'j':
        strncpy(inputFile_proton, optarg, 512);
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

  printf("\n\n%sRunning getLinPadiwaCalib%s\n\n",text::BOLD,text::RESET);
  
  getLinPadiwaCalib(inputFile_pion,inputFile_proton,outputFile,outputCalib,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}