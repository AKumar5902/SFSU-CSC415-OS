/**************************************************************
 * Class:: CSC-415-02 Fall 2024
 * Name:: Anuj Kumar
 * Student ID:: 923792503
 * GitHub-Name:: AKumar5902
 * Project:: Assignment 6 – Device Driver
 *
 * File:: Kumar_Anuj_HW6_DeviceDriver.c
 *
 * Description:: Develop a device driver that can be loaded and run in Linux.
 * The device driver passes in a string to the device driver and the device driver
 * returns an encrypted version of the string or passes in the excrypted string
 * and returns the original string. This file is used for the read,write,
 * open, close and other function that creates the deivce driver.
 *
 **************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include<linux/string.h>

#include <linux/sched.h>
#define MY_MAJOR 415
#define MY_MINOR 0
#define DEVICE_NAME "Kumar_Anuj_HW6_DeviceDriver"

// global vars from proffesor's vid

int major, minor;
char *kernelBuffer;

struct cdev my_cdev;
int actualRxSize = 0;

MODULE_AUTHOR("Anuj Kumar");
MODULE_DESCRIPTION("Simple Encryption Program");
MODULE_LICENSE("GPL");

#define ENCRYPT_MODE 1 // Encryption mode flag
#define DECRYPT_MODE 2 // Decryption mode flag
#define KEY 2          // Key for number of letters to move

// Data structure for keeping track of read/write counts and encryption mode
struct myds
{
    int count;           // Count of read/write operations
    int encryption_mode; // Mode for encryption or decryption
} myds;

ssize_t myWrite(struct file *fs, const char __user *buf, size_t hsize, loff_t *fPos)
{
    struct myds *dataStruct; // data struct pointer
    int notCopied;           // varibale to count bytes copied
    dataStruct = (struct myds *)fs->private_data;
    dataStruct->count = dataStruct->count + 1;

    //printk(KERN_INFO "Kernel buffer before writing: %s\n", kernelBuffer);
    
    // Free previous buffer if any
    if (kernelBuffer != NULL)
    {
        kfree(kernelBuffer);
    }
    // Allocate memory for kernel buffer
    if ((kernelBuffer = kmalloc(hsize + 1, GFP_KERNEL)) == NULL)
    {
        return -1;
    }
     memset(kernelBuffer, 0, hsize + 1);

   
    // Copy data from user space to kernel space
    notCopied = copy_from_user(kernelBuffer, buf, hsize);

    if (notCopied > 0)
    {
        kfree(kernelBuffer);
        return -1;
    }

    kernelBuffer[hsize] = '\0';
    //printk(KERN_INFO "Kernel buffer after writing: %s\n", kernelBuffer);

    printk(KERN_INFO "We wrote : %lu on write number %d\n", hsize, dataStruct->count);
    actualRxSize = hsize - notCopied;
    return hsize;
}

static int myOpen(struct inode *indoe, struct file *fs)
{
    struct myds *dataStruct;

    // Alloc mem for struct
    dataStruct = vmalloc(sizeof(struct myds));

    // check if alloc worked
    if (dataStruct == 0)
    {
        printk(KERN_ERR "Can't vmalloc, File not opened");
        return -1;
    }

    // set ds variables
    dataStruct->count = 0;
    dataStruct->encryption_mode = ENCRYPT_MODE;
    fs->private_data = dataStruct;
    return 0;
}

static int myClose(struct inode *indoe, struct file *fs)
{
    // get struct and free it
    struct myds *dataStruct;
    dataStruct = (struct myds *)fs->private_data;
    vfree(dataStruct);
    return 0;
}
static int encryptOrDecrypt(char *buf, struct myds *ds)
{
    // Loop var
    int i;
  //  printk(KERN_INFO "Buffer before processing: %s\n", buf);
    // changing decrypt or encrypt
    if (ds->encryption_mode == ENCRYPT_MODE)
    {
        for (i = 0; buf[i] != '\0'; i++)
        {
            buf[i] -= KEY;
        }
       // printk(KERN_INFO "Buffer after decryption: %s\n", buf);
    }
    else if (ds->encryption_mode == DECRYPT_MODE)
    {
        for (i = 0; buf[i] != '\0'; i++)
        {
            buf[i] += KEY;
        }
      //  printk(KERN_INFO "Buffer after encryption: %s\n", buf);
    }
    else
    {
        printk(KERN_ERR "Failed to encrypt or decrypt.\n");
        return -1;
    }
    // Null-terminate the string
    buf[i] = '\0';
    return 0;
}
ssize_t myRead(struct file *fs, char *buf, size_t hsize, loff_t *fpos)
{
    struct myds *dataStruct;
    int notCopied;

    dataStruct = (struct myds *)fs->private_data;
    dataStruct->count = dataStruct->count + 1;

    // Check if kernel buffer is empty
    if (kernelBuffer == NULL)
    {
        return -1;
    }

    // Don't copy more than what we have
    if (hsize > actualRxSize)
    {
        hsize = actualRxSize;
    }

    // Encrypt or decrypt data before copying to user space
    encryptOrDecrypt(kernelBuffer, dataStruct);
    kernelBuffer[hsize] = '\0';
    notCopied = copy_to_user(buf, kernelBuffer, hsize);

    if (notCopied > 0)
    {
        kfree(kernelBuffer);
        return -1;
    }
    printk(KERN_INFO "We read : %lu on write number %d\n", hsize, dataStruct->count);
    return notCopied;
}

static long myIoCtl(struct file *fs, unsigned int command, unsigned long data)
{

    struct myds *dataStruct;
    dataStruct = (struct myds *)fs->private_data;
    // changing the mode of the deivce driver

    if (data == ENCRYPT_MODE)
    {
        dataStruct->encryption_mode = data;
        printk("Encryption mode: %d\n", dataStruct->encryption_mode);
    }
    else if (data == DECRYPT_MODE)
    {
        dataStruct->encryption_mode = data;
        printk("Decyrption mode: %d\n", dataStruct->encryption_mode);
    }
    else
    {
        return -1;
    }
    return 0;
}
struct file_operations fileOperations = {
    // what we can do
    .write = myWrite,
    .read = myRead,
    .open = myOpen,
    .release = myClose,
    .unlocked_ioctl = myIoCtl,
    .owner = THIS_MODULE,
};

int init_module(void)
{
    int result, registers;
    dev_t devNo;

    devNo = MKDEV(MY_MAJOR, MY_MINOR);

    registers = register_chrdev_region(devNo, 1, DEVICE_NAME);
    printk(KERN_INFO "Register chardev succeeded 1: %d\n", registers);
    cdev_init(&my_cdev, &fileOperations);
    my_cdev.owner = THIS_MODULE;

    result = cdev_add(&my_cdev, devNo, 1);
    printk(KERN_INFO "Register chardev succeeded 2: %d\n", result);
    printk(KERN_INFO "Welcome - Driver is loaded.\n");

    if (result < 0)
    {
        printk(KERN_ERR "Register chardev failed: %d\n", result);
    }
    return result;
}

// unregistering and removing device from kernal
void cleanup_module(void)
{
    dev_t devNo;
    devNo = MKDEV(MY_MAJOR, MY_MINOR);
    unregister_chrdev_region(devNo, 1);
    cdev_del(&my_cdev);

    printk(KERN_INFO "Goodbye from Simple Encryption Driver");
}
