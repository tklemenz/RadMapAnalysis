#ifndef TRACK_H
#define TRACK_H

#include "Cluster.h"
#include "Utility.h"

/// This class represents a track in the module.
/// Ideally a track consists of 4 clusters, one in each layer.


/// TODO: think about more info that needs to be in the track.
/// E.g. angles, position (could be x-y in layer 1+2 and layer 3+4 separately), particle type.

class Track
{
 public:
  Track() = default;
  ~Track() = default;
  Track(const Track &track);

  inline void addCluster(const Cluster &cluster) { mClusterVec.emplace_back(cluster); }

  inline std::vector<Cluster>& getClusters() { return mClusterVec; }

  inline void setType(ParticleType &type) { mParticleType = type; }
  inline ParticleType getType() { return mParticleType; }

 private:

  std::vector<Cluster> mClusterVec{}; ///< holds all clusters that are assigned to the track
  ParticleType         mParticleType; ///< particle type: can be Pion or Proton

  ClassDef(Track,1);
};

#endif