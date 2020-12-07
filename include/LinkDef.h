#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class Dummy +;
#pragma link C++ class Signal +;
#pragma link C++ class Fiber +;
#pragma link C++ class EventBase +;
#pragma link C++ class CTSEvent +;
#pragma link C++ class Module +;
#pragma link C++ class Cluster +;
#pragma link C++ class CTSEventClusters +;
#pragma link C++ class Track +;

#pragma link C++ function mapping::getModuleSpot +;
#pragma link C++ function mapping::getFiberInfoFromModSpot +;
#pragma link C++ function mapping::odd0 +;
#pragma link C++ function mapping::odd1 +;
#pragma link C++ function mapping::revEven0 +;
#pragma link C++ function mapping::revEven1 +;
#pragma link C++ function mapping::getFiberNr +;
#pragma link C++ function mapping::getLayerNr +;
#pragma link C++ function mapping::getX +;
#pragma link C++ function mapping::getY +;


#endif
