#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <Rtypes.h>
#include <TCanvas.h>
#include <TNtupleD.h>
#include <TF1.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>

#include "Utility.h"

///< usage: ./getPadiwaGainMap -i inputfilePionConfig0 -j inputfilePionConfig3 -k inputfileProtonConfig0
///<                           -o outputfile -c outputfileForCalibData -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

static Int_t   totCutPions   = 8;   // cut away all signals with ToT<totCutPions to get rid of possible bias for gaus fit
static Int_t   totLowerCutProtons = 15;
static Int_t   totUpperCutProtons = 35;

void getPadiwaGainMap(const char *inputFilePi0, const char *inputFilePi3, const char *inputFilePro0, const char *outputFile, const char *outputCalib, ULong_t procNr)
{
  TFile* f0 = TFile::Open(inputFilePi0);
  if (f0->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input filePi0 failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TFile* f3 = TFile::Open(inputFilePi3);
  if (f3->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input filePi3 failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TFile* fPro0 = TFile::Open(inputFilePro0);
  if (fPro0->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input filePro0 failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TNtupleD *signals0 = (TNtupleD*)f0->Get("Signals");
  TNtupleD *signals3 = (TNtupleD*)f3->Get("Signals");
  TNtupleD *signalsPro0 = (TNtupleD*)fPro0->Get("Signals");


  ULong_t nEvents0 = procNr;
  if ((nEvents0 == -1) || (nEvents0 > signals0->GetEntries())) { nEvents0 = signals0->GetEntries(); }

  ULong_t nEvents3 = procNr;
  if ((nEvents3 == -1) || (nEvents3 > signals3->GetEntries())) { nEvents3 = signals3->GetEntries(); }

  ULong_t nEventsPro0 = procNr;
  if ((nEventsPro0 == -1) || (nEventsPro0 > signalsPro0->GetEntries())) { nEventsPro0 = signalsPro0->GetEntries(); }

  /* Define variables
  ==========================================================
  ==========================================================*/
  Double_t eventNr0      = -1;  Double_t eventNr3      = -1;  Double_t eventNrPro0      = -1;
  Double_t timeStamp0    = -1;  Double_t timeStamp3    = -1;  Double_t timeStampPro0    = -1;
  Double_t ToT0          = -1;  Double_t ToT3          = -1;  Double_t ToTPro0          = -1;
  Double_t chID0         = -1;  Double_t chID3         = -1;  Double_t chIDPro0         = -1;
  Double_t TDC0          = -1;  Double_t TDC3          = -1;  Double_t TDCPro0          = -1;
  Double_t layer0        = -1;  Double_t layer3        = -1;  Double_t layerPro0        = -1;
  Double_t x0            = -1;  Double_t x3            = -1;  Double_t xPro0            = -1;
  Double_t y0            = -1;  Double_t y3            = -1;  Double_t yPro0            = -1;
  Double_t signalNr0     = -1;  Double_t signalNr3     = -1;  Double_t signalNrPro0     = -1;
  Double_t padiwaConfig0 = -1;  Double_t padiwaConfig3 = -1;  Double_t padiwaConfigPro0 = -1;
  Double_t refTime0      = -1;  Double_t refTime3      = -1;  Double_t refTimePro0      = -1;

  Int_t fiberNr         = -1;
  Int_t layerIter       =  0;

  /*========================================================
  ==========================================================*/

  signals0->SetBranchAddress("EventNr", &eventNr0);           signals3->SetBranchAddress("EventNr", &eventNr3);           signalsPro0->SetBranchAddress("EventNr", &eventNrPro0);
  signals0->SetBranchAddress("timeStamp", &timeStamp0);       signals3->SetBranchAddress("timeStamp", &timeStamp3);       signalsPro0->SetBranchAddress("timeStamp", &timeStampPro0);
  signals0->SetBranchAddress("ToT", &ToT0);                   signals3->SetBranchAddress("ToT", &ToT3);                   signalsPro0->SetBranchAddress("ToT", &ToTPro0);
  signals0->SetBranchAddress("chID", &chID0);                 signals3->SetBranchAddress("chID", &chID3);                 signalsPro0->SetBranchAddress("chID", &chIDPro0);
  signals0->SetBranchAddress("TDC", &TDC0);                   signals3->SetBranchAddress("TDC", &TDC3);                   signalsPro0->SetBranchAddress("TDC", &TDCPro0);
  signals0->SetBranchAddress("layer", &layer0);               signals3->SetBranchAddress("layer", &layer3);               signalsPro0->SetBranchAddress("layer", &layerPro0);
  signals0->SetBranchAddress("x", &x0);                       signals3->SetBranchAddress("x", &x3);                       signalsPro0->SetBranchAddress("x", &xPro0);
  signals0->SetBranchAddress("y", &y0);                       signals3->SetBranchAddress("y", &y3);                       signalsPro0->SetBranchAddress("y", &yPro0);
  signals0->SetBranchAddress("signalNr", &signalNr0);         signals3->SetBranchAddress("signalNr", &signalNr3);         signalsPro0->SetBranchAddress("signalNr", &signalNrPro0);
  signals0->SetBranchAddress("padiwaConfig", &padiwaConfig0); signals3->SetBranchAddress("padiwaConfig", &padiwaConfig3); signalsPro0->SetBranchAddress("padiwaConfig", &padiwaConfigPro0);
  signals0->SetBranchAddress("refTime", &refTime0);           signals3->SetBranchAddress("refTime", &refTime3);           signalsPro0->SetBranchAddress("refTime", &refTimePro0);

  /* Define histograms and other useful containers
  ==========================================================
  ==========================================================*/
  std::vector<TH2D*> totLayerVec0{};
  std::vector<TH2D*> totLayerVec3{};
  std::vector<TH2D*> totLayerVecPro0{};
  for(Int_t i=0; i<8; i++) {
    totLayerVec0.emplace_back(new TH2D(Form("hToTPi0L%i",i+1),"ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,500,0,50));
    totLayerVec3.emplace_back(new TH2D(Form("hToTPi3L%i",i+1),"ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,500,0,50));
    totLayerVecPro0.emplace_back(new TH2D(Form("hToTPro0L%i",i+1),"ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100));
  }

  std::vector<TH1D*> totLayerGausMean0{};
  std::vector<TH1D*> totLayerGausMean3{};
  std::vector<TH1D*> totLayerGausMeanPro0{};
  std::vector<std::vector<Float_t>> fitContent0{};
  std::vector<std::vector<Float_t>> fitContent3{};
  std::vector<std::vector<Float_t>> fitContentPro0{};
  std::vector<Int_t> layerMarker{};
  /*========================================================
  ==========================================================*/

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  printf("events to process in config 0 pion file: %lu\t %.1f%% of the file\n", nEvents0, Float_t(100*nEvents0)/Float_t(signals0->GetEntries()));

  for (ULong_t entry = 0; entry < nEvents0; entry++) {
    /* Make some fancy terminal output */
    if ((((entry+1)%10000) == 0) || (entry == (nEvents0-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents0<<" % done\r"<<std::flush;
    } /* End of terminal output*/

    signals0->GetEntry(entry);

    ToT0 *= 1e9; // convert ToT value to ns
    if (ToT0 < totCutPions) { continue; }                             // do not use small ToT values to not have gaus fit biased by noise
    if (signalNr0 != 1) { continue; }
    fiberNr = mapping::getFiberNr(padiwaConfig0, chID0, TDC0);

    switch(Int_t(layer0)){
      case 1:
        totLayerVec0.at(0)->Fill(fiberNr, ToT0);
        break;
      case 2:
        totLayerVec0.at(1)->Fill(fiberNr, ToT0);
        break;
      case 3:
        totLayerVec0.at(2)->Fill(fiberNr, ToT0);
        break;
      case 4:
        totLayerVec0.at(3)->Fill(fiberNr, ToT0);
        break;
      case 5:
        totLayerVec0.at(4)->Fill(fiberNr, ToT0);
        break;
      case 6:
        totLayerVec0.at(5)->Fill(fiberNr, ToT0);
        break;
      case 7:
        totLayerVec0.at(6)->Fill(fiberNr, ToT0);
        break;
      case 8:
        totLayerVec0.at(7)->Fill(fiberNr, ToT0);
        break;
      default:
        printf("%s%sThis layer should not appear!%s", text::RED, text::BOLD, text::RESET);
        break;
      }// end layer switch
  }// end of loop over file0

  printf("events to process in config 3 pion file: %lu\t %.1f%% of the file\n", nEvents3, Float_t(100*nEvents3)/Float_t(signals3->GetEntries()));

  for (ULong_t entry = 0; entry < nEvents3; entry++) {
    /* Make some fancy terminal output */
    if ((((entry+1)%10000) == 0) || (entry == (nEvents3-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents3<<" % done\r"<<std::flush;
    } /* End of terminal output*/

    signals3->GetEntry(entry);

    ToT3 *= 1e9; // convert ToT value to ns
    if (ToT3 < totCutPions) { continue; }                             // do not use small ToT values to not have gaus fit biased by noise
    if (signalNr3 != 1) { continue; }
    fiberNr = mapping::getFiberNr(padiwaConfig3, chID3, TDC3);

    switch(Int_t(layer3)){
      case 1:
        totLayerVec3.at(0)->Fill(fiberNr, ToT3);
        break;
      case 2:
        totLayerVec3.at(1)->Fill(fiberNr, ToT3);
        break;
      case 3:
        totLayerVec3.at(2)->Fill(fiberNr, ToT3);
        break;
      case 4:
        totLayerVec3.at(3)->Fill(fiberNr, ToT3);
        break;
      case 5:
        totLayerVec3.at(4)->Fill(fiberNr, ToT3);
        break;
      case 6:
        totLayerVec3.at(5)->Fill(fiberNr, ToT3);
        break;
      case 7:
        totLayerVec3.at(6)->Fill(fiberNr, ToT3);
        break;
      case 8:
        totLayerVec3.at(7)->Fill(fiberNr, ToT3);
        break;
      default:
        printf("%s%sThis layer should not appear in PADIWA config 2!%s", text::RED, text::BOLD, text::RESET);
        break;
      }// end layer switch
  }// end of loop over file0

  printf("events to process in config 0 proton file: %lu\t %.1f%% of the file\n", nEventsPro0, Float_t(100*nEventsPro0)/Float_t(signalsPro0->GetEntries()));

  for (ULong_t entry = 0; entry < nEventsPro0; entry++) {
    /* Make some fancy terminal output */
    if ((((entry+1)%10000) == 0) || (entry == (nEventsPro0-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEventsPro0<<" % done\r"<<std::flush;
    } /* End of terminal output*/

    signalsPro0->GetEntry(entry);

    ToTPro0 *= 1e9; // convert ToT value to ns
    if (ToTPro0 < totLowerCutProtons || ToTPro0 > totUpperCutProtons) { continue; }  // do not use small or large ToT values to not have gaus fit biased by noise
    if (signalNrPro0 != 1) { continue; }
    fiberNr = mapping::getFiberNr(padiwaConfigPro0, chIDPro0, TDCPro0);

    switch(Int_t(layerPro0)){
      case 1:
        totLayerVecPro0.at(0)->Fill(fiberNr, ToTPro0);
        break;
      case 2:
        totLayerVecPro0.at(1)->Fill(fiberNr, ToTPro0);
        break;
      case 3:
        totLayerVecPro0.at(2)->Fill(fiberNr, ToTPro0);
        break;
      case 4:
        totLayerVecPro0.at(3)->Fill(fiberNr, ToTPro0);
        break;
      case 5:
        totLayerVecPro0.at(4)->Fill(fiberNr, ToTPro0);
        break;
      case 6:
        totLayerVecPro0.at(5)->Fill(fiberNr, ToTPro0);
        break;
      case 7:
        totLayerVecPro0.at(6)->Fill(fiberNr, ToTPro0);
        break;
      case 8:
        totLayerVecPro0.at(7)->Fill(fiberNr, ToTPro0);
        break;
      default:
        printf("%s%sThis layer should not appear!%s", text::RED, text::BOLD, text::RESET);
        break;
      }// end layer switch
  }// end of loop over file0

  // loop over TH2D from above, fit 1D ToT distribution for all fibers and extract the mean value
  // meanwhile only take the layers that actually have data and leave the rest ignored
  for(auto& hist : totLayerVec0) {                                                              // loop over histos
    layerIter++;
    if(hist->GetEntries() != 0) {
      layerMarker.emplace_back(layerIter);                                                      // write down which layers are used
      fout->WriteObject(hist, hist->GetName());                                                 // write ToT vs fiber to file
      hist->FitSlicesY(0,2,33);                                                                 // fit 1D distributions with gaus
      totLayerGausMean0.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));    // get fit mean values (written to 1D histos)
      fout->WriteObject(totLayerGausMean0.back(), totLayerGausMean0.back()->GetName());         // write fit results to file
      fitContent0.emplace_back(std::vector<Float_t>());                                         // prepare extraction of fit results
      for(Int_t i=0; i<totLayerGausMean0.back()->GetSize(); i++) {
        fitContent0.back().emplace_back((*totLayerGausMean0.back())[i]);                        // extract fit results
      }
    }
  }

  // loop over TH2D from above, fit 1D ToT distribution for all fibers and extract the mean value
  // meanwhile only take the layers that actually have data and leave the rest ignored
  for(auto& hist : totLayerVec3) {                                                              // loop over histos
    if(hist->GetEntries() != 0) {
      fout->WriteObject(hist, hist->GetName());                                                 // write ToT vs fiber to file
      hist->FitSlicesY(0,2,33);                                                                 // fit 1D distributions with gaus
      totLayerGausMean3.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));    // get fit mean values (written to 1D histos)
      fout->WriteObject(totLayerGausMean3.back(), totLayerGausMean3.back()->GetName());         // write fit results to file
      fitContent3.emplace_back(std::vector<Float_t>());                                         // prepare extraction of fit results
      for(Int_t i=0; i<totLayerGausMean0.back()->GetSize(); i++) {
        fitContent3.back().emplace_back((*totLayerGausMean3.back())[i]);                        // extract fit results
      }
    }
  }

  // loop over TH2D from above, fit 1D ToT distribution for all fibers and extract the mean value
  // meanwhile only take the layers that actually have data and leave the rest ignored
  TF1 *landau = new TF1("landau","TMath::Landau(x,[0],[1],0)");
  for(auto& hist : totLayerVecPro0) {                                                            // loop over histos
    if(hist->GetEntries() != 0) {
      fout->WriteObject(hist, hist->GetName());                                                  // write ToT vs fiber to file
      hist->FitSlicesY(landau,2,33);                                                                  // fit 1D distributions with gaus
      totLayerGausMeanPro0.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));  // get fit mean values (written to 1D histos)
      fout->WriteObject(totLayerGausMeanPro0.back(), totLayerGausMeanPro0.back()->GetName());    // write fit results to file
      fitContentPro0.emplace_back(std::vector<Float_t>());                                       // prepare extraction of fit results
      for(Int_t i=0; i<totLayerGausMeanPro0.back()->GetSize(); i++) {
        fitContentPro0.back().emplace_back((*totLayerGausMeanPro0.back())[i]);                   // extract fit results
      }
    }
  }

  fout->Close();

  // write fit results to file
  /*std::ofstream calibOutput;
  calibOutput.open(outputCalib);
  calibOutput << "Multiply the value for each fiber with the uncalibrated ToT value to pull everything to 15 ns.\n";
  calibOutput << "DO NOT FORGET TO REMOVE THE LAST COMMA FOR EACH LAYER!!!\n";
  Int_t fitIter = 0;
  for (auto& content : fitContent) {
    calibOutput << "Layer ";
    calibOutput << layerMarker.at(fitIter);
    calibOutput << "\n";
    for (Int_t i=2;i<content.size()-1;i++) {
      calibOutput << pullTo/content.at(i);
      calibOutput << ", ";
    }
    calibOutput << "\n";
    fitIter++;
  }

  calibOutput.close();*/
}

int main(int argc, char** argv)
{
  char    inputFilePi0[512]="";
  char    inputFilePi3[512]="";
  char    inputFilePro0[512]="";
  char    outputFile[512]="getPadiwaGainMap_output.root";
  ULong_t procNr=-1;
  char    outputCalib[512]="getPadiwaGainMap_output.txt";

  int argsforloop;
  while ((argsforloop = getopt(argc, argv, "hi:j:k:o:n:c:")) != -1) {
    switch (argsforloop) {
      case '?':
        ///TODO: write usage function
        exit(EXIT_FAILURE);
      case 'i':
        strncpy(inputFilePi0, optarg, 512);
        break;
      case 'j':
        strncpy(inputFilePi3, optarg, 512);
        break;
      case 'k':
        strncpy(inputFilePro0, optarg, 512);
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

  printf("\n\n%sRunning getPadiwaGainMap%s\n\n",text::BOLD,text::RESET);
  
  getPadiwaGainMap(inputFilePi0,inputFilePi3,inputFilePro0,outputFile,outputCalib,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}