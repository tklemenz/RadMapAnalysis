#ifndef TRACKER_H
#define TRACKER_H

#include "CTSEventClusters.h"
#include "Track.h"

/// Tracker class to assign clusters from CTSEventClusters to tracks.
/// Ideally a track consists of 4 clusters, one in each layer.
///
/// Option 1: The Tracker takes a CTSEventClusters and find tracks from the clusters.
/// Option 2: The Tracker gets a vector of Clusters and does the tracking.
///           Then the user needs to make sure that the clusters are all valid (e.g. not 5 seconds apart).

class Tracker
{
 public:
  Tracker() = default;
  ~Tracker() = default;

  /// run the tracking
  void run(const CTSEventClusters& event);
  void run(const std::vector<Cluster>& clusters);

 private:

  std::vector<Track> mTracks{}; ///< contains all found tracks

  ClassDef(Tracker,1);
};

#endif