#include "Signal.h"

ClassImp(Signal);

//________________________________________________________________________________
Signal::Signal(const Signal &signal)
: mTot(signal.mTot),
  mTimeStamp(signal.mTimeStamp),
  mSignalNr(signal.mSignalNr),
  mChID(signal.mChID),
  mLayer(signal.mLayer)
{
  
}

//________________________________________________________________________________
Signal::Signal(const Double_t tot, const Double_t timeStamp, const Int_t signalNr, const Int_t chID, const Int_t layer)
: mTot(tot),
  mTimeStamp(timeStamp),
  mSignalNr(signalNr),
  mChID(chID),
  mLayer(layer)
{
  
}

//________________________________________________________________________________
Signal::~Signal()
{
  // dtor
}