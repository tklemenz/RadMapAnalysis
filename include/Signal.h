#ifndef SIGNAL_H
#define SIGNAL_H

#include <Rtypes.h>

/// This class represents a signal

class Signal
{
 public:
  Signal() = default;
  Signal(const Signal &signal);
  Signal(const Double_t tot, const Double_t timeStamp, const Int_t signalNr, const Int_t chID, const Int_t layer, const Int_t tdcID, const Int_t configuration);
  virtual ~Signal();

  inline void setToT           (Double_t tot)        { mTot = tot; }
  inline void setTimeStamp     (Double_t timeStamp)  { mTimeStamp = timeStamp; }
  inline void setSignalNr      (Int_t signalNr)      { mSignalNr = signalNr; }
  inline void setChannelID     (Int_t chID)          { mChID = chID; }
  inline void setLayer         (Int_t layer)         { mLayer = layer; }
  inline void setTDCID         (Int_t tdcID)         { mTDCID = tdcID; }
  inline void setConfiguration (Int_t config)        { mConfig = config; }

  inline Double_t getToT()                const { return mTot; }
  inline Double_t getTimeStamp()          const { return mTimeStamp; }
  inline Int_t    getSignalNr()           const { return mSignalNr; }
  inline Int_t    getChannelID()          const { return mChID; }
  inline Int_t    getLayer()              const { return mLayer; }
  inline Int_t    getTDCID()              const { return mTDCID; }
  inline Int_t    getConfiguration()      const { return mConfig; }

 private:

  Double_t mTot;         ///< time over threshold in seconds
  Double_t mTimeStamp;   ///< time stamp of rising edge in seconds
  Int_t    mSignalNr;    ///< nth signal in given event and channel
  Int_t    mChID;        ///< channel number
  Int_t    mLayer;       ///< layer of the hit fiber
  Int_t    mTDCID;       ///< TDC ID of the hit fiber
  Int_t    mConfig;      ///< padiwa configuration

  ClassDef(Signal,1);
};

#endif