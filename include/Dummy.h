#ifndef DUMMY_H
#define DUMMY_H

#include <Rtypes.h>

class Dummy
{
public:
  Dummy();
  Dummy(const Dummy &dummy);
  virtual ~Dummy();

  void setMember(Int_t value);
  Int_t getMember() const;
  
private:
  Int_t mMember;
  
  ClassDef(Dummy,1);
};

#endif
