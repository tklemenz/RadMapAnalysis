#ifndef FIBER_H
#define FIBER_H

#include <vector>
#include <Rtypes.h>

#include "Signal.h"

class Fiber
{
 public:
  Fiber() = default;
  Fiber(const Fiber &fiber);
  virtual ~Fiber();

  inline void setLayer  (Int_t layer) { mLayer = layer; }
  inline void setX      (Int_t x)     { mX = x; }
  inline void setY      (Int_t y)     { mY = y; }
  inline void setTDC    (Int_t TDC)   { mTDC = TDC; }

  void addSignal(Signal &signal);

  inline Int_t getLayer()  const { return mLayer; }
  inline Int_t getX()      const { return mX; }
  inline Int_t getY()      const { return mY; }
  inline Int_t getTDC()    const { return mTDC; }

  std::vector<Signal>&       getSignals()       { return mSignalVec; }
  const std::vector<Signal>& getSignals() const { return mSignalVec; }

 private:

 	std::vector<Signal> mSignalVec{}; ///< holds all signals recorded in the fiber
 	Int_t               mLayer;       ///< layer the fiber is located in
 	Int_t               mX;           ///< xth fiber in vertical (odd) layers, counts from left to right
 	Int_t               mY;           ///< yth fiber in horizontal (even) layers, counts from bottom to top
 	Int_t               mTDC;         ///< TDC ID where the fiber was connected, depends on padiwa config, 0=1500, 1=1510, 2=1520, 3=1530

	ClassDef(Fiber,1);
};

#endif