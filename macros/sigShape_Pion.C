#include "TGraph.h"

void sigShape_Pion()
{
  TGraph* sig = new TGraph();
  sig->GetXaxis()->SetTitle("t [ns]");
  sig->GetYaxis()->SetTitle("signal [mV]");

  sig->GetYaxis()->SetRangeUser(0,50);

  sig->SetMarkerStyle(20);

  sig->SetPoint(0, 0.2, 5);
  sig->SetPoint(1, 0.4001, 10);
  sig->SetPoint(2, 0.8003, 20);
  sig->SetPoint(3, 1.0004, 25);
  sig->SetPoint(4, 10.9904, 25);
  sig->SetPoint(5, 11.9003, 20);
  sig->SetPoint(6, 14.2001, 10);
  sig->SetPoint(7, 15.46, 5);

  sig->Draw("APE");
}