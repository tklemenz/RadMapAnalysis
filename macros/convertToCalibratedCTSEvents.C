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

#include "CTSEvent.h"
#include "Utility.h"
#include "Constants.h"

///< usage: ./convertToCalibratedCTSEvents -i inputfile -o outputfileName -n numberOfSignalsToBeProcessed
///< n = -1 by default which means the whole file is processed

///< The calibration data is taken from Constants.h

///< input: raw signals
///< output: 1) single files with fully calibrated (ToT and T0) CTSEvents --> ToT in ns
///<         2) a file with raw and calibrated ToT distributions (in ns)

extern char* optarg;

void convertToCalibratedCTSEvents(const TString inputFiles, const char *outputFile, const char *dataType, const char *outputType, ULong_t procNr)
{
  TChain chain("Signals", "Signals");
  fileHandling::makeChain(chain, inputFiles);
  TObjArray* files = chain.GetListOfFiles();
  printf("%sFiles to be processed:%s\n", text::BOLD, text::RESET);
  for (int ifile=0; ifile<files->GetEntriesFast(); ++ifile){ printf("%s\n", files->At(ifile)->GetTitle()); }

  ParticleType type;

  if ( std::string(dataType) == std::string("pions") ) { type = ParticleType::Pion; }
  else if ( std::string(dataType) == std::string("protons") ) { type = ParticleType::Proton; }
  else { 
    printf("Invalid data type: chose either 'pions' or 'protons'!\n");
    return;
  }

  Int_t outputFormat = -1;

  if ( std::string(outputType) == std::string("amplitude") ) { outputFormat = 0; }
  else if ( std::string(outputType) == std::string("tot") ) { outputFormat = 1; }
  else {
    printf("Invalid output format: chose either 'amplitude' or 'tot'!\n");
    return;
  }

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
  Int_t prevEventNr     = -1;

  ULong_t nSignals      =  0;

  /* Define histograms and other useful containers
  ==========================================================
  ==========================================================*/
  std::vector<TH2D*> totLayerVec{};
  for(Int_t i = 0; i<8; i++) {
    totLayerVec.emplace_back(new TH2D(Form("hToTL%i",i+1),"calibrated signal amplitude distribution vs fiber;fiber;amplitude [mV]",33,0,33,500,0,50));
  }

  std::vector<TH2D*> totUncalibLayerVec{};
  for(Int_t i = 0; i<8; i++) {
    totUncalibLayerVec.emplace_back(new TH2D(Form("hToTrawL%i",i+1),"uncalibrated ToT distribution vs fiber;fiber;ToT",33,0,33,500,0,50));
  }
  /*========================================================
  ==========================================================*/

  CTSEvent *event;
  event = new CTSEvent();

  for (Int_t ifile=0; ifile<files->GetEntriesFast(); ++ifile){
    TFile *SignalFile = new TFile(Form("%s", files->At(ifile)->GetTitle()));
    TTree *signals = (TTree*)SignalFile->Get("Signals");
    TFile *fout = new TFile(Form("%s_%s",fileHandling::splitString(fileHandling::splitString(outputFile).back().data(), ".").front().data(), fileHandling::splitString(files->At(ifile)->GetTitle()).back().data()),"recreate");
    TTree *tree = new TTree("dummy","RadMap data in fancy objects -> CTSEvents");
    tree->Branch("Events","CTSEvent",&event,32000,1);

    // Setup Dataholders
    Signal signal = Signal();
    Module module = Module();

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

      if ((ULong_t(eventNr) != prevEventNr) && (prevEventNr !=1)) {
        module.removeEmpty();
        event->setModule(module);
        tree->Fill();

        module.reset();
      }

      timeCalib = (timeStamp-refTime)*1e9 - constants::padiwaTimeCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(Int_t(chID)));

      ToT = ToT*1e9;

      if (outputFormat == 0) {
        if (type==ParticleType::Pion) {
          totCalib = 0.667*exp(0.146*ToT*constants::padiwaGainCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(Int_t(chID))))/
                     constants::testModuleGainMapping.at(mapping::getPadiwaSocket(mapping::getPadiwa(Int_t(TDC), Int_t(chID)), UInt_t(padiwaConfig))).at(mapping::getPadiwaChannel(Int_t(chID)));
                   //constants::moduleGainConfigMap.at(Int_t(padiwaConfig)).at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID));
                   ///(Int_t(layer) < 5 ? constants::testModuleGainCorrL14.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID))
                   ///                  : constants::testModuleGainCorrL58.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID)));
        }
        else if (type==ParticleType::Proton) {
          totCalib = 0.667*exp(0.146*ToT*constants::padiwaGainCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(Int_t(chID))))/
                     constants::testModuleGainMappingProtons.at(mapping::getPadiwaSocket(mapping::getPadiwa(Int_t(TDC), Int_t(chID)), UInt_t(padiwaConfig))).at(mapping::getPadiwaChannel(Int_t(chID)));
        }
        else {
          printf("What?\n");
          return;
        }
      }
      else if (outputFormat == 1) {
        if (type==ParticleType::Pion) {
          totCalib = ToT*
                     constants::padiwaGainCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(Int_t(chID)))/
                     constants::testModuleGainMappingToT.at(mapping::getPadiwaSocket(mapping::getPadiwa(Int_t(TDC), Int_t(chID)), UInt_t(padiwaConfig))).at(mapping::getPadiwaChannel(Int_t(chID)));
                   //constants::moduleGainConfigMap.at(Int_t(padiwaConfig)).at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID));
                   ///(Int_t(layer) < 5 ? constants::testModuleGainCorrL14.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID))
                   ///                  : constants::testModuleGainCorrL58.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID)));
        }
        else if (type==ParticleType::Proton) {
          totCalib = ToT*
                     constants::padiwaGainCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(Int_t(chID)))/
                     constants::testModuleGainMappingProtonsToT.at(mapping::getPadiwaSocket(mapping::getPadiwa(Int_t(TDC), Int_t(chID)), UInt_t(padiwaConfig))).at(mapping::getPadiwaChannel(Int_t(chID)));
        }
        else {
          printf("What?\n");
          return;
        }
      }
      else { 
        printf("What?\n");
        return;
      }

      totLayerVec.at(Int_t(layer)-1)->Fill(mapping::getFiberNr(padiwaConfig, chID, TDC), totCalib);
      totUncalibLayerVec.at(Int_t(layer)-1)->Fill(mapping::getFiberNr(padiwaConfig, chID, TDC), ToT);

      signal = Signal(totCalib,timeCalib,signalNr,chID,layer,TDC,padiwaConfig);
      module.addSignal(signal);

      event->setEventNr(ULong_t(eventNr));
      event->setPadiwaConfig(UShort_t(padiwaConfig));

      prevEventNr = ULong_t(eventNr);
    } // loop over file

    prevEventNr = -1;
    tree->Write("data");
    fout->Close();

    delete event;
    event=nullptr;
  } // loop over all files

  std::string outputName = std::string();
  Int_t histCounter = 0;
  if (fileHandling::splitString(inputFiles.Data(), ",").size() == 1) {
    outputName = fileHandling::splitString(fileHandling::splitString(outputFile).back().data(), ".").front().data();
    outputName.append("_");
    outputName.append(fileHandling::splitString(fileHandling::splitString(inputFiles.Data()).back().data(), ".").front().data());
    outputName.append(".root");
  }
  TFile *fout2 = new TFile(Form("%s_calibTotDist_%s.root",fileHandling::splitString(fileHandling::splitString(outputFile).back().data(), ".").front().data(),
                                                          fileHandling::splitString(fileHandling::splitString(outputName).back().data(), ".").front().data()),"recreate");
  for(auto& hist : totLayerVec) {
    if(hist->GetEntries() != 0) { fout2->WriteObject(hist, hist->GetName()); histCounter++; }
  }
  for(auto& hist : totUncalibLayerVec) {
    if(hist->GetEntries() != 0) { fout2->WriteObject(hist, hist->GetName()); }
  }

  TCanvas *c1 = new TCanvas("cCalibToTDistsLayers","cCalibToTDistsLayers");
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
  char    inputFiles[1024]="";
  char    outputFile[512]="convertToCalibratedCTSEvents_output.root";
  char    dataType[512]="pions";
  char    outputType[512]="amplitude";
  ULong_t procNr=-1;

  int argsforloop;
  while ((argsforloop = getopt(argc, argv, "hi:o:n:t:c:")) != -1) {
    switch (argsforloop) {
      case '?':
        ///TODO: write usage function
        exit(EXIT_FAILURE);
      case 'i':
        strncpy(inputFiles, optarg, 1024);
        break;
      case 'o':
        strncpy(outputFile, optarg, 512);
        break;
      case 'n':
        procNr = std::atoi(optarg);
        break;
      case 't':
        strncpy(dataType, optarg, 512);
        break;
      case 'c':
        strncpy(outputType, optarg, 512);
        break;
      default:
        printf("\n\n%s%sdefault case%s\n\n",text::BOLD,text::RED,text::RESET);
        exit(EXIT_FAILURE);
    }
  }

  printf("\n\n%sRunning convertToCalibratedCTSEvents%s\n\n",text::BOLD,text::RESET);
  
  convertToCalibratedCTSEvents(inputFiles,outputFile,dataType,outputType,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}