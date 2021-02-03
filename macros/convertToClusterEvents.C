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

///< usage: ./convertToClusterEvents -i inputfile -o outputfile -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

void convertToClusterEvents(const char *inputFile, const char *outputFile, ULong_t procNr)
{
  TFile* f = TFile::Open(inputFile);

  if (f->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input file failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TTree *data = (TTree*)f->Get("data");

  ULong_t nEvents = procNr;
  if ((nEvents == -1) || (nEvents > data->GetEntries())) { nEvents = data->GetEntries(); }

  Float_t eventNr      = -1;
  Int_t   padiwaConfig = -1;
  Module  module       = Module();
  Clusterer clusterer = Clusterer();

  std::vector<Cluster> clusters; 
  std::vector<Fiber> fibers;

  Int_t fiberMult(0), layer(-1), x(-1), y(-1), clusterCounter(0);

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");
  TTree *treeout = new TTree("dummy","RadMap data in fancy objects -> Cluster Events");

  CTSEvent *event;
  CTSEvent eventBuffer;
  event = new CTSEvent();
  eventBuffer = CTSEvent();
  CTSEventClusters *clusterEvent;
  clusterEvent = new CTSEventClusters();

  data->SetBranchAddress("Events", &event);
  treeout->Branch("ClusterEvents","ClusterEvent",clusterEvent,32000,1);

  printf("events to process: %lu\t %.1f%% of the file\n", nEvents, Float_t(100*nEvents)/Float_t(data->GetEntries()));


  for (ULong_t entry = 0; entry < nEvents; entry++) {
    if ((((entry+1)%1000) == 0) || (entry == (nEvents-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
    }
    clusterCounter = 0;
    data->GetEntry(entry);
    eventNr      = event->getEventNr();
    padiwaConfig = event->getPadiwaConfig();
    module       = event->getModule();
    fibers       = module.getFibers();
    eventBuffer.setModule(module);

    clusterer.findClusters(eventBuffer);
    clusters = clusterer.getClusters();

    for(auto& cluster : clusters){
      if(cluster.getNSignals() > clusterCounter) { clusterCounter = cluster.getNSignals();}
      //clusterEvent->addCluster(cluster);  
    }
    clusterEvent->addCluster(clusters);
    treeout->Fill();
    module.reset();
    clusterer.reset();
    
  } /// loop over file

  treeout->Write("data");
  fout->WriteObject(clusterer.MhTimeDiff,"MhTimeDiff");

  fout->Close();

  delete clusterEvent;
  clusterEvent=nullptr;

  printf("\n\n High multiplicity cluster counter:%i\n\n", clusterCounter);

}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="convertToClusterEvents_output.root";
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

  printf("\n\n%sRunning convertToClusterEvents%s\n\n",text::BOLD,text::RESET);
  
  convertToClusterEvents(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}