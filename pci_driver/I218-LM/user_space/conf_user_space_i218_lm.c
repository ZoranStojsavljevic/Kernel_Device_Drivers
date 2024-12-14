#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

// Target PCI device's BDF (Bus:Device.Function)
#define TARGET_BDF "0000:00:19.0"

// PCI Configuration Space is 64 bytes for PCI devices
#define CONFIG_SPACE_SIZE 64

int main(void)
{
	char config_path[256];
	uint8_t config_space[CONFIG_SPACE_SIZE];
	int fd, i, config_space_real;

	// Build the path to the configuration space
	snprintf(config_path, sizeof(config_path), "/sys/bus/pci/devices/%s/config",
		TARGET_BDF);

	printf("config_path is : %s\n", config_path);

	// Open the configuration space file
	fd = open(config_path, O_RDONLY);
	if (fd < 0) {
		perror("Failed to open PCI config file");
		return EXIT_FAILURE;
	}
	printf("Succeed to open PCI config space, fd = %d\n", fd);

	// Read the PCI configuration space
	config_space_real = read(fd, config_space, CONFIG_SPACE_SIZE);
	printf("config_space_real = %d\n", config_space_real);

	if (config_space_real != CONFIG_SPACE_SIZE) {
		perror("Failed to read PCI config space!\n");
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
	printf("\n");

	return EXIT_SUCCESS;
}
