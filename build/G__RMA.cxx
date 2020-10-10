// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__RMA
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// The generated code does not explicitly qualifies STL entities
namespace std {} using namespace std;

// Header files passed as explicit arguments
#include "include/Dummy.h"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void *new_Dummy(void *p = 0);
   static void *newArray_Dummy(Long_t size, void *p);
   static void delete_Dummy(void *p);
   static void deleteArray_Dummy(void *p);
   static void destruct_Dummy(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Dummy*)
   {
      ::Dummy *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Dummy >(0);
      static ::ROOT::TGenericClassInfo 
         instance("Dummy", ::Dummy::Class_Version(), "Dummy.h", 7,
                  typeid(::Dummy), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Dummy::Dictionary, isa_proxy, 4,
                  sizeof(::Dummy) );
      instance.SetNew(&new_Dummy);
      instance.SetNewArray(&newArray_Dummy);
      instance.SetDelete(&delete_Dummy);
      instance.SetDeleteArray(&deleteArray_Dummy);
      instance.SetDestructor(&destruct_Dummy);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Dummy*)
   {
      return GenerateInitInstanceLocal((::Dummy*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::Dummy*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr Dummy::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *Dummy::Class_Name()
{
   return "Dummy";
}

//______________________________________________________________________________
const char *Dummy::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Dummy*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int Dummy::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Dummy*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Dummy::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Dummy*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Dummy::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Dummy*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void Dummy::Streamer(TBuffer &R__b)
{
   // Stream an object of class Dummy.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Dummy::Class(),this);
   } else {
      R__b.WriteClassBuffer(Dummy::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Dummy(void *p) {
      return  p ? new(p) ::Dummy : new ::Dummy;
   }
   static void *newArray_Dummy(Long_t nElements, void *p) {
      return p ? new(p) ::Dummy[nElements] : new ::Dummy[nElements];
   }
   // Wrapper around operator delete
   static void delete_Dummy(void *p) {
      delete ((::Dummy*)p);
   }
   static void deleteArray_Dummy(void *p) {
      delete [] ((::Dummy*)p);
   }
   static void destruct_Dummy(void *p) {
      typedef ::Dummy current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::Dummy

namespace {
  void TriggerDictionaryInitialization_libRMA_Impl() {
    static const char* headers[] = {
"include/Dummy.h",
0
    };
    static const char* includePaths[] = {
"/home/tom/root/ROOTUseFile.cmake",
"/home/tom/RadMapLib",
"/home/tom/RadMapLib/src",
"/home/tom/RadMapLib/include",
"/home/tom/root/include",
"/home/tom/root/include/",
"/home/tom/RadMapLib/build/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libRMA dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
class __attribute__((annotate("$clingAutoload$include/Dummy.h")))  Dummy;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libRMA dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "include/Dummy.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"Dummy", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libRMA",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libRMA_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libRMA_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libRMA() {
  TriggerDictionaryInitialization_libRMA_Impl();
}
