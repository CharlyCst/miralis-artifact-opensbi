#include <linux/module.h> 
#include <linux/kernel.h>  
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

#define MIRALIS_EID 0x08475bcd
#define MIRALIS_CURRENT_STATUS_FID 0x4

#define PROC_NAME "miralis"

#define BUFFER_LEN 2

#define STATS_CORE_0 0
#define STATS_CORE_ALL 1

#define WORLD_SWITCH 0
#define TIME_READ 1
#define SET_TIMER 2
#define MISALIGNED_OP 3
#define IPI 4
#define REMOTE_FENCE 5
#define FIRMWARE_TRAP 6

// Struct declarations

struct miralis_status {
    uint64_t world_switches;
    uint64_t read_time;
    uint64_t timer_request;
    uint64_t misaligned_op;
    uint64_t ipi_request;
    uint64_t remote_fence;
    uint64_t firmware_exits;
};

// Function declarations

struct miralis_status get_measures(unsigned int);

static ssize_t miralis_read(struct file *file, char __user *buffer, size_t count, loff_t *offset);
static ssize_t miralis_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset);

static int __init load_benchmark_module(void);
static void __exit unload_benchmark_module(void);

// Global variables

static const struct proc_ops miralis_fops = {
    .proc_read = miralis_read,
    .proc_write = miralis_write, 
};

static struct proc_dir_entry *miralis_dir_entry;

// Functions definitions

uint64_t get_measure(unsigned int category) {
    uint64_t value = MIRALIS_EID;
    uint64_t fid = MIRALIS_CURRENT_STATUS_FID;

    uint64_t output;

   asm volatile (
        "mv a0, %[category] \n"
        "mv a6, %[fid] \n"
        "mv a7, %[val] \n"
        "ecall \n"
        "mv %0, a0 \n"
        : "=r" (output)
        : [fid] "r" (fid), [category] "r" (category),[val] "r" (value)
        : "a6", "a7", "a0", "a1"            
    );

   return output;
}

static ssize_t miralis_read(struct file *file, char __user *buffer, size_t count, loff_t *offset) {
    // TODO: Ask the core dynamically until we get 0 values
    struct miralis_status status = get_measures(0);

    struct miralis_status res = {
        .world_switches = get_measure(WORLD_SWITCH);
        .read_time = get_measure(TIME_READ);
        .timer_request = get_measure(SET_TIMER);
        .misaligned_op = get_measure(MISALIGNED_OP)
        .ipi_request = get_measure(IPI);
        .remote_fence = get_measure(REMOTE_FENCE);
        .firmware_exits = get_measure(FIRMWARE_TRAP),
    };

    // TODO: Set the time properly
    printk(KERN_INFO "[ %lld | %lld | %lld | %lld | %lld | %lld | %lld | %lld ]\n",
        ktime_get_real_ns(),  
        res.world_switches, 
        res.read_time, 
        res.timer_request, 
        res.misaligned_op, 
        res.ipi_request, 
        res.remote_fence, 
        res.firmware_exits
    );

    if (*offset >= 16) {
        return 0; 
    }

    size_t bytes_to_copy = min(count, (size_t)(16 - *offset));
    if (copy_to_user(buffer, buffer + *offset, bytes_to_copy)) {
        return -EFAULT;
    }

    *offset += bytes_to_copy;
    
    return bytes_to_copy;
}

static ssize_t miralis_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset) {
    return count; // Ignore any writes
}

static int __init load_benchmark_module(void)
{
    printk(KERN_INFO "Inserting the miralis benchmark kernel module\n");

    miralis_dir_entry = proc_create(PROC_NAME, 0666, NULL, &miralis_fops);
    if (!miralis_dir_entry) {
        pr_err("Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("Module loaded, /proc/%s created\n", PROC_NAME);

    return 0; 
}

static void __exit unload_benchmark_module(void)
{
    if (miralis_dir_entry) {
        remove_proc_entry(PROC_NAME, NULL);
    }

    printk(KERN_INFO "Miralis benchmark module unloaded, /proc/miralis removed\n");
}

module_init(load_benchmark_module);
module_exit(unload_benchmark_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Francois Costa");
MODULE_DESCRIPTION("A linux kernel module used for the benchmarking of Miralis. It retrieves the number of world switches and firmware exits");
