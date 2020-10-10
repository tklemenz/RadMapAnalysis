#include <iostream>

#include "Dummy.h"

using namespace std;

int main(int argc,char** argv){

  Dummy dummyObject;

  dummyObject.setMember(7);

  cout<<"Member: "<<dummyObject.getMember()<<endl;

  return 0;
}
