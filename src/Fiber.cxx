#include "Fiber.h"

ClassImp(Fiber);

//________________________________________________________________________________
Fiber::Fiber(const Fiber &fiber)
: mSignalVec(fiber.mSignalVec),
  mLayer(fiber.mLayer),
  mX(fiber.mX),
  mY(fiber.mY),
  mTDC(fiber.mTDC)
{
  
}

//________________________________________________________________________________
Fiber::~Fiber()
{
  // dtor
}

//________________________________________________________________________________
void Fiber::addSignal(Signal &signal)
{
  mSignalVec.emplace_back(signal);
}