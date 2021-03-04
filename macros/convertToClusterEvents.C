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
#include <math.h>

#include "CTSEvent.h"
#include "CTSEventClusters.h"
#include "ClusterEvent.h"
#include "Utility.h"

///< usage: ./convertToClusterEvents -i inputfile -o outputfile -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

static bool useEdges = false;
static Int_t yLayerUpperCut = 24;
static Int_t yLayerLowerCut = 8;
static Int_t xLayerUpperCut = 24;
static Int_t xLayerLowerCut = 4;
static Int_t timeWindow     = 2;
static Int_t spaceCut       = 2;

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

  Int_t eventNrCluster = -1;
  Int_t   padiwaConfig = -1;
  Int_t   numDeleted = -1;

  Int_t nClusters = 0;
  Int_t usedClusters = 0;
  Float_t refFiberX = 0;
  Float_t refFiberY = 0;
  Double_t refTime = 0;

  bool foundL1 = false;
  bool foundL2 = false;
  bool foundL3 = false;
  bool foundL4 = false;

  std::vector<Cluster> clusters;
  std::vector<Cluster> clusterBuffer;
  std::vector<Cluster> layerOneBuffer;
  std::vector<Cluster> layerTwoBuffer;
  std::vector<Cluster> layerThreeBuffer;
  std::vector<Cluster> layerFourBuffer;
  std::vector<Double_t> eventTime;
  std::vector<Int_t> foundClusters;

  Cluster bufferedCluster;
  Int_t bufferedIndex = -1;
  Double_t bufferedTimeDiff = 1e20;

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");
  TTree *treeout = new TTree("dummy","RadMap data in CTSEventClusters -> ClusterEvents");

  CTSEventClusters *ctsEventCluster;
  ctsEventCluster = new CTSEventClusters();
  ClusterEvent *clusterEvent;
  clusterEvent = new ClusterEvent();

  TH1D* hTimeDiff = new TH1D("hTimeDiff","hTimeDiff;Time [ns]",10000,0,1000);
  TH1D* hNClusInCTSEvent = new TH1D("hNClusInCTSEvent",";n;counts",100,0,100);

  data->SetBranchAddress("CTSEventsCluster", &ctsEventCluster);
  treeout->Branch("ClusterEvents","ClusterEvent",clusterEvent,32000,1);

  printf("events to process: %lu\t %.1f%% of the file\n", nEvents, Float_t(100*nEvents)/Float_t(data->GetEntries()));


  for (ULong_t entry = 0; entry < nEvents; entry++) {
    if ((((entry+1)%1000) == 0) || (entry == (nEvents-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
    }
    data->GetEntry(entry);
    eventTime.clear();
    clusterBuffer.clear();
    layerOneBuffer.clear();
    layerTwoBuffer.clear();
    layerThreeBuffer.clear();
    layerFourBuffer.clear();
    bool clustersLeft = true;
    bool foundL1 = false;
    bool foundL2 = false;
    bool foundL3 = false;
    bool foundL4 = false;
    //printf("New event:\n");
    clusters = ctsEventCluster->getClusters(); //get all clusters in CTSEventsCluster 
    //printf("0. Found clusters in event:%i &&&&&&&&&&&&&&&&&&&&&&&&&&&\n", int(clusters.size()));
    for(auto& cluster : clusters){
      if(!isnan(cluster.getMeanTimeStamp()) && cluster.getMeanTimeStamp()<1e6){
        if (cluster.getNSignals()!= 1) { continue; }
        //clusterBuffer.emplace_back(cluster);
        Int_t tmpLayer = cluster.getLayer();
        if (tmpLayer == 1) { layerOneBuffer.emplace_back(std::move(cluster)); }
        else if (tmpLayer == 2) { layerTwoBuffer.emplace_back(std::move(cluster)); }
        else if (tmpLayer == 3) { layerThreeBuffer.emplace_back(std::move(cluster)); }
        else if (tmpLayer == 4) { layerFourBuffer.emplace_back(std::move(cluster)); }
        else { printf("Unhandled layer: %i\n", tmpLayer); }
      }
      else{printf("Layer:%i, Fib:%1.2f, Time:%g, ToT:%g \n", cluster.getLayer(), cluster.getMeanFiber(), cluster.getMeanTimeStamp(), cluster.getQMax());}
    } // Make a list of cluster timestamps

    nClusters = layerOneBuffer.size()+layerTwoBuffer.size()+layerThreeBuffer.size()+layerFourBuffer.size();
    hNClusInCTSEvent->Fill(nClusters);
    clusterEvent->reset();

    for (auto& cluster : layerOneBuffer) {
      refFiberY = cluster.getMeanFiber();
      refTime   = cluster.getMeanTimeStamp();
      if (!useEdges) { 
        if (refFiberY<yLayerLowerCut || refFiberY>yLayerUpperCut) {
          cluster.setIsUsed();
          continue;
        }
      }
      clusterEvent->addCluster(cluster);
      cluster.setIsUsed();
      //printf("Working layer1\n");
      bufferedCluster = Cluster();
      bufferedIndex = -1;
      bufferedTimeDiff = 1e20;
      for (Int_t i=0; i<layerTwoBuffer.size(); i++) {
        if(!(layerTwoBuffer.at(i).isUsed())) {
          refFiberX      = layerTwoBuffer.at(i).getMeanFiber();
          if (!useEdges) { 
            if (refFiberX<xLayerLowerCut || refFiberX>xLayerUpperCut) {
              layerTwoBuffer.at(i).setIsUsed();
              continue;
            }
          }
          Double_t time  = layerTwoBuffer.at(i).getMeanTimeStamp();
          Double_t timeDiff = std::abs(time-refTime);
          if (timeDiff<timeWindow) {
            if (timeDiff<bufferedTimeDiff) {
              bufferedCluster = layerTwoBuffer.at(i);
              bufferedIndex = i;
              bufferedTimeDiff = timeDiff;
            }
          }
        }
      }
      //printf("After layer2\n");
      if (bufferedIndex != -1) {
        clusterEvent->addCluster(bufferedCluster);
        layerTwoBuffer.at(bufferedIndex).setIsUsed();
      }
      //printf("After layer2 2\n");
      bufferedCluster = Cluster();
      bufferedIndex = -1;
      bufferedTimeDiff = 1e20;

      for (Int_t i=0; i<layerThreeBuffer.size(); i++) {
        if(!(layerThreeBuffer.at(i).isUsed())) {
          Float_t  fiber = layerThreeBuffer.at(i).getMeanFiber();
          if (!useEdges) { 
            if (fiber<yLayerLowerCut || fiber>yLayerUpperCut) {
              layerThreeBuffer.at(i).setIsUsed();
              continue;
            }
          }
          Double_t time  = layerThreeBuffer.at(i).getMeanTimeStamp();
          Double_t timeDiff = std::abs(time-refTime);
          if ((std::abs(fiber-refFiberY)<spaceCut) && (timeDiff<timeWindow)) {
            if (timeDiff<bufferedTimeDiff) {
              bufferedCluster = layerThreeBuffer.at(i);
              bufferedIndex = i;
              bufferedTimeDiff = timeDiff;
            }
          }
        }
      }
      //printf("After layer3\n");
      if (bufferedIndex != -1) {
        clusterEvent->addCluster(bufferedCluster);
        layerThreeBuffer.at(bufferedIndex).setIsUsed();
      }
      //printf("After layer3 2\n");
      bufferedCluster = Cluster();
      bufferedIndex = -1;
      bufferedTimeDiff = 1e20;

      for (Int_t i=0; i<layerFourBuffer.size(); i++) {
        if(!(layerFourBuffer.at(i).isUsed())) {
          Float_t  fiber = layerFourBuffer.at(i).getMeanFiber();
          if (!useEdges) { 
            if (fiber<xLayerLowerCut || fiber>xLayerUpperCut) {
              layerFourBuffer.at(i).setIsUsed();
              continue;
            }
          }
          Double_t time  = layerFourBuffer.at(i).getMeanTimeStamp();
          Double_t timeDiff = std::abs(time-refTime);
          if ((std::abs(fiber-refFiberX)<spaceCut) && (timeDiff<timeWindow)) {
            if (timeDiff<bufferedTimeDiff) {
              bufferedCluster = layerFourBuffer.at(i);
              bufferedIndex = i;
              bufferedTimeDiff = timeDiff;
            }
          }
        }
      }
      //printf("After layer4\n");
      if (bufferedIndex != -1) {
        clusterEvent->addCluster(bufferedCluster);
        layerFourBuffer.at(bufferedIndex).setIsUsed();
      }
      //printf("After layer4 2\n");
      bufferedCluster = Cluster();
      bufferedIndex = -1;
      bufferedTimeDiff = 1e20;

      eventNrCluster++;
      clusterEvent->setEventNrCluster(eventNrCluster);
      treeout->Fill();
      clusterEvent->reset();
    }
    //printf("socond round, done with layer 1\n");
    for (auto& cluster2 : layerTwoBuffer) {
      if(!(cluster2.isUsed())) {
        refFiberX      = cluster2.getMeanFiber();
        if (!useEdges) { 
            if (refFiberX<xLayerLowerCut || refFiberX>xLayerUpperCut) {
              cluster2.setIsUsed();
              continue;
            }
          }
        refTime        = cluster2.getMeanTimeStamp();
        foundL2 = true;
        clusterEvent->addCluster(cluster2);
        cluster2.setIsUsed();
        for (Int_t j=0; j<layerThreeBuffer.size(); j++) {
          if(!(layerThreeBuffer.at(j).isUsed())) {
            Float_t  fiber = layerThreeBuffer.at(j).getMeanFiber();
            if (!useEdges) { 
              if (fiber<yLayerLowerCut || fiber>yLayerUpperCut) {
                layerThreeBuffer.at(j).setIsUsed();
                continue;
              }
            }
            Double_t time  = layerThreeBuffer.at(j).getMeanTimeStamp();
            Double_t timeDiff = std::abs(time-refTime);
            if (timeDiff<timeWindow) {
              if (timeDiff<bufferedTimeDiff) {
                bufferedCluster = layerThreeBuffer.at(j);
                bufferedIndex = j;
                bufferedTimeDiff = timeDiff;
              }
            }
          }
        }
        if (bufferedIndex != -1) {
          clusterEvent->addCluster(bufferedCluster);
          layerThreeBuffer.at(bufferedIndex).setIsUsed();
        }
        bufferedCluster = Cluster();
        bufferedIndex = -1;
        bufferedTimeDiff = 1e20;

        for (Int_t j=0; j<layerFourBuffer.size(); j++) {
          if(!(layerFourBuffer.at(j).isUsed())) {
            Float_t  fiber = layerFourBuffer.at(j).getMeanFiber();
            if (!useEdges) { 
              if (fiber<xLayerLowerCut || fiber>xLayerUpperCut) {
                layerFourBuffer.at(j).setIsUsed();
                continue;
              }
            }
            Double_t time  = layerFourBuffer.at(j).getMeanTimeStamp();
            Double_t timeDiff = std::abs(time-refTime);
            if ((std::abs(fiber-refFiberY)<spaceCut) && (timeDiff<timeWindow)) {
              if (timeDiff<bufferedTimeDiff) {
                bufferedCluster = layerFourBuffer.at(j);
                bufferedIndex = j;
                bufferedTimeDiff = timeDiff;
              }
            }
          }
        }
        if (bufferedIndex != -1) {
          clusterEvent->addCluster(bufferedCluster);
          layerFourBuffer.at(bufferedIndex).setIsUsed();
        }
        bufferedCluster = Cluster();
        bufferedIndex = -1;
        bufferedTimeDiff = 1e20;

        eventNrCluster++;
        clusterEvent->setEventNrCluster(eventNrCluster);
        treeout->Fill();
        clusterEvent->reset();
      }
    }
    //printf("third round, done with layer 2\n");
    for (auto& cluster3 : layerThreeBuffer) {
      if(!(cluster3.isUsed())) {
        Float_t fiber  = cluster3.getMeanFiber();
        if (!useEdges) { 
          if (fiber<yLayerLowerCut || fiber>yLayerUpperCut) {
            cluster3.setIsUsed();
            continue;
          }
        }
        refTime        = cluster3.getMeanTimeStamp();
        clusterEvent->addCluster(cluster3);
        cluster3.setIsUsed();
        foundL3 = true;
        for (Int_t j=0; j<layerFourBuffer.size(); j++) {
          if(!(layerFourBuffer.at(j).isUsed())) {
            Float_t  fiber = layerFourBuffer.at(j).getMeanFiber();
            if (!useEdges) { 
              if (fiber<xLayerLowerCut || fiber>xLayerUpperCut) {
              layerFourBuffer.at(j).setIsUsed();
              continue;
              }
            }
            Double_t time  = layerFourBuffer.at(j).getMeanTimeStamp();
            Double_t timeDiff = std::abs(time-refTime);
            if (timeDiff<timeWindow) {
              if (timeDiff<bufferedTimeDiff) {
                bufferedCluster = layerFourBuffer.at(j);
                bufferedIndex = j;
                bufferedTimeDiff = timeDiff;
              }
            }
          }
        }
        if (bufferedIndex != -1) {
          clusterEvent->addCluster(bufferedCluster);
          layerFourBuffer.at(bufferedIndex).setIsUsed();
        }
        bufferedCluster = Cluster();
        bufferedIndex = -1;
        bufferedTimeDiff = 1e20;

        eventNrCluster++;
        clusterEvent->setEventNrCluster(eventNrCluster);
        treeout->Fill();
        clusterEvent->reset();
      }
    }
    //printf("fourth round, done with layer 3\n");
    for (auto& cluster4 : layerFourBuffer) {
      if(!(cluster4.isUsed())) {
        Float_t fiber = cluster4.getMeanFiber();
        if (!useEdges) { 
          if (fiber<xLayerLowerCut || fiber>xLayerUpperCut) {
            cluster4.setIsUsed();
            continue;
          }
        }
        clusterEvent->addCluster(cluster4);
        cluster4.setIsUsed();
        eventNrCluster++;
        clusterEvent->setEventNrCluster(eventNrCluster);
        treeout->Fill();
        clusterEvent->reset();
      }
    }

  } /// loop over file
  printf("\n\nCTSEvents: %i, ClusterEvents: %i\n\n", int(nEvents), int(eventNrCluster));
  treeout->Write("data");
  fout->WriteObject(hTimeDiff,"hTimeDiff");
  fout->WriteObject(hNClusInCTSEvent, "hNClusInCTSEvent");
  fout->Close();

  delete clusterEvent;
  clusterEvent=nullptr;

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