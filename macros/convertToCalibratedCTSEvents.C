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
#include <map>

#include "CTSEvent.h"
#include "Utility.h"
#include "Constants.h"

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

  Double_t totCalib(0),timeCalib(0);

  // These values are generated with getToTCalibrationValuesForPionData
  // 8ns ToT cut before gaus fit
  // Always start with 0! This 0 is NOT included in the file from getToTCalibrationValuesForPionData!
  // This is a quick and dirty workaround to get the fiber mapping easily, since fibers count from 1-32.
  // from single SiPM calib in lab (this goes for config 0 and 4)
  /*std::vector<Float_t> m_layer1={0, 0.980398, 0.978084, 0.975228, 0.988607, 0.969823, 0.969143, 0.959248, 0.995796, 0.989669, 0.991485, 0.968221, 0.997254, 1.01999, 0.98105, 0.975545, 0.978229, 1.01511, 0.96422, 0.984722, 0.987102, 1.03741, 0.971447, 0.984549, 0.986343, 0.988689, 0.96236, 0.97442, 0.983497, 0.992683, 0.982134, 0.986275, 0.987759};
  std::vector<Float_t> m_layer2={0, 0.970717, 0.736973, 0.955337, 0.779739, 0.980622, 0.767107, 0.967731, 0.774137, 0.988615, 0.767526, 0.982822, 0.796816, 0.971367, 0.765580, 0.973434, 0.788902, 0.995368, 0.764792, 0.976379, 0.780512, 1.00028, 0.765414, 0.9845, 0.782801, 1.00812, 0.758892, 0.985479, 0.761687, 1.01829, 0.750317, 0.958597, 0.769731};
  std::vector<Float_t> m_layer3={0, 0.947787, 0.906891, 0.908512, 1.00018, 1.01061, 1.0069, 0.903888, 1.01331, 0.906381, 1.01163, 0.89069, 0.986111, 1.01014, 1.00063, 0.987379, 1.00999, 1.01924, 0.996947, 0.967361, 0.988597, 1.01786, 0.889358, 1.00796, 1.02296, 1.00609, 1.00598, 1.01172, 1.02444, 1.01269, 0.980039, 1.00308, 0.991231};
  std::vector<Float_t> m_layer4={0, 0.981513, 1.00926, 0.963164, 0.996509, 0.981614, 1.00857, 0.906335, 0.899273, 0.996063, 1.00505, 0.99259, 1.02693, 0.961012, 0.977713, 0.975824, 1.01636, 0.987906, 0.992124, 0.997929, 1.0082, 0.929116, 0.978803, 1.00925, 0.992574, 1.01994, 0.963573, 1.00181, 0.977544, 1.01568, 0.974182, 1.01562, 0.981291};
  std::vector<Float_t> m_layer5={0, 0.980398, 0.978084, 0.975228, 0.988607, 0.969823, 0.969143, 0.959248, 0.995796, 0.989669, 0.991485, 0.968221, 0.997254, 1.01999, 0.98105, 0.975545, 0.978229, 1.01511, 0.96422, 0.984722, 0.987102, 1.03741, 0.971447, 0.984549, 0.986343, 0.988689, 0.96236, 0.97442, 0.983497, 0.992683, 0.982134, 0.986275, 0.987759};
  std::vector<Float_t> m_layer6={0, 0.970717, 0.736973, 0.955337, 0.779739, 0.980622, 0.767107, 0.967731, 0.774137, 0.988615, 0.767526, 0.982822, 0.796816, 0.971367, 0.765580, 0.973434, 0.788902, 0.995368, 0.764792, 0.976379, 0.780512, 1.00028, 0.765414, 0.9845, 0.782801, 1.00812, 0.758892, 0.985479, 0.761687, 1.01829, 0.750317, 0.958597, 0.769731};
  std::vector<Float_t> m_layer7={0, 0.947787, 0.906891, 0.908512, 1.00018, 1.01061, 1.0069, 0.903888, 1.01331, 0.906381, 1.01163, 0.89069, 0.986111, 1.01014, 1.00063, 0.987379, 1.00999, 1.01924, 0.996947, 0.967361, 0.988597, 1.01786, 0.889358, 1.00796, 1.02296, 1.00609, 1.00598, 1.01172, 1.02444, 1.01269, 0.980039, 1.00308, 0.991231};
  std::vector<Float_t> m_layer8={0, 0.981513, 1.00926, 0.963164, 0.996509, 0.981614, 1.00857, 0.906335, 0.899273, 0.996063, 1.00505, 0.99259, 1.02693, 0.961012, 0.977713, 0.975824, 1.01636, 0.987906, 0.992124, 0.997929, 1.0082, 0.929116, 0.978803, 1.00925, 0.992574, 1.01994, 0.963573, 1.00181, 0.977544, 1.01568, 0.974182, 1.01562, 0.981291};
*/

  // from rigol 65ns signal in labv --> does not work
  /*std::vector<Float_t> m_layer1={0, 0.995365, 0.99371, 0.994793, 1.00239, 0.996147, 0.996499, 0.995264, 0.999085, 1.0016, 0.995269, 0.996205, 0.99853, 1.00527, 0.998658, 0.994918, 0.999748, 1.00355, 0.994416, 0.996016, 0.99807, 1.00244, 0.996193, 0.995127, 1.00056, 0.998457, 0.996518, 0.995965, 1.0018, 0.998662, 0.996813, 0.991471, 1.00035};
  std::vector<Float_t> m_layer2={0, 0.997772, 1.09523, 0.995485, 1.09928, 1.00095, 1.10442, 0.997023, 1.09458, 1.00011, 1.11442, 0.997326, 1.06048, 0.993939, 1.08489, 0.994572, 1.05147, 0.999901, 1.10176, 0.997254, 1.14222, 0.998542, 1.07496, 0.994113, 1.06252, 0.999818, 1.06005, 0.993536, 1.0904, 1.00164, 1.09084, 0.992823, 1.06163};
  std::vector<Float_t> m_layer3={0, 0.9949, 0.993827, 0.994002, 1.00317, 0.99688, 0.994857, 0.996023, 0.998955, 1.00193, 0.994263, 0.999044, 0.998209, 1.00709, 0.999844, 0.99675, 1.00075, 1.00481, 0.994873, 0.996135, 0.995564, 1.00191, 0.997252, 0.996524, 1.00048, 0.998873, 0.996014, 0.997667, 1.00039, 1.00066, 0.996421, 0.992182, 1.00148};
  std::vector<Float_t> m_layer4={0, 0.999065, 0.992368, 0.995767, 0.997719, 1.00075, 0.994754, 0.99797, 0.996106, 1.00059, 0.994992, 0.997889, 1.00213, 0.995845, 0.99505, 0.994877, 1.00378, 0.999613, 0.995558, 0.999603, 1.00538, 0.997966, 0.996746, 0.994559, 1.00028, 1.00099, 0.995315, 0.994898, 0.996763, 1.00323, 0.99369, 0.993762, 0.994564};
  std::vector<Float_t> m_layer5={};
  std::vector<Float_t> m_layer6={};
  std::vector<Float_t> m_layer7={};
  std::vector<Float_t> m_layer8={};*/
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

    timeCalib = (timeStamp-refTime)*1e9 - constants::padiwaTimeCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID));

    ToT = ToT*1e9;

    totCalib = ToT*
               constants::padiwaGainCorr.at(mapping::getPadiwa(Int_t(TDC), Int_t(chID))).at(mapping::getPadiwaChannel(chID))*
               constants::testModuleGainCorr.at(Int_t(layer)).at(mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID()));

    signal = Signal(totCalib,timeCalib,signalNr,chID,layer,TDC,padiwaConfig);
    module.addSignal(signal);

    event->setEventNr(ULong_t(eventNr));
    event->setPadiwaConfig(UShort_t(padiwaConfig));

    prevEventNr = ULong_t(eventNr);
  }

  tree->Write("data");
  fout->Close();

  /*Int_t layerIter = 0;
  for(const auto& indicator : validCalib) {
    layerIter++;
    if (indicator == false) { printf("No ToT calibration provided for layer %i!\n", layerIter); }
  }*/

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