#ifndef EVENTBASE_H
#define EVENTBASE_H

#include "Fiber.h"
#include "Module.h"

/// This class represents an "undefined" event from which specified event classes 
/// (e.g. CTSEvent or physical event classe) can inherit from.
/// 
/// The philosophy still needs a bit of thought...
/// For now the important members are:
///                - EventNr
///                - PadiwaConfig
///                - Module
///
/// The module object contains all signal information. 

class EventBase
{
 public:
  EventBase() = default;
  EventBase(const EventBase &event);
  virtual ~EventBase();

  inline void setEventNr      (Float_t eventNr) { mEventNr = eventNr; }
  inline void setPadiwaConfig (Int_t config)    { mPadiwaConfig = config; }
  inline void setModule       (Module &module)  { mModule = module; }

  inline void addFiber(Fiber &fiber)                  { mFiberVec.emplace_back(fiber); }
  inline void addFibers(std::vector<Fiber> &fiberVec) { mFiberVec = fiberVec; }

  inline Float_t getEventNr()      const { return mEventNr; }
  inline Int_t   getPadiwaConfig() const { return mPadiwaConfig; }

  inline Float_t getNSignals();                                                ///< overall number of signals

  std::vector<Fiber>&       getFibers()       { return mFiberVec; }
  const std::vector<Fiber>& getFibers() const { return mFiberVec; }

  Module&                   getModule()       { return mModule; }
  const Module&             getModule() const { return mModule; }

 private:

  std::vector<Fiber> mFiberVec{};          ///< fibers with signals
  Float_t            mEventNr;             ///< event number
  Int_t              mPadiwaConfig;        ///< padiwa configuration, 0-4
  Module             mModule;              ///< containing all fibers and signals

  ClassDef(EventBase,1);
};

#endif