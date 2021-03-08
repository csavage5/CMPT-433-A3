#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "commandListener.h"
#include "shutdownManager.h"
#include "audioMixer.h"

#define MAX_LEN_UDP 1500  // 1500 bytes max in UDP packet
#define PORT 12345

static pthread_t threadPID;

static struct sockaddr_in sinLocal;
static unsigned int sin_len;
static int socketDescriptor;

static struct sockaddr_in sinRemote;
static unsigned int sinRemote_len = sizeof(sinRemote);

static char *pMessage;
static char messageBuffer[MAX_LEN_UDP];
static char *commands[3];

static void socketInit();
static void* listenerThread(void *arg);
static void detectCommands();
static void sendReply();


void commandListener_init() {

    pthread_create(&threadPID, NULL, listenerThread, NULL);
    printf("Module [commandListener] initialized\n");

}


static void socketInit() {

    // initialize sockets
    memset(&sinLocal, 0, sizeof(sinLocal));
    sinLocal.sin_family = AF_INET;
    sinLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    sinLocal.sin_port = htons(PORT);
    sin_len = sizeof(sinLocal);

    // create and bind to socket
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    int err = bind(socketDescriptor, (struct sockaddr*) &sinLocal, sin_len);

    if (err == -1) {
        printf("Bind Error: %s\n", strerror(errno));
    }
}


/*
    Thread loop:
        wait for incoming command
        possible command:
            change beat; reply with new beat
            change volume; reply with new volume
            change tempo; reply with new tempo
            play specific sound; reply with success
            get uptime; reply with uptime
*/


static void* listenerThread(void *arg) {

    socketInit();

    static int messageLen; // tracks # of bytes received from packet, -1 if error


    pMessage = (char*)malloc(16000 * sizeof(char)); // malloc space for receiving from getArray
    memcpy(pMessage, messageBuffer, messageLen);
    strcpy(pMessage, "Hello there!\n");

    //char *endptr = NULL; // for strtol

    while(!sm_isShutdown()) {

        // sinRemote captures counterparty address information
        messageLen = recvfrom(socketDescriptor, messageBuffer, MAX_LEN_UDP, 0, (struct sockaddr *) &sinRemote, &sinRemote_len);
        printf("Received %s\n", messageBuffer);

        if (messageLen == -1) {
            printf("Receive Error: %s\n\n", strerror(errno));
        }

        detectCommands();
        printf("0: %s, 1: %s, 2: %s\n", commands[0], commands[1], commands[2]);

        if (strcmp("q", commands[0]) == 0) {
            // CASE: server sent "q" - reply with requested info
            // TODO 

            if (strcmp("vol", commands[1]) == 0) {

            } else if (strcmp("tempo", commands[1]) == 0) {

            } else if (strcmp("uptime", commands[1]) == 0) {

            }

        } else if (strcmp("vol", commands[0]) == 0) {
            // CASE: server sent "vol" - change volume by requested 
            //       amount if possible, reply with new volume
            // TODO 

            if (strcmp("u", commands[1]) == 0) {

            } else if (strcmp("d", commands[1]) == 0) {

            }

        } else if (strcmp("tempo", commands[0]) == 0) {
            // CASE: server sent "tempo" - change tempo by requested 
            //       amount if possible, reply with new tempo
            // TODO 

            if (strcmp("u", commands[1]) == 0) {

            } else if (strcmp("d", commands[1]) == 0) {

            }

        } else if (strcmp("p", commands[0]) == 0) {
            // CASE: server sent "p" - play drum or beat sound
            // TODO 

            if (strcmp("drum", commands[1]) == 0) {

            } else if (strcmp("beat", commands[1]) == 0) {

            } 

        } else {
            sprintf(pMessage, "Error: invalid command \"%s\"\n\n", commands[0]);
        }

        printf("Size of pMessage: %u\n", strlen(pMessage));

        // reply with message
        sendReply();
        
        // wipe buffers for next command
        memset(messageBuffer, '\0', sizeof(messageBuffer));
        memset(pMessage, '\0', sizeof(*pMessage));
        //memcpy(pMessage, messageBuffer, messageLen);

    }

    printf("Thread [commandListener]->listenerThread starting shut down...\n");
    commandListener_shutdown();

    return NULL;

}

static void detectCommands() {

    // TODO modify for new command structure

    int i = 0;
    char *newline = 0;
    char *token = NULL;
    commands[0] = NULL;
    commands[1] = NULL;
    commands[2] = NULL;

    
    token = strtok(messageBuffer, " ");
   
    while (i < 3 && token != NULL ) {
        // will be MAX 3 tokens to the command
        // CASE: token contains a newline character - remove it
        // adapted from https://stackoverflow.com/questions/9628637/how-can-i-get-rid-of-n-from-string-in-c
        if ( (newline = strchr(token, '\n')) != NULL) {
            *newline = '\0';
        }
        commands[i] = token;   
        token = strtok(NULL, " ");
        i += 1;
    }
    
}

// Send reply to host. Splits up reply into multiple packets if 
// it is > 1500 bytes.
static void sendReply() {

    // TODO remove check for large replies, not applicable to this assignment

    // check if pMessage is bigger than MAX_LEN
    //  if it is, loop:
    //      - scan bytes from starting point, keeping pointer to last \n found
    //      - if you hit 1500 bytes, send data from 'starting point' to
    //        the \n pointer
    //      - set new starting point to next character after last \n

    char *pStart = pMessage;
    char *pEnd = NULL;      // last \n encountered
    char *pCursor = pMessage;

    int bytesSinceLastNewline = 0; //bytes since last \n
    int bytesToSend = 0;

    int err;
    //printf("pMessage is initially %d chars long\n", strlen(pCursor));
    while (strlen(pCursor) >= 1500) {

        for (int i = 0; i < 1500; i++) {
            
            bytesSinceLastNewline++;

            if (*pCursor == '\n') {
                pEnd = pCursor;
                bytesToSend += bytesSinceLastNewline;
                bytesSinceLastNewline = 0;
            }

            pCursor++;
            
        }

        printf("Sending %d bytes of pMessage...\n", bytesToSend);
        err = sendto(socketDescriptor, pStart, bytesToSend, 
                        0, (struct sockaddr *) &sinRemote, sinRemote_len);
       
        if (err == -1) {
            printf("Reply Error: %s\n\n", strerror(errno));
        }

        // set pStart, pCursor = pEnd + 1
        pStart = pEnd + 1;
        pCursor = pEnd + 1;
        bytesToSend = 0;
        bytesSinceLastNewline = 0;
    }

    printf("Sending %d bytes of pMessage...\n", strlen(pStart));
    err = sendto(socketDescriptor, pStart, strlen(pStart), 
                    0, (struct sockaddr *) &sinRemote, sinRemote_len);
    
    if (err == -1) {
        printf("Reply Error: %s\n\n", strerror(errno));
    }

}


void commandListener_shutdown() {

    //TODO remove, don't need to support shutdown

    pthread_join(threadPID, NULL);

    printf("Thread [commandListener]->listenerThread shut down\n");

    // close socket
    close(socketDescriptor);

    // free heap memory
    free(pMessage);
    pMessage = NULL;

    printf("Module [commandListener] shut down\n");
}
