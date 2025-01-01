## PCIe Config Space

	typedef struct {
		volatile Uint32 VENDOR_DEVICE_ID;
		volatile Uint32 STATUS_COMMAND;
		volatile Uint32 CLASSCODE_REVID;
		volatile Uint32 BIST_HEADER;
		volatile Uint32 BAR[6];
		volatile Uint32 CARDBUS;
		volatile Uint32 SUBSYS_VNDR_ID;
		volatile Uint32 EXPNSN_ROM;
		volatile Uint32 CAP_PTR;
		volatile Uint8 RSVD0[4];
		volatile Uint32 INT_PIN;
		volatile Uint32 PMCAP;
		volatile Uint32 PM_CTL_STAT;
		volatile Uint8 RSVD1[8];
		volatile Uint32 MSI_CAP;
		...
	} CSL_Pcie_cfg_space_endpointRegs;

* [What is PCIe Config Space?](https://microchip.my.site.com/s/article/What-is-PCIe-Config-Space)

### BAR Explanation:

#### BAR Offsets

The BARs start at offset 0x10 and are each 4 bytes wide. There
are up to 6 BARs (BAR0–BAR5).

#### BAR Address Parsing

For memory-mapped BARs, bits 3–0 are mask bits. Clear these
to get the base address.

For I/O BARs, bits 2–0 are mask bits. Clear these to get the
base address.

#### Read and Parse

After reading the configuration space, parse the BAR fields
directly from the config_space array.

#### Output

Display whether the BAR is I/O or memory-mapped and print its
address.

#### Notes

The BAR values retrieved from the configuration space represent
the base addresses allocated by the OS or firmware.

This program assumes the device is accessible and has valid
BARs. Encounter issues, ensure the target BDF corresponds to an
existing and accessible PCI device on used system.

### Left TO DO

To finish user space data fetching/reading from BAR addresses!

