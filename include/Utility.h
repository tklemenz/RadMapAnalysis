#ifndef UTILITY_H
#define UTILITY_H

#include <Rtypes.h>

namespace mapping
{

  /// get the fiber number (0 - 255) that the layer+chID combination takes in the module 
  Int_t getModuleSpot(Int_t layer, Int_t chID);

  inline UInt_t odd0(UInt_t chID)      { return 2*chID-1; }
  inline UInt_t odd1(UInt_t chID)      { return 2*(chID-16)-1; }
  inline UInt_t revEven0(UInt_t chID)  { return 32-(2*(chID-1)); }
  inline UInt_t revEven1(UInt_t chID)  { return 32-(2*(chID-17)); }

  /// get the fiber number within the layer from the padiwa configuration, channel ID, and TDC ID
  Int_t getFiberNr(UInt_t configuration, UInt_t chID, UInt_t tdcID);

  /// get the layer number from the padiwa configuration, channel ID, and TDC ID
  Int_t getLayerNr(UInt_t configuration, UInt_t chID, UInt_t tdcID);

  /// get the fiber number in x-direction from the fiber number and layer number. returns 0 for even layers.
  Int_t getX(UInt_t fiber, UInt_t layer);

  /// get the fiber number in y-direction from the fiber number and layer number. returns 0 for odd layers.
  Int_t getY(UInt_t fiber, UInt_t layer);

}

#endif