#include "xsAll.h"
#include "mc.xs.h"
#include "xs.h"
#include "xsHost.h"
#include "xsHosts.h"

int main(int argc, char *argv[])
{
  xsMachine *the = NULL;
  while (true) {
    the = modCloneMachine(NULL, NULL);

    modRunMachineSetup(the);
    while (true)
    {
      modTimersExecute();
    }
  }
  return 0;
}
