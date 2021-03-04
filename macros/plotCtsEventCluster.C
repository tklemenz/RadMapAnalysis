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

#include "CTSEventClusters.h"
#include "Utility.h"

///< usage: ./plotCtsEventCluster -i inputfile -o outputfile -n numberOfEventsToBeProcessed
///< n = -1 by default which means the whole file is processed

extern char* optarg;

void plotCtsEventCluster(const char *inputFile, const char *outputFile, ULong_t procNr)
{
  TFile* f = TFile::Open(inputFile);

  if (f->IsOpen()==kFALSE){
    printf("\n\n%s%sOpen input file failed%s\n\n",text::BOLD,text::RED,text::RESET);
    exit(1);
  }

  TTree *data = (TTree*)f->Get("data");

  ULong_t nEvents = procNr;
  if ((nEvents == -1) || (nEvents > data->GetEntries())) { nEvents = data->GetEntries(); }

  CTSEventClusters *clusterEvent = nullptr;

  Float_t eventNr      = -1;
  Int_t   padiwaConfig = -1;
  Module  module       = Module();

  std::vector<Cluster> clusters;

  Int_t     clusterNSignal(-1), layer(-1), clusterNSignalbuffer[4];
  Double_t  qTot(-1), qMax(-1), timeMean(-1), qMaxBuffer[4];
  std::vector<std::vector<Double_t>> qToTBuffer{{},{},{},{}};  // temporary storage of qToT values of all clusters in event
  std::vector<std::vector<Double_t>> timeStampBuffer{{},{},{},{}};  // temporary storage of first timestamp values of all clusters in event
  Float_t   fibMean(-1), fibMeanBuffer[4];
  Bool_t    layerseen[4];
  Int_t     total(0),multi1(0),multi2(0),multi3(0),multi4(0),only1(0),only2(0),only3(0),only4(0),both34(0),buffer(0);
  Int_t     l12(0),l13(0),l14(0),l23(0),l24(0),l34(0);
  Int_t     l123(0),l124(0),l134(0),l234(0);

  data->SetBranchAddress("CTSEventsCluster", &clusterEvent);

  TFile *fout = new TFile(Form("%s",outputFile),"recreate");

  TH1D* hNSigInClus1 = new TH1D("hNSigInClus1","Layer 1;n;counts",10,0,10);
  TH1D* hNSigInClus2 = new TH1D("hNSigInClus2","Layer 2;n;counts",10,0,10);
  TH1D* hNSigInClus3 = new TH1D("hNSigInClus3","Layer 3;n;counts",10,0,10);
  TH1D* hNSigInClus4 = new TH1D("hNSigInClus4","Layer 4;n;counts",10,0,10);

  TH1D* hNClusInLayer1 = new TH1D("hNClusInLayer1","Layer 1;n;counts",50,0,50);
  TH1D* hNClusInLayer2 = new TH1D("hNClusInLayer2","Layer 2;n;counts",50,0,50);
  TH1D* hNClusInLayer3 = new TH1D("hNClusInLayer3","Layer 3;n;counts",50,0,50);
  TH1D* hNClusInLayer4 = new TH1D("hNClusInLayer4","Layer 4;n;counts",50,0,50);

  TH2D* hToTfibL131  = new TH2D("hToTfibL13_1","ToT distribution of clusters vs fiber at mult 2 L13 --> L1;fiber;ToT",33,0,33,1000,0,50);
  TH2D* hToTfibL133  = new TH2D("hToTfibL13_3","ToT distribution of clusters vs fiber at mult 2 L13 --> L3;fiber;ToT",33,0,33,1000,0,50);
  TH2D* hToTfibL242  = new TH2D("hToTfibL24_2","ToT distribution of clusters vs fiber at mult 2 L24 --> L2;fiber;ToT",33,0,33,1000,0,50);
  TH2D* hToTfibL244  = new TH2D("hToTfibL24_4","ToT distribution of clusters vs fiber at mult 2 L24 --> L4;fiber;ToT",33,0,33,1000,0,50);

  TH2D* hToTfibL1  = new TH2D("hToTfibL1","ToT distribution of clusters vs fiber;fiber;ToT",33,0,33,1000,0,50);
  TH2D* hToTfibL2  = new TH2D("hToTfibL2","ToT distribution of clusters vs fiber;fiber;ToT",33,0,33,1000,0,50);
  TH2D* hToTfibL3  = new TH2D("hToTfibL3","ToT distribution of clusters vs fiber;fiber;ToT",33,0,33,1000,0,50);
  TH2D* hToTfibL4  = new TH2D("hToTfibL4","ToT distribution of clusters vs fiber;fiber;ToT",33,0,33,1000,0,50);

  TH1D* hFibMeanL1 = new TH1D("hFibMeanL1","Fiber Mean Layer 1;fiber",330,0,33);
  TH1D* hFibMeanL2 = new TH1D("hFibMeanL2","Fiber Mean Layer 2;fiber",330,0,33);
  TH1D* hFibMeanL3 = new TH1D("hFibMeanL3","Fiber Mean Layer 3;fiber",330,0,33);
  TH1D* hFibMeanL4 = new TH1D("hFibMeanL4","Fiber Mean Layer 4;fiber",330,0,33);

  TH1D* hSigFibL1 = new TH1D("hSigFibL1","Signal Fiber Layer 1;fiber",330,0,33);
  TH1D* hSigFibL2 = new TH1D("hSigFibL2","Signal Fiber Layer 2;fiber",330,0,33);
  TH1D* hSigFibL3 = new TH1D("hSigFibL3","Signal Fiber Layer 3;fiber",330,0,33);
  TH1D* hSigFibL4 = new TH1D("hSigFibL4","Signal Fiber Layer 4;fiber",330,0,33);

  TH2D* hMulti2L12  = new TH2D("hMulti2L12","Multiplicity 2 - Layer 1&2;fiber distance;ToT",66,-33,33,100,0,35);
  TH2D* hMulti2L13  = new TH2D("hMulti2L13","Multiplicity 2 - Layer 1&3;fiber distance;ToT",66,-33,33,100,0,35);
  TH2D* hMulti2L14  = new TH2D("hMulti2L14","Multiplicity 2 - Layer 1&4;fiber distance;ToT",66,-33,33,100,0,35);
  TH2D* hMulti2L23  = new TH2D("hMulti2L23","Multiplicity 2 - Layer 2&3;fiber distance;ToT",66,-33,33,100,0,35);
  TH2D* hMulti2L24  = new TH2D("hMulti2L24","Multiplicity 2 - Layer 2&4;fiber distance;ToT",66,-33,33,100,0,35);
  TH2D* hMulti2L34  = new TH2D("hMulti2L34","Multiplicity 2 - Layer 3&4;fiber distance;ToT",66,-33,33,100,0,35);

  TH2D* hMulti2L12tot  = new TH2D("hMulti2L12tot","Multiplicity 2 - Layer 1&2;ToT L1;ToT L2",100,0,35,100,0,35);
  TH2D* hMulti2L13tot  = new TH2D("hMulti2L13tot","Multiplicity 2 - Layer 1&3;ToT L1;ToT L3",100,0,35,100,0,35);
  TH2D* hMulti2L14tot  = new TH2D("hMulti2L14tot","Multiplicity 2 - Layer 1&4;ToT L1;ToT L4",100,0,35,100,0,35);
  TH2D* hMulti2L23tot  = new TH2D("hMulti2L23tot","Multiplicity 2 - Layer 2&3;ToT L2;ToT L3",100,0,35,100,0,35);
  TH2D* hMulti2L24tot  = new TH2D("hMulti2L24tot","Multiplicity 2 - Layer 2&4;ToT L2;ToT L4",100,0,35,100,0,35);
  TH2D* hMulti2L34tot  = new TH2D("hMulti2L34tot","Multiplicity 2 - Layer 3&4;ToT L3;ToT L4",100,0,35,100,0,35);

  TH2D* hMulti2L12sigcount  = new TH2D("hMulti2L12sigcount","Multiplicity 2 - Layer 1&2;Signal count L1; Signal count L2",6,0,6,6,0,6);
  TH2D* hMulti2L13sigcount  = new TH2D("hMulti2L13sigcount","Multiplicity 2 - Layer 1&3;Signal count L1; Signal count L3",6,0,6,6,0,6);
  TH2D* hMulti2L14sigcount  = new TH2D("hMulti2L14sigcount","Multiplicity 2 - Layer 1&4;Signal count L1; Signal count L4",6,0,6,6,0,6);
  TH2D* hMulti2L23sigcount  = new TH2D("hMulti2L23sigcount","Multiplicity 2 - Layer 2&3;Signal count L2; Signal count L3",6,0,6,6,0,6);
  TH2D* hMulti2L24sigcount  = new TH2D("hMulti2L24sigcount","Multiplicity 2 - Layer 2&4;Signal count L2; Signal count L4",6,0,6,6,0,6);
  TH2D* hMulti2L34sigcount  = new TH2D("hMulti2L34sigcount","Multiplicity 2 - Layer 3&4;Signal count L3; Signal count L4",6,0,6,6,0,6);

  printf("events to process: %lu\t %.1f%% of the file\n", nEvents, Float_t(100*nEvents)/Float_t(data->GetEntries()));

  for (ULong_t entry = 0; entry < nEvents; entry++) {
    if ((((entry+1)%100000) == 0) || (entry == (nEvents-1))) {
      printf("\rprocessing event %lu...", entry+1);
      fflush(stdout);
      std::cout<<std::setw(5)<<std::setiosflags(std::ios::fixed)<<std::setprecision(1)<<" "<<(100.*(entry+1))/nEvents<<" % done\r"<<std::flush;
    }

    data->GetEntry(entry);

    clusters = clusterEvent->getClusters();
    for(int i=0;i<4;i++){ 
      layerseen[i]=false;
      qToTBuffer.at(i).clear();
    }

    for(auto& cluster : clusters) {
      clusterNSignal = cluster.getNSignals();
      layer = cluster.getLayer();
      qTot = cluster.getQTot();
      //if (qTot<0) { printf("ToT: %g\n", qTot); }
      qMax = cluster.getQMax();
      timeMean = cluster.getMeanTimeStamp();
      fibMean = cluster.getMeanFiber();

      if     (layer == 1) { hToTfibL1->Fill(fibMean,qTot);
                            hFibMeanL1->Fill(fibMean);
                            hNSigInClus1->Fill(clusterNSignal);
                            layerseen[0] = true;
                            fibMeanBuffer[0] = fibMean;
                            qMaxBuffer[0] = qTot;
                            qToTBuffer.at(0).emplace_back(qTot);
                            clusterNSignalbuffer[0] = clusterNSignal;
                            for(auto& signal : cluster.getSignals())
                              { hSigFibL1->Fill(mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID())); }
                          }
      else if(layer == 2) { hToTfibL2->Fill(fibMean,qTot);
                            hFibMeanL2->Fill(fibMean);
                            hNSigInClus2->Fill(clusterNSignal);
                            layerseen[1] = true;
                            fibMeanBuffer[1] = fibMean;
                            qMaxBuffer[1] = qTot;
                            qToTBuffer.at(1).emplace_back(qTot);
                            clusterNSignalbuffer[1] = clusterNSignal;
                            for(auto& signal : cluster.getSignals())
                              { hSigFibL2->Fill(mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID())); }
                          }
      else if(layer == 3) { hToTfibL3->Fill(fibMean,qTot);
                            hFibMeanL3->Fill(fibMean);
                            hNSigInClus3->Fill(clusterNSignal);
                            layerseen[2] = true;
                            fibMeanBuffer[2] = fibMean;
                            qMaxBuffer[2] = qTot;
                            qToTBuffer.at(2).emplace_back(qTot);
                            clusterNSignalbuffer[2] = clusterNSignal;
                            for(auto& signal : cluster.getSignals())
                              { hSigFibL3->Fill(mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID())); }
                          }
      else if(layer == 4) { hToTfibL4->Fill(fibMean,qTot);
                            hFibMeanL4->Fill(fibMean);
                            hNSigInClus4->Fill(clusterNSignal);
                            layerseen[3] = true;
                            fibMeanBuffer[3] = fibMean;
                            qMaxBuffer[3] = qTot;
                            qToTBuffer.at(3).emplace_back(qTot);
                            clusterNSignalbuffer[3] = clusterNSignal;
                            for(auto& signal : cluster.getSignals())
                              { hSigFibL4->Fill(mapping::getFiberNr(signal.getConfiguration(),signal.getChannelID(),signal.getTDCID())); }
                          }
      else { printf("\n\n%sNo histogram for given layer!%s", text::BLU, text::RESET); }

    } /// loop over fibers in module

    hNClusInLayer1->Fill(qToTBuffer.at(0).size());  // number of clusters in layer 1
    hNClusInLayer2->Fill(qToTBuffer.at(1).size());  // number of clusters in layer 2
    hNClusInLayer3->Fill(qToTBuffer.at(2).size());  // number of clusters in layer 3
    hNClusInLayer4->Fill(qToTBuffer.at(3).size());  // number of clusters in layer 4

    if(layerseen[0] == true && layerseen[1] == false && layerseen[2] == false && layerseen[3] == false) { only1++; }  // only layer 1 has at least one cluster
    if(layerseen[0] == false && layerseen[1] == true && layerseen[2] == false && layerseen[3] == false) { only2++; }  // only layer 2 has at least one cluster
    if(layerseen[0] == false && layerseen[1] == false && layerseen[2] == true && layerseen[3] == false) { only3++; }  // only layer 3 has at least one cluster
    if(layerseen[0] == false && layerseen[1] == false && layerseen[2] == false && layerseen[3] == true) { only4++; }  // only layer 4 has at least one cluster

    if(layerseen[0] == true && layerseen[1] == true && layerseen[2] == true && layerseen[3] == false) { l123++; }
    if(layerseen[0] == true && layerseen[1] == true && layerseen[2] == false && layerseen[3] == true) { l124++; }
    if(layerseen[0] == true && layerseen[1] == false && layerseen[2] == true && layerseen[3] == true) { l134++; }
    if(layerseen[0] == false && layerseen[1] == true && layerseen[2] == true && layerseen[3] == true) { l234++; }

    for(int i=0;i<4;i++){ if(layerseen[i]==true) { buffer++; } }
    switch(buffer){
      case 1: multi1++; break;
      case 2: multi2++; break;
      case 3: multi3++; break;
      case 4: multi4++; break;
    }

    if(buffer==2){
      if(layerseen[0] == true && layerseen[1] == true){ l12++; hMulti2L12->Fill(fibMeanBuffer[0]-fibMeanBuffer[1],qMaxBuffer[0]);
                                                               //hMulti2L12tot->Fill(qMaxBuffer[0],qMaxBuffer[1]);
                                                               hMulti2L12tot->Fill(qToTBuffer.at(0).at(0),qToTBuffer.at(1).at(0));
                                                               //hMulti2L12tot->Fill(qToTBuffer.at(0).back(),qToTBuffer.at(1).back());
                                                               hMulti2L12sigcount->Fill(clusterNSignalbuffer[0],clusterNSignalbuffer[1]);}
      if(layerseen[0] == true && layerseen[2] == true){ l13++; hMulti2L13->Fill(fibMeanBuffer[0]-fibMeanBuffer[2],qMaxBuffer[0]);
                                                               //hMulti2L13tot->Fill(qMaxBuffer[0],qMaxBuffer[2]);
                                                               hMulti2L13tot->Fill(qToTBuffer.at(0).at(0),qToTBuffer.at(2).at(0));
                                                               hToTfibL131->Fill(fibMeanBuffer[0],qToTBuffer.at(0).at(0));
                                                               hToTfibL133->Fill(fibMeanBuffer[2],qToTBuffer.at(2).at(0));
                                                               //hMulti2L13tot->Fill(qToTBuffer.at(0).back(),qToTBuffer.at(2).back());
                                                               hMulti2L13sigcount->Fill(clusterNSignalbuffer[0],clusterNSignalbuffer[2]);}
      if(layerseen[0] == true && layerseen[3] == true){ l14++; hMulti2L14->Fill(fibMeanBuffer[0]-fibMeanBuffer[3],qMaxBuffer[0]);
                                                               //hMulti2L14tot->Fill(qMaxBuffer[0],qMaxBuffer[3]);
                                                               hMulti2L14tot->Fill(qToTBuffer.at(0).at(0),qToTBuffer.at(3).at(0));
                                                               //hMulti2L14tot->Fill(qToTBuffer.at(0).back(),qToTBuffer.at(3).back());
                                                               hMulti2L14sigcount->Fill(clusterNSignalbuffer[0],clusterNSignalbuffer[3]);}
      if(layerseen[1] == true && layerseen[2] == true){ l23++; hMulti2L23->Fill(fibMeanBuffer[1]-fibMeanBuffer[2],qMaxBuffer[1]);
                                                               //hMulti2L23tot->Fill(qMaxBuffer[1],qMaxBuffer[2]);
                                                               hMulti2L23tot->Fill(qToTBuffer.at(1).at(0),qToTBuffer.at(2).at(0));
                                                               //hMulti2L23tot->Fill(qToTBuffer.at(1).back(),qToTBuffer.at(2).back());
                                                               hMulti2L23sigcount->Fill(clusterNSignalbuffer[1],clusterNSignalbuffer[2]);}
      if(layerseen[1] == true && layerseen[3] == true){ l24++; hMulti2L24->Fill(fibMeanBuffer[1]-fibMeanBuffer[3],qMaxBuffer[1]);
                                                               //hMulti2L24tot->Fill(qMaxBuffer[1],qMaxBuffer[3]);
                                                               hMulti2L24tot->Fill(qToTBuffer.at(1).at(0),qToTBuffer.at(3).at(0));
                                                               hToTfibL242->Fill(fibMeanBuffer[1],qToTBuffer.at(1).at(0));
                                                               hToTfibL244->Fill(fibMeanBuffer[3],qToTBuffer.at(3).at(0));
                                                               //hMulti2L24tot->Fill(qToTBuffer.at(1).back(),qToTBuffer.at(3).back());
                                                               hMulti2L24sigcount->Fill(clusterNSignalbuffer[1],clusterNSignalbuffer[3]);}
      if(layerseen[2] == true && layerseen[3] == true){ l34++; hMulti2L34->Fill(fibMeanBuffer[2]-fibMeanBuffer[3],qMaxBuffer[2]);
                                                               //hMulti2L34tot->Fill(qMaxBuffer[2],qMaxBuffer[3]);
                                                               hMulti2L34tot->Fill(qToTBuffer.at(2).at(0),qToTBuffer.at(3).at(0));
                                                               //hMulti2L34tot->Fill(qToTBuffer.at(2).back(),qToTBuffer.at(3).back());
                                                               hMulti2L34sigcount->Fill(clusterNSignalbuffer[2],clusterNSignalbuffer[3]);}
    }
    total+=buffer;
    buffer=0;
    for(int i=0;i<4;i++){ qMaxBuffer[i]=0; fibMeanBuffer[i]=0; }


  } /// loop over file

  printf("\n\n");
  printf("Events: %i, Total:%i\n",int(nEvents), int(total));
  printf("Multiplicity=1:%i,Multiplicity=2:%i,Multiplicity=3:%i,Multiplicity=4:%i\n",multi1,multi2,multi3,multi4);
  printf("Only 1: %i,Only 2: %i,Only 3: %i,Only 4: %i\n", only1,only2,only3,only4);
  printf("L12:%i,L13:%i,L14:%i,L23:%i,L24:%i,L34:%i\n",l12,l13,l14,l23,l24,l34);
  printf("L123:%i, L124:%i L134:%i, L234:%i\n", l123,l124,l134,l234);
  printf("\n\n");

  

  TCanvas *c1 = new TCanvas("c1","M2", 1500, 700);
  c1->Divide(3,2);
  c1->cd(1);
  gPad->SetLogz();
  hMulti2L12->Draw("COLZ");
  c1->cd(2);
  gPad->SetLogz();
  hMulti2L13->Draw("COLZ");
  c1->cd(3);
  gPad->SetLogz();
  hMulti2L14->Draw("COLZ");
  c1->cd(4);
  gPad->SetLogz();
  hMulti2L23->Draw("COLZ");
  c1->cd(5);
  gPad->SetLogz();
  hMulti2L24->Draw("COLZ");
  c1->cd(6);
  gPad->SetLogz();
  hMulti2L34->Draw("COLZ");

  TCanvas *c2 = new TCanvas("c2","M2ToT", 1500, 700);
  c2->Divide(3,2);
  c2->cd(1);
  //gPad->SetLogz();
  hMulti2L12tot->Draw("COLZ");
  c2->cd(2);
  //gPad->SetLogz();
  hMulti2L13tot->Draw("COLZ");
  c2->cd(3);
  //gPad->SetLogz();
  hMulti2L14tot->Draw("COLZ");
  c2->cd(4);
  //gPad->SetLogz();
  hMulti2L23tot->Draw("COLZ");
  c2->cd(5);
  //gPad->SetLogz();
  hMulti2L24tot->Draw("COLZ");
  c2->cd(6);
  //gPad->SetLogz();
  hMulti2L34tot->Draw("COLZ");

  TCanvas *c3 = new TCanvas("c3","M2SigCount", 1500, 700);
  c3->Divide(3,2);
  c3->cd(1);
  hMulti2L12sigcount->Draw("COLZ");
  c3->cd(2);
  hMulti2L13sigcount->Draw("COLZ");
  c3->cd(3);
  hMulti2L14sigcount->Draw("COLZ");
  c3->cd(4);
  hMulti2L23sigcount->Draw("COLZ");
  c3->cd(5);
  hMulti2L24sigcount->Draw("COLZ");
  c3->cd(6);
  hMulti2L34sigcount->Draw("COLZ");

  TCanvas *c4 = new TCanvas("c4","LayerToT", 1500, 700);
  c4->Divide(4,1);
  c4->cd(1);
  hToTfibL1->Draw("COLZ");
  c4->cd(2);
  hToTfibL2->Draw("COLZ");
  c4->cd(3);
  hToTfibL3->Draw("COLZ");
  c4->cd(4);
  hToTfibL4->Draw("COLZ");

  TCanvas *c5 = new TCanvas("c5","ToT_at_L13_L24_mult2", 1200, 1200);
  c5->Divide(4,1);
  c5->cd(1);
  hToTfibL131->Draw("COLZ");
  c5->cd(2);
  hToTfibL133->Draw("COLZ");
  c5->cd(3);
  hToTfibL242->Draw("COLZ");
  c5->cd(4);
  hToTfibL244->Draw("COLZ");

  fout->WriteObject(hFibMeanL1, "hFibMeanL1");
  fout->WriteObject(hFibMeanL2, "hFibMeanL2");
  fout->WriteObject(hFibMeanL3, "hFibMeanL3");
  fout->WriteObject(hFibMeanL4, "hFibMeanL4");
  fout->WriteObject(hSigFibL1, "hSigFibL1");
  fout->WriteObject(hSigFibL2, "hSigFibL2");
  fout->WriteObject(hSigFibL3, "hSigFibL3");
  fout->WriteObject(hSigFibL4, "hSigFibL4");
  fout->WriteObject(hNSigInClus1, "hNSigInClus1");
  fout->WriteObject(hNSigInClus2, "hNSigInClus2");
  fout->WriteObject(hNSigInClus3, "hNSigInClus3");
  fout->WriteObject(hNSigInClus4, "hNSigInClus4");
  fout->WriteObject(hNClusInLayer1, "hNClusInLayer1");
  fout->WriteObject(hNClusInLayer2, "hNClusInLayer2");
  fout->WriteObject(hNClusInLayer3, "hNClusInLayer3");
  fout->WriteObject(hNClusInLayer4, "hNClusInLayer4");

  fout->WriteObject(c1, "Multi2canvas");
  fout->WriteObject(c2, "Multi2ToTcanvas");
  fout->WriteObject(c3, "Multi2SigCountcanvas");
  fout->WriteObject(c4, "LayerToT");
  fout->WriteObject(c5, "Mult2ToT_L13_L24");

  fout->Close();
}

int main(int argc, char** argv)
{
  char    inputFile[512]="";
  char    outputFile[512]="plotCtsEventCluster_output.root";
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

  printf("\n\n%sRunning plotCtsEventCluster%s\n\n",text::BOLD,text::RESET);
  
  plotCtsEventCluster(inputFile,outputFile,procNr);

  printf("\n\n%s%sDONE!%s\n\n",text::BOLD,text::GRN,text::RESET);
}