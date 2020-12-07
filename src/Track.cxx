#include "Track.h"

ClassImp(Track);

//________________________________________________________________________________
Track::Track(const Track &track)
: mClusterVec(track.mClusterVec),
  mParticleType(track.mParticleType)
{

}