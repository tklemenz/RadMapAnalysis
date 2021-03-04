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

///< usage: ./getEfficiency -i inputfile -o outputfile -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed
///< This macro runs on PADIWA config 2 data --> 3 full layers in Y-direction (1, 3, 5) and one fill layer in X-direction (2)

extern char* optarg;

void getEfficiency(const char *inputFile, const char *outputFile, ULong_t procNr)
{
  TFile* f = TFile::Open(inputFile);

  if (f->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input file failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TTree *data = (TTree*)f->Get("data");

  ULong_t nEvents = procNr;
  if ((nEvents == -1) || (nEvents > data->GetEntries())) { nEvents = data->GetEntries(); }

  /* Define variables
  ==========================================================
  ==========================================================*/
  Float_t eventNr      = -1;
  Int_t   padiwaConfig = -1;
  Module  module = Module();
  CTSEvent *event = nullptr;
  std::vector<Fiber> fibers;

  Int_t layer   = -1;
  Int_t x       = -1;
  Int_t y       = -1;
  Int_t fiberNr = -1;

  Double_t tot  = 0;
  Double_t time = 0;
  /*========================================================
  ==========================================================*/

  /* Define histograms
  ==========================================================
  ==========================================================*/
  TH1I* hLayerDist = new TH1I("hLayerDist","Signal distribution across layers; layer;counts",8,0,8);
  TH2D* hToTL1 = new TH2D("hToTL1","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);
  TH2D* hToTL2 = new TH2D("hToTL2","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);
  TH2D* hToTL3 = new TH2D("hToTL3","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);
  TH2D* hToTL5 = new TH2D("hToTL5","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);
  /*========================================================
  ==========================================================*/

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  data->SetBranchAddress("Events", &event);

  printf("events to process: %lu\t %.1f%% of the file\n", nEvents, Float_t(100*nEvents)/Float_t(data->GetEntries()));

  for (ULong_t entry = 0; entry < nEvents; entry++) {
    /* Make some fancy terminal output */
    if ((((entry+1)%10000) == 0) || (entry == (nEvents-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
    } /* End of terminal output*/

    data->GetEntry(entry);

    /* Reset variables for each event */
    //fibers.clear();
    //module.reset(); // this method is very slow!!
    eventNr      = -1;
    padiwaConfig = -1;

    /* Extract all info from event */
    eventNr      = event->getEventNr();
    padiwaConfig = event->getPadiwaConfig();
    module       = event->getModule();
    fibers       = module.getFibers();

    for(auto& fiber : fibers) {
      if(fiber.getSignals().size() != 0) {
        layer   = fiber.getLayer();
        x       = fiber.getX();
        y       = fiber.getY();
        fiberNr = (x==0 ? y : x);

        hLayerDist->Fill(layer);

        for(auto& signal : fiber.getSignals()) {
          if(signal.getSignalNr() == 1) {
            tot = signal.getToT();
            switch(layer){
              case 1:
                hToTL1->Fill(fiberNr, tot);
                break;
              case 2:
                hToTL2->Fill(fiberNr, tot);
                break;
              case 3:
                hToTL3->Fill(fiberNr, tot);
                break;
              case 5:
                hToTL5->Fill(fiberNr, tot);
                break;
              default:
                printf("%s%sThis layer should not appear in PADIWA config 2!%s", text::RED, text::BOLD, text::RESET);
                break;
            } // end layer switch
          } // end if first signal
        } // end loop over signals
      } // end if fiber not empty
    } // loop over fibers
  } // loop over file

  fout->WriteObject(hLayerDist, "hLayerDist");
  fout->WriteObject(hToTL1, "hToTL1");
  fout->WriteObject(hToTL2, "hToTL2");
  fout->WriteObject(hToTL3, "hToTL3");
  fout->WriteObject(hToTL5, "hToTL5");
  fout->Close();

}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="getEfficiency_output.root";
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

  printf("\n\n%sRunning getEfficiency%s\n\n",text::BOLD,text::RESET);
  
  getEfficiency(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}