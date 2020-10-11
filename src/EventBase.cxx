#include "EventBase.h"

ClassImp(EventBase);

//________________________________________________________________________________
EventBase::EventBase(const EventBase &event)
: mFiberVec(event.mFiberVec),
  mEventNr(event.mEventNr),
  mPadiwaConfig(event.mPadiwaConfig)
{
  
}

//________________________________________________________________________________
EventBase::~EventBase()
{
  // dtor
}

//________________________________________________________________________________
Float_t EventBase::getNSignals()
{
  Int_t multi = 0;

  for (auto &fiber : mFiberVec) {
    multi += fiber.getNSignals();
  }

  return multi;
}