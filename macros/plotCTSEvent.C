#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <Rtypes.h>
#include <TCanvas.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <unistd.h>

#include "CTSEvent.h"
#include "Utility.h"

///< usage: ./plotCTSEvent -i inputfile -o outputfile -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

void plotCTSEvent(const char *inputFile, const char *outputFile, ULong_t procNr)
{
  TFile* f = TFile::Open(inputFile);

  if (f->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input file failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TTree *data = (TTree*)f->Get("data");

  ULong_t nEvents = procNr;
  if ((nEvents == -1) || (nEvents > data->GetEntries())) { nEvents = data->GetEntries(); }

  CTSEvent *event = nullptr;

  Float_t eventNr      = -1;
  Int_t   padiwaConfig = -1;
  Module  module       = Module();

  std::vector<Fiber> fibers;

  Int_t fiberMult(0), layer(-1), x(-1), y(-1);

  data->SetBranchAddress("Events", &event);

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  std::vector<TH2D*> totLayerVec{};
  std::vector<TH2D*> timeLayerVec{};
  for(Int_t i=0; i<8; i++) {
    totLayerVec.emplace_back(new TH2D(Form("hToTL%i",i+1),Form("ToT distribution of first signals vs fiber in L%i;fiber;ToT",i+1),33,0,33,500,0,50));
    timeLayerVec.emplace_back(new TH2D(Form("hTimeL%i",i+1),Form("TimeStamp distribution of first signals vs fiber in L%i;fiber;ToT",i+1),33,0,33,200000,0,200000));
  }

  TH1F* hFiberMult   = new TH1F("hFiberMult","Number of fibers with at least one signal;n fibers",128,0,128);

  printf("events to process: %lu\t %.1f%% of the file\n", nEvents, Float_t(100*nEvents)/Float_t(data->GetEntries()));

  for (ULong_t entry = 0; entry < nEvents; entry++) {
    if ((((entry+1)%100000) == 0) || (entry == (nEvents-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
    }

    data->GetEntry(entry);

    eventNr      = event->getEventNr();
    padiwaConfig = event->getPadiwaConfig();
    module       = event->getModule();
    fibers       = module.getFibers();

    fiberMult = 0;

    for(auto& fiber : fibers) {
      if(fiber.getSignals().size() > 0) {
        layer = fiber.getLayer();
        fiberMult++;
      }

      for(auto& signal : fiber.getSignals()) {
        if(signal.getSignalNr() == 1) {
          //if (signal.getToT()<0) { printf("ToT: %g\n", signal.getToT()); }
          if(signal.getTimeStamp()*-1>1e10){ printf("Layer: %i, ChID: %i, TimeStamp: %g\n",layer,signal.getTDCID(),signal.getTimeStamp() ); }
          Float_t fiberNr = mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID());
          totLayerVec.at(layer-1)->Fill(fiberNr, signal.getToT());
          timeLayerVec.at(layer-1)->Fill(fiberNr, signal.getTimeStamp()*-1);
        }
      } /// loop over signals in fiber

    } /// loop over fibers in module

    hFiberMult->Fill(fiberMult);

  } /// loop over file

  Int_t histCounter = 0;
  fout->WriteObject(hFiberMult,  "hFiberMult");
  for(auto& hist : totLayerVec) { if(hist->GetEntries() != 0) { fout->WriteObject(hist, hist->GetName()); histCounter++; } }
  for(auto& hist : timeLayerVec) { if(hist->GetEntries() != 0) { fout->WriteObject(hist, hist->GetName()); } }


  TCanvas *c1 = new TCanvas("cToTDists","cToTDists");
  c1->DivideSquare(histCounter);

  Int_t padIter = 1;
  for(auto& hist : totLayerVec) {
    if(hist->GetEntries() == 0) { continue; }
    c1->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  fout->WriteObject(c1, c1->GetName());

  fout->Close();
}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="plotCTSEvent_output.root";
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

  printf("\n\n%sRunning plotCTSEvent%s\n\n",text::BOLD,text::RESET);
  
  plotCTSEvent(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}