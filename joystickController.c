#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

#include "joystickController.h"
#include "audioMixer.h"


enum direction {UP, DOWN, LEFT, RIGHT, PUSHED, NONE};

//const char * const enumDrumStrings[3] = {"HIGHHAT", "SNARE", "BASS"};
const char * const enumBeatStrings[3] = {"NO_BEAT", "BEAT1", "BEAT2"};

struct timespec startTime, currTime;

static pthread_t threadPID;

#define INPUT_DELAY_VOL_TEMPO_NSEC 200000000
#define INPUT_DELAY_BEAT_TEMPO_NSEC 1400000000
static enum direction lastDirection = NONE; // the last accepted direction

static void* listenerThread(void *arg);
static void initializePins();
static void exportPin(int pin);
static enum direction detectDirection();
static long getTimespecDifference();


void joystickController_init() {

    pthread_create(&threadPID, NULL, listenerThread, NULL);
    printf("Module [joystickController] initialized\n");

}


static void* listenerThread(void *arg) {
    initializePins();
    //timespec_get(&startTime, TIME_UTC);
    clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
    while(1) {
        
        // TODO wait for joystick direction to be pressed

        switch (detectDirection()) {
            case UP:
                AudioMixer_setVolume(AudioMixer_getVolume() + 5);
                printf("[joystickController] set VOL: %d\n", AudioMixer_getVolume());
                break;
            case DOWN:
                AudioMixer_setVolume(AudioMixer_getVolume() - 5);
                printf("[joystickController] set VOL: %d\n", AudioMixer_getVolume());
                break;
            case LEFT:
                AudioMixer_setBPM(AudioMixer_getBPM() - 5);
                printf("[joystickController] set BPM: %d\n", AudioMixer_getBPM());
                break;
            case RIGHT:
                AudioMixer_setBPM(AudioMixer_getBPM() + 5);
                printf("[joystickController] set BPM: %d\n", AudioMixer_getBPM());
                break;
            case PUSHED:;
                enum beat currentBeat = AudioMixer_getBeat();
                if (currentBeat == BEAT2) {
                    currentBeat = NO_BEAT;
                } else {
                    currentBeat += 1;
                }

                AudioMixer_changeBeat(currentBeat);
                printf("[joystickController] changed beat to %s\n", enumBeatStrings[currentBeat]);
                break;
            default:
                break;
        }

    }

    return NULL;

}


static void initializePins() {
    struct timespec sleepTime;
    sleepTime.tv_nsec = 300000000;

    exportPin(26); // up
    exportPin(46); // down
    exportPin(65); // left
    exportPin(47); // right
    exportPin(27); // pushed

    // wait for final pin to be exported
    nanosleep(&sleepTime, NULL);
}

static void exportPin(int pin) {
    FILE *pFile = fopen("/sys/class/gpio/export", "w");
    if (pFile == NULL) {
        printf("ERROR: Unable to open export file.\n");
        exit(1); 
    }

    fprintf(pFile, "%d", pin); // up

    // Close the file using fclose():
    fclose(pFile);
}

static enum direction detectDirection() {
    // 1 == OFF, 0 == ON
    
    //TODO must allow user to hold button to cycle through
        // modes or change volume/tempo at a reasonable rate
        // Need to use timer to return a non-NONE value every 
        // ~half-second, and lastDirection to tell if the
        // same direction is being held
    
    char *filenames[5] = {"/sys/class/gpio/gpio26/value",
                        "/sys/class/gpio/gpio46/value",
                        "/sys/class/gpio/gpio65/value",
                        "/sys/class/gpio/gpio47/value",
                        "/sys/class/gpio/gpio27/value"};
    

    for (int i = 0; i < 5; i++) {
        FILE *pFile = fopen(filenames[i], "r");
        if (pFile == NULL) {
            printf("ERROR: Unable to open joystick export file.\n");
            break;
            //exit(1); 
        }   

        const int MAX_LENGTH = 1024; 
        char buff[MAX_LENGTH]; 
        fgets(buff, MAX_LENGTH, pFile);
        fclose(pFile);
        //printf("%s", buff);
        if (buff[0] == '0') {
            //printf("direction: %d\n", i);
            //timespec_get(&currTime, TIME_UTC);
            clock_gettime( CLOCK_MONOTONIC_RAW, &currTime);
            if (lastDirection == i && i != PUSHED && getTimespecDifference() < INPUT_DELAY_VOL_TEMPO_NSEC) {
                // CASE: user is holding same input from last check
                //       and delay period hasn't elapsed - pretend no input received
                //printf("Direction: %d\nTime: %ld\n", i, currTime.tv_nsec - startTime.tv_nsec);
                return NONE;
            } else if (lastDirection == i && i == PUSHED && getTimespecDifference() < INPUT_DELAY_BEAT_TEMPO_NSEC) {
                // use different delay for cycling beat
                return NONE; 
            }

            //timespec_get(&startTime, TIME_UTC);
            clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
            lastDirection = i;
            return i;
        }
    }

    //timespec_get(&startTime, TIME_UTC);
    clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
    lastDirection = NONE;
    return NONE;
}

static long getTimespecDifference() {
    return ((long)currTime.tv_sec * 1000000000L + currTime.tv_nsec) - ((long)startTime.tv_sec * 1000000000L + startTime.tv_nsec);
}