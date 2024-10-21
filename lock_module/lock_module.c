#include <linux/module.h>  // Needed by all kernel modules
#include <linux/kernel.h>  // Needed for KERN_INFO

#define MIRALIS_PROTECT_PAYLOAD_EID 0x08475bcd + 1
#define MIRALIS_PROTECT_PAYLOAD_LOCK_FID 0x1

static int __init minimal_init(void)
{
    printk(KERN_INFO "Locking the kernel!\n");

    uint64_t value = MIRALIS_PROTECT_PAYLOAD_EID;
    uint64_t fid = MIRALIS_PROTECT_PAYLOAD_LOCK_FID;

    asm volatile (
            "mv a6, %[fid]\n"
            "mv a7, %[val]\n"
            "ecall"
            :
            : [fid] "r" (fid), [val] "r" (value)
    : "a6", "a7"
    );

    return 0;  // Success
}

static void __exit minimal_exit(void)
{
    printk(KERN_INFO "Kernel module lock unloaded\n");
}

module_init(minimal_init);
module_exit(minimal_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Francois Costa");
MODULE_DESCRIPTION("A linux kernel module to communicate with the protect payload policy of Miralis");
