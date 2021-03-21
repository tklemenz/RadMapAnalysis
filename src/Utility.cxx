#include "Utility.h"
#include <TH1.h>
#include <TH2.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TChain.h>
#include <TSystem.h>
#include <TString.h>
#include <TObjArray.h>

#include <sstream>

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

Padiwa getPadiwa(Int_t TDC, Int_t chID)
{
  switch (TDC) {
    case 0:
      if (chID <= 16) { return Padiwa::p1500_0; }
      else { return Padiwa::p1500_1; }
      break;
    case 1:
      if (chID <= 16) { return Padiwa::p1510_0; }
      else { return Padiwa::p1510_1; }
      break;
    case 2:
      if (chID <= 16) { return Padiwa::p1520_0; }
      else { return Padiwa::p1520_1; }
      break;
    case 3:
      if (chID <= 16) { return Padiwa::p1530_0; }
      else { return Padiwa::p1530_1; }
      break;
    default:
      printf("Invalid TDCID!!\n");
  }
}

} /// namespace mapping

namespace fileHandling
{

void makeChain(TChain& chain, const TString& input)
{
  TString allFiles;

  if (input.EndsWith(".txt")){ allFiles=gSystem->GetFromPipe(Form("cat %s",input.Data())); }
  else { allFiles=gSystem->GetFromPipe(Form("ls %s",input.Data())); }

  TObjArray *arr = allFiles.Tokenize("\n");

  for (int ifile=0; ifile<arr->GetEntriesFast(); ++ifile){
    TString file=arr->At(ifile)->GetName();
    chain.Add(file);
  }

  return;
}

std::vector<std::string> splitString(std::string inString, const char* delimiter)
{
  std::vector<std::string> outVec;
  std::istringstream stream(inString);
  std::string token;

  while (std::getline(stream, token, *delimiter)) {
    outVec.emplace_back(token);
  }

  return std::move(outVec);
}

} //namespace fileHandling

namespace beautify
{

void setStyle()
{
  const Int_t NCont=255;
  TStyle *st = new TStyle("goodstyle","goodstyle");
  gROOT->GetStyle("Plain")->Copy((*st));
  st->SetStatX(.9);
  st->SetStatY(.9);
  st->SetNumberContours(NCont);
  st->SetPalette(1,0);
  st->SetOptFit(0);
  st->SetOptStat(0);
  st->SetGridColor(kGray+1);
  st->SetPadTickX(kTRUE);
  st->SetPadTickY(kTRUE);
  st->SetPadLeftMargin(0.12);
  st->SetPadBottomMargin(0.12);
  st->SetPadRightMargin(0.04);
  st->SetPadTopMargin(0.06);
  st->SetHistLineColor(kBlue+2);
  st->cd();

  Int_t nimTPCFont=42;          // 42 for a non-bold font
  st->SetTextFont(nimTPCFont);
  st->SetTitleFont(nimTPCFont, "T");
  st->SetTitleFont(nimTPCFont, "XYZ");
  st->SetLabelFont(nimTPCFont,"XYZ");
  st->SetLabelSize(0.045,"XYZ");
  st->SetTitleSize(0.05,"XYZ");
  st->SetTitleOffset(0.95,"XYZ");
  //st->SetTitleOffset(0.9,"Y");   //mine
  st->SetLabelOffset(0.01, "XYZ");
  //st->SetNdivisions(505, "Y");
  st->SetStatFont(nimTPCFont);
  st->SetOptTitle(1);
  st->SetTitleAlign(13);
  st->SetTitleBorderSize(0);
  st->SetPalette(1,0);
  st->SetStatBorderSize(1);
  new TColor(2001,1,1,1);
  st->SetFillColor(2001);
  st->SetTickLength(gStyle->GetTickLength()/696.*472.,"y");
  const Int_t NRGBs = 5;
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };  
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  st->cd();
  gROOT->SetStyle("goodstyle");
  gStyle->SetOptStat(1);
}

template <class T>
void setStyleHisto(T* histo)
{
  histo->GetXaxis()->SetLabelSize(0.045);
  histo->GetXaxis()->SetTitleSize(0.05);
  histo->GetXaxis()->SetLabelOffset(0.01);
  histo->GetXaxis()->SetTitleOffset(0.95);
  histo->GetXaxis()->SetLabelFont(42);
  histo->GetYaxis()->SetLabelSize(0.045);
  histo->GetYaxis()->SetTitleSize(0.05);
  histo->GetYaxis()->SetLabelOffset(0.01);
  histo->GetYaxis()->SetTitleOffset(1.0);
}

} /// namespace beautify

template void beautify::setStyleHisto<TH1>(TH1* histo);
template void beautify::setStyleHisto<TH2>(TH2* histo);