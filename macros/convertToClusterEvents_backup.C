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
  Int_t   correctEvent = -1;
  Int_t   numDeleted = -1;

  Double_t   timeWindow = 15;

  std::vector<Cluster> clusters;
  std::vector<Cluster> clusterBuffer;
  std::vector<Double_t> eventTime;
  std::vector<Int_t> foundClusters;

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");
  TTree *treeout = new TTree("dummy","RadMap data in CTSEventClusters -> ClusterEvents");

  CTSEventClusters *ctsEventCluster;
  ctsEventCluster = new CTSEventClusters();
  ClusterEvent *clusterEvent;
  clusterEvent = new ClusterEvent();

  TH1D* hTimeDiff = new TH1D("hTimeDiff","hTimeDiff;Time [ns]",10000,0,1000);

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
    correctEvent = -1;  
    //printf("New event:\n");
    clusters = ctsEventCluster->getClusters(); //get all clusters in CTSEventsCluster 
    //printf("0. Found clusters in event:%i &&&&&&&&&&&&&&&&&&&&&&&&&&&\n", int(clusters.size()));
    for(auto& cluster : clusters){
      if(!isnan(cluster.getMeanTimeStamp()) && cluster.getMeanTimeStamp()<1e6){
        if (cluster.getNSignals()!= 1) { continue; }
        clusterBuffer.emplace_back(cluster);
        eventTime.emplace_back(cluster.getMeanTimeStamp());
      }
      else{printf("Layer:%i, Fib:%1.2f, Time:%g, ToT:%g \n", cluster.getLayer(), cluster.getMeanFiber(), cluster.getMeanTimeStamp(), cluster.getQMax());}
    } // Make a list of cluster timestamps
    
    while(!eventTime.empty()){ //Do while there are clusters to distribute to an event 
      eventNrCluster++;
      //printf("1. EventNr: %i\n", eventNrCluster);
      clusterEvent->reset();
      foundClusters.clear();
      // Make a list of the positions of clusters that are within the time window
      for(int i=0; i<eventTime.size(); i++){ if(std::abs(eventTime.front()-eventTime.at(i))<timeWindow){ foundClusters.emplace_back(i); } }
      //printf("2. #of clusters in time window: %i\n", int(foundClusters.size()));

      // Add found clusters to an event  
      for(auto& position : foundClusters){ clusterEvent->addCluster(clusterBuffer.at(position)); }
        //printf("3. #of clusters in event: %i\n", int(clusterEvent->getClusters().size()));

      // Remove added clusters from clusters list and their timestamps from the time list
      numDeleted=0;
      for(auto& position : foundClusters){ 
        clusterBuffer.erase(clusterBuffer.begin()+position-numDeleted); 
        eventTime.erase(eventTime.begin()+position-numDeleted); 
        numDeleted++;
        //printf("4. #of remaining clusters: %i\n", int(clusterBuffer.size()));
      }
      //printf("5. #of remaining clusters: %i\n", int(clusterBuffer.size()));

      // Add created event to the tree
      clusterEvent->setEventNrCluster(eventNrCluster);
      treeout->Fill();

      for(int i=1; i<clusterEvent->getClusters().size();i++){
        hTimeDiff->Fill(std::abs(clusterEvent->getClusters().at(i).getMeanTimeStamp()-clusterEvent->getClusters().front().getMeanTimeStamp()));
      }
    }
    
  } /// loop over file
  printf("\n\nCTSEvents: %i, ClusterEvents: %i\n\n", int(nEvents), int(eventNrCluster));
  treeout->Write("data");
  fout->WriteObject(hTimeDiff,"hTimeDiff");
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