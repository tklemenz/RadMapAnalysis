#ifndef TRACKER_H
#define TRACKER_H

#include "CTSEventClusters.h"
#include "Track.h"

/// Tracker class to assign clusters from CTSEventClusters to tracks.
/// Ideally a track consists of 4 clusters, one in each layer.

class Tracker
{
 public:
  Tracker() = default;
  ~Tracker() = default;

 private:


  ClassDef(Tracker,1);
};

#endif