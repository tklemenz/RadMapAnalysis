#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <Rtypes.h>
#include <TCanvas.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <unistd.h>

#include "CTSEvent.h"
#include "Utility.h"

///< usage: ./getEfficiencyFromConfig2 -i inputfile -o outputfile -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed
///< This macro runs on PADIWA config 2 data --> 3 full layers in Y-direction (1, 3, 5) and one fill layer in X-direction (2)

static Float_t synchrotronTimeWindow = 4;
static Int_t   totUppercut = 30;
static Int_t   totLowercut = 8;
static Int_t   fiberUppercut = 26;
static Int_t   fiberLowercut = 6;

extern char* optarg;

void getEfficiencyFromConfig2(const char *inputFile, const char *outputFile, ULong_t procNr)
{
  TFile* f = TFile::Open(inputFile);

  if (f->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input file failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TTree *data = (TTree*)f->Get("data");

  ULong_t nEvents = procNr;
  if ((nEvents == -1) || (nEvents > data->GetEntries())) { nEvents = data->GetEntries(); }

  /* Define variables
  ==========================================================
  ==========================================================*/
  Float_t eventNr      = -1;
  Int_t   padiwaConfig = -1;
  Module  module = Module();
  CTSEvent *event = nullptr;
  std::vector<Fiber> fibers;

  Int_t layer    = -1;
  Int_t x        = -1;
  Int_t y        = -1;
  Int_t fiberNr  = -1;
  Int_t fiberNr1 = -1;
  Int_t fiberNr3 = -1;
  Int_t fiberNr5 = -1;
  Double_t ToT1  = -99999;
  Double_t ToT3  = -99999;
  Double_t ToT5  = -99999;
  Double_t time1 = -99999;
  Double_t time3 = -99999;
  Double_t time5 = -99999;

  bool foundL1   = false;
  bool foundL3   = false;
  bool foundL5   = false;
  bool found15Pair = false;

  Double_t tot  = 0;
  Double_t time = 0;

  Int_t sigCounterL1 = 0;
  Int_t sigCounterL2 = 0;
  Int_t sigCounterL3 = 0;
  Int_t sigCounterL5 = 0;

  Int_t sigCounterGoodL3 = 0;
  Int_t sigCounterGoodL5 = 0;

  Float_t sumDFiber15 = 0;
  Float_t meanDFiber15 = 0;

  std::vector<std::vector<Signal>> signalVector{{},{},{},{}};
  /*========================================================
  ==========================================================*/

  /* Define histograms
  ==========================================================
  ==========================================================*/
  //beautify::setStyle();

  TH1I* hLayerDist = new TH1I("hLayerDist","Signal distribution across layers; layer;counts",8,0,8);// beautify::setStyleHisto<TH1>(hLayerDist);
  TH2D* hToTL1 = new TH2D("hToTL1","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);// beautify::setStyleHisto<TH2>(hToTL1);
  TH2D* hToTL2 = new TH2D("hToTL2","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);// beautify::setStyleHisto<TH2>(hToTL2);
  TH2D* hToTL3 = new TH2D("hToTL3","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);// beautify::setStyleHisto<TH2>(hToTL3);
  TH2D* hToTL5 = new TH2D("hToTL5","ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,1000,0,100);// beautify::setStyleHisto<TH2>(hToTL5);

  TH2D* hDFiber15VsToT1NoCuts = new TH2D("hDFiber15VsToT1NoCuts","fiber distance L1-L5 vs ToT L1;ToT L1;#Delta x L1-L5",1000,0,100,64,-32,32);
  TH2D* hDFiber15VsToT5NoCuts = new TH2D("hDFiber15VsToT5NoCuts","fiber distance L1-L5 vs ToT L5;ToT L5;#Delta x L1-L5",1000,0,100,64,-32,32);

  TH2D* hDFiber15VsToT1TimeCut = new TH2D("hDFiber15VsToT1TimeCut","fiber distance L1-L5 vs ToT L1;ToT L1;#Delta x L1-L5",1000,0,100,64,-32,32);
  TH2D* hDFiber15VsToT5TimeCut = new TH2D("hDFiber15VsToT5TimeCut","fiber distance L1-L5 vs ToT L5;ToT L5;#Delta x L1-L5",1000,0,100,64,-32,32);

  TH2D* hDFiber13VsToT1NoCuts = new TH2D("hDFiber13VsToT1NoCuts","fiber distance L1-L3 vs ToT L1;ToT L1;#Delta x L1-L3",1000,0,100,64,-32,32);
  TH2D* hDFiber13VsToT3NoCuts = new TH2D("hDFiber13VsToT3NoCuts","fiber distance L1-L3 vs ToT L3;ToT L3;#Delta x L1-L3",1000,0,100,64,-32,32);

  TH2D* hDFiber13VsToT1TimeCut = new TH2D("hDFiber13VsToT1TimeCut","fiber distance L1-L3 vs ToT L1;ToT L1;#Delta x L1-L3",1000,0,100,64,-32,32);
  TH2D* hDFiber13VsToT3TimeCut = new TH2D("hDFiber13VsToT3TimeCut","fiber distance L1-L3 vs ToT L3;ToT L3;#Delta x L1-L3",1000,0,100,64,-32,32);
  TH2D* hDFiber13VsToT3TimeCut15Found = new TH2D("hDFiber13VsToT3TimeCut15Found","fiber distance L1-L3 vs ToT L3 if there is a L1-L5 pair found;ToT L3;#Delta x L1-L3",1000,0,100,64,-32,32);

  TH2D* hMeanDFiber15VsFiberL2NoCut = new TH2D("hMeanDFiber15VsFiberL2NoCut","mean fiber distance L1-L5 (of CTS event) vs fiber L3;fiber L3;#topbar{|#Delta x L1-L5|}",34,0,34,64,0,32);
  TH2D* hMeanDFiber15VsFiberL2TimeCut = new TH2D("hMeanDFiber15VsFiberL2TimeCut","mean fiber distance L1-L5 (of CTS event) vs fiber L3 (within time window);fiber L3;#topbar{|#Delta x L1-L5|}",34,0,34,64,0,32);

  TH1I* hNCandidatesL3 = new TH1I("hNCandidatesL3","number of signals within time window on layer 3; n candidates;counts",100,0,100);
  TH1I* hNCandidatesL5 = new TH1I("hNCandidatesL5","number of signals within time window on layer 5; n candidates;counts",100,0,100);

  /*========================================================
  ==========================================================*/

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  data->SetBranchAddress("Events", &event);

  printf("events to process: %lu\t %.1f%% of the file\n", nEvents, Float_t(100*nEvents)/Float_t(data->GetEntries()));

  for (ULong_t entry = 0; entry < nEvents; entry++) {
    /* Make some fancy terminal output */
    if ((((entry+1)%10000) == 0) || (entry == (nEvents-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
    } /* End of terminal output*/

    data->GetEntry(entry);

    /* Reset variables for each event */
    //fibers.clear();
    //module.reset(); // this method is very slow!!
    eventNr      = -1;
    padiwaConfig = -1;
    for(auto& layer : signalVector) { layer.clear(); }

    /* Extract all info from event */
    eventNr      = event->getEventNr();
    padiwaConfig = event->getPadiwaConfig();
    module       = event->getModule();
    fibers       = module.getFibers();

    for(auto& fiber : fibers) {
      if(fiber.getSignals().size() != 0) {
        layer   = fiber.getLayer();
        x       = fiber.getX();
        y       = fiber.getY();
        fiberNr = (x==0 ? y : x);

        hLayerDist->Fill(layer);

        for(auto& signal : fiber.getSignals()) {
          if(signal.getSignalNr() == 1) {
            tot = signal.getToT();
            switch(layer){
              case 1:
                hToTL1->Fill(fiberNr, tot);
                signalVector.at(0).emplace_back(signal);
                break;
              case 2:
                hToTL2->Fill(fiberNr, tot);
                signalVector.at(1).emplace_back(signal);
                break;
              case 3:
                hToTL3->Fill(fiberNr, tot);
                signalVector.at(2).emplace_back(signal);
                break;
              case 5:
                hToTL5->Fill(fiberNr, tot);
                signalVector.at(3).emplace_back(signal);
                break;
              default:
                printf("%s%sLayer %i should not appear in PADIWA config 2!%s", text::RED, text::BOLD, layer, text::RESET);
                break;
            } // end layer switch
          } // end if first signal
        } // end loop over signals
      } // end if fiber not empty
    } // loop over fibers

    found15Pair = false;
    sumDFiber15 = 0;
    meanDFiber15 = 0;
    if(signalVector.at(0).size() == 1) {  // take only events where we have only one signal in the first layer --> makes things a lot less complicated
      Signal signal1 = signalVector.at(0).at(0);
      fiberNr1 = mapping::getFiberNr(signal1.getConfiguration(),signal1.getChannelID(),signal1.getTDCID());
      ToT1     = signal1.getToT();
      time1    = signal1.getTimeStamp();
      sigCounterGoodL3 = 0;
      sigCounterGoodL5 = 0;
      for(auto& signal5 : signalVector.at(3)) {
        fiberNr5 = mapping::getFiberNr(signal5.getConfiguration(),signal5.getChannelID(),signal5.getTDCID());
        ToT5     = signal5.getToT();
        time5    = signal5.getTimeStamp();
        hDFiber15VsToT1NoCuts->Fill(ToT1, fiberNr1-fiberNr5);
        hDFiber15VsToT5NoCuts->Fill(ToT5, fiberNr1-fiberNr5);
        if(std::abs(time1-time5) < synchrotronTimeWindow) {
          hDFiber15VsToT1TimeCut->Fill(ToT1, fiberNr1-fiberNr5);
          hDFiber15VsToT5TimeCut->Fill(ToT5, fiberNr1-fiberNr5);
          if(std::abs(fiberNr1-fiberNr5) == 0) {
            sigCounterGoodL5++;
            found15Pair = true;
            sumDFiber15 += std::abs(fiberNr1-fiberNr5);
          }
        }
      } // loop over layer 5
      if (found15Pair) { meanDFiber15 = sumDFiber15/sigCounterGoodL5; }
      for(auto& signal3 : signalVector.at(2)) {
        fiberNr3 = mapping::getFiberNr(signal3.getConfiguration(),signal3.getChannelID(),signal3.getTDCID());
        ToT3     = signal3.getToT();
        time3    = signal3.getTimeStamp();
        hDFiber13VsToT1NoCuts->Fill(ToT1, fiberNr1-fiberNr3);
        hDFiber13VsToT3NoCuts->Fill(ToT3, fiberNr1-fiberNr3);
        if(found15Pair) { hMeanDFiber15VsFiberL2NoCut->Fill(fiberNr3, meanDFiber15); }
        if(std::abs(time1-time3) < synchrotronTimeWindow) {
          hDFiber13VsToT1TimeCut->Fill(ToT1, fiberNr1-fiberNr3);
          hDFiber13VsToT3TimeCut->Fill(ToT3, fiberNr1-fiberNr3);
          sigCounterGoodL3++;
          if(found15Pair) {
            hDFiber13VsToT3TimeCut15Found->Fill(ToT3, fiberNr1-fiberNr3); // here are still events with multiple hit candidates per layer!!
            hMeanDFiber15VsFiberL2TimeCut->Fill(fiberNr3, meanDFiber15);
          }
        }
      }// loop over layer 3

      hNCandidatesL3->Fill(sigCounterGoodL3);
      hNCandidatesL5->Fill(sigCounterGoodL5);
    }
  } // loop over file

  fout->WriteObject(hLayerDist, "hLayerDist");
  fout->WriteObject(hToTL1, "hToTL1");
  fout->WriteObject(hToTL2, "hToTL2");
  fout->WriteObject(hToTL3, "hToTL3");
  fout->WriteObject(hToTL5, "hToTL5");
  fout->WriteObject(hDFiber15VsToT1NoCuts, hDFiber15VsToT1NoCuts->GetName());
  fout->WriteObject(hDFiber15VsToT5NoCuts, hDFiber15VsToT5NoCuts->GetName());
  fout->WriteObject(hDFiber13VsToT1NoCuts, hDFiber13VsToT1NoCuts->GetName());
  fout->WriteObject(hDFiber13VsToT3NoCuts, hDFiber13VsToT3NoCuts->GetName());
  fout->WriteObject(hDFiber15VsToT1TimeCut, hDFiber15VsToT1TimeCut->GetName());
  fout->WriteObject(hDFiber15VsToT5TimeCut, hDFiber15VsToT5TimeCut->GetName());
  fout->WriteObject(hDFiber13VsToT1TimeCut, hDFiber13VsToT1TimeCut->GetName());
  fout->WriteObject(hDFiber13VsToT3TimeCut, hDFiber13VsToT3TimeCut->GetName());
  fout->WriteObject(hDFiber13VsToT3TimeCut15Found, hDFiber13VsToT3TimeCut15Found->GetName());
  fout->WriteObject(hNCandidatesL3, hNCandidatesL3->GetName());
  fout->WriteObject(hNCandidatesL5, hNCandidatesL5->GetName());
  fout->WriteObject(hMeanDFiber15VsFiberL2NoCut, hMeanDFiber15VsFiberL2NoCut->GetName());
  fout->WriteObject(hMeanDFiber15VsFiberL2TimeCut, hMeanDFiber15VsFiberL2TimeCut->GetName());

  fout->Close();

}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="getEfficiencyFromConfig2_output.root";
  ULong_t procNr=-1;

  int argsforloop;
  while ((argsforloop = getopt(argc, argv, "hi:o:n:")) != -1) {
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
      default:
        printf("\n\n%s%sdefault case%s\n\n",text::BOLD,text::RED,text::RESET);
        exit(EXIT_FAILURE);
    }
  }

  printf("\n\n%sRunning getEfficiencyFromConfig2%s\n\n",text::BOLD,text::RESET);

  getEfficiencyFromConfig2(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}