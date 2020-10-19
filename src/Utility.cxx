#include "Utility.h"

Int_t mapping::getModuleSpot(Int_t layer, Int_t chID)
{
  return ((chID + (layer-1)*32)-1);
}

Int_t mapping::getFiberNr(UInt_t configuration, UInt_t chID, UInt_t tdcID)
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

Int_t mapping::getLayerNr(UInt_t configuration, UInt_t chID, UInt_t tdcID)
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

Int_t mapping::getX(UInt_t fiber, UInt_t layer)
{
  if      ((layer == 1) || (layer == 3) || (layer == 5) || (layer == 7)) { return fiber; }
  else if ((layer == 2) || (layer == 4) || (layer == 6) || (layer == 8)) { return 0; }
  else {
    printf("Invalid layer!\n");
    return -1;
  }
}

Int_t mapping::getY(UInt_t fiber, UInt_t layer)
{
  if      ((layer == 1) || (layer == 3) || (layer == 5) || (layer == 7)) { return 0; }
  else if ((layer == 2) || (layer == 4) || (layer == 6) || (layer == 8)) { return fiber; }
  else {
    printf("Invalid layer!\n");
    return -1;
  }
}