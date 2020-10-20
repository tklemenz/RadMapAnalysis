#include "Utility.h"

namespace mapping
{

std::pair<Int_t, Int_t> getFiberInfoFromModSpot(Int_t modSpot)
{
  ///<   0 - 31  --> layer 1, fiber 1-32
  ///<  32 - 63  --> layer 2, fiber 1-32
  ///<  64 - 95  --> layer 3, fiber 1-32
  ///<  96 -127  --> layer 4, fiber 1-32
  ///< 128 -159  --> layer 5, fiber 1-32
  ///< 160 -191  --> layer 6, fiber 1-32
  ///< 192 -223  --> layer 7, fiber 1-32
  ///< 224 -255  --> layer 8, fiber 1-32

  Int_t layer = 0;
  Int_t fiber = 0;

  if ((modSpot >= 0) && (modSpot < 32)) {
    layer = 1;
  }
  else if ((modSpot >= 32) && (modSpot < 64)) {
    layer = 2;
  }
  else if ((modSpot >= 64) && (modSpot < 96)) {
    layer = 3;
  }
  else if ((modSpot >= 96) && (modSpot < 128)) {
    layer = 4;
  }
  else if ((modSpot >= 128) && (modSpot < 160)) {
    layer = 5;
  }
  else if ((modSpot >= 160) && (modSpot < 192)) {
    layer = 6;
  }
  else if ((modSpot >= 192) && (modSpot < 224)) {
    layer = 7;
  }
  else if ((modSpot >= 224) && (modSpot < 256)) {
    layer = 8;
  }
  else {
    printf("\n\n%s%sInvalid module spot given!%s\n\n", text::BOLD, text::RED, text::RESET);
    exit(1);
  }

  fiber = (modSpot-(layer-1)*32)+1;

  return std::make_pair(layer,fiber);
}

Int_t getFiberNr(UInt_t configuration, UInt_t chID, UInt_t tdcID)
{
  if (chID == 0) { return 0; }
  switch (configuration) {
    case 0:
      if (tdcID == 0) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      else if (tdcID == 1) {
        if (chID<=16) { return odd0(chID); }
        else { return revEven1(chID); }
      }
      else if (tdcID == 2) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
	  }
      else if (tdcID == 3) {
        if (chID<=16) { return odd0(chID); }
        else { return revEven1(chID); }
      }
      else {
        printf("Wrong TDCID specified!\n");
        return -1;
      }
      break;

    case 1:
      if (tdcID == 0) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      else if (tdcID == 1) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      if (tdcID == 2) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      if (tdcID == 3) {
        if (chID<=16) { return revEven0(chID); }
        else { return revEven1(chID); }
      }
      else {
        printf("Wrong TDCID specified!\n");
        return -1;
      }
      break;

    case 2:
      if (tdcID == 0) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      else if (tdcID == 1) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      if (tdcID == 2) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      if (tdcID == 3) {
        if (chID<=16) { return odd0(chID); }
        else { return revEven1(chID); }
      }
      else {
        printf("Wrong TDCID specified!\n");
        return -1;
      }
      break;

    case 3:
      if (tdcID == 0) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      else if (tdcID == 1) {
        if (chID<=16) { return odd0(chID); }
        else { return revEven1(chID); }
      }
      if (tdcID == 2) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      if (tdcID == 3) {
        if (chID<=16) { return odd0(chID); }
        else { return revEven1(chID); }
      }
      else {
        printf("Wrong TDCID specified!\n");
        return -1;
      }
      break;

    case 4:
      if (tdcID == 0) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      else if (tdcID == 1) {
        if (chID<=16) { return odd0(chID); }
        else { return revEven1(chID); }
      }
      else if (tdcID == 2) {
        if (chID<=16) { return revEven0(chID); }
        else { return odd1(chID); }
      }
      else if (tdcID == 3) {
        if (chID<=16) { return odd0(chID); }
        else { return revEven1(chID); }
      }
      else {
        printf("Wrong TDCID specified!\n");
        return -1;
      }
      break;

    default:
      printf("Given Padiwa config is not covered!");
      return -1;
  }
}

Int_t getLayerNr(UInt_t configuration, UInt_t chID, UInt_t tdcID)
{
  switch (configuration) {
    case 0:
      if      (tdcID == 0)  { return 1; }
      else if (tdcID == 1)  { return 2; }
      else if (tdcID == 2)  { return 3; }
      else if (tdcID == 3)  { return 4; }
      else { 
        printf("Invalid TDCID!\n");
        return -1;
      }
      break;

    case 1:
      if      (tdcID == 0)  { return 1; }
      else if (tdcID == 1)  { return 3; }
      else if (tdcID == 2)  { return 5; }
      else if ((tdcID == 3) && (chID <= 16))  { return 2; }
      else if ((tdcID == 3) && (chID >  16))  { return 6; }
      else { 
        printf("Invalid TDCID!\n");
        return -1;
      }
      break;

    case 2:
      if      (tdcID == 0)  { return 1; }
      else if (tdcID == 1)  { return 3; }
      else if (tdcID == 2)  { return 5; }
      else if (tdcID == 3)  { return 2; }
      else { 
        printf("Invalid TDCID!\n");
        return -1;
      }
      break;

    case 3:
      if      (tdcID == 0)  { return 3; }
      else if (tdcID == 1)  { return 4; }
      else if (tdcID == 2)  { return 1; }
      else if (tdcID == 3)  { return 2; }
      else { 
        printf("Invalid TDCID!\n");
        return -1;
      }
      break;

    case 4:
      if      (tdcID == 0)  { return 5; }
      else if (tdcID == 1)  { return 6; }
      else if (tdcID == 2)  { return 7; }
      else if (tdcID == 3)  { return 8; }
      else { 
        printf("Invalid TDCID!\n");
        return -1;
      }
      break;

    default:
      printf("Given Padiwa config is not covered!");
      return -1;
  }
}

Int_t getX(Int_t layer, Int_t fiber)
{
  if      ((layer == 1) || (layer == 3) || (layer == 5) || (layer == 7)) { return fiber; }
  else if ((layer == 2) || (layer == 4) || (layer == 6) || (layer == 8)) { return 0; }
  else {
    printf("Invalid layer!\n");
    return -1;
  }
}

Int_t getY(Int_t layer, Int_t fiber)
{
  if      ((layer == 1) || (layer == 3) || (layer == 5) || (layer == 7)) { return 0; }
  else if ((layer == 2) || (layer == 4) || (layer == 6) || (layer == 8)) { return fiber; }
  else {
    printf("Invalid layer!\n");
    return -1;
  }
}

} /// namespace mapping