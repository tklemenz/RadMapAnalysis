#include "Utility.h"

Int_t mapping::getModuleSpot(Int_t layer, Int_t chID)
{
  return ((chID + (layer-1)*32)-1);
}