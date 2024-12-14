### PCI commands

The lspci command is the most common tool to list and query PCI
devices on a Linux system.

#### lspci tool

To view a list of PCI devices:

	$ lspci

To get detailed information about a specific PCI device:

	$ lspci -vvv

This command will provide verbose output, showing detailed
information about each PCI device, including configuration
space details like vendor ID, device ID, class, and the
memory-mapped I/O regions.

#### setpci tool

To check the configuration space of a specific device (e.g.,
device with address 00:1f.2):

	$ setpci -s 00:1f.2

This will display the PCIe registers for the specified device.

setpci Command: The setpci command can be used to directly read
and modify PCI configuration space registers.

To read a particular PCI register:

	$ setpci -s <device_address> <register>

Example:

	$ setpci -s 00:1f.2 0x10.l

This reads a 32-bit value (0x10.l) from the PCIe configuration
space of the device at address 00:1f.2.

#### lspci -xxx

One can get the raw hexadecimal dump of a device's PCIe
configuration space using this command:

	$ lspci -xxx -s 00:1f.2

This will show a dump of the PCI configuration registers in
hexadecimal.
