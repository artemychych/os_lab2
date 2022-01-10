#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
 
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
 
#include <linux/ioctl.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/pci.h>
 
#define WR_VALUE _IOW('a','a', int32_t*)
#define RD_VALUE _IOR('a','b', char[]) 
 
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Stab linux module");
MODULE_VERSION("1.0");
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
 
int32_t pid = 0;
 
struct vm_area_struct_info {
    unsigned long vm_start;		
    unsigned long vm_end;
    unsigned long vm_flags;
    unsigned long vm_pgoff;
    unsigned long rb_subtree_gap;
};
 
struct signal_struct_info {
    bool valid;
    int     nr_threads;
    int     group_exit_code;
    int     notify_count;
    int     group_stop_count;
    unsigned int    flags;
};
 
struct message {
    struct signal_struct_info* ssi;
    struct vm_area_struct_info* vasi;
    pid_t pid;
};
 
struct task_struct* ts;
 
/*
** Function Prototypes
*/
static int      __init etx_driver_init(void);
static void     __exit etx_driver_exit(void);
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 
struct message get_message_with_struct_info(void);
 
/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .unlocked_ioctl = etx_ioctl,
        .release        = etx_release,
};
 
/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}
 
/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}
 
/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read Function\n");
        return 0;
}
 
/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");
        return len;
}
 
 
/*
** This function will be called when we write IOCTL on the Device file
*/
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                       if(copy_from_user(&pid ,(int32_t*) arg, sizeof(pid)))
                        {
                                pr_err("Data Write : Err!\n");
                        }
                        pr_info("Value = %d\n", pid);
                        break;
    		        case RD_VALUE:
	                      char buf[4096];
                        size_t len = 0;
	
	                      struct task_struct *tsk = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
                        struct signal_struct *sig;
                        struct vm_area_struct *vma;

                        sig = tsk->signal;
                        vma = tsk->mm->mmap;

                          if(task_struct_ref == NULL){
                            len += sprintf(buf,"task_struct for pid %d is NULL.Can not get any information\n", pid);

                          }else{

                          len += sprintf(buf, "vm_area_struct_info : ");
                          len += sprintf(buf+len, "vm_start = %lu\n", sig.vm_start);
                          len += sprintf(buf+len, "vm_end = %hu\n", sig.vm_end);
                          len += sprintf(buf+len, "flags = %hu\n", sig.vm_flags); 
                          len += sprintf(buf+len, "vm_pgoff = %hu\n", sig.vm_pgoff);
                          len += sprintf(buf+len, "rb_subtree_gap = %hu\n", sig.rb_subtree_gap);
                          len += sprintf(buf+len, "signal_struct_info : ");
                          len += sprintf(buf+len, "valid = %hu\n", vma.valid);
                          len += sprintf(buf+len, "nr_threads = %hu\n", vma.nr_threads);
                          len += sprintf(buf+len, "group_exit_code = %hu\n", vma.group_exit_code);
                          len += sprintf(buf+len, "notify_count = %hu\n", vma.notify_count);
                          len += sprintf(buf+len, "group_stop_count = %hu\n", vma.group_stop_count);
                          len += sprintf(buf+len, "flags = %hu\n", vma.flags);

                          }
      
 
 
                          if( copy_to_user((int32_t*) arg, buf, len) )
                            {
                              pr_err("Data Read : Err!\n");
                            }
                            break;
 
				
                 default:
                                  pr_info("Default\n");
                                  break;
                  }
        return 0;
}
 
/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&etx_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&etx_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        pr_info("Device Driver Insert...Done!!!\n");
        return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
 
/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(etx_driver_init);
module_exit(etx_driver_exit);
