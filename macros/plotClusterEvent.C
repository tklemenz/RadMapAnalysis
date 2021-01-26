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
  Bool_t    layerseen[4];
  Int_t     total(0),multi1(0),multi2(0),multi3(0),multi4(0),only1(0),only2(0),only3(0),only4(0),both34(0),buffer(0);
  Int_t     l12(0),l13(0),l14(0),l23(0),l24(0),l34(0);
  Int_t     l123(0),l124(0),l134(0),l234(0);

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
    for(int i=0;i<4;i++){ layerseen[i]=false; }

    for(auto& cluster : clusters) {

      clusterNSignal = cluster.getNSignals();
      layer = cluster.getLayer();
      qTot = cluster.getQTot();
      qMax = cluster.getQMax();
      timeMean = cluster.getMeanTimeStamp();
      fibMean = cluster.getMeanFiber();
      if     (layer == 1) { hToTfibL1->Fill(fibMean,qMax); layerseen[0] = true;}
      else if(layer == 2) { hToTfibL2->Fill(fibMean,qMax); layerseen[1] = true;}
      else if(layer == 3) { hToTfibL3->Fill(fibMean,qMax); layerseen[2] = true;}
      else if(layer == 4) { hToTfibL4->Fill(fibMean,qMax); layerseen[3] = true;}
      else { printf("\n\n%sNo histogram for given layer!%s", text::BLU, text::RESET); }

    } /// loop over fibers in module

    if(layerseen[0] == true && layerseen[1] == false && layerseen[2] == false && layerseen[3] == false) { only1++; }
    if(layerseen[0] == false && layerseen[1] == true && layerseen[2] == false && layerseen[3] == false) { only2++; }
    if(layerseen[0] == false && layerseen[1] == false && layerseen[2] == true && layerseen[3] == false) { only3++; }
    if(layerseen[0] == false && layerseen[1] == false && layerseen[2] == false && layerseen[3] == true) { only4++; }

    if(layerseen[0] == true && layerseen[1] == true && layerseen[2] == true && layerseen[3] == false) { l123++; }
    if(layerseen[0] == true && layerseen[1] == true && layerseen[2] == false && layerseen[3] == true) { l124++; }
    if(layerseen[0] == true && layerseen[1] == false && layerseen[2] == true && layerseen[3] == true) { l134++; }
    if(layerseen[0] == false && layerseen[1] == true && layerseen[2] == true && layerseen[3] == true) { l234++; }

    for(int i=0;i<4;i++){ if(layerseen[i]==true) { buffer++; } }
    switch(buffer){
      case 1: multi1++; break;
      case 2: multi2++; break;
      case 3: multi3++; break;
      case 4: multi4++; break;
    }

    if(buffer==2){
      if(layerseen[0] == true && layerseen[1] == true){ l12++; }
      if(layerseen[0] == true && layerseen[2] == true){ l13++; }
      if(layerseen[0] == true && layerseen[3] == true){ l14++; }
      if(layerseen[1] == true && layerseen[2] == true){ l23++; }
      if(layerseen[1] == true && layerseen[3] == true){ l24++; }
      if(layerseen[2] == true && layerseen[3] == true){ l34++; }
    }
    total+=buffer;
    buffer=0;


  } /// loop over file

  printf("\n\n");
  printf("Events: %i, Total:%i\n",int(nEvents), int(total));
  printf("Multiplicity=1:%i,Multiplicity=2:%i,Multiplicity=3:%i,Multiplicity=4:%i\n",multi1,multi2,multi3,multi4);
  printf("Only 1: %i,Only 2: %i,Only 3: %i,Only 4: %i\n", only1,only2,only3,only4);
  printf("L12:%i,L13:%i,L14:%i,L23:%i,L24:%i,L34:%i\n",l12,l13,l14,l23,l24,l34);
  printf("L123:%i, L124:%i L134:%i, L234:%i\n", l123,l124,l134,l234);
  printf("\n\n");

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