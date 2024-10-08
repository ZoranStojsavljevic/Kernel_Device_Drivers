/* How the User-Space Program Works:
 *
 * Opening the device:
 *	The program attempts to open the device file
 *	(/dev/generic_device) with read/write access (O_RDWR). If
 *	the device cannot be opened, it prints an error and exits
 *
 * Writing to the device:
 *	The program prompts the user to enter a message, which is
 *	written to the device using the write() system call
 *
 * Reading from the device:
 *	The program attempts to read the data from the device
 *	using the read() system call. If successful, it prints
 *	the message received from the driver
 *
 * Closing the device:
 *	The program closes the device file after the operations
 *	are completed
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>	// open
#include <unistd.h>	// read, write, close
#include <errno.h>	// error handling
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/generic_driver"

#define IOCTL_SET_REGISTER _IOW('s', 1, int32_t*)
#define IOCTL_GET_REGISTER _IOR('s', 2, int32_t*)

int main()
{
	int fd;
	char write_buffer[256];
	char read_buffer[256];
	int ret;
	int read_ioctl_value =0;
	int write_ioctl_value = 0x2442;

	// Open the device
	fd = open(DEVICE_PATH, O_RDWR); // Open the device for read/write
	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}

	ioctl(fd, IOCTL_SET_REGISTER, &write_ioctl_value);

	ioctl(fd, IOCTL_GET_REGISTER, &read_ioctl_value);
	printf("Read IOCTL value: %x\n", read_ioctl_value);

	// Prepare data to write to the device
	printf("Enter a message to send to the device: ");
	fgets(write_buffer, sizeof(write_buffer), stdin);
	write_buffer[strcspn(write_buffer, "\n")] = 0; // Remove trailing newline

	// Write to the device
	printf("Writing message to the device [%s]\n", write_buffer);
	ret = write(fd, write_buffer, strlen(write_buffer));
	if (ret < 0) {
		perror("Failed to write the message to the device");
		return errno;
	}

	// Read from the device
	printf("Reading from the device...\n");
	ret = read(fd, read_buffer, sizeof(read_buffer));
	if (ret < 0) {
		perror("Failed to read the message from the device");
		return errno;
	}

	// printf("read_buffer is of a %d length\n", sizeof(read_buffer));
	read_buffer[sizeof(read_buffer)] = '\0'; // Null-terminate the read buffer

	printf("The received message is: [%s]\n", read_buffer);

	// Close the device
	close(fd);

	return 0;
}
