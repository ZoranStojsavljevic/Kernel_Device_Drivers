#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci_ids.h>
#include <linux/io.h>		// for ioremap, ioread32, iowrite32
#include <linux/pci.h>		// for PCI device support

// Determined by: lspci -xxx -s 00:19.0
#define PCI_VENDOR_ID_INTEL 0x8086	// Vendor ID
#define PCI_DEVICE_ID_I218  0x155A	// Example device ID for Intel I218-LM

/*
 * 00:19.0 Ethernet controller: Intel Corporation Ethernet
 * Connection I218-LM (rev 04)
 * Subsystem: Hewlett-Packard Company Device 198f
 * Latency: 0
 * Interrupt: pin A routed to IRQ 57
 *	Region 0: Memory at d0800000 (32-bit, non-prefetchable) [size=128K]
 *	Region 1: Memory at d083e000 (32-bit, non-prefetchable) [size=4K]
 *	Region 2: I/O ports at 4080 [size=32]
 */

/*
 * PCI_STD_RESOURCE_END: This constant is used
 * to iterate over the standard BAR resources,
 * typically ranging from 0 to 5 for most devices.
 */
#define PCI_STD_RESOURCE_END 5

// Show first 64 bytes from mapped MMIO space
#define SPACE_SIZE 64

void	*mapped_addr;

// Target PCI device's bus, device, and function numbers
#define TARGET_BUS 0
#define TARGET_DEVICE 0x19
#define TARGET_FUNCTION 0

// Acess the configuration space of the PCI device
static int if_pdev_matches(struct pci_dev *pdev)
{
	u16	vendor_id, device_id;
	u8	revision_id;
	u32	bar0;

	// pr_info("pdev at address 0x%p", pdev);
	if ((TARGET_BUS == pdev->bus->number) &&
		(TARGET_DEVICE == PCI_SLOT(pdev->devfn)) &&
		(TARGET_FUNCTION == PCI_FUNC(pdev->devfn))) {

		pr_info("Found PCI device at %02x:%02x.%x",
			TARGET_BUS, TARGET_DEVICE, TARGET_FUNCTION);

		// Read Vendor ID and Device ID
		pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor_id);
		pci_read_config_word(pdev, PCI_DEVICE_ID, &device_id);

		pr_info("Vendor ID: 0x%x, Device ID: 0x%x", vendor_id, device_id);

		// Read Revision ID (offset 0x08)
		pci_read_config_byte(pdev, PCI_REVISION_ID, &revision_id);
		pr_info("Revision ID: 0x%x", revision_id);

		// Read Base Address Register 0 (BAR0) (offset 0x10)
		pci_read_config_dword(pdev, PCI_BASE_ADDRESS_0, &bar0);
		pr_info("Lowest BAR0 is at 0x%x PHY address", bar0);

		return 0;
	}
	else
		return -1;
}

static int __init I218_LM_driver_init(void)
{
	struct pci_dev	*pdev = NULL;
	unsigned long	mem_start, addr_s;
	unsigned long	mem_len;
	int		i, ret_code, space_size;
	int		top = 4;

	// @ the beginning, iterate via all PCI
	// devices to find the target device
	for_each_pci_dev(pdev) {
		if (0 == if_pdev_matches(pdev)) {
			ret_code = 0;
			break;
		} else
			ret_code = -1;
	}

	if (-1 == ret_code) {
		pr_err("PCI device %02x:%02x.%x not found",
			TARGET_BUS, TARGET_DEVICE, TARGET_FUNCTION);

		return -ENODEV;
	}

	// Find the PCI device
	pdev = pci_get_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_I218, NULL);
	if (!pdev) {
		pr_err("PCI device not found");
		return -ENODEV;
	} else
		pr_info("pdev at address 0x%p", pdev);

	/*
	 * PCI devices can have multiple BARs, and not all
	 * of them may be memory-mapped. Some might be I/O
	 * ports, so it's important to check if the resource
	 * is actually memory (IORESOURCE_MEM) before
	 * processing.
	 */
	for (int bar_index = 0; bar_index < PCI_STD_RESOURCE_END; bar_index++) {
		printk("\n");
		if (pci_resource_flags(pdev, bar_index) & IORESOURCE_MEM) {
			// Base address of BAR
			addr_s = mem_start = pci_resource_start(pdev, bar_index);
			// BAR size
			mem_len = pci_resource_len(pdev, bar_index);

			pr_info("Found BAR%x at 0x%lx, mapping %lu bytes",
				bar_index, mem_start, mem_len);

#if 1
			// Print the MMIO region (hex dump)
			space_size = SPACE_SIZE;
			// printf("PCI Configuration Space for %s:\n", TARGET_BDF);
			for (i = 0; i < SPACE_SIZE; i++) {
				mapped_addr = ioremap(addr_s, space_size);
				if (!mapped_addr) {
					pr_err("ioremap region BAR%x failed", bar_index);
					return -ENOMEM;
				}

				u8 reg_val8 = ioread8(mapped_addr);
				if (0 == i % 16)
					pr_info("0x%02x: ", i);

				printk(KERN_CONT "%02x ", reg_val8);

				// Unmap the current chunk after use
				iounmap(mapped_addr);
				mapped_addr = NULL;

				// Move to the next chunk
				addr_s += 1;
				space_size -= 1;
			}
			printk("\n");
#endif

			// Read designated 32-bit values from the MMIO region
			addr_s = mem_start;
			space_size = SPACE_SIZE;
			// addr_l = mem_len;
			for (i=0; i < top; i++) {
				// pr_info("mem_start 0x%lx, space_size %lu",
				//	addr_s, space_size);
				mapped_addr = ioremap(addr_s, space_size);
				if (!mapped_addr) {
					pr_err("ioremap region BAR%x failed", bar_index);
					return -ENOMEM;
				}

				u32 reg_val32 = ioread32(mapped_addr);
				pr_info("Read value from mapped_addr register %p: 0x%08x",
					mapped_addr, reg_val32);

				// Unmap the current chunk after use
				iounmap(mapped_addr);
				mapped_addr = NULL;

				// Move to the next chunk
				addr_s += 4;
				space_size -= 4;
			}
		} else {
			pr_info("Highest BAR index is at BAR%x", bar_index - 1);
			break;
		}
	}
	pr_info("Driver I218_LM loaded\n");

	return 0;
}

static void __exit I218_LM_driver_exit(void)
{
	if (mapped_addr)
		iounmap(mapped_addr);	// Unmap the MMIO region

	pr_info("Driver I218_LM unloaded\n");
}

module_init(I218_LM_driver_init);
module_exit(I218_LM_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zoran Stojsavljevic");
MODULE_DESCRIPTION("Test PCI Driver to access Intel I218-LM PCI device MMIO");
