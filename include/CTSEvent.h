#ifndef CTSEVENT_H
#define CTSEVENT_H

#include "EventBase.h"
#include "Module.h"

/// This class represents a CTS event.
///
/// The module object contains all signal information.

class CTSEvent : public EventBase
{
 public:
  CTSEvent() = default;
  ~CTSEvent() = default;
  CTSEvent(const CTSEvent &event);

  inline void setModule (Module &module)  { mModule = module; }

  Module&       getModule()       { return mModule; }
  const Module& getModule() const { return mModule; }

 private:
  Module mModule;  ///< containing all fibers and signals

  ClassDef(CTSEvent,1);
};

#endif