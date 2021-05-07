#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <Rtypes.h>
#include <TCanvas.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <unistd.h>

#include "Utility.h"
#include "Constants.h"
#include "TGraph.h"

///< usage: ./compareGainMaps

void compareGainMaps()
{

  TFile *fout = new TFile("gainMapComparison.root","recreate");

  TH1F* hRatioDist = new TH1F("hRatioDist","E62/E18 gain map;ratio",200,0,2);
  TH2F* hRatioDist2D = new TH2F("hRatioDist2D","E18 vs E62 gain map; E62 value; E18 value",200,0,2,200,0,2);
  TGraph* gRatio = new TGraph();

  std::vector<TH1F*> ratioDistLayer{};
  for (Int_t i = 0; i<8; i++) {
    ratioDistLayer.emplace_back(new TH1F(Form("hRatioDistL%i",i+1), Form("E62/E18 gain map, Layer %i;ratio",i+1),200,0,2));
  }

  Float_t ratio = 0;
  bool count = false;
  Int_t layer = 0;
  Int_t gCounter = 1;
  for(auto const& pair : constants::gainMapCompareHelper) {
    for (Int_t i = 1; i<pair.first.size(); i++) {
      ratio = pair.first.at(i)/pair.second.at(i);
      hRatioDist2D->Fill(pair.first.at(i),pair.second.at(i));
      hRatioDist->Fill(ratio);
      ratioDistLayer.at(layer)->Fill(ratio);
      gRatio->SetPoint(gCounter, gCounter, ratio);
      gCounter++;
    }
    if (count) { layer++; count = false; }
    else { count = true; }
  }

  TCanvas *c1 = new TCanvas("cRatioDists","cRatioDists");
  c1->DivideSquare(8);

  Int_t padIter = 1;
  for(auto& hist : ratioDistLayer) {
    c1->cd(padIter);
    hist->Draw();
    padIter++;
  }

  fout->WriteObject(hRatioDist, hRatioDist->GetName());
  fout->WriteObject(hRatioDist2D, hRatioDist2D->GetName());
  fout->WriteObject(c1, c1->GetName());
  fout->WriteObject(gRatio, "gRatio");
  fout->Close();
}

int main(int argc, char** argv)
{
  printf("\n\n%sRunning compareGainMaps%s\n\n",text::BOLD,text::RESET);
  
  compareGainMaps();

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}