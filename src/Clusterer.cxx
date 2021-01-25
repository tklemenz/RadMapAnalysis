#include "Clusterer.h"

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

  Int_t x(-1), y(-1), layer(-1);

  Double_t cluster_fib_range = 2; // The allowed range in fiber distance for cluster building

  // Clusters can only contain signals from the same layer
  // That's why I made 4 buffer cluster containers (1 for each layer)
  Cluster layer1cluster = Cluster();
  Cluster layer2cluster = Cluster();
  Cluster layer3cluster = Cluster();
  Cluster layer4cluster = Cluster();

  for(auto& fiber : fibers) {
    if(fiber.getNSignals() > 0) {
      layer = fiber.getLayer();
      x     = fiber.getX();
      y     = fiber.getY();
      for(auto& signal : fiber.getSignals()) {
        if(signal.getSignalNr()==1) { 
          // Since the signals are separeted in fibers but not layer,
          // this switch makes sure only signals from same layer are compared
          switch(layer){  
            case 1: if(layer1cluster.getNSignals()==0 || abs(x-layer1cluster.getMeanFiber())<cluster_fib_range){ // Cuts are Work in Progress
                      layer1cluster.addSignal(signal);
                    }
                    break;
            case 2: if(layer2cluster.getNSignals()==0 || abs(x-layer2cluster.getMeanFiber())<cluster_fib_range){
                      layer2cluster.addSignal(signal); 
                    }
                    break;
            case 3: if(layer3cluster.getNSignals()==0 || abs(x-layer3cluster.getMeanFiber())<cluster_fib_range){
                     layer3cluster.addSignal(signal); 
                    }
                    break;
            case 4: if(layer4cluster.getNSignals()==0 || abs(x-layer4cluster.getMeanFiber())<cluster_fib_range){
                      layer4cluster.addSignal(signal); 
                    }
                    break;
          } 
        } 
      } /// loop over signals in fiber
    }
  } /// loop over fibers in module

  // Add clusters to final vector if they are not empty
  if(layer1cluster.getNSignals()>0) { mClusterVec.emplace_back(layer1cluster); }
  if(layer2cluster.getNSignals()>0) { mClusterVec.emplace_back(layer2cluster); }
  if(layer3cluster.getNSignals()>0) { mClusterVec.emplace_back(layer3cluster); }
  if(layer4cluster.getNSignals()>0) { mClusterVec.emplace_back(layer4cluster); }
  ///-----------------------------------------
}
