#ifndef UTILITY_H
#define UTILITY_H

#include <Rtypes.h>

namespace mapping
{

  /// get the fiber number (0 - 255) that the layer+chID combination takes in the module 
  inline Int_t getModuleSpot(Int_t layer, Int_t chID) { return ((chID + (layer-1)*32)-1); }

  /// mapping functions from channel number to fiber
  /// odd maps channel number to odd numbers from 1-31
  /// revEven maps channel number to even numbers from 32-2
  /// 0 is for channel number 1-16
  /// 1 is for channel number 17-32
  inline UInt_t odd0(UInt_t chID)      { return 2*chID-1; }
  inline UInt_t odd1(UInt_t chID)      { return 2*(chID-16)-1; }
  inline UInt_t revEven0(UInt_t chID)  { return 32-(2*(chID-1)); }
  inline UInt_t revEven1(UInt_t chID)  { return 32-(2*(chID-17)); }

  /// get the fiber number within the layer from the padiwa configuration, channel ID, and TDC ID
  Int_t getFiberNr(UInt_t configuration, UInt_t chID, UInt_t tdcID);

  /// get the layer number from the padiwa configuration, channel ID, and TDC ID
  Int_t getLayerNr(UInt_t configuration, UInt_t chID, UInt_t tdcID);

  /// get the fiber number in x-direction from the fiber number and layer number. returns 0 for even layers.
  Int_t getX(Int_t fiber, Int_t layer);

  /// get the fiber number in y-direction from the fiber number and layer number. returns 0 for odd layers.
  Int_t getY(Int_t fiber, Int_t layer);

}

#endif