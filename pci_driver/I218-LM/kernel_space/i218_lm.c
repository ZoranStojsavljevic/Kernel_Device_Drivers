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

// mmio_addr determined by function: pci_resource_start(pdev, 0);
#define MMIO_ADDR 0xd0800000	// This is the Region 0 start address from lspci output

// mem_len determined by function: pci_resource_len(pdev, 0);
#define MMIO_SIZE 131072	// Automatic mapping the region

// Region 1: Memory at d083e000 (32-bit, non-prefetchable) [size=4K]
#define PHY_ADDR  0xd083e000	// Physical address of Region 1
#define SIZE	  4096

static void __iomem *mmio_base;
static void __iomem *mapped_addr;

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

	// pr_info("pdev at address 0x%p\n", pdev);
	if ((TARGET_BUS == pdev->bus->number) &&
		(TARGET_DEVICE == PCI_SLOT(pdev->devfn)) &&
		(TARGET_FUNCTION == PCI_FUNC(pdev->devfn))) {

		printk(KERN_INFO "Found PCI device at %02x:%02x.%x\n",
			TARGET_BUS, TARGET_DEVICE, TARGET_FUNCTION);

		// Read Vendor ID and Device ID
		pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor_id);
		pci_read_config_word(pdev, PCI_DEVICE_ID, &device_id);

		printk(KERN_INFO "Vendor ID: 0x%x, Device ID: 0x%x\n",
			vendor_id, device_id);

		// Read Revision ID (offset 0x08)
		pci_read_config_byte(pdev, PCI_REVISION_ID, &revision_id);
		printk(KERN_INFO "Revision ID: 0x%x\n", revision_id);

		// Read Base Address Register 0 (BAR0) (offset 0x10)
		pci_read_config_dword(pdev, PCI_BASE_ADDRESS_0, &bar0);
		printk(KERN_INFO "BAR0: 0x%x\n", bar0);

		return 0;
	}
	else
		return -1;
}

static int __init I218_LM_driver_init(void)
{
	struct pci_dev	*pdev = NULL;
	unsigned long	mem_start;
	unsigned long	mem_len;
	int 		ret_code, top = 4;

	// @ the beginning, iterate via all PCI
	// devices to find the target device
	for_each_pci_dev(pdev) {
		if (0 == if_pdev_matches(pdev)) {
			ret_code = 0;
			break;
		}
		else {
			ret_code = -1;
			// printk(KERN_INFO "PCI device %02x:%02x.%x not found\n",
			//	TARGET_BUS, TARGET_DEVICE, TARGET_FUNCTION);
		}
	}

	if (-1 == ret_code) {
		printk(KERN_ERR "PCI device %02x:%02x.%x not found\n",
			TARGET_BUS, TARGET_DEVICE, TARGET_FUNCTION);

		return -ENODEV;
	}

	// Find the PCI device
	pdev = pci_get_device(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_I218, NULL);
	if (!pdev) {
		pr_err("PCI device not found\n");
		return -ENODEV;
	} else
		pr_info("pdev at address 0x%p\n", pdev);

	// Get the memory start address and size from the PCI device (using lspci info)
	mem_start = pci_resource_start(pdev, 0); // Memory base address
	mem_len = pci_resource_len(pdev, 0);	 // Memory size

	pr_info("Found PCI device at 0x%lx, mapping %lu bytes\n", mem_start, mem_len);

	// Map the MMIO region 0
	mmio_base = ioremap(mem_start, mem_len);
	if (!mmio_base) {
		pr_err("ioremap region 0 failed\n");
		return -ENOMEM;
	}
	pr_info("Memory mapped at virtual address region 0: %p\n", mmio_base);

	mapped_addr = ioremap(PHY_ADDR, SIZE);
	if (!mapped_addr) {
		pr_err("ioremap region 1 failed\n");
		return -ENOMEM;
	}
	pr_info("Memory mapped at virtual address region 1: %p\n", mapped_addr);

	// Read a 32-bit value from the MMIO region (for example, the first register)
	u32 reg_val = ioread32(mmio_base);
	for (int i=0; i < top; i += 1) {
		pr_info("Read value from MMIO_0 register: 0x%08x\n", reg_val);
		reg_val = ioread32(++mmio_base);
	}

	pr_info("_____PHY_ADDR_MMIO_1 (0xd083e000)___SIZE (4096)_____\n");

	reg_val = ioread32(mapped_addr);
	for (int i=0; i < top; i += 1) {
		pr_info("Read value from MMIO_1 register: 0x%08x\n", reg_val);
		reg_val = ioread32(++mapped_addr);
	}
	pr_info("Driver I218_LM loaded\n");

	return 0;
}

static void __exit I218_LM_driver_exit(void)
{
	if (mmio_base)
		iounmap(mmio_base);  // Unmap the MMIO region

	pr_info("Driver I218_LM unloaded\n");
}

module_init(I218_LM_driver_init);
module_exit(I218_LM_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zoran Stojsavljevic");
MODULE_DESCRIPTION("Test PCI Driver to access Intel I218-LM PCI device MMIO");
