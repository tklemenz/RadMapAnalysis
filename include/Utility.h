#ifndef UTILITY_H
#define UTILITY_H

#include <Rtypes.h>

namespace mapping
{

  /// get the fiber number (0 - 255) that the layer+chID combination takes in the module 
  Int_t getModuleSpot(Int_t layer, Int_t chID);

}

#endif