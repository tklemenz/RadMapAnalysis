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

///< input: raw signals
///< output: 1) single files with padiwa gain calibrated (ToT) signal tuples --> ToT in s
///<         2) a file with raw and calibrated ToT distributions (in ns)

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

  /* Define histograms and other useful containers
  ==========================================================
  ==========================================================*/
  std::vector<TH2D*> totLayerVec{};
  for(Int_t i = 0; i<8; i++) {
    totLayerVec.emplace_back(new TH2D(Form("hToTcalL%i",i+1),"padiwa gain calibrated ToT distribution vs fiber;fiber;ToT",33,0,33,500,0,50));
  }

  std::vector<TH2D*> totUncalibLayerVec{};
  for(Int_t i = 0; i<8; i++) {
    totUncalibLayerVec.emplace_back(new TH2D(Form("hToTrawL%i",i+1),"uncalibrated ToT distribution vs fiber;fiber;ToT",33,0,33,500,0,50));
  }
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

      /*totCalib = ToT*(ToT*constants::padiwaGainCorrSlope.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID))
                         +constants::padiwaGainCorrOffset.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID)));*/

      totCalib = ToT*constants::padiwaGainCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID));

      nt->Fill(eventNr,timeCalib,totCalib,chID,TDC,layer,x,y,signalNr,padiwaConfig,refTime);

      totLayerVec.at(Int_t(layer)-1)->Fill(mapping::getFiberNr(padiwaConfig, chID, TDC), totCalib*1e9);
      totUncalibLayerVec.at(Int_t(layer)-1)->Fill(mapping::getFiberNr(padiwaConfig, chID, TDC), ToT*1e9);

    }// loop over input file

    nt->Write("Signals", 1);
    fout->Close();
  }// loop over input files

  std::string outputName = std::string();
  if (fileHandling::splitString(inputFiles.Data(), ",").size() == 1) {
    outputName = fileHandling::splitString(fileHandling::splitString(outputFile).back().data(), ".").front().data();
    outputName.append("_");
    outputName.append(fileHandling::splitString(fileHandling::splitString(inputFiles.Data()).back().data(), ".").front().data());
    outputName.append(".root");
  }
  TFile *fout2 = new TFile(Form("%s_padiwaCalibTotDist_%s.root",fileHandling::splitString(fileHandling::splitString(outputFile).back().data(), ".").front().data(),
                                                                fileHandling::splitString(fileHandling::splitString(outputName).back().data(), ".").front().data()),"recreate");

  Int_t histCounter = 0;
  for(auto& hist : totLayerVec) {
    if(hist->GetEntries() != 0) { fout2->WriteObject(hist, hist->GetName()); histCounter++; }
  }
  for(auto& hist : totUncalibLayerVec) {
    if(hist->GetEntries() != 0) { fout2->WriteObject(hist, hist->GetName()); }
  }

  TCanvas *c1 = new TCanvas("cPadiwaCalibToTDistsLayers","cPadiwaCalibToTDistsLayers");
  c1->DivideSquare(histCounter);

  Int_t padIter = 1;
  for(auto& hist : totLayerVec) {
    if(hist->GetEntries() == 0) { continue; }
    c1->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c2 = new TCanvas("cRawToTDistsLayers","cRawToTDistsLayers");
  c2->DivideSquare(histCounter);

  padIter = 1;
  for(auto& hist : totUncalibLayerVec) {
    if(hist->GetEntries() == 0) { continue; }
    c2->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  fout2->WriteObject(c1, c1->GetName());
  fout2->WriteObject(c2, c2->GetName());
  fout2->Close();
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