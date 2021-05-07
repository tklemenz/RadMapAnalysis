#include <TTree.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <Rtypes.h>
#include <TCanvas.h>
#include <TNtupleD.h>
#include <TChain.h>

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

static Int_t   totCut = 0;   // cut away all signals with ToT<totCut to get rid of possible bias for gaus fit
static Float_t synchrotronTimeWindow = 4.2; // 3 sigma von da Vadeilung
static Int_t   spaceCut = 2;
static Int_t   fiberLowerCut = 0;
static Int_t   fiberUpperCut = 33;
static bool    useFiberCuts  = true;

static std::vector<std::string> distNames{"0-8mm","8-16mm","16-24mm","24-32mm","32-40mm","40-48mm","48-56mm","56-64mm"};

Int_t getFiberDistPos(Int_t fiber) {
  if (fiber >= 1 && fiber < 5) { return 0; }
  else if (fiber >= 5 && fiber < 9) { return 1; }
  else if (fiber >= 9 && fiber < 13) { return 2; }
  else if (fiber >= 13 && fiber < 17) { return 3; }
  else if (fiber >= 17 && fiber < 21) { return 4; }
  else if (fiber >= 21 && fiber < 25) { return 5; }
  else if (fiber >= 25 && fiber < 29) { return 6; }
  else if (fiber >= 29 && fiber < 33) { return 7; }
  else { return -1; }
}

/// returs pair of vecposition of L13 distance and vecposition of L24 distance
std::pair<Int_t, Int_t> getDist(Signal& vert, Signal& hor) {
  Int_t L13Pos = 0;
  Int_t L24Pos  = 0;
  bool L13odd = false;
  bool L24odd = false;
  PadiwaSocket vertSocket = mapping::getPadiwaSocket(mapping::getPadiwa(vert.getTDCID(), vert.getChannelID()), 0);
  PadiwaSocket horSocket  = mapping::getPadiwaSocket(mapping::getPadiwa(hor.getTDCID(), hor.getChannelID()), 0);

  if (vertSocket == PadiwaSocket::L1odd || vertSocket == PadiwaSocket::L3odd) { L13odd = true; }
  if (horSocket  == PadiwaSocket::L2odd || horSocket  == PadiwaSocket::L4odd) { L24odd = true; }

  Int_t fiberNrVer = mapping::getFiberNr(vert.getConfiguration(),vert.getChannelID(),vert.getTDCID());
  Int_t fiberNrHor = mapping::getFiberNr(hor.getConfiguration(), hor.getChannelID(), hor.getTDCID());

  if (L13odd && L24odd) {
    return { getFiberDistPos(fiberNrHor), getFiberDistPos(fiberNrVer) };
  }
  else if (!L13odd && L24odd) {
    return { 7-getFiberDistPos(fiberNrHor), getFiberDistPos(fiberNrVer) };
  }
  else if (L13odd && !L24odd) {
    return { getFiberDistPos(fiberNrHor), 7-getFiberDistPos(fiberNrVer) };
  }
  else {
    return { 7-getFiberDistPos(fiberNrHor), 7-getFiberDistPos(fiberNrVer) };
  }
}

void getEfficiencyFromConfig0(const TString inputFiles, const char *outputFile, ULong_t procNr)
{
  TChain chain("data", "data");
  fileHandling::makeChain(chain, inputFiles);
  TObjArray* files = chain.GetListOfFiles();
  printf("%sFiles to be processed:%s\n", text::BOLD, text::RESET);
  for (int ifile=0; ifile<files->GetEntriesFast(); ++ifile){ printf("%s\n", files->At(ifile)->GetTitle()); }

  /* Define variables
  ==========================================================
  ==========================================================*/
  ULong_t nEvents      = -1;
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

  bool hitL1 = false;
  bool hitL2 = false;
  bool hitL3 = false;
  bool hitL4 = false;

  bool multiHitL1 = false;
  bool multiHitL2 = false;
  bool multiHitL3 = false;
  bool multiHitL4 = false;

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
  std::string outputName = std::string();

  std::vector<std::vector<Signal>> hits{{},{},{},{}};

  /* Define histograms and other useful containers
  ==========================================================
  ==========================================================*/
  std::vector<TH2D*> totLayerVec{};
  std::vector<TH1D*> timeDiffVec{};
  std::vector<TH1I*> nSigPerEventPerLayerVec{};
  std::vector<TH1I*> layerNotHitVec{};

  for(int i = 0; i<4; i++) {
    timeDiffVec.emplace_back(new TH1D(Form("hTimeDiffL%i", i+1),Form("time diff of L%i sig to first sig in layer 1;time diff [ns]", i+1),1000,-10,10));
    totLayerVec.emplace_back(new TH2D(Form("hToTL%i", i+1),Form("ToT distribution of first signals vs fiber in L%i;fiber;ToT", i+1),33,0,33,500,0,50));
    nSigPerEventPerLayerVec.emplace_back(new TH1I(Form("hNSigPerEventL%i", i+1),Form("number of signals per CTS event Layer %i;n signals;n events", i+1),50,0,50));
    layerNotHitVec.emplace_back(new TH1I(Form("hEventQualityL%i", i+1),Form("Multiplicity indicator L%i;bin nr;counts", i+1),4,0,4));
  }

  std::vector<TH1I*> eventQualityVsDistVec{};
  std::vector<TH1I*> eventQualityVsDistVecL1{};
  std::vector<TH1I*> eventQualityVsDistVecL2{};
  std::vector<TH1I*> eventQualityVsDistVecL3{};
  std::vector<TH1I*> eventQualityVsDistVecL4{};
  std::vector<TH2D*> totVsDistVecL1{};
  std::vector<TH2D*> totVsDistVecL2{};
  std::vector<TH2D*> totVsDistVecL3{};
  std::vector<TH2D*> totVsDistVecL4{};
  for(auto& dist : distNames) {
    eventQualityVsDistVec.emplace_back(new TH1I(Form("hEventQuality_%s",dist.c_str()),"Multiplicity indicator;bin nr;counts",4,0,4));
    eventQualityVsDistVecL1.emplace_back(new TH1I(Form("hEventQualityL1_%s",dist.c_str()),Form("Multiplicity indicator %s;bin nr;counts", dist.c_str()),4,0,4));
    eventQualityVsDistVecL2.emplace_back(new TH1I(Form("hEventQualityL2_%s",dist.c_str()),Form("Multiplicity indicator %s;bin nr;counts", dist.c_str()),4,0,4));
    eventQualityVsDistVecL3.emplace_back(new TH1I(Form("hEventQualityL3_%s",dist.c_str()),Form("Multiplicity indicator %s;bin nr;counts", dist.c_str()),4,0,4));
    eventQualityVsDistVecL4.emplace_back(new TH1I(Form("hEventQualityL4_%s",dist.c_str()),Form("Multiplicity indicator %s;bin nr;counts", dist.c_str()),4,0,4));
    totVsDistVecL1.emplace_back(new TH2D(Form("hToTL1_%s", dist.c_str()),Form("ToT distribution vs fiber in L1, %s;fiber;ToT", dist.c_str()),33,0,33,500,0,50));
    totVsDistVecL2.emplace_back(new TH2D(Form("hToTL2_%s", dist.c_str()),Form("ToT distribution vs fiber in L2, %s;fiber;ToT", dist.c_str()),33,0,33,500,0,50));
    totVsDistVecL3.emplace_back(new TH2D(Form("hToTL3_%s", dist.c_str()),Form("ToT distribution vs fiber in L3, %s;fiber;ToT", dist.c_str()),33,0,33,500,0,50));
    totVsDistVecL4.emplace_back(new TH2D(Form("hToTL4_%s", dist.c_str()),Form("ToT distribution vs fiber in L4, %s;fiber;ToT", dist.c_str()),33,0,33,500,0,50));
  }

  for(auto& hist : totVsDistVecL1) { beautify::setStyleHisto<TH2>(hist); }
  for(auto& hist : totVsDistVecL2) { beautify::setStyleHisto<TH2>(hist); }
  for(auto& hist : totVsDistVecL3) { beautify::setStyleHisto<TH2>(hist); }
  for(auto& hist : totVsDistVecL4) { beautify::setStyleHisto<TH2>(hist); }
  for(auto& hist : totLayerVec)    { beautify::setStyleHisto<TH2>(hist); }

  TH3D* hTimeDiffDistMultL2 = new TH3D("hTimeDiffDistMultL2","TimeDiff btw L1 and L2 vs L1 sig number;sig nr;time diff",50,0,50,40,-25,25, 1.7e4, 0, 1.7e4);
  TH3D* hTimeDiffDistMultL3 = new TH3D("hTimeDiffDistMultL3","TimeDiff btw L1 and L3 vs L1 sig number;sig nr;time diff",50,0,50,40,-25,25, 1.7e4, 0, 1.7e4);
  TH3D* hTimeDiffDistMultL4 = new TH3D("hTimeDiffDistMultL4","TimeDiff btw L1 and L4 vs L1 sig number;sig nr;time diff",50,0,50,40,-25,25, 1.7e4, 0, 1.7e4);

  TH1I* eventNumbers  = new TH1I("EventNumbers",";eventNr;counts",2000000,0,2000000);
  TH1I* hNSigPerLayer = new TH1I("hNSigPerLayer","number of valid signals per layer;layer;counts",8,0,8);
  TH1I* hNMultDistL2  = new TH1I("hNMultDistL2",Form("N sig in layer 2 within +- %i ns of layer 1 signal;n sig;counts", Int_t(synchrotronTimeWindow)),50,0,50);
  TH1I* hNMultDistL3  = new TH1I("hNMultDistL3",Form("N sig in layer 3 within +- %i ns of layer 1 signal;n sig;counts", Int_t(synchrotronTimeWindow)),50,0,50);
  TH1I* hNMultDistL4  = new TH1I("hNMultDistL4",Form("N sig in layer 4 within +- %i ns of layer 1 signal;n sig;counts", Int_t(synchrotronTimeWindow)),50,0,50);

  ///< bin 0: exactly 3 layers have a signal within time window
  ///< bin 1: all layers have a signal within space and time windows
  ///< bin 2: all layers have a signal within time but not within space windows
  ///< bin 3: exactly 3 layers have one signal within time window, 1 layer has more than one signal within time window
  TH1I* hEventQuality = new TH1I("hEventQuality","Multiplicity indicator;bin nr;counts",4,0,4);
  TH1I* hEventQuality2 = new TH1I("hEventQuality2","Multiplicity indicator;bin nr;counts",4,0,4);

  /*========================================================
  ==========================================================*/
  if (fileHandling::splitString(inputFiles.Data(), ",").size() == 1) {
    outputName = fileHandling::splitString(fileHandling::splitString(outputFile).back().data(), ".").front().data();
    outputName.append("_");
    outputName.append(fileHandling::splitString(fileHandling::splitString(inputFiles.Data()).back().data(), ".").front().data());
    outputName.append(".root");
  }
  else { outputName = outputFile; }

  TFile *fout = new TFile(Form("%s",outputName.c_str()),"recreate");

  for (Int_t ifile=0; ifile<files->GetEntriesFast(); ++ifile){
    TFile *EventFile = new TFile(Form("%s", files->At(ifile)->GetTitle()));
    TTree *data = (TTree*)EventFile->Get("data");
    data->SetBranchAddress("Events", &event);
    nEvents = procNr;
    if ((nEvents == -1) || (nEvents > data->GetEntries())) { nEvents = data->GetEntries(); }
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
        if (layer == 5) { layer = 1; }
        if (layer == 6) { layer = 2; }
        if (layer == 7) { layer = 3; }
        if (layer == 8) { layer = 4; }
        for(auto& signal : fiber.getSignals()) {
          if(signal.getSignalNr() == 1) {
            ToT  = signal.getToT();
            if(ToT < totCut) { continue; }
            fiberNr = mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID());
            if (useFiberCuts) { if(fiberNr > fiberUpperCut || fiberNr < fiberLowerCut) { continue; } } // get rid of rim fibers
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
                if ( signal.getConfiguration()==0 && fiberNr == 11 ) { continue; }
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
        hitL1 = false;
        hitL2 = false;
        hitL3 = false;
        hitL4 = false;
        multiHitL1 = false;
        multiHitL2 = false;
        multiHitL3 = false;
        multiHitL4 = false;
        std::vector<Signal> goodSignalVec{Signal(),Signal(),Signal(),Signal()};
        for(auto& vec : hits) { vec.clear(); }
        time = signal1.getTimeStamp();
        signalCounter++;
        hits.at(0).emplace_back(signal1);

        goodSignalVec.at(0) = signal1;
        hitL1 = true;

        for(auto& signal2 : sigBufferL2) {
          ((TH3D*)hTimeDiffDistMultL2)->Fill(signalCounter,time-signal2.getTimeStamp(), eventNr);
          if(std::abs(time-signal2.getTimeStamp()) < synchrotronTimeWindow) { 
            sigInWindowCounterL2++;
            hits.at(1).emplace_back(signal2);
            if(sigInWindowCounterL2 == 1 ) { goodSignalVec.at(1) = signal2; goodL2=0; hitL2 = true; }
            if(sigInWindowCounterL2 >1) { goodL2=1; multiHitL2 = true; }
          }
        }
        for(auto& signal3 : sigBufferL3) {
          ((TH3D*)hTimeDiffDistMultL3)->Fill(signalCounter,time-signal3.getTimeStamp(), eventNr);
          if(std::abs(time-signal3.getTimeStamp()) < synchrotronTimeWindow/* && 
             std::abs(fiberNr-mapping::getFiberNr(signal3.getConfiguration(),signal3.getChannelID(),signal3.getTDCID()))<spaceCut*/) { 
            sigInWindowCounterL3++;
            hits.at(2).emplace_back(signal3);
            if(sigInWindowCounterL3 == 1 ) { goodSignalVec.at(2) = signal3; goodL3=0; hitL3 = true; }
            if(sigInWindowCounterL3 >1) { goodL3=1; multiHitL3 = true; }
          }
        }
        for(auto& signal4 : sigBufferL4) {
          ((TH3D*)hTimeDiffDistMultL4)->Fill(signalCounter,time-signal4.getTimeStamp(), eventNr);
          if(std::abs(time-signal4.getTimeStamp()) < synchrotronTimeWindow) { 
            sigInWindowCounterL4++;
            hits.at(3).emplace_back(signal4);
            if(sigInWindowCounterL4 == 1 ) { goodSignalVec.at(3) = signal4; goodL4=0; hitL4 = true; }
            if(sigInWindowCounterL4 >1) { goodL4=1; multiHitL4 = true; }
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
          if (std::abs(fiberNr1 - fiberNr3) < spaceCut && std::abs(fiberNr2 - fiberNr4) < spaceCut) { 
            hEventQuality->Fill(1); bin1Counter++;
            layerNotHitVec.at(3)->Fill(1);
            layerNotHitVec.at(0)->Fill(1);
            layerNotHitVec.at(1)->Fill(1);
            layerNotHitVec.at(2)->Fill(1);
          }
          else { 
            hEventQuality->Fill(2); bin2Counter++; 
            /*layerNotHitVec.at(3)->Fill(2);
            layerNotHitVec.at(0)->Fill(2);
            layerNotHitVec.at(1)->Fill(2);
            layerNotHitVec.at(2)->Fill(2);*/
          }
        }
        else if (howgood == 1) { // layer 1 and two other layers have exactly one signal, no info about other layer, yet
          if (sigInWindowCounterL2+sigInWindowCounterL3+sigInWindowCounterL4 ==2) { 
            hEventQuality->Fill(0); bin0Counter++;
            if (hitL1 && hitL2 && hitL3) { layerNotHitVec.at(3)->Fill(0); }
            if (hitL2 && hitL3 && hitL4) { layerNotHitVec.at(0)->Fill(0); }
            if (hitL3 && hitL4 && hitL1) { layerNotHitVec.at(1)->Fill(0); }
            if (hitL4 && hitL1 && hitL2) { layerNotHitVec.at(2)->Fill(0); }
          }  // other layer has no signal
          if (sigInWindowCounterL2+sigInWindowCounterL3+sigInWindowCounterL4 >=4) { 
            hEventQuality->Fill(3); bin3Counter++; 
            layerNotHitVec.at(3)->Fill(3);
            layerNotHitVec.at(0)->Fill(3);
            layerNotHitVec.at(1)->Fill(3);
            layerNotHitVec.at(2)->Fill(3);
          }  // other layer has more than one signal
        }
//---------------------------------------------------------------------------------------------------------
        std::vector<Int_t> nLayerHitsInTime{0,0,0,0};
        std::vector<Int_t> goodPart{1,1,1,1};
        for(UInt_t i = 0; i<4; i++) {
          nLayerHitsInTime.at(i) = hits.at(i).size();
          if( hits.at(i).size() == 1) { goodPart.at(i) = 0; }
        }

        // all layers have exactly 1 hit within time cut
        if(goodPart.at(1) + goodPart.at(2) + goodPart.at(3) == 0) {
          Int_t fiberNr1 = mapping::getFiberNr(hits.at(0).at(0).getConfiguration(),hits.at(0).at(0).getChannelID(),hits.at(0).at(0).getTDCID());
          Int_t fiberNr2 = mapping::getFiberNr(hits.at(1).at(0).getConfiguration(),hits.at(1).at(0).getChannelID(),hits.at(1).at(0).getTDCID());
          Int_t fiberNr3 = mapping::getFiberNr(hits.at(2).at(0).getConfiguration(),hits.at(2).at(0).getChannelID(),hits.at(2).at(0).getTDCID());
          Int_t fiberNr4 = mapping::getFiberNr(hits.at(3).at(0).getConfiguration(),hits.at(3).at(0).getChannelID(),hits.at(3).at(0).getTDCID());

          if (std::abs(fiberNr1 - fiberNr3) < spaceCut && std::abs(fiberNr2 - fiberNr4) < spaceCut) {
            hEventQuality2->Fill(1);
            std::pair<Int_t, Int_t> dists = getDist(hits.at(0).at(0), hits.at(1).at(0));
            eventQualityVsDistVecL1.at(std::get<0>(dists))->Fill(1);
            eventQualityVsDistVecL2.at(std::get<1>(dists))->Fill(1);
            eventQualityVsDistVecL3.at(std::get<0>(dists))->Fill(1);
            eventQualityVsDistVecL4.at(std::get<1>(dists))->Fill(1);
            totVsDistVecL1.at(std::get<0>(dists))->Fill(fiberNr1, hits.at(0).at(0).getToT());
            totVsDistVecL2.at(std::get<1>(dists))->Fill(fiberNr2, hits.at(1).at(0).getToT());
            totVsDistVecL3.at(std::get<0>(dists))->Fill(fiberNr3, hits.at(2).at(0).getToT());
            totVsDistVecL4.at(std::get<1>(dists))->Fill(fiberNr4, hits.at(3).at(0).getToT());

          } // space cut is good --> nice track
          else { 
            hEventQuality2->Fill(2);
            bool bad13 = true;
            bool bad24 = true;
            if(std::abs(fiberNr1 - fiberNr3) < spaceCut) { bad13 = false; }
            if(std::abs(fiberNr2 - fiberNr4) < spaceCut) { bad24 = false; }

            if(bad13) {
              // leave this one out because not enough info is there on the hit position, it is very few events anyway
            }
          }
        } // space cut is bad

        // layer 1 and two other layers have exactly 1 hit, no info on the other
        if(goodPart.at(1) + goodPart.at(2) + goodPart.at(3) == 1) {
          for(UInt_t i = 0; i<4; i++) {
            if(goodPart.at(i) == 1) {
              if(hits.at(i).size() == 0) {
                //layer i+i has no signal
                hEventQuality2->Fill(0);
                std::pair<Int_t, Int_t> dists = getDist(hits.at(0).at(0), hits.at(1).size() == 1 ? hits.at(1).at(0) : hits.at(3).at(0));
                if(i==1) { eventQualityVsDistVecL2.at(std::get<1>(dists))->Fill(0); }
                else if (i==2) { eventQualityVsDistVecL3.at(std::get<0>(dists))->Fill(0); }
                else if (i==3) { eventQualityVsDistVecL4.at(std::get<1>(dists))->Fill(0); }
                else { printf("there's again something going on\n"); }
              }
              else if (hits.at(i).size() > 1) {
                //layer i+1 has more than one signal
                hEventQuality2->Fill(3);

                Int_t fiberNr1 = mapping::getFiberNr(hits.at(0).at(0).getConfiguration(),hits.at(0).at(0).getChannelID(),hits.at(0).at(0).getTDCID());
                Int_t fiberNr2 = mapping::getFiberNr(hits.at(1).at(0).getConfiguration(),hits.at(1).at(0).getChannelID(),hits.at(1).at(0).getTDCID());
                Int_t fiberNr3 = mapping::getFiberNr(hits.at(2).at(0).getConfiguration(),hits.at(2).at(0).getChannelID(),hits.at(2).at(0).getTDCID());
                Int_t fiberNr4 = mapping::getFiberNr(hits.at(3).at(0).getConfiguration(),hits.at(3).at(0).getChannelID(),hits.at(3).at(0).getTDCID());
                if(i==1) { 
                  for(auto& signal : hits.at(i)) {
                    fiberNr2 = mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID());
                    if (std::abs(fiberNr2 - fiberNr4) < spaceCut) {
                      std::pair<Int_t, Int_t> dists = getDist(hits.at(0).at(0), signal);
                      eventQualityVsDistVecL1.at(std::get<0>(dists))->Fill(3);
                      eventQualityVsDistVecL2.at(std::get<1>(dists))->Fill(3);
                      eventQualityVsDistVecL3.at(std::get<0>(dists))->Fill(3);
                      eventQualityVsDistVecL4.at(std::get<1>(dists))->Fill(3);
                    }
                    break;
                  }
                }
                else if(i==2) { 
                  for(auto& signal : hits.at(i)) {
                    fiberNr3 = mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID());
                    if (std::abs(fiberNr1 - fiberNr3) < spaceCut) {
                      std::pair<Int_t, Int_t> dists = getDist(signal, hits.at(1).at(0));
                      eventQualityVsDistVecL1.at(std::get<0>(dists))->Fill(3);
                      eventQualityVsDistVecL2.at(std::get<1>(dists))->Fill(3);
                      eventQualityVsDistVecL3.at(std::get<0>(dists))->Fill(3);
                      eventQualityVsDistVecL4.at(std::get<1>(dists))->Fill(3);
                    }
                    break;
                  }
                }
                else if(i==3) { 
                  for(auto& signal : hits.at(i)) {
                    fiberNr4 = mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID());
                    if (std::abs(fiberNr2 - fiberNr4) < spaceCut) {
                      std::pair<Int_t, Int_t> dists = getDist(hits.at(0).at(0), signal);
                      eventQualityVsDistVecL1.at(std::get<0>(dists))->Fill(3);
                      eventQualityVsDistVecL2.at(std::get<1>(dists))->Fill(3);
                      eventQualityVsDistVecL3.at(std::get<0>(dists))->Fill(3);
                      eventQualityVsDistVecL4.at(std::get<1>(dists))->Fill(3);
                    }
                    break;
                  }
                }
                else { printf("there's again again something going on\n"); }
              }
              else {printf("there's something going on\n");}
            }
          }

        }
//---------------------------------------------------------------------------------------------------------

      }



      for(auto& signal2 : sigBufferL2) {

        goodL1 = 1;
        goodL3 = 1;
        goodL4 = 1;
        Int_t sigInWindowCounterL1 = 0;
        Int_t sigInWindowCounterL3 = 0;
        Int_t sigInWindowCounterL4 = 0;
        hitL1 = false;
        hitL2 = false;
        hitL3 = false;
        hitL4 = false;
        multiHitL1 = false;
        multiHitL2 = false;
        multiHitL3 = false;
        multiHitL4 = false;
        for(auto& vec : hits) { vec.clear(); }
        time = signal2.getTimeStamp();
        hitL2 = true;
        hits.at(1).emplace_back(signal2);
        for(auto& signal1 : sigBufferL1) {
          if(std::abs(time-signal1.getTimeStamp()) < synchrotronTimeWindow) { 
            sigInWindowCounterL1++;
            hits.at(0).emplace_back(signal1);
            if(sigInWindowCounterL1 == 1 ) { goodL1=0; hitL1 = true; }
            if(sigInWindowCounterL1 >1) { goodL1=1; multiHitL1 = true; }
          }
        }
        for(auto& signal3 : sigBufferL3) {
          if(std::abs(time-signal3.getTimeStamp()) < synchrotronTimeWindow) { 
            sigInWindowCounterL3++;
            hits.at(2).emplace_back(signal3);
            if(sigInWindowCounterL3 == 1 ) { goodL3=0; hitL3 = true; }
            if(sigInWindowCounterL3 >1) { goodL3=1; multiHitL3 = true; }
          }
        }
        for(auto& signal4 : sigBufferL4) {
          if(std::abs(time-signal4.getTimeStamp()) < synchrotronTimeWindow) { 
            sigInWindowCounterL4++;
            hits.at(3).emplace_back(signal4);
            if(sigInWindowCounterL4 == 1 ) { goodL4=0; hitL4 = true; }
            if(sigInWindowCounterL4 >1) { goodL4=1; multiHitL4 = true; }
          }
        }

        howgood = goodL1+goodL3+goodL4;
        if(howgood == 1 && goodL1 == 1) { // layer 2-4 have exactly 1 signal, layer 1 has 0 or more than 1
          if(sigInWindowCounterL1 == 0) { hEventQuality->Fill(0); bin0Counter++; layerNotHitVec.at(0)->Fill(0);} // layer 1 has no signal
          else if (sigInWindowCounterL1 > 1) { /*hEventQuality->Fill(3); bin3Counter++;*/ } // layer 1 has more than one signal
                                                                                            //(this is already looped over in the layer 1 loop)
          else { printf("This should never happen!\n"); } // layer 1 has exactly 1 signal
        }

//---------------------------------------------------------------------------------------------------------
        std::vector<Int_t> nLayerHitsInTime{0,0,0,0};
        std::vector<Int_t> goodPart{1,1,1,1};
        for(UInt_t i = 0; i<4; i++) {
          nLayerHitsInTime.at(i) = hits.at(i).size();
          if( hits.at(i).size() == 1) { goodPart.at(i) = 0; }
        }

        // layer 2 and two other layers have exactly 1 hit, no info on the other
        if(goodPart.at(0) + goodPart.at(2) + goodPart.at(3) == 1) {
          if(goodPart.at(0) == 1) {
            if(hits.at(0).size() == 0) {
              //layer 1 has no signal
              hEventQuality2->Fill(0);
              std::pair<Int_t, Int_t> dists = getDist(hits.at(2).at(0), hits.at(1).at(0));
              eventQualityVsDistVecL1.at(std::get<0>(dists))->Fill(0);
            }
          }
        }
//---------------------------------------------------------------------------------------------------------
      }

    }// end of loop over file
  }// end of loop over all files

  for (auto& hist : layerNotHitVec) {
    if (hist->GetEntries() > 0) { fout->WriteObject(hist, hist->GetName()); }
  }

  fout->WriteObject(hEventQuality, hEventQuality->GetName());
  fout->WriteObject(hEventQuality2, hEventQuality2->GetName());
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

  TCanvas *c1 = new TCanvas("cToTDists","cToTDists", 1500, 700);
  c1->Divide(2,2);
  c1->cd(1);
  gPad->SetLogz();
  totLayerVec.at(0)->Draw("COLZ");
  c1->cd(2);
  gPad->SetLogz();
  totLayerVec.at(1)->Draw("COLZ");
  c1->cd(3);
  gPad->SetLogz();
  totLayerVec.at(2)->Draw("COLZ");
  c1->cd(4);
  gPad->SetLogz();
  totLayerVec.at(3)->Draw("COLZ");

  TCanvas *c2 = new TCanvas("cTimeDiff","cTimeDiff", 1500, 700);
  c2->Divide(2,2);
  c2->cd(1);
  gPad->SetLogy();
  timeDiffVec.at(0)->Draw();
  c2->cd(2);
  gPad->SetLogy();
  timeDiffVec.at(1)->Draw();
  c2->cd(3);
  gPad->SetLogy();
  timeDiffVec.at(2)->Draw();
  c2->cd(4);
  gPad->SetLogy();
  timeDiffVec.at(3)->Draw();

  TCanvas *c3 = new TCanvas("cNMultDist","cNMultDist", 1500, 700);
  c3->Divide(3,1);
  c3->cd(1);
  gPad->SetLogy();
  hNMultDistL2->Draw();
  c3->cd(2);
  gPad->SetLogy();
  hNMultDistL3->Draw();
  c3->cd(3);
  gPad->SetLogy();
  hNMultDistL4->Draw();

  fout->WriteObject(c1, c1->GetName());
  fout->WriteObject(c2, c2->GetName());
  fout->WriteObject(c3, c3->GetName());

  Int_t padIter = 1;
  TCanvas *c4 = new TCanvas("cEventQualityVsDistL1","cEventQualityVsDistL1");
  c4->DivideSquare(8);
  padIter = 1;
  for(auto& hist : eventQualityVsDistVecL1) {
    c4->cd(padIter);
    hist->Draw();
    padIter++;
  }

  TCanvas *c5 = new TCanvas("cEventQualityVsDistL2","cEventQualityVsDistL2");
  c5->DivideSquare(8);
  padIter = 1;
  for(auto& hist : eventQualityVsDistVecL2) {
    c5->cd(padIter);
    hist->Draw();
    padIter++;
  }

  TCanvas *c6 = new TCanvas("cEventQualityVsDistL3","cEventQualityVsDistL3");
  c6->DivideSquare(8);
  padIter = 1;
  for(auto& hist : eventQualityVsDistVecL3) {
    c6->cd(padIter);
    hist->Draw();
    padIter++;
  }
  TCanvas *c7 = new TCanvas("cEventQualityVsDistL4","cEventQualityVsDistL4");
  c7->DivideSquare(8);
  padIter = 1;
  for(auto& hist : eventQualityVsDistVecL4) {
    c7->cd(padIter);
    hist->Draw();
    padIter++;
  }

  TCanvas *c8 = new TCanvas("cToTvsDistL1","cToTVsDistL1");
  c8->DivideSquare(8);
  padIter = 1;
  for(auto& hist : totVsDistVecL1) {
    c8->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c9 = new TCanvas("cToTvsDistL2","cToTVsDistL2");
  c9->DivideSquare(8);
  padIter = 1;
  for(auto& hist : totVsDistVecL2) {
    c9->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c10 = new TCanvas("cToTvsDistL3","cToTVsDistL3");
  c10->DivideSquare(8);
  padIter = 1;
  for(auto& hist : totVsDistVecL3) {
    c10->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  TCanvas *c11 = new TCanvas("cToTvsDistL4","cToTVsDistL4");
  c11->DivideSquare(8);
  padIter = 1;
  for(auto& hist : totVsDistVecL4) {
    c11->cd(padIter);
    gPad->SetLogz();
    hist->Draw("COLZ");
    padIter++;
  }

  fout->WriteObject(c4, c4->GetName());
  fout->WriteObject(c5, c5->GetName());
  fout->WriteObject(c6, c6->GetName());
  fout->WriteObject(c7, c7->GetName());
  fout->WriteObject(c8, c8->GetName());
  fout->WriteObject(c9, c9->GetName());
  fout->WriteObject(c10, c10->GetName());
  fout->WriteObject(c11, c11->GetName());

  fout->Close();

  printf("\n\nhEventquality 0: Number of events with exactly one signal on three layers, meeting time cuts, but no signal on the other layer:\n%s%i%s\n\n", text::BOLD,bin0Counter,text::RESET);
  printf("hEventQuality 1: Number of events with exactly one signal meeting time and space cuts in each layer:\n%s%i%s\n\n", text::BOLD,bin1Counter,text::RESET);
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