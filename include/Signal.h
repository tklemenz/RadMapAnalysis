#ifndef SIGNAL_H
#define SIGNAL_H

#include <Rtypes.h>

class Signal
{
 public:
  Signal() = default;
  Signal(const Signal &signal);
  Signal(const Double_t tot, const Double_t timeStamp, const Int_t signalNr);
  virtual ~Signal();

  inline void setToT        (Double_t tot)        { mTot = tot; }
  inline void setTimeStamp  (Double_t timeStamp)  { mTimeStamp = timeStamp; }
  inline void setSignalNr   (Int_t signalNr)      { mSignalNr = signalNr; }

  inline Double_t getToT()       const { return mTot; }
  inline Double_t getTimeStamp() const { return mTimeStamp; }
  inline Int_t    getSignalNr()  const { return mSignalNr; }

 private:

  Double_t mTot;         ///< time over threshold in seconds
  Double_t mTimeStamp;   ///< time stamp of rising edge in seconds
  Int_t    mSignalNr;    ///< nth signal in given event and channel

  ClassDef(Signal,1);
};

#endif