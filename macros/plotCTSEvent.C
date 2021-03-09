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

  TH1F* hFiberMult   = new TH1F("hFiberMult","Number of fibers with at least one signal;n fibers",128,0,128);
  TH2D* hToTfirstL1  = new TH2D("hToTfirstL1","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);
  TH2D* hToTfirstL2  = new TH2D("hToTfirstL2","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);
  TH2D* hToTfirstL3  = new TH2D("hToTfirstL3","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);
  TH2D* hToTfirstL4  = new TH2D("hToTfirstL4","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);
  TH2D* hTimefirstL1  = new TH2D("hTimefirstL1","TimeStamp distribution of first signals vs fiber;fiber;ToT",33,0,33,200000,0,200000);
  TH2D* hTimefirstL2  = new TH2D("hTimefirstL2","TimeStamp distribution of first signals vs fiber;fiber;ToT",33,0,33,200000,0,200000);
  TH2D* hTimefirstL3  = new TH2D("hTimefirstL3","TimeStamp distribution of first signals vs fiber;fiber;ToT",33,0,33,200000,0,200000);
  TH2D* hTimefirstL4  = new TH2D("hTimefirstL4","TimeStamp distribution of first signals vs fiber;fiber;ToT",33,0,33,200000,0,200000);

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
        x     = fiber.getX();
        y     = fiber.getY();
        fiberMult++;
      }

      for(auto& signal : fiber.getSignals()) {
        if(signal.getSignalNr() == 1) {
          //if (signal.getToT()<0) { printf("ToT: %g\n", signal.getToT()); }
          if(signal.getTimeStamp()*-1>1e10){ printf("Layer: %i, ChID: %i, TimeStamp: %g\n",layer,signal.getTDCID(),signal.getTimeStamp() ); }
          if     (layer == 1) { hToTfirstL1->Fill(mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID()),signal.getToT()); 
                                hTimefirstL1->Fill(x,signal.getTimeStamp()*-1);
                              }
          else if(layer == 2) { hToTfirstL2->Fill(mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID()),signal.getToT()); 
                                hTimefirstL2->Fill(y,signal.getTimeStamp()*-1);
                              }
          else if(layer == 3) { hToTfirstL3->Fill(mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID()),signal.getToT()); 
                                hTimefirstL3->Fill(x,signal.getTimeStamp()*-1);
                              }
          else if(layer == 4) { hToTfirstL4->Fill(mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID()),signal.getToT());
                                hTimefirstL4->Fill(y,signal.getTimeStamp()*-1);
                              }
          else { printf("\n\n%sNo histogram for given layer!%s", text::BLU, text::RESET); }
        }
      } /// loop over signals in fiber

    } /// loop over fibers in module

    hFiberMult->Fill(fiberMult);

  } /// loop over file

  fout->WriteObject(hFiberMult,  "hFiberMult");
  fout->WriteObject(hToTfirstL1, "hToTfirstL1");
  fout->WriteObject(hToTfirstL2, "hToTfirstL2");
  fout->WriteObject(hToTfirstL3, "hToTfirstL3");
  fout->WriteObject(hToTfirstL4, "hToTfirstL4");
  fout->WriteObject(hTimefirstL1, "hTimefirstL1");
  fout->WriteObject(hTimefirstL2, "hTimefirstL2");
  fout->WriteObject(hTimefirstL3, "hTimefirstL3");
  fout->WriteObject(hTimefirstL4, "hTimefirstL4");

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