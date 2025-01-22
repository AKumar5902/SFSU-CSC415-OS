/**************************************************************
 * Class:: CSC-415-02 Fall 2024
 * Name:: Anuj Kumar
 * Student ID:: 923792503
 * GitHub-Name:: AKumar5902
 * Project:: Assignment 6 – Device Driver
 *
 * File:: Kumar_Anuj_HW6_main.c
 *
 * Description:: Develop a device driver that can be loaded and run in Linux.
 * The device driver passes in a string to the device driver and the device driver
 * returns an encrypted version of the string or passes in the excrypted string
 * and returns the original string. This file is where we test the device driver
 * we let the user input a string and it returns the encrypted and decrptyed 
 * version of the string.
 *
 **************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DEVICE_NAME "/dev/Kumar_Anuj_HW6_DeviceDriver"
#define MAX_LENGTH 256

int main(int argc, char const *argv[])
{
    //var
    int fileDes;
    char readBuf[MAX_LENGTH];
    long readDevice, writeDevice;
    char inputString[MAX_LENGTH];
    int num;


    fileDes = open(DEVICE_NAME, O_RDWR);
    printf("Returned from open file. %d\n", fileDes);
    if (fileDes < 0) 
    {
        printf("Cannot Open Device.\n");
        perror("Error Opening Device.\n");
        return -1;
    }
    else 
    {
        printf("Opening Device.\n");
    }

    //Get user input for mode
    while(1){
    printf("Choose 1 for encrypt or 2 for decrypt\n");
    scanf("%d",&num);
    if(num ==1){
       printf("You choose encrypt mode\n"); 
    }else if(num ==2){
        printf("You choose decrypt mode\n");
    }else{
        printf("invalid choice exiting program...\n");
        close(fileDes);
        return 0;
    }

    //Get user input for string 
    printf(">> Enter String ");
    scanf("%s", inputString);
    printf("\n");

    writeDevice = write(fileDes, inputString, strlen(inputString));
    if (writeDevice < 0) {
        perror("Write to device failed");
        close(fileDes);
        return -1;
    }
    if(num==1){
    ioctl(fileDes, 1, 1);
	readDevice = read(fileDes, readBuf, strlen(inputString));
	printf("Encypted String: %s\n",readBuf);
    }else{
    ioctl(fileDes, 1, 2);
	readDevice = read(fileDes, readBuf, strlen(inputString));
	printf("Decytped String: %s\n", readBuf);
    }
    }

    close(fileDes);
    

    return 0;


}
