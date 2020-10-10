#include "Dummy.h"

ClassImp(Dummy);

Dummy::Dummy()
: mMember(0)
{

}

//________________________________________________________________________________
Dummy::Dummy(const Dummy &dummy)
: mMember(dummy.mMember)
{
  
}

//________________________________________________________________________________
Dummy::~Dummy()
{
  // dtor
}

//________________________________________________________________________________
void Dummy::setMember(Int_t value)
{
  mMember = value;
}

//________________________________________________________________________________
Int_t Dummy::getMember() const
{
  return mMember;
}