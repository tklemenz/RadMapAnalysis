#include "Module.h"
#include "Utility.h"

ClassImp(Module);

//________________________________________________________________________________
Module::Module(const Module &module)
: mFibers(module.mFibers)
{
  
}

//________________________________________________________________________________
Module::~Module()
{
  // dtor
}

//________________________________________________________________________________
void Module::addSignal(Signal &signal)
{
  mFibers.at(mapping::getModuleSpot(signal.getLayer(), signal.getChannelID())).addSignal(signal);
}

//________________________________________________________________________________
Float_t Module::getNSignals()
{
  Float_t nSignals = 0;

  for (auto &fiber : mFibers) {
    nSignals += fiber.getNSignals();
  }

  return nSignals;
}

//________________________________________________________________________________
Int_t Module::getNFibers()
{
  Int_t nFibers = 0;

  for (auto &fiber : mFibers) {
    if (fiber.getNSignals() > 0) {
      nFibers += 1;
    }
  }

  return nFibers;
}