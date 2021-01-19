#include "Tracker.h"

#include <experimental/random>
#include <cstdlib>
#include <ctime>

int main()
{
  std::vector<Cluster*> clusterVec;
  std::srand(std::time(nullptr));

  for (int i = 0; i<20; i++) {
    clusterVec.emplace_back(new Cluster(std::rand(), std::rand(), std::rand(), std::rand(), std::rand(), std::rand(),
                                    std::rand(), std::experimental::randint(1,4), std::experimental::randint(0,3), 0));
  }

  Tracker tracker;

  tracker.run(clusterVec);

  std::vector<Track*> tracks = tracker.getTracks();

  int counter = 0;
  for (auto track : tracks) {
    counter++;
    printf("number of clusters in track %d: %lu\n", counter, track->getClusters().size());
    int clusterCounter = 0;
    for (auto cluster : track->getClusters()) {
      clusterCounter++;
      printf("\tcluster %d q tot: %g\n", clusterCounter, cluster->getQTot());
    }
  }

  printf("tracks found: %lu\n", tracks.size());


  return 0;
}