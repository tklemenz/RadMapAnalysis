#include "Tracker.h"

ClassImp(Tracker);

//________________________________________________________________________________
void Tracker::run(const std::vector<Cluster>& clusters)
{
  /// do something
}

//________________________________________________________________________________
void Tracker::run(const CTSEventClusters& event)
{
  Tracker::run(event.getClusters());
}
