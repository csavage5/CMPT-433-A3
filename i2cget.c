#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

// I2C Device Info
#define I2C_DEVICE_ADDRESS 0x1C
#define REG_WHO_AM_I 0x0D


// Prototypes
int initI2cBus(char* bus, int address);
void readI2cReg(int i2cFileDesc, unsigned char regAddr);

int main(){

    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

    // Read the register:
    long seconds = 0;
    long nanoseconds = 10000000;
    struct timespec reqDelay = {seconds, nanoseconds};
    while(1) {
        readI2cReg(i2cFileDesc, REG_WHO_AM_I);  
        nanosleep(&reqDelay, (struct timespec *) NULL);
    }
    readI2cReg(i2cFileDesc, REG_WHO_AM_I);   

    // Cleanup I2C access
    close(i2cFileDesc);

    return 0;
    
}

int initI2cBus(char* bus, int address){
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

void readI2cReg(int i2cFileDesc, unsigned char regAddr){
	long seconds = 1;
    long nanoseconds = 0;
    struct timespec reqDelay = {seconds, nanoseconds};
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
        

		// Output data to screen
		// printf("Acceleration in X-Axis : %d \n", xAccl);
		// printf("Acceleration in Y-Axis : %d \n", yAccl);
		// printf("Acceleration in Z-Axis : %d \n\n\n", zAccl);
		if(xAccl > 500 || xAccl < -300) {
			printf("play x-axis sound\n");
			nanosleep(&reqDelay, (struct timespec *) NULL);
		}
		if(yAccl > 500 || yAccl < -300) {
			printf("play y-axis sound\n");
			nanosleep(&reqDelay, (struct timespec *) NULL);
		}
		if(zAccl > 1300|| zAccl < 800) {
			printf("play z-axis sound\n");
			nanosleep(&reqDelay, (struct timespec *) NULL);
		}
		
	}

}