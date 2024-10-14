## Test code (blkram_test.c)

A simple user-space C program that can be used to test the block
driver should:

1.	To open the block device
2.	To Write data to it
3.	To read data from it
4.	To Verify the data read matches the data written

### Explanation of the Code

#### Device Path:

The path /dev/blkram is the location of the block device that
corresponds to the RAM disk you created. This path may vary
depending on how the device is registered, so ensure to use
the correct device path.

#### Buffer Setup:

write_buffer: Filled with 'A' characters. This is the data to
be written to the RAM disk.

read_buffer: Used to store the data read from the RAM
disk for comparison.

#### Opening the Block Device:

	open(DEVICE_PATH, O_RDWR);

Opens the block device with read/write permissions. If the
open call fails, it returns an error and exits the program.

#### Writing to the Block Device:

	write(fd, write_buffer, sizeof(write_buffer));

Writes data to the block device. The number of bytes written is
stored in bytes_written.

If the write operation fails, it prints an error message.

#### Seeking:

	lseek(fd, 0, SEEK_SET);

Resets the file descriptor to the start of the device before
performing a read.

#### Reading from the Block Device:

	read(fd, read_buffer, sizeof(read_buffer));

Reads from the block device into the read_buffer.

#### Data Comparison:

	memcmp(write_buffer, read_buffer, sizeof(write_buffer));

Compares the data written and read. If they match, the test
passes.

#### Closing the Device:

	close(fd);

The block device is closed at the end with close(fd).
