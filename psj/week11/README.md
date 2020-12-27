# On-memory File System 1

## Homework

### 1. Your Gentoo Linux has two disks: "/dev/sda3" and "/dev/sda1". Which one is the root file system? Where is the mounting point for the other one? Use `mount` command to answer this.

```bash
$ mount
```

"/dev/sda3"은 "/"에 연결되었고, "/dev/sda1"은 "/boot"에 연결되었다.
따라서 "/dev/sda3"은 루트 파일 파티션이고, "/dev/sda1"은 부팅 파티션이다.

### 2. Add another entry in "/boot/grub/grub.conf" as below. This boot selection does not use initrd directive to prevent initramfs loading(initramfs is a temporary in-ram file system used for performance improvement).

```text
title=MyLinux3
root (hd0,0)
kernel /boot/bzImage root=/dev/sda3
```

My Linux를 복사해 `initrd`를 지우고 `root`를 "/dev/ram0"에서 "/dev/sda3"로 바꿔준다.

재부팅을 하면 "My Linux 3" 옵션이 생긴 것을 볼 수 있고, 부팅 메시지에서 "/dev/sda3"로 루트 파일 디렉토리가 잡히는 것을 확인할 수 있다.

### 3. The kernel calls `mount_root` to cache the root file system. Starting from `start_kernel`, find out the call chain that leads to `mount_root`.

`init/main.c`:

```c
asmlinkage void __init start_kernel(void) {
    ...
    rest_init();
}

static void noinline __init_refok rest_init(void)
    __releases(kernel_lock) {
    int pid;

    kernel_thread(kernel_init, NULL, CLONE_FS | CLONE_SIGHAND);
    numa_default_policy();
    pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
    kthreadd_task = find_task_by_pid(pid);
    unlock_kernel();

    ...
}

static int __init kernel_init(void *unused) {
    ...
    if (!ramdisk_execute_command)
        ramdisk_execute_command = "/init";

    if (sys_access((const char __user *) ramdisk_execute_command, 0) != 0) {
        ramdisk_execute_command = NULL;
        prepare_namespace();
    }
    ...
    return 0;
}
```

`init/do_mounts.c`:

```c
void __init prepare_namespace(void) {
    ...
    if (saved_root_name[0]) {
        root_device_name = saved_root_name;
        if (!strncmp(root_device_name, "mtd", 3)) {
            mount_block_root(root_device_name, root_mountflags);
            goto out;
        }
        ROOT_DEV = name_to_dev_t(root_device_name);
        if (strncmp(root_device_name, "/dev/", 5) == 0)
            root_device_name += 5;
    }

    if (initrd_load())
        goto out;
    ...
    mount_root();
out:
    sys_mount(".", "/", NULL, MS_MOVE, NULL);
    sys_chroot(".");
}

void __init mount_root(void) {
#ifdef CONFIG_ROOT_NFS
    if (MAJOR(ROOT_DEV) == UNNAMED_MAJOR) {
        if (mount_nfs_root())
            return;

        printk(KERN_ERR "VFS: Unable to mount root fs via NFS, trying floppy.\n");
        ROOT_DEV = Root_FD0;
    }
#endif
#ifdef CONFIG_BLK_DEV_FD
    if (MAJOR(ROOT_DEV) == FLOPPY_MAJOR) {
        /* rd_doload is 2 for a dual initrd/ramload setup */
        if (rd_doload == 2) {
            if (rd_load_disk(1)) {
                ROOT_DEV = Root_RAM1;
                root_device_name = NULL;
            }
        } else
            change_floppy("root floppy");
    }
#endif
#ifdef CONFIG_BLOCK
    create_dev("/dev/root", ROOT_DEV);
    mount_block_root("/dev/root", root_mountflags);
#endif
}
```

`start_kernel` -> `rest_init` -> `kernel_init` -> `prepare_namespace` -> `mount_root` 순으로 호출한다.
`prepare_namespace`는 `saved_root_name`을 보고 어떤 것을 어디에 연결할 지 결정하는 함수이고, `mount_root`는 Superblock과 루트 inode 등의 루프 파일 시스템을 캐싱하는 함수이다.

### 4. Find the data type for each added variable for `super_block`, `inode`, `buffer_head`, and `dentry`.

`include/linux/fs.h`:

```c
struct super_block {
    struct list_head s_list; /* Keep this first */
    dev_t s_dev;             /* search index; _not_ kdev_t */
    unsigned long s_blocksize;
    unsigned char s_blocksize_bits;
    unsigned char s_dirt;
    unsigned long long s_maxbytes; /* Max file size */
    struct file_system_type *s_type;
    const struct super_operations *s_op;
    struct dquot_operations *dq_op;
    struct quotactl_ops *s_qcop;
    const struct export_operations *s_export_op;
    unsigned long s_flags;
    unsigned long s_magic;
    struct dentry *s_root;
    struct rw_semaphore s_umount;
    struct mutex s_lock;
    ...
};

struct inode {
    struct hlist_node i_hash;
    struct list_head i_list;
    struct list_head i_sb_list;
    struct list_head i_dentry;
    unsigned long i_ino;
    atomic_t i_count;
    unsigned int i_nlink;
    uid_t i_uid;
    gid_t i_gid;
    dev_t i_rdev;
    u64 i_version;
    loff_t i_size;
#ifdef __NEED_I_SIZE_ORDERED
    seqcount_t i_size_seqcount;
#endif
    struct timespec i_atime;
    struct timespec i_mtime;
    struct timespec i_ctime;
    unsigned int i_blkbits;
    blkcnt_t i_blocks;
    unsigned short i_bytes;
    umode_t i_mode;
    spinlock_t i_lock; /* i_blocks, i_bytes, maybe i_size */
    struct mutex i_mutex;
    ...
};
```

`include/linux/buffer_head.h`:

```c
struct buffer_head {
    unsigned long b_state;           /* buffer state bitmap (see above) */
    struct buffer_head *b_this_page; /* circular list of page's buffers */
    struct page *b_page;             /* the page this bh is mapped to */

    sector_t b_blocknr; /* start block number */
    size_t b_size;      /* size of mapping */
    char *b_data;       /* pointer to data within the page */

    struct block_device *b_bdev;
    bh_end_io_t *b_end_io;             /* I/O completion */
    void *b_private;                   /* reserved for b_end_io */
    struct list_head b_assoc_buffers;  /* associated with another mapping */
    struct address_space *b_assoc_map; /* mapping this buffer is associated with */
    atomic_t b_count;                  /* users using this buffer_head */
};
```

`include/linux/deache.h`:

```c
struct dentry {
    atomic_t d_count;
    unsigned int d_flags;  /* protected by d_lock */
    spinlock_t d_lock;     /* per dentry lock */
    struct inode *d_inode; /* Where the name belongs to - NULL is negative */
    /*
     * The next three fields are touched by __d_lookup.  Place them here
     * so they all fit in a cache line.
     */
    struct hlist_node d_hash; /* lookup hash list */
    struct dentry *d_parent;  /* parent directory */
    struct qstr d_name;

    struct list_head d_lru; /* LRU list */
    /*
     * d_child and d_rcu can share memory
     */
    union {
        struct list_head d_child; /* child of parent list */
        struct rcu_head d_rcu;
    } d_u;
    struct list_head d_subdirs; /* our children */
    struct list_head d_alias;   /* inode alias list */
    unsigned long d_time;       /* used by d_revalidate */
    struct dentry_operations *d_op;
    struct super_block *d_sb; /* The root of the dentry tree */
    void *d_fsdata;           /* fs-specific data */
#ifdef CONFIG_PROFILING
    struct dcookie_struct *d_cookie; /* cookie, if any */
#endif
    int d_mounted;
    unsigned char d_iname[DNAME_INLINE_LEN_MIN]; /* small names */
};
```

### 5. Change the kernel such that it displays all superblocks before it calls `mount_root` and after `mount_root`. Boot with "MyLinux 3" to see what happens.

`init/do_mounts.c`:

```c
void display_super_blocks(void) {
    struct super_block *sb;
    list_for_each_entry(sb, &super_blocks, s_list) {
        printk("dev name: %s, dev major num: %d, dev minor num: %d, root ino: %d\n", sb->s_id, MAJOR(sb->s_dev), MINOR(sb->s_dev), sb->s_root->d_inode->i_ino);
    }
}

void __init prepare_namespace(void) {
    ...

    if (is_floppy && rd_doload && rd_load_disk(0))
        ROOT_DEV = Root_RAM0;

    printk("before mount_root():\n");
    display_super_blocks();
    mount_root();
    printk("after mount_root():\n");
    display_super_blocks();
out:
    sys_mount(".", "/", NULL, MS_MOVE, NULL);
    sys_chroot(".");
}
```

`mount_root`가 호출된 이후에는 "dev name: sda3, dev major num: 8, dev minor num: 3, root ino: 2"가 더 출력된다.

디바이스 번호는 각 디바이스의 고유번호이다.
"/dev"에 각 디바이스의 파일 이름이 적혀 있고, `ls -l`을 통해 각 디바이스의 major, minor 번호를 볼 수 있다.
major 번호는 그 디바이스의 번호이고, minor 번호는 그 디바이스 종류 안에서의 구별 번호를 의미한다.
위 내용은 "Documentation/devices.txt"으로 가면 자세한 정보를 볼 수 있다.

### 6. Change the kernel such that it displays all cached inodes before it calls `mount_root` and after `mount_root`. Boot with "MyLinux 3" to see what happens.

`init/do_mounts.c`:

```c
extern struct list_head inode_in_use;
void display_all_inodes(void) {
    struct inode *in;
    list_for_each_entry(in, &inode_in_use, i_list) {
        printk("dev major num: %d, dev minor num: %d, inode num: %d, sb dev: %s\n", MAJOR(in->i_rdev), MINOR(in->i_rdev), in->i_ino, in->i_sb->s_id);
    }
}

void __init prepare_namespace(void) {
    ...
    if (is_floppy && rd_doload && rd_load_disk(0))
        ROOT_DEV = Root_RAM0;

    printk("before mount_root():\n");
    display_all_inodes();
    mount_root();
    printk("after mount_root():\n");
    display_all_inodes();
out:
    sys_mount(".", "/", NULL, MS_MOVE, NULL);
    sys_chroot(".");
}
```

cahce된 inode를 볼 수 있다.

### 7. The `pid=1` process(`kernel_init`) eventually execs to "/sbin/init" with `run_init_process("/sbin/init");` by calling `kernel_execve("/sbin/init", ...)` in `init/main.c/init_post()`. Change the kernel such that it execs to "/bin/sh". Boot the kernel, and you will find you cannot access "/boot/grub/grub.conf". Explain why.

`init/main.c`:

```c
static int noinline init_post(void) {
    ...
    if (execute_command) {
        run_init_process(execute_command);
        printk(KERN_WARNING "Failed to execute %s.  Attempting "
                            "defaults...\n",
               execute_command);
    }
    run_init_process("/bin/sh");
    run_init_process("/etc/init");
    run_init_process("/bin/init");
    run_init_process("/bin/sh");

    panic("No init found.  Try passing init= option to kernel.");
}
```

커널이 로드되면 메모리, 프로세서, I/O 등 여러 하드웨어를 초기화하고 설정한다.
압축된 `initramfs` 이미지를 메모리의 미리 정해진 위치로부터 읽어 "/sysroot/"에 직접 풀고, 모든 필요한 드라이버를 로드한다.
그 후, 커널은 루트 장치를 생성하여 읽기 전용으로 루트 파티션을 마운트하고 사용되지 않는 메모리를 해제한다.

커널이 로드되면 사용자 환경을 설정하기 위해 "/sbin/init" 프로그램을 실행한다.
"/sbin/init" 프로그램은 최상위 프로세스(pid = 1)로, 나머지 부트 프로세스를 주관하며 사용자를 위한 환경을 설정하는 역할을 한다.

"/sbin/init"는 파일 시스템의 구조를 검사하고, 시스템을 마운트하고, 서버 데몬을 띄우고, 사용자 로그인을 기다리는 등의 역할을 한다.
만약 "/sbin/init"을 실행하지 않고 "/bin/sh"를 실행하면, "/dev/sda1"가 "/boot"에 연결되지 않을 것이다.

