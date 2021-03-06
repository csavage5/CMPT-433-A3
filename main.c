#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "commandListener.h"
#include "shutdownManager.h"

int main() {
    
    // Create pipes
    // int pipePotToArraySorter[2];
    // pipe(pipePotToArraySorter);
    // int pipeArraySorterToDisplay[2];
    // pipe(pipeArraySorterToDisplay);

    // Call thread constructors
    commandListener_init();

    // Wait until shutdown is triggered 
    sm_waitForShutdownOnMainThread();

    // Call thread destructors
    commandListener_shutdown();

    return 0;
}