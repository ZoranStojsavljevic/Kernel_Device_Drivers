### PCIe Config Space

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

* [What is PCIe Config Space](https://microchip.my.site.com/s/article/What-is-PCIe-Config-Space)

