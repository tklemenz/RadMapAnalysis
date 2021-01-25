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
#include "CTSEventClusters.h"
#include "Utility.h"

///< usage: ./plotClusterEvent -i inputfile -o outputfile -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

void plotClusterEvent(const char *inputFile, const char *outputFile, ULong_t procNr)
{
  TFile* f = TFile::Open(inputFile);

  if (f->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input file failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TTree *data = (TTree*)f->Get("data");

  ULong_t nEvents = procNr;
  if ((nEvents == -1) || (nEvents > data->GetEntries())) { nEvents = data->GetEntries(); }

  CTSEventClusters *clusterEvent = nullptr;

  Float_t eventNr      = -1;
  Int_t   padiwaConfig = -1;
  Module  module       = Module();

  std::vector<Cluster> clusters;

  Int_t     clusterNSignal(-1), layer(-1);
  Double_t  qTot(-1), qMax(-1), timeMean(-1);
  Float_t   fibMean(-1);

  data->SetBranchAddress("ClusterEvents", &clusterEvent);

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  TH2D* hToTfibL1  = new TH2D("hToTfibL1","ToT distribution of clusters vs fiber;fiber;ToT",33,0,33,1000,0,50);
  TH2D* hToTfibL2  = new TH2D("hToTfibL2","ToT distribution of clusters vs fiber;fiber;ToT",33,0,33,1000,0,50);
  TH2D* hToTfibL3  = new TH2D("hToTfibL3","ToT distribution of clusters vs fiber;fiber;ToT",33,0,33,1000,0,50);
  TH2D* hToTfibL4  = new TH2D("hToTfibL4","ToT distribution of clusters vs fiber;fiber;ToT",33,0,33,1000,0,50);

  printf("events to process: %lu\t %.1f%% of the file\n", nEvents, Float_t(100*nEvents)/Float_t(data->GetEntries()));

  for (ULong_t entry = 0; entry < nEvents; entry++) {
    if ((((entry+1)%100000) == 0) || (entry == (nEvents-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
    }

    data->GetEntry(entry);

    clusters = clusterEvent->getClusters();

    for(auto& cluster : clusters) {

      clusterNSignal = cluster.getNSignals();
      layer = cluster.getLayer();
      qTot = cluster.getQTot();
      qMax = cluster.getQMax();
      timeMean = cluster.getMeanTimeStamp();
      fibMean = cluster.getMeanFiber();
      if     (layer == 1) { hToTfibL1->Fill(fibMean,qMax*1e9); }
      else if(layer == 2) { hToTfibL2->Fill(fibMean,qMax*1e9); }
      else if(layer == 3) { hToTfibL3->Fill(fibMean,qMax*1e9); }
      else if(layer == 4) { hToTfibL4->Fill(fibMean,qMax*1e9); }
      else { printf("\n\n%sNo histogram for given layer!%s", text::BLU, text::RESET); }

    } /// loop over fibers in module

  } /// loop over file

  fout->WriteObject(hToTfibL1, "hToTfibL1");
  fout->WriteObject(hToTfibL2, "hToTfibL2");
  fout->WriteObject(hToTfibL3, "hToTfibL3");
  fout->WriteObject(hToTfibL4, "hToTfibL4");

  fout->Close();
}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="plotClusterEvent_output.root";
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

  printf("\n\n%sRunning plotClusterEvent%s\n\n",text::BOLD,text::RESET);
  
  plotClusterEvent(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}