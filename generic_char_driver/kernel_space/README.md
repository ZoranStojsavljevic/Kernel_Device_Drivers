## The source code of the generic Linux Driver

A generic Linux driver source code depends on the type of the
driver (e.g., character device, block device, network driver,
etc.). Here, an example of a simple character device driver in
Linux is shown. This code demonstrates how to write a basic
loadable kernel module (LKM) for a Linux character device,
which can be dynamically loaded into the kernel.

### Example: Basic Linux Character Device Driver

```
	basic_linux_char_dd.c

/* basic_linux_char_dd.c
 *
 * Key Components:
 *
 * Initialization (char_driver_init):
 *	Registers the device with the kernel, dynamically
 *	allocates a major number, and creates the device file
 *
 * Exit (char_driver_exit):
 *	Cleans up the resources when the module is removed,
 *	including unregistering the device and destroying the
 *	class
 *
 ^ File Operations (fops):
 *	open: Handles opening the device
 *	read: Copies data from the kernel to the user-space
 *	write: Takes input from the user-space and stores it
 *	in the kernel
 *	release: Closes the device
 *
 * Module Macros:
 *	module_init and module_exit macros tell the kernel
 *	the initialization and cleanup functions
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#define DEVICE_NAME "generic_driver"
#define CLASS_NAME  "generic_class"

static int major_number;
static struct class* char_class = NULL;
static struct device* char_device = NULL;
static char message[256] = {0};
static short message_size;
// static struct cdev char_cdev;

// Function prototypes
static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = {
	.open = device_open,
	.read = device_read,
	.write = device_write,
	.release = device_release,
};

// Initialize the module
static int __init char_driver_init(void)
{
	printk(KERN_INFO "GenericDriver: Initializing the driver\n");

	// Dynamically allocate a major number for the device
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0) {
		printk(KERN_ALERT "GenericDriver failed to register a major number\n");
		return major_number;
	}
	printk(KERN_INFO "GenericDriver: Registered device [%s] with major number %d\n",
		DEVICE_NAME, major_number);

	// Register the device class
	// Old i/f: char_class = class_create(THIS_MODULE, CLASS_NAME);
	char_class = class_create(CLASS_NAME);
	if (IS_ERR(char_class)) {
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class [%s]\n", CLASS_NAME);
		return PTR_ERR(char_class);
	}
	printk(KERN_INFO "GenericDriver: Device class registered [%s]\n", CLASS_NAME);

	// Register the device driver
	char_device = device_create(char_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
	if (IS_ERR(char_device)) {
		class_destroy(char_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device [%s]\n", DEVICE_NAME);
		return PTR_ERR(char_device);
	}
	printk(KERN_INFO "GenericDriver: Device created [%s]\n", DEVICE_NAME);

	return 0;
}

// Cleanup the module
static void __exit char_driver_exit(void)
{
	device_destroy(char_class, MKDEV(major_number, 0));
	class_unregister(char_class);
	class_destroy(char_class);
	unregister_chrdev(major_number, DEVICE_NAME);
	printk(KERN_INFO "GenericDriver: Exit fron the from the device driver [%s]\n", DEVICE_NAME);
}

// Open the device
static int device_open(struct inode* inodep, struct file* filep)
{
	printk(KERN_INFO "GenericDriver: Device [%s] opened\n", DEVICE_NAME);
	return 0;
}

// Read from the device
static ssize_t device_read(struct file* filep, char* buffer, size_t len, loff_t* offset)
{
	int error_count = 0;

	error_count = copy_to_user(buffer, message, message_size);

	if (error_count == 0) {
		printk(KERN_INFO "GenericDriver: Sent %d characters to the user\n", message_size);
		return (message_size = 0);
	} else {
		printk(KERN_INFO "GenericDriver: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}
}

// Write to the device
static ssize_t device_write(struct file* filep, const char* buffer, size_t len, loff_t* offset)
{
	sprintf(message, "%s(%zu letters)", buffer, len);
	message_size = strlen(message);
	printk(KERN_INFO "GenericDriver: Received %zu characters from the user\n", len);
	return len;
}

// Release the device
static int device_release(struct inode* inodep, struct file* filep)
{
	printk(KERN_INFO "GenericDriver: Device [%s] closed\n", DEVICE_NAME);
	return 0;
}

// Register module entry and exit points
module_init(char_driver_init);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple generic Linux character driver");
MODULE_VERSION("1.0");

```

### Key Components:

	Initialization (char_driver_init):
		Registers the device with the kernel, dynamically allocates
		a major number, and creates the device file

	Exit (char_driver_exit):
		Cleans up the resources when the module is removed, including
		unregistering the device and destroying the class

	File Operations (fops):
		open: Handles opening the device

	read:
		Copies data from the kernel to the user-space

	write:
		Takes input from the user-space and stores it in the kernel

	release:
		Closes the device

	Module Macros:
		module_init and module_exit macros tell the kernel the
		initialization and cleanup functions

### How to Compile

```
Compile: Use a Makefile to compile the kernel module. Create a Makefile like this:

	makefile

obj-m += basic_linux_char_dd.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

### How to run

```
	bash shell

make

Insert the Module: Load the module into the kernel using:

	bash shell

sudo insmod basic_linux_char_dd.o

Check Logs: To see the kernel logs, use:

	bash shell

sudo dmesg

Create Device File: If the device file is not created automatically, create it manually:

	bash shell

sudo mknod /dev/basic_linux_char_dd c <major_number> 0

Remove the Module: Unload the module with:

	bash shell

sudo rmmod basic_linux_char_dd
```

This is a very basic character device driver (sans ioctl
function) that interacts with user-space applications
through standard system calls like read, write, and open.
It can be extended it to perform more complex tasks like
handling interrupts, memory mapping, etc., based on the
needs.
