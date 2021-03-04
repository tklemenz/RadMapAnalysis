#include "Clusterer.h"
#include "Utility.h"

#include <vector> 
#include <algorithm>

ClassImp(Clusterer);

void Clusterer::findClusters(CTSEvent& event)
{
  // Data available within the CTS event
  Float_t eventNr      = -1;
  Int_t   padiwaConfig = -1;
  Module  module       = Module();
  std::vector<Fiber> fibers;
  eventNr      = event.getEventNr();
  padiwaConfig = event.getPadiwaConfig();
  module       = event.getModule();
  fibers       = module.getFibers();
  
  std::vector<Double_t> distances;
  std::vector<Double_t> time_distances;
  Double_t min_dist_index=-1;
  Double_t min_time_index=-1; 
  
  Int_t x(-1), y(-1), layer(-1);

  Double_t cluster_fib_range  = 2;   // The allowed range in fiber distance for cluster building
  Double_t cluster_time_range = 12;  // The allowed time window for cluster building [ns], since the calibrated data is already in ns
  Double_t tot_uppercut       = 23; // Make sure there is no total BS
  Double_t tot_lowercut       = 10;  // Get rid of noise

  // Clusters can only contain signals from the same layer
  // Depending on the padiwa configuration a few of the vectors will always be empty
  std::vector<Cluster> layer1clusters;
  std::vector<Cluster> layer2clusters;
  std::vector<Cluster> layer3clusters;
  std::vector<Cluster> layer4clusters;
  std::vector<Cluster> layer5clusters;
  std::vector<Cluster> layer6clusters;
  std::vector<Cluster> layer7clusters;
  std::vector<Cluster> layer8clusters;

  for(auto& fiber : fibers) {
    if(fiber.getNSignals() > 0) {
      layer = fiber.getLayer();
      x     = fiber.getX();
      y     = fiber.getY();
      for(auto& signal : fiber.getSignals()) {
        if(signal.getSignalNr()==1) { 
          if(signal.getToT()<tot_lowercut || signal.getToT()>tot_uppercut) { continue; }  //allow only good signals
          // Since the signals are separeted in fibers but not layer,
          // this switch makes sure only signals from same layer are compared
          switch(layer){  
            case 1: if(layer1clusters.size()==0){
                      layer1clusters.emplace_back(Cluster());
                      layer1clusters.at(0).addSignal(signal);
                    }
                    else{
                      distances.clear();
                      min_dist_index=-1;
                      time_distances.clear();
                      min_time_index=-1;
                      for(auto& cluster : layer1clusters){ 
                        distances.emplace_back(std::abs(cluster.getMeanFiber()-(x==0 ? y : x)));
                        time_distances.emplace_back(std::abs(cluster.getMeanTimeStamp()-signal.getTimeStamp()));
                      }
                      min_dist_index = std::min_element(distances.begin(),distances.end())-distances.begin();
                      min_time_index = std::min_element(time_distances.begin(),time_distances.end())-time_distances.begin();
                      MhTimeDiff->Fill(time_distances.at(min_time_index));
                      MhSpaceDiff->Fill(distances.at(min_dist_index));
                      if(distances.at(min_dist_index)<cluster_fib_range && time_distances.at(min_time_index)<cluster_time_range/* && min_dist_index==min_time_index*/){
                        layer1clusters.at(min_dist_index).addSignal(signal);
                      }
                      else{
                        layer1clusters.emplace_back(Cluster());
                        layer1clusters.back().addSignal(signal);
                      }
                    }
                    break;
            case 2: if(layer2clusters.size()==0){
                      layer2clusters.emplace_back(Cluster());
                      layer2clusters.at(0).addSignal(signal);
                    }
                    else{
                      distances.clear();
                      min_dist_index=-1;
                      time_distances.clear();
                      min_time_index=-1;
                      for(auto& cluster : layer2clusters){ 
                        distances.emplace_back(std::abs(cluster.getMeanFiber()-(x==0 ? y : x)));
                        time_distances.emplace_back(std::abs(cluster.getMeanTimeStamp()-signal.getTimeStamp()));
                      }
                      min_dist_index = std::min_element(distances.begin(),distances.end())-distances.begin();
                      min_time_index = std::min_element(time_distances.begin(),time_distances.end())-time_distances.begin();
                      MhTimeDiff->Fill(time_distances.at(min_time_index));
                      MhSpaceDiff->Fill(distances.at(min_dist_index));
                      if(distances.at(min_dist_index)<cluster_fib_range && time_distances.at(min_time_index)<cluster_time_range/* && min_dist_index==min_time_index*/){
                        layer2clusters.at(min_dist_index).addSignal(signal);
                      }
                      else{
                        layer2clusters.emplace_back(Cluster());
                        layer2clusters.back().addSignal(signal);
                      }
                    }
                    break;
            case 3: if(layer3clusters.size()==0){
                      layer3clusters.emplace_back(Cluster());
                      layer3clusters.at(0).addSignal(signal);
                    }
                    else{
                      distances.clear();
                      min_dist_index=-1;
                      time_distances.clear();
                      min_time_index=-1;
                      for(auto& cluster : layer3clusters){ 
                        distances.emplace_back(std::abs(cluster.getMeanFiber()-(x==0 ? y : x)));
                        time_distances.emplace_back(std::abs(cluster.getMeanTimeStamp()-signal.getTimeStamp()));
                      }
                      min_dist_index = std::min_element(distances.begin(),distances.end())-distances.begin();
                      min_time_index = std::min_element(time_distances.begin(),time_distances.end())-time_distances.begin();
                      MhTimeDiff->Fill(time_distances.at(min_time_index));
                      MhSpaceDiff->Fill(distances.at(min_dist_index));
                      if(distances.at(min_dist_index)<cluster_fib_range && time_distances.at(min_time_index)<cluster_time_range/* && min_dist_index==min_time_index*/){
                        layer3clusters.at(min_dist_index).addSignal(signal);
                      }
                      else{
                        layer3clusters.emplace_back(Cluster());
                        layer3clusters.back().addSignal(signal);
                      }
                    }
                    break;
            case 4: if(layer4clusters.size()==0){
                      layer4clusters.emplace_back(Cluster());
                      layer4clusters.at(0).addSignal(signal);
                    }
                    else{
                      distances.clear();
                      min_dist_index=-1;
                      time_distances.clear();
                      min_time_index=-1;
                      for(auto& cluster : layer4clusters){ 
                        distances.emplace_back(std::abs(cluster.getMeanFiber()-(x==0 ? y : x)));
                        time_distances.emplace_back(std::abs(cluster.getMeanTimeStamp()-signal.getTimeStamp()));
                      }
                      min_dist_index = std::min_element(distances.begin(),distances.end())-distances.begin();
                      min_time_index = std::min_element(time_distances.begin(),time_distances.end())-time_distances.begin();
                      MhTimeDiff->Fill(time_distances.at(min_time_index));
                      MhSpaceDiff->Fill(distances.at(min_dist_index));
                      if(distances.at(min_dist_index)<cluster_fib_range && time_distances.at(min_time_index)<cluster_time_range/* && min_dist_index==min_time_index*/){
                        layer4clusters.at(min_dist_index).addSignal(signal);
                      }
                      else{
                        layer4clusters.emplace_back(Cluster());
                        layer4clusters.back().addSignal(signal);
                      }
                    }
                    break;
            case 5: if(layer5clusters.size()==0){
                      layer5clusters.emplace_back(Cluster());
                      layer5clusters.at(0).addSignal(signal);
                    }
                    else{
                      distances.clear();
                      min_dist_index=-1;
                      time_distances.clear();
                      min_time_index=-1;
                      for(auto& cluster : layer5clusters){ 
                        distances.emplace_back(std::abs(cluster.getMeanFiber()-(x==0 ? y : x)));
                        time_distances.emplace_back(std::abs(cluster.getMeanTimeStamp()-signal.getTimeStamp()));
                      }
                      min_dist_index = std::min_element(distances.begin(),distances.end())-distances.begin();
                      min_time_index = std::min_element(time_distances.begin(),time_distances.end())-time_distances.begin();
                      MhTimeDiff->Fill(time_distances.at(min_time_index));
                      MhSpaceDiff->Fill(distances.at(min_dist_index));
                      if(distances.at(min_dist_index)<cluster_fib_range && time_distances.at(min_time_index)<cluster_time_range/* && min_dist_index==min_time_index*/){
                        layer5clusters.at(min_dist_index).addSignal(signal);
                      }
                      else{
                        layer5clusters.emplace_back(Cluster());
                        layer5clusters.back().addSignal(signal);
                      }
                    }
                    break;
            case 6: if(layer6clusters.size()==0){
                      layer6clusters.emplace_back(Cluster());
                      layer6clusters.at(0).addSignal(signal);
                    }
                    else{
                      distances.clear();
                      min_dist_index=-1;
                      time_distances.clear();
                      min_time_index=-1;
                      for(auto& cluster : layer6clusters){ 
                        distances.emplace_back(std::abs(cluster.getMeanFiber()-(x==0 ? y : x)));
                        time_distances.emplace_back(std::abs(cluster.getMeanTimeStamp()-signal.getTimeStamp()));
                      }
                      min_dist_index = std::min_element(distances.begin(),distances.end())-distances.begin();
                      min_time_index = std::min_element(time_distances.begin(),time_distances.end())-time_distances.begin();
                      MhTimeDiff->Fill(time_distances.at(min_time_index));
                      MhSpaceDiff->Fill(distances.at(min_dist_index));
                      if(distances.at(min_dist_index)<cluster_fib_range && time_distances.at(min_time_index)<cluster_time_range/* && min_dist_index==min_time_index*/){
                        layer6clusters.at(min_dist_index).addSignal(signal);
                      }
                      else{
                        layer6clusters.emplace_back(Cluster());
                        layer6clusters.back().addSignal(signal);
                      }
                    }
                    break;
            case 7: if(layer7clusters.size()==0){
                      layer7clusters.emplace_back(Cluster());
                      layer7clusters.at(0).addSignal(signal);
                    }
                    else{
                      distances.clear();
                      min_dist_index=-1;
                      time_distances.clear();
                      min_time_index=-1;
                      for(auto& cluster : layer7clusters){ 
                        distances.emplace_back(std::abs(cluster.getMeanFiber()-(x==0 ? y : x)));
                        time_distances.emplace_back(std::abs(cluster.getMeanTimeStamp()-signal.getTimeStamp()));
                      }
                      min_dist_index = std::min_element(distances.begin(),distances.end())-distances.begin();
                      min_time_index = std::min_element(time_distances.begin(),time_distances.end())-time_distances.begin();
                      MhTimeDiff->Fill(time_distances.at(min_time_index));
                      MhSpaceDiff->Fill(distances.at(min_dist_index));
                      if(distances.at(min_dist_index)<cluster_fib_range && time_distances.at(min_time_index)<cluster_time_range/* && min_dist_index==min_time_index*/){
                        layer7clusters.at(min_dist_index).addSignal(signal);
                      }
                      else{
                        layer7clusters.emplace_back(Cluster());
                        layer7clusters.back().addSignal(signal);
                      }
                    }
                    break;
            case 8: if(layer8clusters.size()==0){
                      layer8clusters.emplace_back(Cluster());
                      layer8clusters.at(0).addSignal(signal);
                    }
                    else{
                      distances.clear();
                      min_dist_index=-1;
                      time_distances.clear();
                      min_time_index=-1;
                      for(auto& cluster : layer8clusters){ 
                        distances.emplace_back(std::abs(cluster.getMeanFiber()-(x==0 ? y : x)));
                        time_distances.emplace_back(std::abs(cluster.getMeanTimeStamp()-signal.getTimeStamp()));
                      }
                      min_dist_index = std::min_element(distances.begin(),distances.end())-distances.begin();
                      min_time_index = std::min_element(time_distances.begin(),time_distances.end())-time_distances.begin();
                      MhTimeDiff->Fill(time_distances.at(min_time_index));
                      MhSpaceDiff->Fill(distances.at(min_dist_index));
                      if(distances.at(min_dist_index)<cluster_fib_range && time_distances.at(min_time_index)<cluster_time_range/* && min_dist_index==min_time_index*/){
                        layer8clusters.at(min_dist_index).addSignal(signal);
                      }
                      else{
                        layer8clusters.emplace_back(Cluster());
                        layer8clusters.back().addSignal(signal);
                      }
                    }
                    break;
          }
        }
      } /// loop over signals in fiber
    }
  } /// loop over fibers in module

  // Add clusters to final vector if they are not empty
  if(layer1clusters.size()>0) { for(auto& cluster : layer1clusters) { mClusterVec.emplace_back(std::move(cluster)); } }
  if(layer2clusters.size()>0) { for(auto& cluster : layer2clusters) { mClusterVec.emplace_back(std::move(cluster)); } }
  if(layer3clusters.size()>0) { for(auto& cluster : layer3clusters) { mClusterVec.emplace_back(std::move(cluster)); } }
  if(layer4clusters.size()>0) { for(auto& cluster : layer4clusters) { mClusterVec.emplace_back(std::move(cluster)); } }
  if(layer5clusters.size()>0) { for(auto& cluster : layer5clusters) { mClusterVec.emplace_back(std::move(cluster)); } }
  if(layer6clusters.size()>0) { for(auto& cluster : layer6clusters) { mClusterVec.emplace_back(std::move(cluster)); } }
  if(layer7clusters.size()>0) { for(auto& cluster : layer7clusters) { mClusterVec.emplace_back(std::move(cluster)); } }
  if(layer8clusters.size()>0) { for(auto& cluster : layer8clusters) { mClusterVec.emplace_back(std::move(cluster)); } }
  ///-----------------------------------------
}
