#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <pthread.h>

#include "audioMixer.h"

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

// I2C Device Info
#define I2C_DEVICE_ADDRESS 0x1C
#define REG_WHO_AM_I 0x0D

static pthread_t threadPID;

// Prototypes
static void* accelerometerThread(void *arg);
static int initI2cBus(char* bus, int address);
static void readI2cReg(int i2cFileDesc);

void i2c_init() {

	pthread_create(&threadPID, NULL, accelerometerThread, NULL);
	printf("Module [accelerometer] initialized\n");

}

static void* accelerometerThread(void *__va_arg_pack_len){
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

    // Read the register:
    long seconds = 0;
    long nanoseconds = 10000000;
    struct timespec reqDelay = {seconds, nanoseconds};
    while(1) {
        readI2cReg(i2cFileDesc);
        nanosleep(&reqDelay, (struct timespec *) NULL);
    }

    // Cleanup I2C access
    close(i2cFileDesc);
	return 0;    
}

static int initI2cBus(char* bus, int address){
    int i2cFileDesc;
    if((i2cFileDesc = open(bus, O_RDWR)) < 0){
		printf("Failed to open the bus.\n");
		exit(1);
	}
    ioctl(i2cFileDesc, I2C_SLAVE, 0x1C);

    // Set mode to 1
	char config[2] = {0};
	config[0] = 0x2A;
	config[1] = 0x01;
	write(i2cFileDesc, config, 2);

    config[0] = 0x0E;
	config[1] = 0x00;
	write(i2cFileDesc, config, 2);

    return i2cFileDesc;
}

static void readI2cReg(int i2cFileDesc){
	long seconds = 0;
    long nanoseconds = 300000000;
    struct timespec reqDelay = {seconds, nanoseconds};

	// Adapted from https://github.com/ControlEverythingCommunity/MMA8452Q/blob/master/C/MMA8452Q.c
    char reg[1] = {0x00};
	write(i2cFileDesc, reg, 1);
	char data[7] = {0};
	if(read(i2cFileDesc, data, 7) != 7)
	{
		printf("Error : Input/Output error \n");
	}
	else
	{
		// Convert to 12 bit values
		int xAccl = ((data[1] << 8) + data[2]) / 16;
		if(xAccl > 2047){
			xAccl -= 4096;
		}

		int yAccl = ((data[3] << 8) + data[4]) / 16;
		if(yAccl > 2047){
			yAccl -= 4096;
		}

		int zAccl = ((data[5] << 8) + data[6]) / 16;
		if(zAccl > 2047){
			zAccl -= 4096;
		}
    
		// thresholds
		if(xAccl > 600 || xAccl < -600) {
			printf("Acceleration in X-Axis : %d \n", xAccl);
			AudioMixer_playSound(HIGHHAT);
			printf("[accelerometerModule] played HIGHHAT\n");
			nanosleep(&reqDelay, (struct timespec *) NULL);
			return;
		}
		if(yAccl > 600 || yAccl < -600) {
			printf("Acceleration in Y-Axis : %d \n", yAccl);
			printf("[accelerometerModule] played SNARE\n");
			AudioMixer_playSound(SNARE);
			nanosleep(&reqDelay, (struct timespec *) NULL);
			return;
		}
		if(zAccl > 1500|| zAccl < 400) {
			printf("Acceleration in Z-Axis : %d \n", zAccl);
			AudioMixer_playSound(BASS);
			printf("[accelerometerModule] played BASS\n");
			nanosleep(&reqDelay, (struct timespec *) NULL);
			return;
		}
		
	}

}