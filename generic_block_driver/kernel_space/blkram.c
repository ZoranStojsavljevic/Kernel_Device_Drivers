/*
 * This code defines a simple Linux kernel module that implements
 * a block device driver backed by RAM, commonly referred to as a
 * "RAM disk". It allows the system to treat a portion of system
 * memory as if it were a block device (like a disk), meaning it
 * can be formatted, mounted, and used to store files, albeit
 * non-persistently (since the contents will be lost when the
 * module is removed or the system is rebooted)
 */

/*
 * asm/page.h: Provides page size definitions (used in block size
 * setup)
 *
 * blk_types.h, blkdev.h, blk-mq.h: Used for block devices and
 * multiqueue request handling
 *
 * module.h: Defines macros and functions for building kernel
 * modules
 */

#include <asm/page.h>
#include <linux/blk_types.h>
#include <linux/sysfb.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/idr.h>

// capacity_mb defines the capacity of the RAM
// RAM disk in megabytes (default 40 MB)
unsigned long capacity_mb = 40;

// module_param makes this parameter configurable via module
// loading allowing it to be set from user space (insmod)
module_param(capacity_mb, ulong, 0644);
MODULE_PARM_DESC(capacity_mb, "capacity of the block device in MB");

// EXPORT_SYMBOL_GPL allows the parameter to be
// accessed from other GPL-licensed modules
EXPORT_SYMBOL_GPL(capacity_mb);

// max_segment_size: Defines the maximum size of each segment
unsigned long max_segments = 32;
module_param(max_segments, ulong, 0644);
MODULE_PARM_DESC(max_segments, "maximum segments");
EXPORT_SYMBOL_GPL(max_segments);

unsigned long max_segment_size = 65536;
module_param(max_segment_size, ulong, 0644);
MODULE_PARM_DESC(max_segment_size, "maximum segment size");
EXPORT_SYMBOL_GPL(max_segment_size);

// lbs (logical block size) and pbs (physical block size):
// Define the sizes of logical and physical blocks
// (typically equal to PAGE_SIZE)

unsigned long lbs = PAGE_SIZE;
module_param(lbs, ulong, 0644);
MODULE_PARM_DESC(lbs, "Logical block size");
EXPORT_SYMBOL_GPL(lbs);

unsigned long pbs = PAGE_SIZE;
module_param(pbs, ulong, 0644);
MODULE_PARM_DESC(pbs, "Physical block size");
EXPORT_SYMBOL_GPL(pbs);

// structure struct blk_ram_dev_t represents the
// RAM-backed block device:
struct blk_ram_dev_t {
	// total capacity in sectors
	sector_t capacity;
	// pointer to the memory region used as the storage
	u8 *data;
	// used by the block multiqueue (blk-mq) layer to manage request tags
	struct blk_mq_tag_set tag_set;
	// Represents the device at a higher level, connecting to the kernel's block layer
	struct gendisk *disk;
};

static int major;
static DEFINE_IDA(blk_ram_indexes);
static struct blk_ram_dev_t *blk_ram_dev = NULL;

// blk_ram_queue_rq () function processes block requests for reading or writing
// hctx: Represents the hardware context for the queue
// bd: Provides details about the current request
static blk_status_t blk_ram_queue_rq(struct blk_mq_hw_ctx *hctx,
				     const struct blk_mq_queue_data *bd)
{
	struct request *rq = bd->rq;
	blk_status_t err = BLK_STS_OK;
	struct bio_vec bv;
	struct req_iterator iter;
	loff_t pos = blk_rq_pos(rq) << SECTOR_SHIFT;
	struct blk_ram_dev_t *blkram = hctx->queue->queuedata;
	loff_t data_len = (blkram->capacity << SECTOR_SHIFT);

	blk_mq_start_request(rq);

	// Iterates over all segments in the request using rq_for_each_segment
	rq_for_each_segment(bv, rq, iter) {
		// For each segment, it calculates the buffer location, checks if
		// the request is valid, and copies data between the RAM disk and
		// the buffer
		unsigned int len = bv.bv_len;
		void *buf = page_address(bv.bv_page) + bv.bv_offset;

		if (pos + len > data_len) {
			err = BLK_STS_IOERR;
			break;
		}

		switch (req_op(rq)) {
			// If the request is a read (REQ_OP_READ), it copies from the RAM
			// disk to the buffer
			case REQ_OP_READ:
				memcpy(buf, blkram->data + pos, len);
				break;
			// If the requestis a write (REQ_OP_WRITE), it copies from the
			// buffer to the RAM disk
			case REQ_OP_WRITE:
				memcpy(blkram->data + pos, buf, len);
				break;
			default:
				err = BLK_STS_IOERR;
				goto end_request;
		}

		pos += len;
	}

// blk_mq_end_request is called to complete the request
end_request:
	blk_mq_end_request(rq, err);
	return BLK_STS_OK;
}

// This structure defines the operations for the block multiqueue
// (blk-mq), and it associates the blk_ram_queue_rq function with
// the queue_rq callback
static const struct blk_mq_ops blk_ram_mq_ops = {
	.queue_rq = blk_ram_queue_rq,
};

// This structure defines basic operations for the block device
// (blk_ram_rq_ops), but the only operation here is setting the
// owner of the module
static const struct block_device_operations blk_ram_rq_ops = {
	.owner = THIS_MODULE,
};

// This function initializes the module
static int __init blk_ram_init(void)
{
	int ret = 0;
	int minor;
	struct gendisk *disk;
	loff_t data_size_bytes = capacity_mb << 20;

	struct queue_limits lim = {
		.max_hw_sectors	= 64,
		// .features	= BLKROTATIONAL,
	};

	// Registers a block device (register_blkdev), obtaining a major
	// number
	ret = register_blkdev(0, "blkram");
	if (ret < 0)
		return ret;

	// Allocates memory for the block device structure (blk_ram_dev_t)
	major = ret;
	blk_ram_dev = kzalloc(sizeof(struct blk_ram_dev_t), GFP_KERNEL);

	if (blk_ram_dev == NULL) {
		pr_err("memory allocation failed for blk_ram_dev\n");
		ret = -ENOMEM;
		goto unregister_blkdev;
	}

	// Allocates memory for the RAM disk itself using kvmalloc (kernel
	// virtual memory allocation)
	blk_ram_dev->capacity = data_size_bytes >> SECTOR_SHIFT;
	blk_ram_dev->data = kvmalloc(data_size_bytes, GFP_KERNEL);
	if (blk_ram_dev->data == NULL) {
		pr_err("memory allocation failed for the RAM disk\n");
		ret = -ENOMEM;
		goto data_err;
	}

	// Sets up the tag_set for the blk-mq layer and allocates tags
	// using blk_mq_alloc_tag_set
	memset(&blk_ram_dev->tag_set, 0, sizeof(blk_ram_dev->tag_set));
	blk_ram_dev->tag_set.ops = &blk_ram_mq_ops;
	blk_ram_dev->tag_set.queue_depth = 128;
	blk_ram_dev->tag_set.numa_node = NUMA_NO_NODE;
	blk_ram_dev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
	blk_ram_dev->tag_set.cmd_size = 0;
	blk_ram_dev->tag_set.driver_data = blk_ram_dev;
	blk_ram_dev->tag_set.nr_hw_queues = 1;

	ret = blk_mq_alloc_tag_set(&blk_ram_dev->tag_set);
	if (ret)
		goto data_err;

	// Allocates a gendisk structure (representing the block device)
	disk = blk_ram_dev->disk =
		blk_mq_alloc_disk(&blk_ram_dev->tag_set, &lim, blk_ram_dev->tag_set.driver_data);

	// Sets block sizes (logical and physical) using
	// queue_logical_block_size and queue_physical_block_size
	queue_logical_block_size(disk->queue);
	queue_physical_block_size(disk->queue);
	queue_max_segments(disk->queue);
	queue_max_segment_size(disk->queue);

	if (IS_ERR(disk)) {
		ret = PTR_ERR(disk);
		pr_err("Error allocating a disk\n");
		goto tagset_err;
	}

	// This is not necessary as we don't support partitions, and creating
	// more RAM backed devices with the existing module
	minor = ret = ida_alloc(&blk_ram_indexes, GFP_KERNEL);
	if (ret < 0)
		goto cleanup_disk;

	disk->major = major;
	disk->first_minor = minor;
	disk->minors = 1;
	snprintf(disk->disk_name, DISK_NAME_LEN, "blkram");
	disk->fops = &blk_ram_rq_ops;
	disk->flags = GENHD_FL_NO_PART;
	set_capacity(disk, blk_ram_dev->capacity);

	// Registers the disk with the block subsystem using add_disk
	ret = add_disk(disk);
	if (ret < 0)
		goto cleanup_disk;

	pr_info("module loaded\n");
	return 0;

cleanup_disk:
	put_disk(blk_ram_dev->disk);
tagset_err:
	kfree(blk_ram_dev->data);
data_err:
	kfree(blk_ram_dev);
unregister_blkdev:
	unregister_blkdev(major, "blkram");

	return ret;
}

// This function cleans up when the module is unloaded
static void __exit blk_ram_exit(void)
{
	if (blk_ram_dev->disk) {
		// Deletes the disk with del_gendisk
		del_gendisk(blk_ram_dev->disk);
		put_disk(blk_ram_dev->disk);
	}

	// Unregisters the block device
	unregister_blkdev(major, "blkram");
	// Frees allocated memory for the disk and RAM
	kfree(blk_ram_dev);

	pr_info("module unloaded\n");
}

module_init(blk_ram_init);
module_exit(blk_ram_exit);

// These macros define the module's metadata, such as its license
// (GPL), description, and version
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Generic Block Driver Example");
MODULE_VERSION("1.0");
