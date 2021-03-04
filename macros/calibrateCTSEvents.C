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

///< usage: ./calibrateCTSEvents -i inputfile -o outputfileName -n numberOfSignalsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

void calibrateCTSEvents(const char *inputFile, const char *outputFile, ULong_t procNr)
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
  float m_layer1[33]={0.99, 0.97, 0.96, 1.0, 0.88, 0.95, 1.01, 0.96, 0.96, 0.91, 0.98, 0.96, 0.94, 0.98, 0.95, 0.95, 1.0, 1.01, 1.03, 1.03, 1.01, 1.04, 1.01, 0.99, 0.98, 0.96, 0.99, 0.91, 0.95, 1.01, 1.01, 0.96};
  float y0_layer1[33]={-0.56, 0.01, 0.16, -0.3, 1.19, -0.29, -0.31, 0.11, 1.02, 1.84, 0.27, 0.68, 0.99, -0.16, 0.39, 1.7, -0.5, -1.63, -0.95, -1.39, -0.39, -1.56, 0.17, -1.28, 0.48, -0.59, -1.2, 1.15, -0.3, -0.41, -1.5, -0.28};
  float m_layer2[33]={0.9, 0.85, 0.81, 0.9, 0.79, 0.82, 0.85, 0.82, 0.82, 0.79, 0.82, 0.84, 0.81, 0.89, 0.82, 0.87, 0.69, 0.69, 0.65, 0.59, 0.63, 0.67, 0.62, 0.63, 0.66, 0.68, 0.65, 0.68, 0.66, 0.68, 0.66, 0.6};
  float y0_layer2[33]={1.96, 1.95, 3.23, 1.97, 3.27, 2.77, 2.2, 2.14, 2.9, 3.05, 2.63, 1.54, 3.13, 1.32, 2.99, 2.48, 2.09, 2.1, 2.35, 3.22, 3.16, 2.48, 4.18, 2.86, 2.78, 2.27, 3.17, 1.9, 3.18, 2.0, 3.11, 3.78};
  float m_layer3[33]={1.25, 0.66, 1.23, 1.25, 1.17, 1.34, 1.26, 1.17, 1.14, 1.15, 1.2, 1.22, 1.19, 1.22, 1.16, 1.24, 1.22, 1.27, 1.33, 1.31, 1.29, 0.36, 1.24, 1.23, 1.23, 1.25, 1.22, 1.26, 1.15, 1.26, 1.24, 1.17};
  float y0_layer3[33]={-3.89, 2.46, -1.88, -4.09, -2.92, -6.09, -5.11, -1.42, -0.59, -2.47, -3.89, -3.66, -3.03, -4.23, -3.34, -4.73, -5.15, -5.66, -7.47, -6.34, -6.16, 14.12, -5.35, -5.09, -3.56, -3.58, -3.05, -4.27, -2.02, -3.65, -4.7, -2.97};
  float m_layer4[33]={1.14, 1.21, 1.15, 1.19, 1.09, 1.12, 1.08, 1.15, 1.16, 1.07, 1.05, 1.12, 1.13, 1.11, 1.09, 1.03, 1.22, 1.17, 1.19, 1.2, 1.19, 1.18, 1.14, 1.15, 1.15, 1.15, 1.14, 1.05, 1.13, 1.19, 1.16, 1.04};
  float y0_layer4[33]={-1.72, -5.08, -0.94, -3.44, -0.9, -3.02, -3.14, -3.65, -2.13, -1.14, 0.34, -1.86, -2.78, -2.01, -1.03, -1.16, -5.06, -3.55, -3.31, -4.24, -3.01, -3.39, -1.93, -2.53, -1.21, -4.04, -1.84, 0.15, -0.92, -3.89, -3.16, 0.22};

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

    //-----> Make channel mapping!
    if (int(layer) == 1){  
      switch(int(x)) {
        case 1: totCalib=ToT*m_layer1[16]+y0_layer1[16]; break;
        case 2: totCalib=ToT*m_layer1[15]+y0_layer1[15]; break;
        case 3: totCalib=ToT*m_layer1[17]+y0_layer1[17]; break;
        case 4: totCalib=ToT*m_layer1[14]+y0_layer1[14]; break;
        case 5: totCalib=ToT*m_layer1[18]+y0_layer1[18]; break;
        case 6: totCalib=ToT*m_layer1[13]+y0_layer1[13]; break;
        case 7: totCalib=ToT*m_layer1[19]+y0_layer1[19]; break;
        case 8: totCalib=ToT*m_layer1[12]+y0_layer1[12]; break;
        case 9: totCalib=ToT*m_layer1[20]+y0_layer1[20]; break;
        case 10: totCalib=ToT*m_layer1[11]+y0_layer1[11]; break;
        case 11: totCalib=ToT*m_layer1[21]+y0_layer1[21]; break;
        case 12: totCalib=ToT*m_layer1[10]+y0_layer1[10]; break;
        case 13: totCalib=ToT*m_layer1[22]+y0_layer1[22]; break;
        case 14: totCalib=ToT*m_layer1[9]+y0_layer1[9]; break;
        case 15: totCalib=ToT*m_layer1[23]+y0_layer1[23]; break;
        case 16: totCalib=ToT*m_layer1[8]+y0_layer1[8]; break;
        case 17: totCalib=ToT*m_layer1[24]+y0_layer1[24]; break;
        case 18: totCalib=ToT*m_layer1[7]+y0_layer1[7]; break;
        case 19: totCalib=ToT*m_layer1[25]+y0_layer1[25]; break;
        case 20: totCalib=ToT*m_layer1[6]+y0_layer1[6]; break;
        case 21: totCalib=ToT*m_layer1[26]+y0_layer1[26]; break;
        case 22: totCalib=ToT*m_layer1[5]+y0_layer1[5]; break;
        case 23: totCalib=ToT*m_layer1[27]+y0_layer1[27]; break;
        case 24: totCalib=ToT*m_layer1[4]+y0_layer1[4]; break;
        case 25: totCalib=ToT*m_layer1[28]+y0_layer1[28]; break;
        case 26: totCalib=ToT*m_layer1[3]+y0_layer1[3]; break;
        case 27: totCalib=ToT*m_layer1[29]+y0_layer1[29]; break;
        case 28: totCalib=ToT*m_layer1[2]+y0_layer1[2]; break;
        case 29: totCalib=ToT*m_layer1[30]+y0_layer1[30]; break;
        case 30: totCalib=ToT*m_layer1[1]+y0_layer1[1]; break;
        case 31: totCalib=ToT*m_layer1[31]+y0_layer1[31]; break;
        case 32: totCalib=ToT*m_layer1[0]+y0_layer1[0]; break;
        }
        
      } else if (int(layer) == 2){ 
      switch(int(y)) {
        case 1: totCalib=ToT*m_layer2[0]+y0_layer2[0]; break;
        case 2: totCalib=ToT*m_layer2[31]+y0_layer2[31]; break;
        case 3: totCalib=ToT*m_layer2[1]+y0_layer2[1]; break;
        case 4: totCalib=ToT*m_layer2[30]+y0_layer2[30]; break;
        case 5: totCalib=ToT*m_layer2[2]+y0_layer2[2]; break;
        case 6: totCalib=ToT*m_layer2[29]+y0_layer2[29]; break;
        case 7: totCalib=ToT*m_layer2[3]+y0_layer2[3]; break;
        case 8: totCalib=ToT*m_layer2[28]+y0_layer2[28]; break;
        case 9: totCalib=ToT*m_layer2[4]+y0_layer2[4]; break;
        case 10: totCalib=ToT*m_layer2[27]+y0_layer2[27]; break;
        case 11: totCalib=ToT*m_layer2[5]+y0_layer2[5]; break;
        case 12: totCalib=ToT*m_layer2[26]+y0_layer2[26]; break;
        case 13: totCalib=ToT*m_layer2[6]+y0_layer2[6]; break;
        case 14: totCalib=ToT*m_layer2[25]+y0_layer2[25]; break;
        case 15: totCalib=ToT*m_layer2[7]+y0_layer2[7]; break;
        case 16: totCalib=ToT*m_layer2[24]+y0_layer2[24]; break;
        case 17: totCalib=ToT*m_layer2[8]+y0_layer2[8]; break;
        case 18: totCalib=ToT*m_layer2[23]+y0_layer2[23]; break;
        case 19: totCalib=ToT*m_layer2[9]+y0_layer2[9]; break;
        case 20: totCalib=ToT*m_layer2[22]+y0_layer2[22]; break;
        case 21: totCalib=ToT*m_layer2[10]+y0_layer2[10]; break;
        case 22: totCalib=ToT*m_layer2[21]+y0_layer2[21]; break;
        case 23: totCalib=ToT*m_layer2[11]+y0_layer2[11]; break;
        case 24: totCalib=ToT*m_layer2[20]+y0_layer2[20]; break;
        case 25: totCalib=ToT*m_layer2[12]+y0_layer2[12]; break;
        case 26: totCalib=ToT*m_layer2[19]+y0_layer2[19]; break;
        case 27: totCalib=ToT*m_layer2[13]+y0_layer2[13]; break;
        case 28: totCalib=ToT*m_layer2[18]+y0_layer2[18]; break;
        case 29: totCalib=ToT*m_layer2[14]+y0_layer2[14]; break;
        case 30: totCalib=ToT*m_layer2[17]+y0_layer2[17]; break;
        case 31: totCalib=ToT*m_layer2[15]+y0_layer2[15]; break;
        case 32: totCalib=ToT*m_layer2[16]+y0_layer2[16]; break;
        } 
    }else if (int(layer) == 3){   
      switch(int(x)) {
        case 1: totCalib=ToT*m_layer3[16]+y0_layer3[16]; break;
        case 2: totCalib=ToT*m_layer3[15]+y0_layer3[15]; break;
        case 3: totCalib=ToT*m_layer3[17]+y0_layer3[17]; break;
        case 4: totCalib=ToT*m_layer3[14]+y0_layer3[14]; break;
        case 5: totCalib=ToT*m_layer3[18]+y0_layer3[18]; break;
        case 6: totCalib=ToT*m_layer3[13]+y0_layer3[13]; break;
        case 7: totCalib=ToT*m_layer3[19]+y0_layer3[19]; break;
        case 8: totCalib=ToT*m_layer3[12]+y0_layer3[12]; break;
        case 9: totCalib=ToT*m_layer3[20]+y0_layer3[20]; break;
        case 10: totCalib=ToT*m_layer3[11]+y0_layer3[11]; break;
        case 11: totCalib=ToT*m_layer3[21]+y0_layer3[21]; break;
        case 12: totCalib=ToT*m_layer3[10]+y0_layer3[10]; break;
        case 13: totCalib=ToT*m_layer3[22]+y0_layer3[22]; break;
        case 14: totCalib=ToT*m_layer3[9]+y0_layer3[9]; break;
        case 15: totCalib=ToT*m_layer3[23]+y0_layer3[23]; break;
        case 16: totCalib=ToT*m_layer3[8]+y0_layer3[8]; break;
        case 17: totCalib=ToT*m_layer3[24]+y0_layer3[24]; break;
        case 18: totCalib=ToT*m_layer3[7]+y0_layer3[7]; break;
        case 19: totCalib=ToT*m_layer3[25]+y0_layer3[25]; break;
        case 20: totCalib=ToT*m_layer3[6]+y0_layer3[6]; break;
        case 21: totCalib=ToT*m_layer3[26]+y0_layer3[26]; break;
        case 22: totCalib=ToT*m_layer3[5]+y0_layer3[5]; break;
        case 23: totCalib=ToT*m_layer3[27]+y0_layer3[27]; break;
        case 24: totCalib=ToT*m_layer3[4]+y0_layer3[4]; break;
        case 25: totCalib=ToT*m_layer3[28]+y0_layer3[28]; break;
        case 26: totCalib=ToT*m_layer3[3]+y0_layer3[3]; break;
        case 27: totCalib=ToT*m_layer3[29]+y0_layer3[29]; break;
        case 28: totCalib=ToT*m_layer3[2]+y0_layer3[2]; break;
        case 29: totCalib=ToT*m_layer3[30]+y0_layer3[30]; break;
        case 30: totCalib=ToT*m_layer3[1]+y0_layer3[1]; break;
        case 31: totCalib=ToT*m_layer3[31]+y0_layer3[31]; break;
        case 32: totCalib=ToT*m_layer3[0]+y0_layer3[0]; break;   
        }
      }else if (int(layer) == 4){
      switch(int(y)) {
        case 1: totCalib=ToT*m_layer4[0]+y0_layer4[0]; break;
        case 2: totCalib=ToT*m_layer4[31]+y0_layer4[31]; break;
        case 3: totCalib=ToT*m_layer4[1]+y0_layer4[1]; break;
        case 4: totCalib=ToT*m_layer4[30]+y0_layer4[30]; break;
        case 5: totCalib=ToT*m_layer4[2]+y0_layer4[2]; break;
        case 6: totCalib=ToT*m_layer4[29]+y0_layer4[29]; break;
        case 7: totCalib=ToT*m_layer4[3]+y0_layer4[3]; break;
        case 8: totCalib=ToT*m_layer4[28]+y0_layer4[28]; break;
        case 9: totCalib=ToT*m_layer4[4]+y0_layer4[4]; break;
        case 10: totCalib=ToT*m_layer4[27]+y0_layer4[27]; break;
        case 11: totCalib=ToT*m_layer4[5]+y0_layer4[5]; break;
        case 12: totCalib=ToT*m_layer4[26]+y0_layer4[26]; break;
        case 13: totCalib=ToT*m_layer4[6]+y0_layer4[6]; break;
        case 14: totCalib=ToT*m_layer4[25]+y0_layer4[25]; break;
        case 15: totCalib=ToT*m_layer4[7]+y0_layer4[7]; break;
        case 16: totCalib=ToT*m_layer4[24]+y0_layer4[24]; break;
        case 17: totCalib=ToT*m_layer4[8]+y0_layer4[8]; break;
        case 18: totCalib=ToT*m_layer4[23]+y0_layer4[23]; break;
        case 19: totCalib=ToT*m_layer4[9]+y0_layer4[9]; break;
        case 20: totCalib=ToT*m_layer4[22]+y0_layer4[22]; break;
        case 21: totCalib=ToT*m_layer4[10]+y0_layer4[10]; break;
        case 22: totCalib=ToT*m_layer4[21]+y0_layer4[21]; break;
        case 23: totCalib=ToT*m_layer4[11]+y0_layer4[11]; break;
        case 24: totCalib=ToT*m_layer4[20]+y0_layer4[20]; break;
        case 25: totCalib=ToT*m_layer4[12]+y0_layer4[12]; break;
        case 26: totCalib=ToT*m_layer4[19]+y0_layer4[19]; break;
        case 27: totCalib=ToT*m_layer4[13]+y0_layer4[13]; break;
        case 28: totCalib=ToT*m_layer4[18]+y0_layer4[18]; break;
        case 29: totCalib=ToT*m_layer4[14]+y0_layer4[14]; break;
        case 30: totCalib=ToT*m_layer4[17]+y0_layer4[17]; break;
        case 31: totCalib=ToT*m_layer4[15]+y0_layer4[15]; break;
        case 32: totCalib=ToT*m_layer4[16]+y0_layer4[16]; break;
      }      
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
  char    outputFile[512]="calibratedCTSEvents.root";
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

  printf("\n\n%sRunning calibrateCTSEvents%s\n\n",text::BOLD,text::RESET);
  
  calibrateCTSEvents(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}