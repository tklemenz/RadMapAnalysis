#include <TH2.h>
#include <TH1.h>
#include <TTree.h>
#include <TFile.h>
#include <TNtupleD.h>
#include <TCanvas.h>
#include <Rtypes.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <unistd.h>

#include "CTSEvent.h"
#include "Utility.h"

///< usage: ./convertToCalibratedCTSEventsConfig2 -i inputfile -o outputfileName -n numberOfSignalsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

void convertToCalibratedCTSEventsConfig2(const char *inputFile, const char *outputFile, ULong_t procNr)
{
  TFile* f = TFile::Open(inputFile);

  if (f->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input file failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TNtupleD *signals = (TNtupleD*)f->Get("Signals");

  Double_t eventNr(-1), chID(-1), TDC(-1), layer(-1), x(-1), y(-1), signalNr(-1), timeStamp(-1), ToT(-1), padiwaConfig(-1), refTime(-1);
  Int_t prevSigNr(0), prevCh(-1), prevEventNr(-1), firstCounter(0), secondCounter(0);

  ULong_t nSignals = procNr;

  signals->SetBranchAddress("EventNr",      &eventNr);
  signals->SetBranchAddress("timeStamp",    &timeStamp);    // in seconds
  signals->SetBranchAddress("ToT",          &ToT);          // in seconds
  signals->SetBranchAddress("chID",         &chID);
  signals->SetBranchAddress("TDC",          &TDC);          // 0 = TDC1500, 1= 1510 etc
  signals->SetBranchAddress("layer",        &layer);        // 1-8
  signals->SetBranchAddress("x",            &x);            // odd layers have  x != 0
  signals->SetBranchAddress("y",            &y);            // even layers have y != 0
  signals->SetBranchAddress("signalNr",     &signalNr);     // Nth Signal per channel and event
  signals->SetBranchAddress("padiwaConfig", &padiwaConfig);
  signals->SetBranchAddress("refTime",      &refTime);  

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");
  TTree *tree = new TTree("dummy","RadMap data in fancy objects -> CTSEvents");

  // Setup Dataholders
  Signal    signal = Signal();
  Module    module = Module();
  CTSEvent *event;

  event = new CTSEvent();

  tree->Branch("Events","CTSEvent",&event,32000,1);

  if ((nSignals == -1) || (nSignals > signals->GetEntries())) { nSignals = signals->GetEntries(); }

  printf("signals to process: %lu\t %.1f%% of the file\n", nSignals, Float_t(100*nSignals)/Float_t(signals->GetEntries()));

  Double_t totCalib(0),timeCalib(0),ch1time(0);

  // -------------|| Hard Coded Calibration Data ||--------------------

  // Time shift on each channel relative to chID=1.
  // Measured with pulser on Padiwa.
  // These values are subtracted from timeStamp value to get calibrated time.
  Double_t time_calibration[4][33] = {
    {0,6.93889E-16,1.11542,0.8119,2.251916,-0.922404,-0.453114,0.8883776,0.155795,-0.938464,-1.540544,-0.809134,-1.552644,-0.692764,-0.464704,-2.004604,-2.406544,
      2.517826,2.66,3.397055,4.096766,2.263641,1.699249,0.276526,1.177086,-1.945154,-0.044184,-1.795594,-1.466974,5.072366,5.563236,3.990506,1.566279},
    {0,6.93889E-16,1.11542,1.24223,2.67931,-0.456091,-0.0149989,1.3712,0.504082,-0.370934,-1.08655,-0.454727,-1.31195,-0.195293,-0.0461142,-1.59216,-1.91812,
      2.891333,2.330744,2.8694499,4.381363,2.458993,2.126462,0.312753,1.072663,-2.297937,-0.179747,-1.741817,-1.241587,5.008823,5.596693,4.036363,1.282623},
    {0,6.93889E-16,1.11542,0.934946,2.728777,-0.540214,-0.227886,1.211543,0.485613,-0.730095,-1.434413,-0.420441,-1.241343,-0.531263,-0.35043,-1.803833,-2.009693,
      2.891333,2.0,2.890576515,3.845229,1.818433,1.217863,-0.1303969,0.809853,-2.326017,-0.38771699,-2.312257,-1.806617,4.719783,5.478213,3.806037,1.246643},
    {0,6.93889E-16,1.32542,1.16687,2.82989,-0.452611,-0.0387911,1.36946,0.700189,-0.505421,-0.957971,-0.212678,-1.27323,-0.0845989,0.0808505,-1.39277,-1.85497,
      2.517826,3.275819,4.234145,4.983406,3.082846,2.491707,1.043966,2.101356,-1.143034,0.760086,-0.998524,-0.563224,5.807756,6.502226,4.881126,2.503614}
  };

  // ToT calibration with linear fit.
  // Using both pion and proton data to calibrate the ToT values.
  // m_values for the slope and y0_values for teh shift of each channel
  // chID starts from 1, ref channel (chID=0) not calibratred!
  float m_layer1[33]={0, 0.975106, 1.09125, 0.906774, 0.988923, 0.966477, 1.00902, 0.932432, 1.01097, 0.979082, 1.02142, 0.941, 1.04641, 0.997607, 1.02764, 0.908964, 1.03419, 1.00324, 0.971538, 0.925964, 0.987523, 0.914487, 0.938877, 0.994346, 0.933565, 0.949536, 0.985283, 1.00796, 0.968422, 0.929738, 0.989346, 1.01149, 0.965194};
  float m_layer2[33]={0, 1.04855, 0.990932, 0.962627, 1.03055, 1.00831, 0.971632, 1.06059, 0.99613, 1.0197, 0.947494, 0.997609, 0.995147, 1.01973, 0.987422, 0.973528, 0.996918, 1.02655, 0.965132, 0.987713, 1.01601, 0.994695, 0.944142, 0.955496, 0.955997, 1.03921, 0.950993, 1.04568, 0.942561, 1.03856, 0.958633, 1.03989, 0.9636};
  float m_layer3[33]={0, 0.76377, 0.961713, 0.758617, 0.968433, 0.745293, 0.961146, 0.76199, 1.00553, 0.764594, 0.987596, 5.62301, 0.971614, 0.735597, 0.995886, 0.776896, 1.07954, 0.816225, 1.0636, 0.862682, 0.97399, 0.818613, 0.967282, 0.835445, 0.987879, 0.829486, 0.999452, 0.839513, 1.13684, 0.754693, 0.772621, 0.818451, 0.978579};
  float m_layer5[33]={0, 0.899763, 1.0084, 0.929395, 1.06833, 0.930941, 1.05063, 0.872938, 0.960517, 0.908892, 1.0441, 0.931786, 1.00519, 0.986032, 0.979411, 0.911216, 0.950648, 0.979689, 0.935476, 0.972321, 0.908471, 0.979762, 0.914682, 0.967147, 0.998851, 0.967698, 0.995074, 0.928137, 1.00586, 0.96538, 0.944436, 0.985556, 0.921549};

  // The loop
  for (ULong_t entry = 0; entry < nSignals; entry++) {
    if ((((entry+1)%100000) == 0) || (entry == (nSignals-1))) {
      printf("\rprocessing signal %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nSignals<<" % done\r"<<std::flush;
    }

    signals->GetEntry(entry);

    if ((ULong_t(eventNr) != prevEventNr) && (prevEventNr !=1)) {
      module.removeEmpty();
      event->setModule(module);
      tree->Fill();

      module.reset();
    }
   
    timeCalib = (timeStamp-refTime)*1e9 - time_calibration[int(layer)-1][int(chID)];
    //if(timeCalib<-1e10) { printf("Layer: %g, ChID: %g, TimeStamp: %g, Calibrated: %g, refTime: %g, Calibration: %g\n",layer,chID,timeStamp,timeCalib,refTime,time_calibration[int(layer)-1][int(chID)]);}
    if(int(chID)==1) { ch1time = timeCalib; }
    //printf("Layer:%g, ChID:%g,\t Timestamp:%1.15g,\t RefTime:%1.15g,\t timediff:%1.15g\t, timeToCh1:%1.15g\n",layer,chID,timeStamp,refTime,timeCalib,timeCalib-ch1time);
    ToT = ToT*1e9;

    Int_t fiberNr = mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID());

    //-----> Make channel mapping!
    if (int(layer) == 1){
      totCalib=ToT*m_layer1[fiberNr];
    } else if (int(layer) == 2){
      totCalib=ToT*m_layer2[fiberNr];
    }else if (int(layer) == 3){
      totCalib=ToT*m_layer3[fiberNr];
    }else if (int(layer) == 5){
      totCalib=ToT*m_layer5[fiberNr];
    }
    signal = Signal(totCalib,timeCalib,signalNr,chID,layer,TDC,padiwaConfig);
    //printf("timestamp: %g, calibStamp: %g, ToT: %g, calibToT: %g\n", timeStamp, timeCalib, ToT, totCalib);
    module.addSignal(signal);

    event->setEventNr(ULong_t(eventNr));
    event->setPadiwaConfig(UShort_t(padiwaConfig));

    prevEventNr = ULong_t(eventNr);
  }

  tree->Write("data");
  fout->Close();

  delete event;
  event=nullptr;
}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="convertToCalibratedCTSEventsConfig2_output.root";
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

  printf("\n\n%sRunning convertToCalibratedCTSEventsConfig2%s\n\n",text::BOLD,text::RESET);
  
  convertToCalibratedCTSEventsConfig2(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}