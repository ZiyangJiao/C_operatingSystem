/*
 * family_reader implementation
 *
 * Copyright (C) 2004-2007 Greg Kroah-Hartman <greg@kroah.com>
 * Copyright (C) 2007 Novell Inc.
 *
 * Released under the GPL version 2 only.
 *
 */
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/pid.h>   //fing_vpid() pid_task()
#include <linux/sched.h> //task_struct
/*
 * this module reads a PID through the sysfs interface and prints that process' ancestry in the system log.
 */

static int usrpid;

/*
 * The "usrpid" file where a static userspace pid variable is read from and written to.
 */
static ssize_t usrpid_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", usrpid);
}

static ssize_t usrpid_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;
	struct pid *kpid;
	struct task_struct *task;
	ret = kstrtoint(buf, 10, &usrpid);
	if (ret < 0){
		printk(KERN_INFO "Invalid pid number!\n");
		return ret;
	}
	kpid = find_vpid(usrpid);
	if(!IS_ERR(kpid)){
		task = pid_task(kpid, PIDTYPE_PID);
		if(!IS_ERR(task)){
			while(!IS_ERR(task) && (task->pid!=0)){
				printk(KERN_INFO "real pid is %u-----commend name is %s\n",task->pid,task->comm);
				if(task->pid == 1){
					break;
				}
				task = task->real_parent;
				
				if(task->pid == 1){
					printk(KERN_INFO "real pid is %u-----commend name is %s\n",task->pid,task->comm);
					break;
				}
			}
		}else{
			printk(KERN_ERR	"pid_task() error!\n");
			return -1;
		}
	}else{
		printk(KERN_ERR	"find_vpid() error!\n");
		return -1;
	}
	return count;
}

/* Sysfs attributes cannot be world-writable. */
static struct kobj_attribute usrpid_attribute =
	__ATTR(usrpid, 0664, usrpid_show, usrpid_store);


/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&usrpid_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

/*
 * An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *ancestry_reader;

static int __init example_init(void)
{
	int retval;

	/*
	 * Create a simple kobject with the name of "ancestry_reader",
	 * located under /sys/kernel/
	 *
	 * As this is a simple directory, no uevent will be sent to
	 * userspace.  That is why this function should not be used for
	 * any type of dynamic kobjects, where the name and number are
	 * not known ahead of time.
	 */
	ancestry_reader = kobject_create_and_add("ancestry_reader", kernel_kobj);
	if (!ancestry_reader)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(ancestry_reader, &attr_group);
	if (retval)
		kobject_put(ancestry_reader);

	return retval;
}

static void __exit example_exit(void)
{
	kobject_put(ancestry_reader);
}

module_init(example_init);
module_exit(example_exit);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Greg Kroah-Hartman <greg@kroah.com>");
