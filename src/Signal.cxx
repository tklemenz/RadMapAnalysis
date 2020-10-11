#include "Signal.h"

ClassImp(Signal);

//________________________________________________________________________________
Signal::Signal(const Signal &signal)
: mTot(signal.mTot),
  mTimeStamp(signal.mTimeStamp),
  mSignalNr(signal.mSignalNr)
{
  
}

//________________________________________________________________________________
Signal::Signal(const Double_t tot, const Double_t timeStamp, const Int_t signalNr)
: mTot(tot),
  mTimeStamp(timeStamp),
  mSignalNr(signalNr)
{
  
}

//________________________________________________________________________________
Signal::~Signal()
{
  // dtor
}