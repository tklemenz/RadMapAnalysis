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
///< the m_layer vectors in line 92ff.

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
  std::vector<Float_t> m_layer1={0, 0.980398, 0.978084, 0.975228, 0.988607, 0.969823, 0.969143, 0.959248, 0.995796, 0.989669, 0.991485, 0.968221, 0.997254, 1.01999, 0.98105, 0.975545, 0.978229, 1.01511, 0.96422, 0.984722, 0.987102, 1.03741, 0.971447, 0.984549, 0.986343, 0.988689, 0.96236, 0.97442, 0.983497, 0.992683, 0.982134, 0.986275, 0.987759};
  std::vector<Float_t> m_layer2={0, 0.970717, 0.736973, 0.955337, 0.779739, 0.980622, 0.767107, 0.967731, 0.774137, 0.988615, 0.767526, 0.982822, 0.796816, 0.971367, 0.765580, 0.973434, 0.788902, 0.995368, 0.764792, 0.976379, 0.780512, 1.00028, 0.765414, 0.9845, 0.782801, 1.00812, 0.758892, 0.985479, 0.761687, 1.01829, 0.750317, 0.958597, 0.769731};
  std::vector<Float_t> m_layer3={0, 0.947787, 0.906891, 0.908512, 1.00018, 1.01061, 1.0069, 0.903888, 1.01331, 0.906381, 1.01163, 0.89069, 0.986111, 1.01014, 1.00063, 0.987379, 1.00999, 1.01924, 0.996947, 0.967361, 0.988597, 1.01786, 0.889358, 1.00796, 1.02296, 1.00609, 1.00598, 1.01172, 1.02444, 1.01269, 0.980039, 1.00308, 0.991231};
  std::vector<Float_t> m_layer4={0, 0.981513, 1.00926, 0.963164, 0.996509, 0.981614, 1.00857, 0.906335, 0.899273, 0.996063, 1.00505, 0.99259, 1.02693, 0.961012, 0.977713, 0.975824, 1.01636, 0.987906, 0.992124, 0.997929, 1.0082, 0.929116, 0.978803, 1.00925, 0.992574, 1.01994, 0.963573, 1.00181, 0.977544, 1.01568, 0.974182, 1.01562, 0.981291};
  std::vector<Float_t> m_layer5={};
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
   
    timeCalib = (timeStamp-refTime)*1e9 - time_calibration[Int_t(TDC)][Int_t(chID)];
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