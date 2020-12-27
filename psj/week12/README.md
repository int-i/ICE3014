# On-memory file system 2

## Homework

### 8. Try following code. Make "/aa/bb" and type some text with length longer than 50 bytes. Explain the result.

See [HW 8](./hw08/main.c)

`dup`는 `x`를 복사해 같은 곳에서 파일을 읽어오는 함수이다.
커서 위치도 복사하게 되므로 이전에 읽었던 곳부터 계속 읽게 된다.

`link`는 파일의 별칭을 지정하는 함수로, "newbb"에 접근하면 "bb"에 접근할 수 있게 넘겨준다.

### 9. Check the inode number of "/aa/bb" and "/aa/newbb" and confirm they are same.

```bash
$ ls -i /aa
405603 /aa/bb  405603 /aa/newbb
```

"newbb"는 "bb"의 별칭이기 때문에 inode 값이 같다. 따라서 inode table상의 이름만 다르고 두 테이블이 같은 inode를 가리킨다고 생각할 수 있다.

### 10. Try `fork()` and confirm the parent and child can access the same file.

See [HW 10](./hw10/main.c)

```bash
$ cat /aa/bb
qwertyuiopasdfghjklzxcvbnm
$ ./hw10
child read qwertyuiop
parent read asdfghjklz
```

프로세스가 fork되면 `x`의 `f_pos`가 저장되는 위치도 같이 복사되므로 두 프로세스가 이를 공유하게 된다.
따라서 parent는 child가 읽었던 부분부터 계속 읽게 된다.

### 11. (Using `chroot` and `chdir`) Do following and explain the result of "hw11".

#### a. Make f1 in several places with different content (in "/", in "/root", and in "/root/d1") as follows.

```bash
$ cd /
$ echo hello1 > f1
$ cd
$ echo hello2 > f1
$ mkdir d1
$ echo hello3 > d1/f1
```

#### b. Make "hw11.c" that will display "/f1" before and after `chroot`, and "f1" before and after `chdir` as follows.

See [HW 11](./hw11/main.c)

"/f1"을 출력하고 `chroot`로 루트를 현재 디렉토리로 바꾸면 "/f1"이 "./f1"이 되어 home의 "f1"을 출력한다.
따라서 `display_root_f1`과 `display_f1`의 출력이 같다.
`chdir`은 디렉토리를 변경하는 함수로 "d1"으로 변경하게 되면 "./d1/f1"의 내용을 출력한다.

### 12. Make a new system call, `show_fpos()`, which will display the current process ID and the file position for fd=3 and fd=4 of the current process. Use this system call to examine file position as follows. (Use `%lld` to print the file position since `f_pos` is long long integer)

`arch/x86/kernel/syscall_table_32.S`:

```s
...
.long sys_chmod      /* 15 */
.long sys_lchown16
.long my_show_fpos
.long sys_stat
.long sys_lseek
.long sys_getpid     /* 20 */
...
```

`my_show_fpos` 시스템 콜을 등록해준다.

`fs/read_write.c`:

```c
asmlinkage void my_show_fpos(void) {
    printk("fd=3, f_pos=%lld\n", current->files->fdt->fd[3]->f_pos);
    printk("fd=4, f_pos=%lld\n", current->files->fdt->fd[4]->f_pos);
}
```

`my_show_fpos`를 구현한다.
`f_pos`는 아래와 같이 찾아갈 수 있다.

`include/linux/sched.h`:

```c
struct task_struct {
    ...
    /* filesystem information */
    struct fs_struct *fs;
    /* open file information */
    struct files_struct *files;
    /* namespaces */
    ...
};
```

`include/linux/fs.h`:

```c
struct files_struct {
    /*
     * read mostly part
     */
    atomic_t count;
    struct fdtable *fdt;
    struct fdtable fdtab;
    ...
};

struct fdtable {
    unsigned int max_fds;
    struct file **fd; /* current fd array */
    fd_set *close_on_exec;
    fd_set *open_fds;
    struct rcu_head rcu;
    struct fdtable *next;
};

struct file {
    union {
        struct list_head fu_list;
        struct rcu_head fu_rcuhead;
    } f_u;
    struct path f_path;
#define f_dentry f_path.dentry
#define f_vfsmnt f_path.mnt
    const struct file_operations *f_op;
    atomic_t f_count;
    unsigned int f_flags;
    mode_t f_mode;
    loff_t f_pos;
    ...
};
```

`f_pos`를 출력하는 시스템 콜을 만들었다.
`syscall(17)`로 이를 호출할 수 있다.

See [HW 12](./hw12/main.c)

```bash
$ echo 8 > /proc/sys/kernel/printk
$ ./hw12
fd=3, f_pos=0
fd=4, f_pos=0
fd=3, f_pos=10
fd=4, f_pos=10
```

`x`와 `y`는 각각 파일 디스크립터 3과 4를 의미한다.
둘 다 10글자씩 읽었으므로 `f_pos`가 0에서 10이 되었다.

### 13. Modify your `show_fpos()` such that it also displays the address of `f_op->read` and `f_op->write` function for fd 0, fd 1, fd 2, fd 3, and fd 4, respectively. Find the corresponding function names in "System.map". Why the system uses different functions for fd 0, 1, 2 and fd 3 or 4?

`fs/read_write.c`:

```c
asmlinkage void my_show_fpos(void) {
    printk("fd=3, f_pos=%lld\n", current->files->fdt->fd[3]->f_pos);
    printk("fd=4, f_pos=%lld\n", current->files->fdt->fd[4]->f_pos);
    int i;
    for(i = 0; i < 5; i += 1) {
        printk("fd=%d, read=%p\n", i, current->files->fdt->fd[i]->f_op->read);
        printk("fd=%d, write=%p\n", i, current->files->fdt->fd[i]->f_op->write);
    }
}
```

```bash
$ echo 8 > /proc/sys/kernel/printk
$ ./hw12
fd=3, f_pos=0
fd=4, f_pos=0
fd=0, read=c024ece2
fd=0, write=c024e412
fd=1, read=c024ece2
fd=1, write=c024e412
fd=2, read=c024ece2
fd=2, write=c024e412
fd=3, read=c015c451
fd=3, write=c015c351
...
```

read와 write 함수의 주소를 출력할 수 있게 했다.
출력된 주소를 리눅스 코드의 "System.map"에서 찾아보면 아래와 같이 나온다.
"System.map"은 컴파일할 때마다 리눅스 코드 디렉토리에 생성된다.

`System.map`:

```text
...
c024e412 t tty_write
...
c024ece2 t tty_read
...
c015c351 T do_sync_write
...
c015c451 T do_sync_read
...
```

|          | read         | write         |
| -------- | ------------ | ------------- |
| fd=0,1,2 | tty_read     | tty_write     |
| fd=3,4   | do_sync_read | do_sync_write |

표로 정리해보면 위와 같이 파일 디스크립터가 0, 1, 2일 때와 3, 4일 때로 나누어져 결과가 나온다.
그 이유는 파일 디스크립터 0, 1, 2는 리눅스에서 각각 표준입력, 표준출력, 표준에러출력 스트림에 사용하도록 예약되어 있고, 사용자 파일에 대한 파일 디스크립터는 3부터 시작하기 때문이다.

`include/unistd.h`:

```c
#define STDIN_FILENO 0 /* Standard input. */
#define STDOUT_FILENO 1 /* Standard output. */
#define STDERR_FILENO 2 /* Standard error output. */
```

### 14. Use `show_fpos()` to explain the result of the following code. File "f1" has "ab" and File "f2" has "q". When you run the program, File "f2" will have "ba". Explain why "f2" have "ba" after the execution.

See [HW 14](./hw14/main.c)

`fork`에 의해 `f_pos`를 공유하는 프로세스 2개로 나누어진다.
가장 먼저 자식 프로세스에서 `f1`과 `f2` 초기 상태를 출력하고 둘 다 `f_pos`는 0이다.
그후 "f1" 파일을 읽어 `buf`에 저장한다. 현재 `buf`에서는 `['a']`가 저장되어 있다.

자식 프로세스가 2초간 대기하는 사이에, 부모 프로세스는 `f1`과 `f2` 상태를 출력하고 이때 `f1`의 `f_pos`가 읽은 글자 수만큼 증가한 것을 확인할 수 있다.
다시 한 글자 읽어 `buf`에 저장하면 `buf`에는 `['b']`가 저장되게 된다.
두 프로세스 사이에 `buf`와 같은 지역변수는 공유되지 않는다.

부모 프로세스의 `buf`를 "f2"에 저장하고, 1초 후 자식 프로세스의 `buf`를 "f2"에 저장하면 "f2"는 "ba"가 된다.

### 15. Find corresponding kernel code for each step below in open and read system calls:

#### `x = open(fpath, .......);`

#### 1) find empty fd

`fs/open.c`:

```c
asmlinkage long sys_open(const char __user *filename, int flags, int mode) {
    long ret;

    if (force_o_largefile())
        flags |= O_LARGEFILE;

    ret = do_sys_open(AT_FDCWD, filename, flags, mode);
    ... return ret;
}

long do_sys_open(int dfd, const char __user *filename, int flags, int mode) {
    char *tmp = getname(filename);
    int fd = PTR_ERR(tmp);

    if (!IS_ERR(tmp)) {
        fd = get_unused_fd_flags(flags);
        if (fd >= 0) {
            struct file *f = do_filp_open(dfd, tmp, flags, mode);
            if (IS_ERR(f)) {
                put_unused_fd(fd);
                fd = PTR_ERR(f);
            } else {
                fsnotify_open(f->f_path.dentry);
                fd_install(fd, f);
            }
        }
        putname(tmp);
    }
    return fd;
}
```

`get_unused_fd_flags`로 사용하지 않는 파일 디스크립터 번호를 찾는다.
0, 1, 2는 각각 표준입력, 표준출력, 표준에러스트림으로 사용되므로 사용자 파일 디스크립터는 3부터 할당된다.

`do_filp_open`은 `open_namei`를 호출하는데 `open_namei`는 파일 시스템에서 파일의 inode를 찾아 반환하는 함수이다.

#### 2) search the inode for "fpath"

##### 2-1) if "fpath" starts with "/", start from `fs->root` of the current process

##### 2-2) otherwise, start from `fs->pwd`

##### 2-3) visit each directory in "fpath" to find the inode of the "fpath"

##### 2-4) while following mounted file path if it is a mounting point.

`fs/namei.c`:

```c
int open_namei(int dfd, const char *pathname, int flag,
        int mode, struct nameidata *nd)
{
    ...
    if (!(flag & O_CREAT)) {
        error = path_lookup_open(dfd, pathname, lookup_flags(flag),
                     nd, flag);
        if (error)
            return error;
        goto ok;
    }
    ...

    if (__follow_mount(&path)) {
        error = -ELOOP;
        if (flag & O_NOFOLLOW)
            goto exit_dput;
    }

}

static int do_path_lookup(int dfd, const char *name,
                unsigned int flags, struct nameidata *nd)
{
    int retval = 0;
    int fput_needed;
    struct file *file;
    struct fs_struct *fs = current->fs;

    nd->last_type = LAST_ROOT; /* if there are only slashes... */
    nd->flags = flags;
    nd->depth = 0;

    if (*name=='/') {
        read_lock(&fs->lock);
        if (fs->altroot.dentry && !(nd->flags & LOOKUP_NOALT)) {
            nd->path = fs->altroot;
            path_get(&fs->altroot);
            read_unlock(&fs->lock);
            if (__emul_lookup_dentry(name,nd))
                goto out; /* found in altroot */
            read_lock(&fs->lock);
        }
        nd->path = fs->root;
        path_get(&fs->root);
        read_unlock(&fs->lock);
    } else if (dfd == AT_FDCWD) {
        read_lock(&fs->lock);
        nd->path = fs->pwd;
        path_get(&fs->pwd);
        read_unlock(&fs->lock);
    } else {
        struct dentry *dentry;

        file = fget_light(dfd, &fput_needed);
        retval = -EBADF;
        if (!file)
            goto out_fail;

        dentry = file->f_path.dentry;

        retval = -ENOTDIR;
        if (!S_ISDIR(dentry->d_inode->i_mode))
            goto fput_fail;

        retval = file_permission(file, MAY_EXEC);
        if (retval)
            goto fput_fail;

        nd->path = file->f_path;
        path_get(&file->f_path);

        fput_light(file, fput_needed);
    }
...
}

static int __follow_mount(struct path *path)
{
    int res = 0;
    while (d_mountpoint(path->dentry)) {
        struct vfsmount *mounted = lookup_mnt(path->mnt, path->dentry);
        if (!mounted)
            break;
        dput(path->dentry);
        if (res)
            mntput(path->mnt);
        path->mnt = mounted;
        path->dentry = dget(mounted->mnt_root);
        res = 1;
    }
    return res;
}
```

`open_namei`에서는 파일 경로를 찾는 `do_path_lookup`을 호출한다.
`do_path_lookup`에서는 파일의 첫 문자가 "/"면 절대경로로 인식해 주어진 경로를 그대로 사용하고, 디렉토리 파일 디스크립터(dfd)가 `AT_FDCWD`로 설정되어 있으며 상대경로로 인식하고 `pwd`를 이용해 경로를 찾는다.
이때 마운트할 파일이 있으면 찾아서 마운드한다.

#### 3) find empty file{} entry and fill-in relevant information.

`fs/open.c`:

```c
static struct file *do_filp_open(int dfd, const char *filename, int flags,
                 int mode)
{
    int namei_flags, error;
    struct nameidata nd;

    namei_flags = flags;
    if ((namei_flags+1) & O_ACCMODE)
        namei_flags++;

    error = open_namei(dfd, filename, namei_flags, mode, &nd);
    if (!error)
        return nameidata_to_filp(&nd, flags);

    return ERR_PTR(error);
}
```

`do_flip_open`에서 오류가 발생하지 않는다면 `nameidata_to_flip`을 호출한다.

`fs/open.c`:

```c
struct file *nameidata_to_filp(struct nameidata *nd, int flags)
{
    struct file *filp;

    /* Pick up the filp from the open intent */
    filp = nd->intent.open.file;
    /* Has the filesystem initialised the file for us? */
    if (filp->f_path.dentry == NULL)
        filp = __dentry_open(nd->path.dentry, nd->path.mnt, flags, filp,
                     NULL);
    else
        path_put(&nd->path);
    return filp;
}
```

`nameidata_to_flip`는 `__dentry_open`를 호출해 nameidata를 open flip으로 변환해 작성한다.
이때 기존의 내용은 모두 삭제한다.

#### 4) chaining

`fs/open.c`:

```c
static struct file *__dentry_open(struct dentry *dentry, struct vfsmount *mnt,
                    int flags, struct file *f,
                    int (*open)(struct inode *, struct file *))
{
    struct inode *inode;
    int error;

    f->f_flags = flags;
    f->f_mode = ((flags+1) & O_ACCMODE) | FMODE_LSEEK |
                FMODE_PREAD | FMODE_PWRITE;
    inode = dentry->d_inode;
    if (f->f_mode & FMODE_WRITE) {
        error = get_write_access(inode);
        if (error)
            goto cleanup_file;
    }

    f->f_mapping = inode->i_mapping;
    f->f_path.dentry = dentry;
    f->f_path.mnt = mnt;
    f->f_pos = 0;
    f->f_op = fops_get(inode->i_fop);
    file_move(f, &inode->i_sb->s_files);

    error = security_dentry_open(f);
    if (error)
        goto cleanup_all;

    if (!open && f->f_op)
        open = f->f_op->open;
    if (open) {
        error = open(inode, f);
        if (error)
            goto cleanup_all;
    }

    f->f_flags &= ~(O_CREAT | O_EXCL | O_NOCTTY | O_TRUNC);

    file_ra_state_init(&f->f_ra, f->f_mapping->host->i_mapping);

    /* NB: we're sure to have correct a_ops only after f_op->open */
    if (f->f_flags & O_DIRECT) {
        if (!f->f_mapping->a_ops ||
            ((!f->f_mapping->a_ops->direct_IO) &&
            (!f->f_mapping->a_ops->get_xip_page))) {
            fput(f);
            f = ERR_PTR(-EINVAL);
        }
    }

    return f;

cleanup_all:
    fops_put(f->f_op);
    if (f->f_mode & FMODE_WRITE)
        put_write_access(inode);
    file_kill(f);
    f->f_path.dentry = NULL;
    f->f_path.mnt = NULL;
cleanup_file:
    put_filp(f);
    dput(dentry);
    mntput(mnt);
    return ERR_PTR(error);
}
```

`fs/namei.c`:

```c
void path_put(struct path *path)
{
    dput(path->dentry);
    mntput(path->mnt);
}
EXPORT_SYMBOL(path_put);
```

`__dentry_open`과 `path_put` 함수에서 호출하는 `dput`과 `mntput`이 파일디스크립터와 파일을 연결하는 코드이다.

#### 5) return fd

최종적으로 `do_sys_open`에서 파일이 할당된 `fd`를 반환한다.

#### `read(x, buf, n);`

#### 1) go to the inode for x

`fs/read_write.c`:

```c
asmlinkage ssize_t sys_read(unsigned int fd, char __user * buf, size_t count)
{
    struct file *file;
    ssize_t ret = -EBADF;
    int fput_needed;

    file = fget_light(fd, &fput_needed);
    if (file) {
        loff_t pos = file_pos_read(file);
        ret = vfs_read(file, buf, count, &pos);
        file_pos_write(file, pos);
        fput_light(file, fput_needed);
    }

    return ret;
}
```

`read` 함수는 `sys_read`를 호출한다.

`fs/file_table.c`:

```c
struct file *fget_light(unsigned int fd, int *fput_needed)
{
    struct file *file;
    struct files_struct *files = current->files;

    *fput_needed = 0;
    if (likely((atomic_read(&files->count) == 1))) {
        file = fcheck_files(files, fd);
    } else {
        ...
        file = fcheck_files(files, fd);
        ...
    }

    return file;
}
```

`include/linux/file.h`:

```c
static inline struct file * fcheck_files(struct files_struct *files, unsigned int fd)
{
    struct file * file = NULL;
    struct fdtable *fdt = files_fdtable(files);

    if (fd < fdt->max_fds)
        file = rcu_dereference(fdt->fd[fd]);
    return file;
}
```

`fcheck_files`는 파일 디스크립터가 가리키는 참조를 가져온다.

#### 2) read n bytes starting from the current file position

`fs/read_write.c`:

```c
static inline loff_t file_pos_read(struct file *file)
{
    return file->f_pos;
}

ssize_t vfs_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    ...
    ret = rw_verify_area(READ, file, pos, count);
    if (ret >= 0) {
        count = ret;
        if (file->f_op->read)
            ret = file->f_op->read(file, buf, count, pos);
        else
            ret = do_sync_read(file, buf, count, pos);
        if (ret > 0) {
            fsnotify_access(file->f_path.dentry);
            add_rchar(current, ret);
        }
        inc_syscr(current);
    }

    return ret;
}
```

`file_pos_read`는 현재 `f_pos`의 위치를 가져온다.
`vfs_read`는 파일을 읽어 `buf`에 저장하는 함수를 호출하는 함수이다.

#### 3) save the data in `buf`

`file->f_op->read`와 `do_sync_read`를 이용해 파일을 내용을 읽어 `buf`에 저장한다.

`fs/sysfs/bin.c`:

```c
static ssize_t
read(struct file *file, char __user *userbuf, size_t bytes, loff_t *off)
{
    struct bin_buffer *bb = file->private_data;
    struct dentry *dentry = file->f_path.dentry;
    int size = dentry->d_inode->i_size;
    loff_t offs = *off;
    int count = min_t(size_t, bytes, PAGE_SIZE);

    if (size) {
        if (offs > size)
            return 0;
        if (offs + count > size)
            count = size - offs;
    }

    mutex_lock(&bb->mutex);

    count = fill_read(dentry, bb->buffer, offs, count);
    if (count < 0)
        goto out_unlock;

    if (copy_to_user(userbuf, bb->buffer, count)) {
        count = -EFAULT;
        goto out_unlock;
    }

    pr_debug("offs = %lld, *off = %lld, count = %d\n", offs, *off, count);

    *off = offs + count;

 out_unlock:
    mutex_unlock(&bb->mutex);
    return count;
}
```

`file->f_op->read` 함수를 가져온 것으로, 파일을 읽어 `bb->buffer`에 저장하고 이를 인자의 `userbuf`로 복사하는 것을 확인할 수 있다.

#### 4) increase the file position by n

`file->f_op->read`에서 `*off = offs + count;`으로 읽은 크기만큼 `pos`를 증가시킨다.

`fs/read_write.c`:

```c
static inline void file_pos_write(struct file *file, loff_t pos)
{
    file->f_pos = pos;
}
```

최종적으로 `file_pos_write` 함수를 이용해 `f_pos`를 증가된 위치로 수정해 저장한다.
