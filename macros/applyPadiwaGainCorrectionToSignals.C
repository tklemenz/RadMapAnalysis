#include <TH2.h>
#include <TH1.h>
#include <TTree.h>
#include <TFile.h>
#include <TNtupleD.h>
#include <TCanvas.h>
#include <Rtypes.h>
#include <TChain.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <map>

#include "Utility.h"
#include "Constants.h"

///< usage: ./applyPadiwaGainCorrectionToSignals -i inputfile -o outputfileName -n numberOfSignalsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

void applyPadiwaGainCorrectionToSignals(const TString inputFiles, const char *outputFile, ULong_t procNr)
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

  ULong_t nSignals      =  0;

  /*========================================================
  ==========================================================*/
  

  for (Int_t ifile=0; ifile<files->GetEntriesFast(); ++ifile){
    TFile *SignalFile = new TFile(Form("%s", files->At(ifile)->GetTitle()));
    TTree *signals = (TTree*)SignalFile->Get("Signals");
    TFile *fout = new TFile(Form("%s_%s",fileHandling::splitString(outputFile, ".").front().data(), fileHandling::splitString(files->At(ifile)->GetTitle()).back().data()),"recreate");
    TNtupleD* nt = new TNtupleD("Dummy", "nt", "EventNr:timeStamp:ToT:chID:TDC:layer:x:y:signalNr:padiwaConfig:refTime");

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

    nSignals = procNr;
    if ((nSignals == -1) || (nSignals > signals->GetEntries())) { nSignals = signals->GetEntries(); }

    printf("events to process: %lu\t %.1f%% of file %s\n", nSignals, Float_t(100*nSignals)/Float_t(signals->GetEntries()), files->At(ifile)->GetTitle());

    Double_t totCalib(0),timeCalib(0);

    // The loop
    for (ULong_t entry = 0; entry < nSignals; entry++) {
      if ((((entry+1)%100000) == 0) || (entry == (nSignals-1))) {
        printf("\rprocessing signal %lu...", entry+1);
        fflush(stdout);
        std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nSignals<<" % done\r"<<std::flush;
      }

      signals->GetEntry(entry);

      timeCalib = (timeStamp-refTime)*1e9 - constants::padiwaTimeCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID));

      totCalib = ToT*constants::padiwaGainCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID));

      nt->Fill(eventNr,timeCalib,totCalib,chID,TDC,layer,x,y,signalNr,padiwaConfig,refTime);

    }// loop over input file

    nt->Write("Signals", 1);
    fout->Close();
  }// loop over input files
}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="applyPadiwaGainCorrectionToSignals_output.root";
  ULong_t procNr=-1;

  int argsforloop;
  while ((argsforloop = getopt(argc, argv, "hi:o:n:")) != -1) {
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
      default:
        printf("\n\n%s%sdefault case%s\n\n",text::BOLD,text::RED,text::RESET);
        exit(EXIT_FAILURE);
    }
  }

  printf("\n\n%sRunning applyPadiwaGainCorrectionToSignals%s\n\n",text::BOLD,text::RESET);
  
  applyPadiwaGainCorrectionToSignals(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}