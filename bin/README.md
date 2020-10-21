This folder contains the executables created from the macros.

Do not commit this stuff to the remote git repository!

# What does what?

### convertToEventBase

This one converts TNtupleD files to EventBase objects.

options:

* -i: input file
* -o: output file, `default = "convertedToEventBase.root"`
* -n: number of signals from the tuple to be processed, `default = -1` --> all events, if you overshoot then the whole file will be processed!


### readEventTree

Example macro that reads an EventBase tree produced by `convertToEventBase` and stores some useless example histograms to file.

*The purpose of this one is to show you how to read an EventBase tree.*

options:

* -i: input file
* -o: output file, `default = "readEventTree_output.root"`
* -n: number of signals from the tuple to be processed, `default = -1` --> all events, if you overshoot then the whole file will be processed!


### runDummy + runCrappyTest

Just some *very* basic test stuff you can forget about.