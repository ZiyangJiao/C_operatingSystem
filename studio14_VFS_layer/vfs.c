//
// Created by ziyang jiao on 2019-11-07.
//
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/fs_struct.h>
#include <linux/nsproxy.h>
#include <linux/dcache.h>
#include <linux/mount.h>
//access by task - for step 2
struct fs_struct *fs;//the process's filesystem structure
struct files_struct *files;//open file table structure
struct nsproxy *nsproxy;//namespace proxy structure
struct task_struct *task;

//access by fs - for step 3
struct path *pwd;//the process' current working directory
struct path *root;//the process'the root directory
/* Each of these path structures contains two fields:
 * mnt which points to a VFS mount structure (struct vfsmount, declared in <linux/mount.h>) and
 * dentry which points to a directory entry structure (struct dentry, declared in <linux/dcache.h>).
 */
struct vfsmount *pwd_mnt;
struct dentry *pwd_dentry;
struct vfsmount *root_mnt;
struct dentry *root_dentry; //access by task -> fs -> pwd / root -> dentry - for step 4

//for step 5 - traverses the list of directory entries whose head is in the d_subdirs field of the directory entry structure to which the path struct for the root directory points.
struct dentry *entry;
//for step 6 - it checks the d_subdirs list of each directory entry it visits: if the list in is non-empty, the kernel thread should print out the d_iname field of each directory entry in that list.
struct dentry *eentry;



/*
 * my kernel thread function
 * A static function in C is a function that has a scope that is limited to its object file.
 */
static int mykthread(void* f){
        //Step 2:
        fs = current->fs;
        files = current->files;
        nsproxy = current->nsproxy;
        printk(KERN_WARNING "The address of the process's filesystem structure: %p\n",fs);
        printk(KERN_WARNING "The address of the process's open file table structure: %p\n",files);
        printk(KERN_WARNING "The address of the process's namespace proxy structure: %p\n",nsproxy);

        //step 3:
        *pwd = fs->pwd;
        *root = fs->root;
        pwd_mnt = pwd->mnt;
        pwd_dentry = pwd->dentry;
        root_mnt = root->mnt;
        root_dentry = root->dentry;
        printk(KERN_WARNING "The address of the process's current working directory structure's vfsmount structure: %p\n",pwd_mnt);
        printk(KERN_WARNING "The address of the process's root working directory structure's vfsmount structure: %p\n",root_mnt);
        if(pwd_mnt != root_mnt){
            printk(KERN_WARNING "pwd_mnt != root_mnt\n");
            printk(KERN_WARNING "The pointer values of pwd_mnt's mnt_root structure: %p\n",pwd_mnt->mnt_root);
            printk(KERN_WARNING "The pointer values of pwd_mnt's mnt_sb structure: %p\n",pwd_mnt->mnt_sb);
            printk(KERN_WARNING "The pointer values of root_mnt's mnt_root structure: %p\n",root_mnt->mnt_root);
            printk(KERN_WARNING "The pointer values of root_mnt's mnt_sb structure: %p\n",root_mnt->mnt_sb);

        }

        //step 4:
        printk(KERN_WARNING "The address of the process's current working directory structure's dentry structure: %p\n",pwd_dentry);
        printk(KERN_WARNING "The address of the process's root working directory structure's dentry structure: %p\n",root_dentry);
        if(pwd_dentry != root_dentry){
            printk(KERN_WARNING "pwd_dentry != root_dentry\n");
            printk(KERN_WARNING "The strings in the d_iname fields of the pwd's directory entry structures: %s\n",pwd_dentry->d_iname);
            printk(KERN_WARNING "The strings in the d_iname fields of the root's directory entry structures: %s\n",root_dentry->d_iname);
        }

        //step 5:
        list_for_each_entry(entry, &(root_dentry->d_subdirs), d_child){
            printk(KERN_WARNING "[step 5 the first layer of iteration]%s\n", entry->d_iname);
            //step 6:
            if(&(entry->d_subdirs) != NULL){
                list_for_each_entry(eentry, &(eentry->d_subdirs), d_child){
                    printk(KERN_WARNING "[step 6 the second layer of iteration]%s\n", eentry->d_iname);
                }
            }
        }

        while(!kthread_should_stop()){

        }

        return 0;
}

//init function
static int simple_init(void){
    printk(KERN_WARNING "VFS module start!\n");
    task = (struct task_struct*)kthread_run(mykthread, NULL, "VFS_Kthread");
    return 0;
}

//exit function
static void simple_exit(void){
    kthread_stop(task);
    printk(KERN_WARNING "VFS module finish!\n");
    return;
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("GWL");
MODULE_DESCRIPTION("422 VFS module");