#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "commandListener.h"
#include "shutdownManager.h"
#include "joystickController.h"

int main() {
    
    // Create pipes
    // int pipePotToArraySorter[2];
    // pipe(pipePotToArraySorter);
    // int pipeArraySorterToDisplay[2];
    // pipe(pipeArraySorterToDisplay);

    // Call thread constructors
    commandListener_init();
    joystickController_init();


    // Wait until shutdown is triggered 
    sm_waitForShutdownOnMainThread();

    // Call thread destructors
    commandListener_shutdown();

    return 0;
}