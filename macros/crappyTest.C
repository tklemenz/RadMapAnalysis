#include "EventBase.h"

int main()
{
	EventBase event;

	Signal signal;

	signal.setToT(5);

	printf("ToT: %g\n", signal.getToT());

	Fiber fiber;

	fiber.addSignal(signal);

	printf("nSignals: %d\n", fiber.getNSignals());

	event.addFiber(fiber);

	printf("fiber multi: %d\n", int(event.getFiberMultiplicity()));

  std::vector<Fiber> fiberVec;

  fiberVec = event.getFibers();

  printf("nFibers: %d\n", int(fiberVec.size()));

  return 0;
}