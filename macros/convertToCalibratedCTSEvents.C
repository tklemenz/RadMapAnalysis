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

///< usage: ./convertToCalibratedCTSEvents -i inputfile -o outputfileName -n numberOfSignalsToBeProcessed
///< n = -1 by default which means the whole file is processed

///< Put the ToT calibration values obtained from getToTCalibrationValuesForPionData in 
///< the m_layer vectors in line 94ff.

///< ****************************DISCLAIMER************************************
///< This macro should only be used for configurations where we have no proton data.
///< If you are using config 0 data, used the macro calibrateCTSEvents instead!!

extern char* optarg;

void convertToCalibratedCTSEvents(const char *inputFile, const char *outputFile, ULong_t procNr)
{
  TFile* f = TFile::Open(inputFile);

  if (f->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input file failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TNtupleD *signals = (TNtupleD*)f->Get("Signals");

  Double_t eventNr(-1), chID(-1), TDC(-1), layer(-1), x(-1), y(-1), signalNr(-1), timeStamp(-1), ToT(-1), padiwaConfig(-1), refTime(-1);
  Int_t prevEventNr(-1);
  std::vector<bool> validCalib{true, true, true, true, true, true, true, true};

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

  // These values are generated with getToTCalibrationValuesForPionData
  // 8ns ToT cut before gaus fit
  // Always start with 0! This 0 is NOT included in the file from getToTCalibrationValuesForPionData!
  // This is a quick and dirty workaround to get the fiber mapping easily, since fibers count from 1-32.
  std::vector<Float_t> m_layer1={0, 0.973623, 1.08884, 0.906606, 0.988692, 0.965917, 1.00841, 0.932258, 1.0104, 0.978365, 1.02079, 0.940607, 1.04544, 0.996903, 1.02625, 0.908814, 1.03361, 1.00199, 0.971239, 0.92579, 0.987413, 0.914471, 0.938814, 0.993366, 0.933484, 0.949151, 0.984715, 1.00621, 0.968344, 0.929649, 0.989006, 1.01015, 0.965033};
  std::vector<Float_t> m_layer2={0, 1.0468, 0.99013, 0.961835, 1.03008, 1.00682, 0.971521, 1.05855, 0.995901, 1.01866, 0.947466, 0.996692, 0.994994, 1.01873, 0.987281, 0.973199, 0.996706, 1.02566, 0.964769, 0.987154, 1.01531, 0.994223, 0.944047, 0.95533, 0.955714, 1.03778, 0.950734, 1.04406, 0.942525, 1.03755, 0.95852, 1.03678, 0.963331};
  std::vector<Float_t> m_layer3={0, 0.763727, 0.96156, 0.758461, 0.968221, 0.74528, 0.961028, 0.761833, 1.00513, 0.764522, 0.987437, 1.78224, 0.971418, 0.735598, 0.995543, 0.776752, 1.07689, 0.815314, 1.06239, 0.860992, 0.97387, 0.817949, 0.967225, 0.834694, 0.987269, 0.828428, 0.999144, 0.838402, 1.13491, 0.754524, 0.772622, 0.817306, 0.977953};
  std::vector<Float_t> m_layer4={};
  std::vector<Float_t> m_layer5={0, 0.899585, 1.00759, 0.929334, 1.06754, 0.930478, 1.0497, 0.872915, 0.960402, 0.908676, 1.04326, 0.931423, 1.0049, 0.984783, 0.979128, 0.910883, 0.950631, 0.978994, 0.93545, 0.972078, 0.908462, 0.978967, 0.914649, 0.966401, 0.998615, 0.967059, 0.99463, 0.927711, 1.00524, 0.964449, 0.944301, 0.984086, 0.921494};
  std::vector<Float_t> m_layer6={};
  std::vector<Float_t> m_layer7={};
  std::vector<Float_t> m_layer8={};
  // ------------------------------------------------------------------

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
   
    timeCalib = (timeStamp-refTime)*1e9 - time_calibration[Int_t(layer)-1][Int_t(chID)];
    //if(timeCalib<-1e10) { printf("Layer: %g, ChID: %g, TimeStamp: %g, Calibrated: %g, refTime: %g, Calibration: %g\n",layer,chID,timeStamp,timeCalib,refTime,time_calibration[int(layer)-1][int(chID)]);}
    if(Int_t(chID)==1) { ch1time = timeCalib; }
    //printf("Layer:%g, ChID:%g,\t Timestamp:%1.15g,\t RefTime:%1.15g,\t timediff:%1.15g\t, timeToCh1:%1.15g\n",layer,chID,timeStamp,refTime,timeCalib,timeCalib-ch1time);
    ToT = ToT*1e9;

    Int_t fiberNr = mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID());

    //-----> Make channel mapping!
    if (Int_t(layer) == 1){
      if (m_layer1.size()==33) { totCalib=ToT*m_layer1.at(fiberNr); }
      else { validCalib.at(0) = false; }
    } else if (Int_t(layer) == 2){
      if (m_layer2.size()==33) { totCalib=ToT*m_layer2.at(fiberNr); }
      else { validCalib.at(1) = false; }
    } else if (Int_t(layer) == 3){
      if (m_layer3.size()==33) { totCalib=ToT*m_layer3.at(fiberNr); }
      else { validCalib.at(2) = false; }
    } else if (Int_t(layer) == 4){
      if (m_layer4.size()==33) { totCalib=ToT*m_layer4.at(fiberNr); }
      else { validCalib.at(3) = false; }
    } else if (Int_t(layer) == 5){
      if (m_layer5.size()==33) { totCalib=ToT*m_layer5.at(fiberNr); }
      else { validCalib.at(4) = false; }
    } else if (Int_t(layer) == 6){
      if (m_layer6.size()==33) { totCalib=ToT*m_layer6.at(fiberNr); }
      else { validCalib.at(5) = false; }
    } else if (Int_t(layer) == 7){
      if (m_layer7.size()==33) { totCalib=ToT*m_layer7.at(fiberNr); }
      else { validCalib.at(6) = false; }
    } else if (Int_t(layer) == 8){
      if (m_layer8.size()==33) { totCalib=ToT*m_layer8.at(fiberNr); }
      else { validCalib.at(7) = false; }
    } else { printf("Unknown layer: %i! Something went REALLY wrong...\n", Int_t(layer)); }

    signal = Signal(totCalib,timeCalib,signalNr,chID,layer,TDC,padiwaConfig);
    //printf("timestamp: %g, calibStamp: %g, ToT: %g, calibToT: %g\n", timeStamp, timeCalib, ToT, totCalib);
    module.addSignal(signal);

    event->setEventNr(ULong_t(eventNr));
    event->setPadiwaConfig(UShort_t(padiwaConfig));

    prevEventNr = ULong_t(eventNr);
  }

  tree->Write("data");
  fout->Close();

  Int_t layerIter = 0;
  for(const auto& indicator : validCalib) {
    layerIter++;
    if (indicator == false) { printf("No ToT calibration provided for layer %i!\n", layerIter); }
  }

  delete event;
  event=nullptr;
}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="convertToCalibratedCTSEvents_output.root";
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

  printf("\n\n%sRunning convertToCalibratedCTSEvents%s\n\n",text::BOLD,text::RESET);
  
  convertToCalibratedCTSEvents(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}