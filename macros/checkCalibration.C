#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <Rtypes.h>
#include <TCanvas.h>
#include <TF1.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <unistd.h>

#include "Utility.h"

extern char* optarg;

void checkCalibration(const TString inputFile1, const TString inputFile2, const char *outputFile)
{
  TFile *file1 = new TFile(inputFile1);
  TFile *file2 = new TFile(inputFile2);

  std::vector<TH2D*> file1Histos{};
  std::vector<TH2D*> file2Histos{};
  for (Int_t i=0; i<4; i++) {
    file1Histos.emplace_back(new TH2D());
    file1->GetObject(Form("hToTL%i",i+1), file1Histos.back());
    file2Histos.emplace_back(new TH2D());
    file2->GetObject(Form("hToTL%i",i+1), file2Histos.back());
  }

  std::vector<TH1D*> totMPVDistLayerVec{};
  std::vector<TH1D*> totMPVRatioDistLayerVec{};
  for(Int_t i=0; i<4; i++) {
    totMPVDistLayerVec.emplace_back(new TH1D(Form("hMPVDistL%i",i+1),Form("distribution of MPV values in L%i;amplitude MPV;counts",i+1),300,0,30));
    totMPVRatioDistLayerVec.emplace_back(new TH1D(Form("hMPVRatioDistL%i",i+1),Form("distribution of MPV ratios in L%i;amplitude MPV ratio;counts",i+1),200,0,2));
  }

  TH1D *totMPVDistAll = new TH1D("hMPVDistAll","distribution of MPV values in all layers;amplitude MPV;counts",300,0,30);
  TH1D *totMPVRatioDistAll = new TH1D("hMPVRatioDistAll","distribution of MPV ratio in all channels;amplitude MPV ratio;counts",200,0,2);

  std::vector<TH1D*> totLayerFitFile1{};
  std::vector<std::vector<Float_t>> fitContentLayerFile1{};
  std::vector<TH1D*> totLayerFitFile2{};
  std::vector<std::vector<Float_t>> fitContentLayerFile2{};

  TF1* landau1 = new TF1("fit","landau",2,150);
  Int_t histCounter = 0;
  for(auto& hist : file1Histos) {                                                             // loop over histos
    if(hist->GetEntries() != 0) {
      hist->FitSlicesY(landau1,2,33,0,"QNR");                                                // fit 1D distributions with gaus
      //hist->FitSlicesY(0,2,33);
      totLayerFitFile1.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));         // get fit mean values (written to 1D histos)
      fitContentLayerFile1.emplace_back(std::vector<Float_t>());
      for(Int_t i=0; i<totLayerFitFile1.back()->GetSize(); i++) {
        fitContentLayerFile1.back().emplace_back((*totLayerFitFile1.back())[i]);                   // extract fit results
      }
      histCounter++;
    }
  }

  TF1* landau2 = new TF1("fit","landau",10,150);
  histCounter = 0;
  for(auto& hist : file2Histos) {                                                             // loop over histos
    if(hist->GetEntries() != 0) {
      hist->FitSlicesY(landau2,2,33,0,"QNR");                                                // fit 1D distributions with gaus
      //hist->FitSlicesY(0,2,33);
      totLayerFitFile2.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));         // get fit mean values (written to 1D histos)
      fitContentLayerFile2.emplace_back(std::vector<Float_t>());
      for(Int_t i=0; i<totLayerFitFile2.back()->GetSize(); i++) {
        fitContentLayerFile2.back().emplace_back((*totLayerFitFile2.back())[i]);                   // extract fit results
      }
      histCounter++;
    }
  }

  Int_t layerCounter = 0;
  for (auto& layer : fitContentLayerFile1) {
    for (Int_t i=2; i<layer.size()-1; i++) {
      totMPVDistLayerVec.at(layerCounter)->Fill(layer.at(i));
      totMPVDistLayerVec.at(layerCounter)->Fill(fitContentLayerFile2.at(layerCounter).at(i));

      totMPVDistAll->Fill(layer.at(i));
      totMPVDistAll->Fill(fitContentLayerFile2.at(layerCounter).at(i));

      totMPVRatioDistLayerVec.at(layerCounter)->Fill(layer.at(i)/fitContentLayerFile2.at(layerCounter).at(i));
      totMPVRatioDistAll->Fill(layer.at(i)/fitContentLayerFile2.at(layerCounter).at(i));
    }
    layerCounter++;
  }

  //=================================================================================
  //=========================  output  ==============================================
  TFile *fout = new TFile(outputFile, "recreate");

  TCanvas *c1 = new TCanvas("cCalibAmpDistsLayersFile1","cCalibAmpDistsLayersFile1");
  c1->DivideSquare(4);
  Int_t padIter = 1;
  for(auto& hist : file1Histos) {
    if(hist->GetEntries() == 0) { continue; }
    c1->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c2 = new TCanvas("cCalibAmpDistsLayersFile2","cCalibAmpDistsLayersFile2");
  c2->DivideSquare(4);
  padIter = 1;
  for(auto& hist : file2Histos) {
    if(hist->GetEntries() == 0) { continue; }
    c2->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c3 = new TCanvas("cMPVDistLayers","cMPVDistLayers");
  c3->DivideSquare(4);
  padIter = 1;
  for(auto& hist : totMPVDistLayerVec) {
    if(hist->GetEntries() == 0) { continue; }
    c3->cd(padIter);
    hist->Draw();
    padIter++;
  }

  TCanvas *c4 = new TCanvas("cMPVRatioDistLayers","cMPVRatioDistLayers");
  c4->DivideSquare(4);
  padIter = 1;
  for(auto& hist : totMPVRatioDistLayerVec) {
    if(hist->GetEntries() == 0) { continue; }
    c4->cd(padIter);
    hist->Draw();
    padIter++;
  }

  fout->WriteObject(c1, c1->GetName());
  fout->WriteObject(c2, c2->GetName());
  fout->WriteObject(c3, c3->GetName());
  fout->WriteObject(c4, c4->GetName());
  fout->WriteObject(totMPVDistAll, totMPVDistAll->GetName());
  fout->WriteObject(totMPVRatioDistAll, totMPVRatioDistAll->GetName());
  fout->Close();
  //=================================================================================
  //=================================================================================
}

int main(int argc, char** argv)
{
  char    inputFile1[1024]="";
  char    inputFile2[1024]="";
  char    outputFile[512]="checkCalibration_output.root";

  int argsforloop;
  while ((argsforloop = getopt(argc, argv, "hi:j:o:n:c:")) != -1) {
    switch (argsforloop) {
      case '?':
        ///TODO: write usage function
        exit(EXIT_FAILURE);
      case 'i':
        strncpy(inputFile1, optarg, 1024);
        break;
      case 'j':
        strncpy(inputFile2, optarg, 1024);
        break;
      case 'o':
        strncpy(outputFile, optarg, 512);
        break;
      default:
        printf("\n\n%s%sdefault case%s\n\n",text::BOLD,text::RED,text::RESET);
        exit(EXIT_FAILURE);
    }
  }

  printf("\n\n%sRunning getModuleGain%s\n\n",text::BOLD,text::RESET);

  checkCalibration(inputFile1,inputFile2,outputFile);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}