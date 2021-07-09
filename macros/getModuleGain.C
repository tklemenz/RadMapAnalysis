#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <Rtypes.h>
#include <TCanvas.h>
#include <TNtupleD.h>
#include <TChain.h>
#include <TObjArray.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <map>
#include <math.h>

#include "Utility.h"
#include "Constants.h"

///< usage: ./getModuleGain -i inputfile1 -j inputfile2-o outputfile -c outputfileForCalibData -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed
///< Get the gain map for a module
///< Input:
///< inputfile1: tuple file of signals from config0, that was produced with applyPadiwaGainCorrectionToSignals --> can also be a file list in txt file
///< inputfile2: tuple file of signals from config4, that was produced with applyPadiwaGainCorrectionToSignals --> can also be a file list in txt file
///< Like this one has input signals with applied T0 correction (not important for the gain map) and applied padiwa gain correction

extern char* optarg;

static Int_t   totCut = 5;   // cut away all signals with ToT<totCut to get rid of possible bias for gaus fit

void getModuleGain(const TString inputFiles0, const TString inputFiles4, const char *outputFile, const char *outputCalib, ULong_t procNr)
{
  /// Prepare chain of all config 0 files
  //=====================================
  TChain chain0("Signals", "Signals");
  fileHandling::makeChain(chain0, inputFiles0);
  TObjArray* files0 = chain0.GetListOfFiles();
  printf("%sConfig 0 files to be processed:%s\n", text::BOLD, text::RESET);
  for (int ifile=0; ifile<files0->GetEntriesFast(); ++ifile){ printf("%s\n", files0->At(ifile)->GetTitle()); }

  /// Prepare chain of all config 4 files
  //=====================================
  TChain chain4("Signals", "Signals");
  fileHandling::makeChain(chain4, inputFiles4);
  TObjArray* files4 = chain4.GetListOfFiles();
  printf("%sConfig 4 files to be processed:%s\n", text::BOLD, text::RESET);
  for (int ifile=0; ifile<files4->GetEntriesFast(); ++ifile){ printf("%s\n", files4->At(ifile)->GetTitle()); }

  /* Define variables
  ==========================================================
  ==========================================================*/
  Double_t eventNr      = -1;
  Double_t timeStamp    = -1;
  Double_t ToT          = -1;
  Double_t chID         = -1;
  Double_t TDC          = -1;
  Double_t layer        = -1;
  Double_t x            = -1;
  Double_t y            = -1;
  Double_t signalNr     = -1;
  Double_t padiwaConfig = -1;
  Double_t refTime      = -1;

  Int_t fiberNr         = -1;
  Int_t layerIter       =  0;

  ULong_t nEvents       =  0;

  Float_t refValue      =  0;

  /* Define histograms and other useful containers
  ==========================================================
  ==========================================================*/
  std::vector<TH2D*> totLayerVec{};
  std::vector<TH1F*> gainSpreadLayerVec{};
  for(Int_t i = 0; i<8; i++) {
    totLayerVec.emplace_back(new TH2D(Form("hSigL%i",i+1),"amplitude distribution vs fiber;fiber;amplitude [mV]",33,0,33,1500,0,150));
    gainSpreadLayerVec.emplace_back(new TH1F(Form("hGainDistL%i",i+1),Form("gain distribution layer %i",i+1),100,0,2));
  }

  std::vector<TH2D*> totPadiwaVec0{};
  std::vector<TH2D*> totPadiwaVec4{};
  std::vector<TH1F*> gainSpreadPadiwaVec0{};
  std::vector<TH1F*> gainSpreadPadiwaVec4{};
  for(auto& name : constants::padiwaNames) {
    totPadiwaVec0.emplace_back(new TH2D(Form("hSigPadiwa%s_C0",name.c_str()),"amplitude distribution vs padiwa channel;channel;amplitude [mV]",17,0,17,1500,0,150));
    totPadiwaVec4.emplace_back(new TH2D(Form("hSigPadiwa%s_C4",name.c_str()),"amplitude distribution vs padiwa channel;channel;amplitude [mV]",17,0,17,1500,0,150));
    gainSpreadPadiwaVec0.emplace_back(new TH1F(Form("hGainDistPadiwa%s_C0",name.c_str()),Form("gain distribution padiwa %s",name.c_str()),100,0,2));
    gainSpreadPadiwaVec4.emplace_back(new TH1F(Form("hGainDistPadiwa%s_C4",name.c_str()),Form("gain distribution padiwa %s",name.c_str()),100,0,2));
  }

  TH2D* gainMapHor = new TH2D("hGainHor","Gain Map Rel (horizontal); U; V",68,0,34,8,0,8); beautify::setStyleHisto<TH2>(gainMapHor);
  TH2D* gainMapVer = new TH2D("hGainVer","Gain Map Rel (vertical); U; V",68,0,34,8,0,8);   beautify::setStyleHisto<TH2>(gainMapVer);

  std::vector<TH1F*> meanToTDistVec0{};
  std::vector<TH1F*> meanToTDistVec4{};
  for(Int_t i = 0; i<8; i++) {
    meanToTDistVec0.emplace_back(new TH1F(Form("hMeanToTDistL%i_C0", i+1),Form("mean ToT distribution L%i", i+1),1000,0,100));
    meanToTDistVec4.emplace_back(new TH1F(Form("hMeanToTDistL%i_C4", i+1),Form("mean ToT distribution L%i", i+1),1000,0,100));
  }

  std::vector<TH1F*> meanToTDistVec{};
  for(Int_t i = 0; i<8; i++) {
    meanToTDistVec.emplace_back(new TH1F(Form("hMeanToTDistL%i", i+1),Form("mean ToT distribution L%i", i+1),1000,0,100));
  }

  std::vector<TH1D*> totLayerGausMean{};
  std::vector<std::vector<Float_t>> fitContentLayer{};

  std::vector<TH1D*> totPadiwaGausMean0{};
  std::vector<std::vector<Float_t>> fitContentPadiwa0{};
  std::vector<TH1D*> totPadiwaGausMean4{};
  std::vector<std::vector<Float_t>> fitContentPadiwa4{};

  std::vector<Int_t> layerMarker{};
  /*========================================================
  ==========================================================*/

  /// Loop over config 0 files
  //==========================
  for (Int_t ifile=0; ifile<files0->GetEntriesFast(); ++ifile){
    TFile *SignalFile = new TFile(Form("%s", files0->At(ifile)->GetTitle()));
    TTree *signals = (TTree*)SignalFile->Get("Signals");

    signals->SetBranchAddress("EventNr", &eventNr);
    signals->SetBranchAddress("timeStamp", &timeStamp);
    signals->SetBranchAddress("ToT", &ToT);
    signals->SetBranchAddress("chID", &chID);
    signals->SetBranchAddress("TDC", &TDC);
    signals->SetBranchAddress("layer", &layer);
    signals->SetBranchAddress("x", &x);
    signals->SetBranchAddress("y", &y);
    signals->SetBranchAddress("signalNr", &signalNr);
    signals->SetBranchAddress("padiwaConfig", &padiwaConfig);
    signals->SetBranchAddress("refTime", &refTime);

    nEvents = procNr;
    if ((nEvents == -1) || (nEvents > signals->GetEntries())) { nEvents = signals->GetEntries(); }

    printf("events to process: %lu\t %.1f%% of file %s\n", nEvents, Float_t(100*nEvents)/Float_t(signals->GetEntries()), files0->At(ifile)->GetTitle());

    for (ULong_t entry = 0; entry < nEvents; entry++) {
      /* Make some fancy terminal output */
      if ((((entry+1)%10000) == 0) || (entry == (nEvents-1))) {
        printf("\rprocessing event %lu...", entry+1);
        fflush(stdout);
        std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
      } /* End of terminal output*/

      signals->GetEntry(entry);

      ToT *= 1e9; // convert ToT value to ns
      if (ToT < totCut) { continue; }                             // do not use small ToT values to not have gaus fit biased by noise
      if (signalNr != 1) { continue; }
      fiberNr = mapping::getFiberNr(padiwaConfig, chID, TDC);
      if (layer == 3 && fiberNr == 11) { continue; }
      if (layer == 3 && fiberNr == 30) { continue; }

      ///This is a not really working conversion from ToT to signal height
      //ToT = (ToT-8.46)/1.69;

      ///This is from the proper measurement with single SiPM (positive signal) and threshold 100 for different light intensities
      //ToT = 0.667*exp(0.146*ToT);

      totPadiwaVec0.at(constants::padiwaPosMap.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))))->Fill(mapping::getPadiwaChannel(chID), ToT);
      totLayerVec.at(Int_t(layer)-1)->Fill(fiberNr, ToT);
    }// end of loop over file
  }// end of loop over config 0 files

  /// Loop over config 4 files
  //==========================
  for (Int_t ifile=0; ifile<files4->GetEntriesFast(); ++ifile){
    TFile *SignalFile = new TFile(Form("%s", files4->At(ifile)->GetTitle()));
    TTree *signals = (TTree*)SignalFile->Get("Signals");

    signals->SetBranchAddress("EventNr", &eventNr);
    signals->SetBranchAddress("timeStamp", &timeStamp);
    signals->SetBranchAddress("ToT", &ToT);
    signals->SetBranchAddress("chID", &chID);
    signals->SetBranchAddress("TDC", &TDC);
    signals->SetBranchAddress("layer", &layer);
    signals->SetBranchAddress("x", &x);
    signals->SetBranchAddress("y", &y);
    signals->SetBranchAddress("signalNr", &signalNr);
    signals->SetBranchAddress("padiwaConfig", &padiwaConfig);
    signals->SetBranchAddress("refTime", &refTime);

    nEvents = procNr;
    if ((nEvents == -1) || (nEvents > signals->GetEntries())) { nEvents = signals->GetEntries(); }

    printf("events to process: %lu\t %.1f%% of file %s\n", nEvents, Float_t(100*nEvents)/Float_t(signals->GetEntries()), files4->At(ifile)->GetTitle());
    for (ULong_t entry = 0; entry < nEvents; entry++) {
      /* Make some fancy terminal output */
      if ((((entry+1)%10000) == 0) || (entry == (nEvents-1))) {
        printf("\rprocessing event %lu...", entry+1);
        fflush(stdout);
        std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
      } /* End of terminal output*/

      signals->GetEntry(entry);

      ToT *= 1e9; // convert ToT value to ns
      if (ToT < totCut) { continue; }                             // do not use small ToT values to not have gaus fit biased by noise
      if (signalNr != 1) { continue; }
      fiberNr = mapping::getFiberNr(padiwaConfig, chID, TDC);
      if (layer == 3 && fiberNr == 11) { continue; }
      if (layer == 3 && fiberNr == 30) { continue; }

      ///This is a not really working conversion from ToT to signal height
      //ToT = (ToT-8.46)/1.69;

      ///This is from the proper measurement with single SiPM (positive signal) and threshold 100 for different light intensities
      //ToT = 0.667*exp(0.146*ToT);

      totPadiwaVec4.at(constants::padiwaPosMap.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))))->Fill(mapping::getPadiwaChannel(chID), ToT);
      totLayerVec.at(Int_t(layer)-1)->Fill(fiberNr, ToT);
    }// end of loop over file
  }// end of loop over config 4 files

  /// Create output file and get output content
  //===========================================
  TFile *fout = new TFile(Form("%s",outputFile),"recreate");
  //TF1* landau = new TF1("fit","landau",10,150);                /// CAUTION: If this macro is used for Pions, the lower limit should be lower than 10!!

  Int_t histCounter = 0;
  // loop over TH2D from above, fit 1D ToT distribution for all fibers and extract the mean value
  // meanwhile only take the layers that actually have data and leave the rest ignored
  for(auto& hist : totLayerVec) {                                                             // loop over histos
    layerIter++;
    if(hist->GetEntries() != 0) {
      layerMarker.emplace_back(layerIter);                                                    // write down which layers are used
      fout->WriteObject(hist, hist->GetName());                                               // write ToT vs fiber to file
      //hist->FitSlicesY(landau,2,33,0,"QNR");                                                               // fit 1D distributions with gaus
      hist->FitSlicesY(0,2,33);
      totLayerGausMean.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));   // get fit mean values (written to 1D histos)
      fout->WriteObject(totLayerGausMean.back(), totLayerGausMean.back()->GetName());         // write fit results to file
      fitContentLayer.emplace_back(std::vector<Float_t>());                                   // prepare extraction of fit results
      for(Int_t i=0; i<totLayerGausMean.back()->GetSize(); i++) {
        fitContentLayer.back().emplace_back((*totLayerGausMean.back())[i]);                   // extract fit results
      }
      histCounter++;
    }
  }

  for(auto& hist : totPadiwaVec0) {                                                           // loop over histos
    fout->WriteObject(hist, hist->GetName());                                                 // write ToT vs channel to file
    //hist->FitSlicesY(landau,2,17,0,"QNR");                                                                 // fit 1D distributions with gaus
    hist->FitSlicesY(0,2,17);
    totPadiwaGausMean0.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));   // get fit mean values (written to 1D histos)
    fout->WriteObject(totPadiwaGausMean0.back(), totPadiwaGausMean0.back()->GetName());       // write fit results to file
    fitContentPadiwa0.emplace_back(std::vector<Float_t>());                                   // prepare extraction of fit results
    for(Int_t i=0; i<totPadiwaGausMean0.back()->GetSize(); i++) {
      fitContentPadiwa0.back().emplace_back((*totPadiwaGausMean0.back())[i]);                 // extract fit results
    }
  }

  for(auto& hist : totPadiwaVec4) {                                                           // loop over histos
    fout->WriteObject(hist, hist->GetName());                                                 // write ToT vs channel to file
    //hist->FitSlicesY(landau,2,17,0,"QNR");                                                                 // fit 1D distributions with gaus
    hist->FitSlicesY(0,2,17);
    totPadiwaGausMean4.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));   // get fit mean values (written to 1D histos)
    fout->WriteObject(totPadiwaGausMean4.back(), totPadiwaGausMean4.back()->GetName());       // write fit results to file
    fitContentPadiwa4.emplace_back(std::vector<Float_t>());                                   // prepare extraction of fit results
    for(Int_t i=0; i<totPadiwaGausMean4.back()->GetSize(); i++) {
      fitContentPadiwa4.back().emplace_back((*totPadiwaGausMean4.back())[i]);                 // extract fit results
    }
  }

  TCanvas *c1 = new TCanvas("cToTDistsLayers","cToTDistsLayers");
  c1->DivideSquare(histCounter);

  Int_t padIter = 1;
  for(auto& hist : totLayerVec) {
    if(hist->GetEntries() == 0) { continue; }
    c1->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c2 = new TCanvas("cToTDistsPadiwas_C0","cToTDistsPadiwas_C0");
  c2->DivideSquare(8);

  padIter = 1;
  for(auto& hist : totPadiwaVec0) {
    c2->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c3 = new TCanvas("cToTDistsPadiwas_C4","cToTDistsPadiwas_C4");
  c3->DivideSquare(8);

  padIter = 1;
  for(auto& hist : totPadiwaVec4) {
    c3->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  fout->WriteObject(c1, c1->GetName());
  fout->WriteObject(c2, c2->GetName());
  fout->WriteObject(c3, c3->GetName());

  // get mean of measured values --> refValue for calibration
  Int_t fitIter = 0;
  for (auto& content : fitContentLayer) {
    for (Int_t i=2;i<content.size()-1;i++) {
      if (content.at(i) == 0) { continue; }
        meanToTDistVec.at(fitIter)->Fill(content.at(i));
      }
    fitIter++;
  }

  fitIter = 0;
  for (auto& content : fitContentPadiwa0) {
    for (Int_t i=2;i<content.size()-1;i++) {
      if (content.at(i) == 0) { continue; }
        meanToTDistVec0.at(fitIter)->Fill(content.at(i));
      }
    fitIter++;
  }

  fitIter = 0;
  for (auto& content : fitContentPadiwa4) {
    for (Int_t i=2;i<content.size()-1;i++) {
      if (content.at(i) == 0) { continue; }
        meanToTDistVec4.at(fitIter)->Fill(content.at(i));
      }
    fitIter++;
  }

  // write fit results to file
  std::ofstream calibOutput;
  calibOutput.open(outputCalib);
  calibOutput << "This file contains the gain map of the module in Padiwa and Fiber (in layer) mapping.\n";
  calibOutput << "Multiply the value for each fiber with the padiwa gain calibrated ToT value.\n";
  calibOutput << "DO NOT FORGET TO REMOVE THE LAST COMMA FOR EACH PADIWA/LAYER!!!\n\n\n";
  calibOutput << "Measurement with padiwa config 0:\n";
  

  //Float_t refValue = fitContentPadiwa0.at(0).at(2);
  fitIter = 0;

  for (auto& content : fitContentPadiwa0) {
    calibOutput << "Padiwa ";
    calibOutput << constants::padiwaNames.at(fitIter);
    calibOutput << "\n";
    refValue = meanToTDistVec0.at(fitIter)->GetMean();
    for (Int_t i=2;i<content.size()-1;i++) {
      calibOutput << content.at(i)/refValue;
      calibOutput << ", ";
      gainSpreadPadiwaVec0.at(fitIter)->Fill(content.at(i)/refValue);
    }
    calibOutput << "\n";
    fitIter++;
  }
  calibOutput << "\nMeasurement with padiwa config 4:\n";
  fitIter = 0;
  for (auto& content : fitContentPadiwa4) {
    calibOutput << "Padiwa ";
    calibOutput << constants::padiwaNames.at(fitIter);
    calibOutput << "\n";
    refValue = meanToTDistVec4.at(fitIter)->GetMean();
    for (Int_t i=2;i<content.size()-1;i++) {
      calibOutput << content.at(i)/refValue;
      calibOutput << ", ";
      gainSpreadPadiwaVec4.at(fitIter)->Fill(content.at(i)/refValue);
    }
    calibOutput << "\n";
    fitIter++;
  }
  calibOutput << "\n\n\n";
  fitIter = 0;
  for (auto& content : fitContentLayer) {
    calibOutput << "Layer ";
    calibOutput << layerMarker.at(fitIter);
    calibOutput << "\n";
    refValue = meanToTDistVec.at(fitIter)->GetMean();
    Int_t binIter = 1;
    for (Int_t i=2;i<content.size()-1;i++) {
      Float_t corrVal = content.at(i)/refValue;
      calibOutput << corrVal;
      calibOutput << ", ";
      gainSpreadLayerVec.at(fitIter)->Fill(corrVal);
      //meanToTDist->Fill(corrVal);
      Int_t layer = fitIter+1;
      Int_t maxLayerBin = 9;
      if (corrVal == 0) { binIter+=2; continue; }
      switch(layer) {
        case 1:
          gainMapHor->SetBinContent(binIter, maxLayerBin-layer, corrVal);
          gainMapHor->SetBinContent(binIter+1, maxLayerBin-layer, corrVal);
          break;
        case 2:
          gainMapVer->SetBinContent(binIter+1, maxLayerBin-layer, corrVal);
          gainMapVer->SetBinContent(binIter+2, maxLayerBin-layer, corrVal);
          break;
        case 3:
          gainMapHor->SetBinContent(binIter+1, maxLayerBin-layer, corrVal);
          gainMapHor->SetBinContent(binIter+2, maxLayerBin-layer, corrVal);
          break;
        case 4:
          gainMapVer->SetBinContent(binIter, maxLayerBin-layer, corrVal);
          gainMapVer->SetBinContent(binIter+1, maxLayerBin-layer, corrVal);
          break;
        case 5:
          gainMapHor->SetBinContent(binIter, maxLayerBin-layer, corrVal);
          gainMapHor->SetBinContent(binIter+1, maxLayerBin-layer, corrVal);
          break;
        case 6:
          gainMapVer->SetBinContent(binIter+1, maxLayerBin-layer, corrVal);
          gainMapVer->SetBinContent(binIter+2, maxLayerBin-layer, corrVal);
          break;
        case 7:
          gainMapHor->SetBinContent(binIter+1, maxLayerBin-layer, corrVal);
          gainMapHor->SetBinContent(binIter+2, maxLayerBin-layer, corrVal);
          break;
        case 8:
          gainMapVer->SetBinContent(binIter, maxLayerBin-layer, corrVal);
          gainMapVer->SetBinContent(binIter+1, maxLayerBin-layer, corrVal);
          break;
        default:
          printf("What happened?? layer: %i\n", layer);
      }
      binIter+=2;
    }
    calibOutput << "\n";

    fitIter++;
  }

  calibOutput.close();

  TCanvas *c4 = new TCanvas("cGainMapModule","cGainMapModule");
  c4->Divide(2,1);
  c4->cd(1);
  gainMapHor->SetStats(0);
  gainMapHor->GetZaxis()->SetRangeUser(0,2);
  gainMapHor->Draw("COLZ");
  c4->cd(2);
  gainMapVer->SetStats(0);
  gainMapVer->GetZaxis()->SetRangeUser(0,2);
  gainMapVer->Draw("COLZ");


  TCanvas *c5 = new TCanvas("cGainDistLayer","cGainDistLayer");
  c5->DivideSquare(histCounter);

  padIter = 1;
  for(auto& hist : gainSpreadLayerVec) {
    if(hist->GetEntries() == 0) { continue; }
    c5->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c6 = new TCanvas("cGainDistPadiwa_C0","cGainDistPadiwa_C0");
  c6->DivideSquare(8);

  padIter = 1;
  for(auto& hist : gainSpreadPadiwaVec0) {
    c6->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c7 = new TCanvas("cGainDistPadiwa_C4","cGainDistPadiwa_C4");
  c7->DivideSquare(8);

  padIter = 1;
  for(auto& hist : gainSpreadPadiwaVec4) {
    c7->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c9 = new TCanvas("cMeanSigDist","cMeanSigDist");
  c9->DivideSquare(8);

  padIter = 1;
  for(auto& hist : meanToTDistVec) {
    c9->cd(padIter);
    padIter++;
    hist->Draw("COLZ");
  }

  fout->WriteObject(c5, c5->GetName());
  fout->WriteObject(c6, c6->GetName());
  fout->WriteObject(c7, c7->GetName());
  fout->WriteObject(c9, c9->GetName());
  fout->WriteObject(c4, c4->GetName());

  TCanvas *c8 = new TCanvas("cGainMapModuleSmallerRange","cGainMapModuleSmallerRange");
  c8->Divide(2,1);
  c8->cd(1);
  gainMapHor->SetStats(0);
  gainMapHor->GetZaxis()->SetRangeUser(0.8,1.2);
  gainMapHor->Draw("COLZ");
  c8->cd(2);
  gainMapVer->SetStats(0);
  gainMapVer->GetZaxis()->SetRangeUser(0.8,1.2);
  gainMapVer->Draw("COLZ");

  fout->WriteObject(c8, c8->GetName());

  fout->Close();
}

int main(int argc, char** argv)
{
  char    inputFile0[1024]="";
  char    inputFile4[1024]="";
  char    outputFile[512]="getModuleGain_output.root";
  ULong_t procNr=-1;
  char    outputCalib[512]="getModuleGain_output.txt";

  int argsforloop;
  while ((argsforloop = getopt(argc, argv, "hi:j:o:n:c:")) != -1) {
    switch (argsforloop) {
      case '?':
        ///TODO: write usage function
        exit(EXIT_FAILURE);
      case 'i':
        strncpy(inputFile0, optarg, 1024);
        break;
      case 'j':
        strncpy(inputFile4, optarg, 1024);
        break;
      case 'o':
        strncpy(outputFile, optarg, 512);
        break;
      case 'n':
        procNr = std::atoi(optarg);
        break;
      case 'c':
        strncpy(outputCalib, optarg, 512);
        break;
      default:
        printf("\n\n%s%sdefault case%s\n\n",text::BOLD,text::RED,text::RESET);
        exit(EXIT_FAILURE);
    }
  }

  printf("\n\n%sRunning getModuleGain%s\n\n",text::BOLD,text::RESET);
  
  getModuleGain(inputFile0,inputFile4,outputFile,outputCalib,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}