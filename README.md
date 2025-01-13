## Kernel_Device_Drivers

The repo for some initial modern generic kernel device drivers'
architectures! The attempt to keep Kernel Programming Interfaces
(KPIs) up to date!

This is why this repo matters!

Furthermore, this repo evolved into much deeper by technical
context, complexity and use cases, and uses PRIVATE repo:

https://github.com/ZoranStojsavljevic/Private_Kernel_Device_Drivers

Please, do understand why this repo is special, made as private.

It does explore indepth both normal and a PCIe architectures of
a x86_64 hardware architecture with some UEFI considerations,
focusing mainly on the notebooks' x86_64 motherboards/platforms
on the various (used for such a purpose) distro Linux kernels.

Both initially created generic_block_driver's code and also
generic_char_driver's code currently compile and link, then
run under (starting from) the following Fedora kernels:

	6.11.5-200.fc40.x86_64
	6.11.10-300.fc41.x86_64
	6.12.6-200.fc41.x86_64

Archlinux is also actively used for this development:

	Linux archlinux 6.12.11-1-lts
	Linux archlinux 6.13.0-zen1-1-zen
	Linux archlinux 6.13.0-arch1-1

There are more added code, especially in kernel space, to
perform and explore much more tests about kthreads' (kernel
threads) behaviour.

Thank you,

Author

Date: January 22nd, 2025.
