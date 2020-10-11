#ifndef EVENTBASE_H
#define EVENTBASE_H

#include "Fiber.h"

class EventBase
{
 public:
  EventBase() = default;
  EventBase(const EventBase &event);
  virtual ~EventBase();

  inline void setEventNr      (Float_t eventNr) { mEventNr = eventNr; }
  inline void setPadiwaConfig (Int_t config)    { mPadiwaConfig = config; }

  void addFiber(Fiber &fiber);
  void addFibers(std::vector<Fiber> &fiberVec) { mFiberVec = fiberVec; }

  inline Float_t getEventNr()      const { return mEventNr; }
  inline Float_t getFiberMultiplicity()  { return mFiberVec.size(); }          ///< number of fibers with signal
  inline Float_t getSignalMultiplicity();                                      ///< overall number of signals

 private:

  std::vector<Fiber> mFiberVec{};          ///< fibers with signals
  Float_t            mEventNr;             ///< event number
  Int_t              mPadiwaConfig;        ///< padiwa configuration, 0-4

  ClassDef(EventBase,1);
};

#endif