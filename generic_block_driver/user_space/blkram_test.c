#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define DEVICE_PATH "/dev/blkram"	// Path to the block device
#define BLOCK_SIZE 4096			// Define block size (can be PAGE_SIZE)

int main()
{
	int fd;
	char write_buffer[BLOCK_SIZE];
	char read_buffer[BLOCK_SIZE];
	ssize_t bytes_written, bytes_read;

	// Fill the write buffer with test data (a repeated character 'A')
	memset(write_buffer, 'A', sizeof(write_buffer));

	// Open the block device with read/write permissions
	fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		perror("Failed to open the block device");
		return errno;
	}

	// Writing data to the block device
	printf("Writing data to the block device...\n");
	bytes_written = write(fd, write_buffer, sizeof(write_buffer));
	if (bytes_written < 0) {
		perror("Failed to write to the block device");
		close(fd);
		return errno;
	}

	printf("Successfully wrote %ld bytes to %s.\n", bytes_written, DEVICE_PATH);

	// Seek to the beginning of the block device before reading
	if (lseek(fd, 0, SEEK_SET) == (off_t) -1) {
		perror("Failed to seek in the block device");
		close(fd);
		return errno;
	}

	// Reading data from the block device
	printf("Reading data from the block device...\n");
	bytes_read = read(fd, read_buffer, sizeof(read_buffer));
	if (bytes_read < 0) {
		perror("Failed to read from the block device");
		close(fd);
		return errno;
	}

	printf("Successfully read %ld bytes from %s.\n", bytes_read, DEVICE_PATH);

	// Compare the read data with the written data
	if (memcmp(write_buffer, read_buffer, sizeof(write_buffer)) == 0) {
		printf("Data read matches data written. Test passed!\n");
	} else {
		printf("Data read does NOT match data written. Test failed!\n");
	}

	// Close the block device
	close(fd);
	return 0;
}
