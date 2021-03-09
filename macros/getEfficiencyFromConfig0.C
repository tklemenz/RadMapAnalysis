#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <Rtypes.h>
#include <TCanvas.h>
#include <TNtupleD.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>

#include "CTSEvent.h"
#include "Utility.h"

///< usage: ./getEfficiencyFromConfig0 -i inputfile -o outputfile -c outputfileForCalibData -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

static Int_t   totCut = 4;   // cut away all signals with ToT<totCut to get rid of possible bias for gaus fit
static Float_t synchrotronTimeWindow = 4.2; // 3 sigma von da Vadeilung
static Int_t   spaceCut = 2;
static Int_t   fiberLowerCut = 5;
static Int_t   fiberUpperCut = 27;

void getEfficiencyFromConfig0(const char *inputFile, const char *outputFile, ULong_t procNr)
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

  CTSEvent *event      = nullptr;
  Module  module       = Module();
  std::vector<Fiber> fibers;

  Int_t fiberNr          = -1;
  Int_t layer            = -1;
  Double_t time          = -9999;
  Double_t ToT           = 0;

  Double_t timeRefLayer1 = -9999;

  Double_t refTime     = -99999;
  Double_t refX        = -1;
  Double_t refY        = -1;

  bool firstSigInLayer = true;
  bool foundLayer1Sig  = false;

  Int_t goodL1 = 1;
  Int_t goodL2 = 1;
  Int_t goodL3 = 1;
  Int_t goodL4 = 1;
  Int_t howgood = 0;

  std::vector<Signal> goodSignalVec{Signal(),Signal(),Signal(),Signal()};

  std::vector<Int_t> nSigCounterPerLayer{0,0,0,0,0,0,0,0};

  std::vector<Signal> sigBufferL1{};
  std::vector<Signal> sigBufferL2{};
  std::vector<Signal> sigBufferL3{};
  std::vector<Signal> sigBufferL4{};

  Int_t signalCounter = 0;
  Int_t bin0Counter   = 0;
  Int_t bin1Counter   = 0;
  Int_t bin2Counter   = 0;
  Int_t bin3Counter   = 0;

  Int_t iter = 0;
  /*========================================================
  ==========================================================*/

  data->SetBranchAddress("Events", &event);

  /* Define histograms and other useful containers
  ==========================================================
  ==========================================================*/
  std::vector<TH2D*> totLayerVec{};
  std::vector<TH1D*> timeDiffVec{};
  std::vector<TH1I*> nSigPerEventPerLayerVec{};

  for(int i = 0; i<4; i++) {
    timeDiffVec.emplace_back(new TH1D(Form("hTimeDiffL%i", i+1),Form("time diff to first sig in layer %i;time diff [ns]", i+1),1000,-10,10));
    totLayerVec.emplace_back(new TH2D(Form("hToTL%i", i+1),"ToT distribution of first signals vs fiber;fiber;ToT",33,0,33,500,0,50));
    nSigPerEventPerLayerVec.emplace_back(new TH1I(Form("hNSigPerEventL%i", i+1),Form("number of signals per event Layer %i;n signals;n events", i+1),50,0,50));
  }

  TH3D* hTimeDiffDistMultL2 = new TH3D("hTimeDiffDistMultL2","TimeDiff btw L1 and L2 vs L1 sig number;sig nr;time diff",50,0,50,40,-25,25, 1.7e4, 0, 1.7e4);
  TH3D* hTimeDiffDistMultL3 = new TH3D("hTimeDiffDistMultL3","TimeDiff btw L1 and L3 vs L1 sig number;sig nr;time diff",50,0,50,40,-25,25, 1.7e4, 0, 1.7e4);
  TH3D* hTimeDiffDistMultL4 = new TH3D("hTimeDiffDistMultL4","TimeDiff btw L1 and L4 vs L1 sig number;sig nr;time diff",50,0,50,40,-25,25, 1.7e4, 0, 1.7e4);

  TH1I* eventNumbers = new TH1I("EventNumbers",";eventNr;counts",2000000,0,2000000);
  TH1I* hNSigPerLayer = new TH1I("hNSigPerLayer","number of valid signals per layer;layer;counts",8,0,8);
  TH1I* hNMultDistL2 = new TH1I("hNMultDistL2","N sig in layer per event within +- 15 ns;n sig;counts",50,0,50);
  TH1I* hNMultDistL3 = new TH1I("hNMultDistL3","N sig in layer per event within +- 15 ns;n sig;counts",50,0,50);
  TH1I* hNMultDistL4 = new TH1I("hNMultDistL4","N sig in layer per event within +- 15 ns;n sig;counts",50,0,50);

  ///< bin 0: exactly 3 layers have a signal within time window
  ///< bin 1: all layers have a signal within space and time windows
  ///< bin 2: all layers have a signal within time but not within space windows
  ///< bin 3: exactly 3 layers have one signal within time window, 1 layer has more than one signal within time window
  TH1I* hEventQuality = new TH1I("hEventQuality","Multiplicity indicator;bin nr;counts",4,0,4);

  /*========================================================
  ==========================================================*/

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  printf("events to process: %lu\t %.1f%% of the file\n", nEvents, Float_t(100*nEvents)/Float_t(data->GetEntries()));

  for (ULong_t entry = 0; entry < nEvents; entry++) {
    /* Make some fancy terminal output */
    if ((((entry+1)%100000) == 0) || (entry == (nEvents-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
    } /* End of terminal output*/

    data->GetEntry(entry);

    eventNr      = event->getEventNr();
    padiwaConfig = event->getPadiwaConfig();
    module       = event->getModule();
    fibers       = module.getFibers();
    firstSigInLayer = true;
    foundLayer1Sig  = false;
    for(auto& counter : nSigCounterPerLayer) { counter = 0; }
    sigBufferL1.clear();
    sigBufferL2.clear();
    sigBufferL3.clear();
    sigBufferL4.clear();

    for(auto& fiber : fibers) {
      layer = fiber.getLayer();
      for(auto& signal : fiber.getSignals()) {
        if(signal.getSignalNr() == 1) {
          ToT  = signal.getToT();
          if(ToT < totCut) { continue; }
          fiberNr = mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID());
          time = signal.getTimeStamp();
          switch(layer){
            case 1:
              sigBufferL1.emplace_back(signal);
              if (firstSigInLayer) {
                timeRefLayer1 = signal.getTimeStamp();
                firstSigInLayer = false;
                foundLayer1Sig = true;
              }
              totLayerVec.at(0)->Fill(fiberNr, ToT);
              timeDiffVec.at(0)->Fill(timeRefLayer1-time);
              eventNumbers->Fill(eventNr);
              nSigCounterPerLayer.at(0) += 1;
              break;
            case 2:
              sigBufferL2.emplace_back(signal);
              if (!foundLayer1Sig) { continue; }
              totLayerVec.at(1)->Fill(fiberNr, ToT);
              timeDiffVec.at(1)->Fill(timeRefLayer1-time);
              eventNumbers->Fill(eventNr);
              nSigCounterPerLayer.at(1) += 1;
              break;
            case 3:
              sigBufferL3.emplace_back(signal);
              if (!foundLayer1Sig) { continue; }
              totLayerVec.at(2)->Fill(fiberNr, ToT);
              timeDiffVec.at(2)->Fill(timeRefLayer1-time);
              eventNumbers->Fill(eventNr);
              nSigCounterPerLayer.at(2) += 1;
              break;
            case 4:
              sigBufferL4.emplace_back(signal);
              if (!foundLayer1Sig) { continue; }
              totLayerVec.at(3)->Fill(fiberNr, ToT);
              timeDiffVec.at(3)->Fill(timeRefLayer1-time);
              eventNumbers->Fill(eventNr);
              nSigCounterPerLayer.at(3) += 1;
              break;
            default:
              printf("%s%sThis layer should not appear!%s", text::RED, text::BOLD, text::RESET);
              break;
          }// end layer switch
          hNSigPerLayer->Fill(layer);
        }
      } /// loop over signals in fiber
    } /// loop over fibers in module
    iter = 0;
    for (auto& hist : nSigPerEventPerLayerVec) {
      hist->Fill(nSigCounterPerLayer.at(iter));
      iter++;
    }
    signalCounter = 0;
    
    for(auto& signal1 : sigBufferL1) {
      Int_t sigInWindowCounterL2 = 0;
      Int_t sigInWindowCounterL3 = 0;
      Int_t sigInWindowCounterL4 = 0;
      goodL2=1;
      goodL3=1;
      goodL4=1;
      std::vector<Signal> goodSignalVec{Signal(),Signal(),Signal(),Signal()};
      time = signal1.getTimeStamp();
      fiberNr = mapping::getFiberNr(signal1.getConfiguration(),signal1.getChannelID(),signal1.getTDCID());
      signalCounter++;

      goodSignalVec.at(0) = signal1;

      for(auto& signal2 : sigBufferL2) {
        ((TH3D*)hTimeDiffDistMultL2)->Fill(signalCounter,time-signal2.getTimeStamp(), eventNr);
        if(std::abs(time-signal2.getTimeStamp()) < synchrotronTimeWindow) { 
          sigInWindowCounterL2++;
          if(sigInWindowCounterL2 == 1 ) { goodSignalVec.at(1) = signal2; goodL2=0; }
          if(sigInWindowCounterL2 >1) { goodL2=1; }
        }

      }
      for(auto& signal3 : sigBufferL3) {
        ((TH3D*)hTimeDiffDistMultL3)->Fill(signalCounter,time-signal3.getTimeStamp(), eventNr);
        if(std::abs(time-signal3.getTimeStamp()) < synchrotronTimeWindow/* && 
           std::abs(fiberNr-mapping::getFiberNr(signal3.getConfiguration(),signal3.getChannelID(),signal3.getTDCID()))<spaceCut*/) { 
          sigInWindowCounterL3++;
          if(sigInWindowCounterL3 == 1 ) { goodSignalVec.at(2) = signal3; goodL3=0; }
          if(sigInWindowCounterL3 >1) { goodL3=1; }
        }
      }
      for(auto& signal4 : sigBufferL4) {
        ((TH3D*)hTimeDiffDistMultL4)->Fill(signalCounter,time-signal4.getTimeStamp(), eventNr);
        if(std::abs(time-signal4.getTimeStamp()) < synchrotronTimeWindow) { 
          sigInWindowCounterL4++;
          if(sigInWindowCounterL4 == 1 ) { goodSignalVec.at(3) = signal4; goodL4=0; }
          if(sigInWindowCounterL4 >1) { goodL4=1; }
        }
      }
      hNMultDistL2->Fill(sigInWindowCounterL2);
      hNMultDistL3->Fill(sigInWindowCounterL3);
      hNMultDistL4->Fill(sigInWindowCounterL4);

      // Fill gathered info about event in histigram
      howgood = goodL2+goodL3+goodL4; 
      if(howgood == 0) { // all layers have exactly one signal
        Int_t fiberNr1 = mapping::getFiberNr(goodSignalVec.at(0).getConfiguration(),goodSignalVec.at(0).getChannelID(),goodSignalVec.at(0).getTDCID());
        Int_t fiberNr2 = mapping::getFiberNr(goodSignalVec.at(1).getConfiguration(),goodSignalVec.at(1).getChannelID(),goodSignalVec.at(1).getTDCID());
        Int_t fiberNr3 = mapping::getFiberNr(goodSignalVec.at(2).getConfiguration(),goodSignalVec.at(2).getChannelID(),goodSignalVec.at(2).getTDCID());
        Int_t fiberNr4 = mapping::getFiberNr(goodSignalVec.at(3).getConfiguration(),goodSignalVec.at(3).getChannelID(),goodSignalVec.at(3).getTDCID());
        if (std::abs(fiberNr1 - fiberNr3) < spaceCut && std::abs(fiberNr2 - fiberNr4) < spaceCut) { hEventQuality->Fill(1); bin1Counter++; }
        else { hEventQuality->Fill(2); bin2Counter++; }
      }
      else if (howgood == 1) { // layer 1 and two other layers have exactly one signal, no info about other layer, yet
        if (sigInWindowCounterL2+sigInWindowCounterL3+sigInWindowCounterL4 ==2) { hEventQuality->Fill(0); bin0Counter++; }  // other layer has no signal
        if (sigInWindowCounterL2+sigInWindowCounterL3+sigInWindowCounterL4 >=4) { hEventQuality->Fill(3); bin3Counter++; }  // other layer has more than one signal
      }
    }



    for(auto& signal2 : sigBufferL2) {
      goodL1 = 1;
      goodL3 = 1;
      goodL4 = 1;
      Int_t sigInWindowCounterL1 = 0;
      Int_t sigInWindowCounterL3 = 0;
      Int_t sigInWindowCounterL4 = 0;
      time = signal2.getTimeStamp();
      fiberNr = mapping::getFiberNr(signal2.getConfiguration(),signal2.getChannelID(),signal2.getTDCID());

      for(auto& signal1 : sigBufferL1) {
        if(std::abs(time-signal1.getTimeStamp()) < synchrotronTimeWindow) { 
          sigInWindowCounterL1++;
          if(sigInWindowCounterL1 == 1 ) { goodL1=0; }
          if(sigInWindowCounterL1 >1) { goodL1=1; }
        }
      }
      for(auto& signal3 : sigBufferL3) {
        if(std::abs(time-signal3.getTimeStamp()) < synchrotronTimeWindow) { 
          sigInWindowCounterL3++;
          if(sigInWindowCounterL3 == 1 ) { goodL3=0; }
          if(sigInWindowCounterL3 >1) { goodL3=1; }
        }
      }
      for(auto& signal4 : sigBufferL4) {
        if(std::abs(time-signal4.getTimeStamp()) < synchrotronTimeWindow) { 
          sigInWindowCounterL4++;
          if(sigInWindowCounterL4 == 1 ) { goodL4=0; }
          if(sigInWindowCounterL4 >1) { goodL4=1; }
        }
      }

      howgood = goodL1+goodL3+goodL4;
      if(howgood == 1 && goodL1 == 1) { // layer 2-4 have exactly 1 signal, layer 1 has 0 or more than 1
        if(sigInWindowCounterL1 == 0) { hEventQuality->Fill(0); bin0Counter++; } // layer 1 has no signal
        else if (sigInWindowCounterL1 > 1) { /*hEventQuality->Fill(3); bin3Counter++;*/ } // layer 1 has more than one signal
                                                                                          //(this is already loopen over in the layer 1 loop)
        else { printf("This should never happen!\n"); } // layer 1 has exactly 1 signal
      }
    }

  }// end of loop over file

  fout->WriteObject(hEventQuality, hEventQuality->GetName());
  fout->WriteObject(eventNumbers, eventNumbers->GetName());
  fout->WriteObject(hNSigPerLayer, hNSigPerLayer->GetName());
  fout->WriteObject(hTimeDiffDistMultL2, hTimeDiffDistMultL2->GetName());
  fout->WriteObject(hTimeDiffDistMultL3, hTimeDiffDistMultL3->GetName());
  fout->WriteObject(hTimeDiffDistMultL4, hTimeDiffDistMultL4->GetName());
  fout->WriteObject(hNMultDistL2, hNMultDistL2->GetName());
  fout->WriteObject(hNMultDistL3, hNMultDistL3->GetName());
  fout->WriteObject(hNMultDistL4, hNMultDistL4->GetName());


  for (auto& hist : totLayerVec) {
    if (hist->GetEntries() > 0) { fout->WriteObject(hist, hist->GetName()); }
  }

  for (auto& hist : timeDiffVec) {
    if (hist->GetEntries() > 0) { fout->WriteObject(hist, hist->GetName()); }
  }

  for (auto& hist : nSigPerEventPerLayerVec) {
    if (hist->GetEntries() > 0) { fout->WriteObject(hist, hist->GetName()); }
  }

  fout->Close();

  printf("\n\nhEventquality 0: Number of events with exactly one signal on three layers, meeting time cuts, but no signal on the other layer:\n%s%i%s\n\n", text::BOLD,bin0Counter,text::RESET);
  printf("hEventQuality 1:Number of events with exactly one signal meeting time and space cuts in each layer:\n%s%i%s\n\n", text::BOLD,bin1Counter,text::RESET);
  printf("hEventQuality 2: Number of events with exactly one signal in each layer meeting time cut but not meeting space cuts:\n%s%i%s\n\n", text::BOLD,bin2Counter,text::RESET);
  printf("hEventQuality 3: Number of events with exactly one signal on three layers but the other layer has more than one signal, all meet time cuts:\n%s%i%s\n\n", text::BOLD,bin3Counter,text::RESET);
  printf("SUM:\n%s%i%s\n", text::BOLD,bin0Counter+bin1Counter+bin2Counter+bin3Counter,text::RESET);
}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="getEfficiencyFromConfig0_output.root";
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

  printf("\n\n%sRunning getEfficiencyFromConfig0%s\n\n",text::BOLD,text::RESET);
  
  getEfficiencyFromConfig0(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}