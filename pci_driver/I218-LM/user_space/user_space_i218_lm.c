#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>	// open
#include <unistd.h>	// close
#include <sys/mman.h>	// mmap, munmap
#include <errno.h>	// perror, errno
#include <stdint.h>	// uint32_t, etc.

// Target PCI device's BDF (Bus:Device.Function)
#define TARGET_BDF "0000:00:19.0"

// PCI Configuration Space is 64 bytes for PCI devices
#define CONFIG_SPACE_SIZE 64

#define MAP_SIZE	8		// Map 4KB (one page)
#define MAP_MASK	(MAP_SIZE - 1)	// Mask for alignment

// BAR offsets in PCI configuration space
#define BAR0_OFFSET 0x10
#define BAR1_OFFSET 0x14
#define BAR2_OFFSET 0x18
#define BAR3_OFFSET 0x1C
#define BAR4_OFFSET 0x20
#define BAR5_OFFSET 0x24
#define NUM_BARS 6

int main(void)
{
	char config_path[256];
	uint8_t config_space[CONFIG_SPACE_SIZE];
	int fd, i, config_space_real;
	uint32_t bar_array[NUM_BARS];

	void *mapped_base;
	// void *mapped_dev_base;
	uint32_t read_result;
	uint32_t write_val;

	// Build the path to the configuration space
	snprintf(config_path, sizeof(config_path), "/sys/bus/pci/devices/%s/config",
		TARGET_BDF);

	printf("config_path is: %s\n", config_path);

	// Open the configuration space file
	fd = open(config_path, O_RDONLY);
	if (fd < 0) {
		perror("Failed to open PCI config file");
		return EXIT_FAILURE;
	}
	printf("Succeeded in opening PCI config space, fd = %d\n", fd);

	// Read the PCI configuration space
	config_space_real = read(fd, config_space, CONFIG_SPACE_SIZE);
	printf("config_space_real = %d\n", config_space_real);

	if (config_space_real != CONFIG_SPACE_SIZE) {
		perror("Failed to read PCI config space");
		close(fd);
		return EXIT_FAILURE;
	}

	close(fd);

	// Print the configuration space (hex dump)
	printf("PCI Configuration Space for %s:\n", TARGET_BDF);
	for (i = 0; i < CONFIG_SPACE_SIZE; i++) {
		if (i % 16 == 0)
			printf("\n%02x: ", i);

		printf("%02x ", config_space[i]);
	}
	printf("\n\n");

	printf("config_path is: %s\n", config_path);

	// Extract and display BAR addresses
	printf("BAR Addresses:\n");
	for (i = 0; i < NUM_BARS; i++) {
		int offset = BAR0_OFFSET + (i * 4);
		uint32_t bar_value = *(uint32_t *)&config_space[offset];

		// Check if the BAR is I/O space or memory-mapped
		if (bar_value & 0x1) {
			bar_array[i] = bar_value & ~0x3;
			printf("BAR%d (I/O Space): 0x%08x\n", i, bar_array[i]);
		} else {
			bar_array[i] = bar_value & ~0xf;
			printf("BAR%d (Memory Space): 0x%08x\n", i, bar_array[i]);
		}
	}

	printf("\n");
	return EXIT_SUCCESS;
}
