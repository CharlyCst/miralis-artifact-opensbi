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

uint64_t get_measure(uint64_t,uint64_t);

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
uint64_t get_measure(uint64_t category, uint64_t hart_id) {
    uint64_t value = MIRALIS_EID;
    uint64_t fid = MIRALIS_CURRENT_STATUS_FID;

    uint64_t output;

    asm volatile (
        "mv a0, %[hart_id] \n"
        "mv a1, %[category] \n"
        "mv a6, %[fid] \n"
        "mv a7, %[val] \n"
        "ecall \n"
        "mv %0, a0 \n"
        : "=r" (output)
        : [fid] "r" (fid), [hart_id] "r" (hart_id), [category] "r" (category),[val] "r" (value)
        : "a6", "a7", "a0", "a1"            
    );

    return output;
}

struct miralis_status get_status_for_core(unsigned int hart_id) {
    struct miralis_status res = {
        .world_switches = get_measure(WORLD_SWITCH, hart_id),
        .read_time = get_measure(TIME_READ, hart_id),
        .timer_request = get_measure(SET_TIMER, hart_id),
        .misaligned_op = get_measure(MISALIGNED_OP, hart_id),
        .ipi_request = get_measure(IPI, hart_id),
        .remote_fence = get_measure(REMOTE_FENCE, hart_id),
        .firmware_exits = get_measure(FIRMWARE_TRAP, hart_id)
    };

    return res;
}

static ssize_t miralis_read(struct file *file, char __user *buffer, size_t count, loff_t *offset) {
    // TODO: Ask the core dynamically until we get 0 values
    struct miralis_status res[4] = {get_status_for_core(1),get_status_for_core(2),get_status_for_core(3),get_status_for_core(4)};

    printk(KERN_INFO "Timestamp: %lld [ %lld | %lld | %lld | %lld | %lld | %lld | %lld ] \
        & [ %lld | %lld | %lld | %lld | %lld | %lld | %lld ] \
        & [ %lld | %lld | %lld | %lld | %lld | %lld | %lld ] \
        & [ %lld | %lld | %lld | %lld | %lld | %lld | %lld ]\n",
        ktime_get_real_ns(),  
        res[0].world_switches, 
        res[0].read_time, 
        res[0].timer_request, 
        res[0].misaligned_op, 
        res[0].ipi_request, 
        res[0].remote_fence, 
        res[0].firmware_exits,
        res[1].world_switches, 
        res[1].read_time, 
        res[1].timer_request, 
        res[1].misaligned_op, 
        res[1].ipi_request, 
        res[1].remote_fence, 
        res[1].firmware_exits,
        res[2].world_switches, 
        res[2].read_time, 
        res[2].timer_request, 
        res[2].misaligned_op, 
        res[2].ipi_request, 
        res[2].remote_fence, 
        res[2].firmware_exits,
        res[3].world_switches, 
        res[3].read_time, 
        res[3].timer_request, 
        res[3].misaligned_op, 
        res[3].ipi_request, 
        res[3].remote_fence, 
        res[3].firmware_exits
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
