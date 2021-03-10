#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <Rtypes.h>
#include <TCanvas.h>
#include <TNtupleD.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>

#include "Utility.h"

///< usage: ./getToTCalibrationValuesForPionData -i inputfile -o outputfile -c outputfileForCalibData -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed
///< Get calibration data for all fibers to pull the mean ToT to 15 ns.
///< This macro should be used to get the calibration data for different PADIWA configs.
///< Sabrina's calibration where pions and protons were used only works for config 0!
///< Since there is only proton data for config 0 we can only use pion data for the other configs.

extern char* optarg;

static Int_t   totCut = 8;   // cut away all signals with ToT<totCut to get rid of possible bias for gaus fit
static Float_t pullTo = 15;  // calib value that is written to file is pullTo/gausMean

void getToTCalibrationValuesForPionData(const char *inputFile, const char *outputFile, const char *outputCalib, ULong_t procNr)
{
  TFile* f = TFile::Open(inputFile);

  if (f->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input file failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }
  
  TNtupleD *signals = (TNtupleD*)f->Get("Signals");

  ULong_t nEvents = procNr;
  if ((nEvents == -1) || (nEvents > signals->GetEntries())) { nEvents = signals->GetEntries(); }

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

  /*========================================================
  ==========================================================*/

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

  /* Define histograms and other useful containers
  ==========================================================
  ==========================================================*/
  std::vector<TH2D*> totLayerVec{};
  for(Int_t i = 0; i<8; i++) {
    totLayerVec.emplace_back(new TH2D(Form("hToTL%i",i+1),"ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,500,0,50));
  }

  std::vector<TH1D*> totLayerGausMean{};
  std::vector<std::vector<Float_t>> fitContent{};
  std::vector<Int_t> layerMarker{};
  /*========================================================
  ==========================================================*/

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  printf("events to process: %lu\t %.1f%% of the file\n", nEvents, Float_t(100*nEvents)/Float_t(signals->GetEntries()));

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

    switch(Int_t(layer)){
      case 1:
        totLayerVec.at(0)->Fill(fiberNr, ToT);
        break;
      case 2:
        totLayerVec.at(1)->Fill(fiberNr, ToT);
        break;
      case 3:
        totLayerVec.at(2)->Fill(fiberNr, ToT);
        break;
      case 4:
        totLayerVec.at(3)->Fill(fiberNr, ToT);
        break;
      case 5:
        totLayerVec.at(4)->Fill(fiberNr, ToT);
        break;
      case 6:
        totLayerVec.at(5)->Fill(fiberNr, ToT);
        break;
      case 7:
        totLayerVec.at(6)->Fill(fiberNr, ToT);
        break;
      case 8:
        totLayerVec.at(7)->Fill(fiberNr, ToT);
        break;
      default:
        printf("%s%sThis layer should not appear!%s", text::RED, text::BOLD, text::RESET);
        break;
      }// end layer switch
  }// end of loop over file

  // loop over TH2D from above, fit 1D ToT distribution for all fibers and extract the mean value
  // meanwhile only take the layers that actually have data and leave the rest ignored
  for(auto& hist : totLayerVec) {                                                             // loop over histos
    layerIter++;
    if(hist->GetEntries() != 0) {
      layerMarker.emplace_back(layerIter);                                                    // write down which layers are used
      fout->WriteObject(hist, hist->GetName());                                               // write ToT vs fiber to file
      hist->FitSlicesY(0,2,33);                                                               // fit 1D distributions with gaus
      totLayerGausMean.emplace_back((TH1D*)gDirectory->Get(Form("%s_1", hist->GetName())));   // get fit mean values (written to 1D histos)
      fout->WriteObject(totLayerGausMean.back(), totLayerGausMean.back()->GetName());         // write fit results to file
      fitContent.emplace_back(std::vector<Float_t>());                                        // prepare extraction of fit results
      for(Int_t i=0; i<totLayerGausMean.back()->GetSize(); i++) {
        fitContent.back().emplace_back((*totLayerGausMean.back())[i]);                        // extract fit results
      }
    }
  }

  fout->Close();

  // write fit results to file
  std::ofstream calibOutput;
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

  calibOutput.close();
}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="getToTCalibrationValuesForPionData_output.root";
  ULong_t procNr=-1;
  char    outputCalib[512]="getToTCalibrationValuesForPionData_output.txt";

  int argsforloop;
  while ((argsforloop = getopt(argc, argv, "hi:o:n:c:")) != -1) {
    switch (argsforloop) {
      case '?':
        ///TODO: write usage function
        exit(EXIT_FAILURE);
      case 'i':
        strncpy(inputFile, optarg, 512);
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

  printf("\n\n%sRunning getToTCalibrationValuesForPionData%s\n\n",text::BOLD,text::RESET);
  
  getToTCalibrationValuesForPionData(inputFile,outputFile,outputCalib,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}