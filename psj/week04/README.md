# Interrupt 2

## Homework

### 7. `sys_call_table[]` is in `arch/x86/kernel/syscall_table_32.S`. How many system calls does Linux 2.6 support? What are the system call numbers for exit, fork, execve, wait4, read, write, and mkdir? Find system call numbers for `sys_ni_syscall`, which is defined at `kernel/sys_ni.c`. What is the role of sys_ni_syscall?

`sys_call_table`은 0부터 326번까지 있으므로 총 327개의 시스템 콜을 가지고 있다.

| function | system call | system call number |
| -------- | ----------- | ------------------ |
| exit     | sys_exit    | 1                  |
| fork     | sys_fork    | 2                  |
| execve   | sys_execve  | 11                 |
| wait4    | sys_wait4   | 114                |
| read     | sys_read    | 3                  |
| write    | sys_write   | 4                  |
| mkdir    | sys_mkdir   | 39                 |

`sys_ni_syscall`의 시스템 콜 번호로 17, 31, 32 등 여러 개가 있는데 `kernel/sys_ni.c`로 가서 파일을 열어보면 아래와 같다.

```c
asmlinkage long sys_ni_syscall(void)
{
    return -ENOSYS;
}
```

`sys_ni_syscall`는 구현되지 않은 시스템 콜을 가리키는 함수이며 `-ENOSYS`을 반환한다.
`ENOSYS`는 구현되지 않은 함수를 사용할 때 발생하는 오류 코드이다.

### 8. Change the kernel such that it prints "length 17 string found" for each `printf(s)` when the length of `s` is 17. Run a program that contains a `printf` statement to see the effect.

`printf(s)`는 내부적으로 `write(1, s, strlen(s))`를 호출한다.
따라서 `printf`를 호출할 때 같이 실행되는 코드를 삽입하기 위해서는 `write` 함수가 호출되는 시점을 알아야 한다.

`fs/read_write.c`:

```c
asmlinkage ssize_t sys_write(unsigned int fd, const char __user * buf, size_t count)
{
    struct file *file;
    ssize_t ret = -EBADF;
    int fput_needed;

    if (count == 17) {
        printk("length 17 string found\n");
    }

    file = fget_light(fd, &fput_needed);
    if (file) {
        loff_t pos = file_pos_read(file);
        ret = vfs_write(file, buf, count, &pos);
        file_pos_write(file, pos);
        fput_light(file, fput_needed);
    }

    return ret;
}
```

`write` 함수는 `sys_write`를 호출하므로 `count`가 17인지 확인하는 코드를 삽입하면 된다.

커널을 컴파일하고 재부팅한다.

프로그램이 원하는대로 수정되었는지 확인해보자.

`hw08.c`:

```c
#include <stdio.h>

int main(void) {
    printf("hello, world\n");
    printf("0123456789012345\n");
    return 0;
}
```

```bash
$ gcc -o hw08 hw08.c
$ echo 8 > /proc/sys/kernel/printk
$ ./hw08
hello, world
length 17 string found
0123456789012345
$ echo 1 > /proc/sys/kernel/printk
```

로그 레벨이 낮아 `printk` 출력이 보이지 않으므로 8로 수정했다.

임의의 17글자 문자열을 `printf`로 출력하게 한다.
"hello, world"는 17자가 아니어서 `printk`가 호출되지 않지만, "01234..."는 마지막의 개행문자(`\n`)까지 총 17자 이므로 "length 17 string found"가 출력되었다.

### 9. You can call a system call indirectly with `syscall`.

```c
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
    syscall(SYS_write, STDOUT_FILENO, "SYS_write is 4\n", 15);
    return 0;
}
```

`syscall`을 이용하면 시스템 콜 함수를 직접 호출할 수 있다.
`SYS_write`는 `sys/syscall.h` 헤더에 정의된 상수이다.
`write`는 `write(fd, str, length)`를 인자로 가지므로, `syscall`에도 똑같이 넣어야 한다.

```bash
$ gcc -o hw09 ./hw09.c
$ ./hw09
SYS_write is 4
```

`write`와 동일한 결과가 나온다.

### 10. Create a new system call, `my_sys_call` with system call number 17(system call number 17 is one that is not being used currently). Define `my_sys_call()` just before `sys_write()` in `read_write.c`. Write a program that uses this system call:

`arch/x86/kernel/syscall_table_32.S`:

```s
...
    .long sys_unlink     /* 10 */
    .long sys_execve
    .long sys_chdir
    .long sys_time
    .long sys_mknod
    .long sys_chmod      /* 15 */
    .long sys_lchown16
    .long my_sys_call    /* my_sys_call */
    .long sys_stat
    .long sys_lseek
    .long sys_getpid     /* 20 */
...
```

17번의 `sys_ni_syscall`을 `my_sys_call`로 바꿔준다.

`fs/read_write.c`:

```c
asmlinkage void my_sys_call(void)
{
    printk("hello from my_sys_call\n");
}

asmlinkage ssize_t sys_write(unsigned int fd, const char __user * buf, size_t count)
{
...
```

`my_sys_call`에 대응하는 함수를 만들어준다.
`asmlinkage`는 어셈블리 코드에서 이 함수를 직접 호출할 수 있다는 의미이다.
입력과 출력이 없으니 모두 `void`로 선언한다.

```c
#include <unistd.h>

int main(void) {
    syscall(17);
    return 0;
}
```

17번 `my_sys_call`을 `syscall`로 직접 호출한다.

```bash
$ gcc -o hw10 ./hw10.c
$ echo 8 > /proc/sys/kernel/printk
$ ./hw10
hello from my_sys_call
$ echo 1 > /proc/sys/kernel/printk
```

`printk`로 출력하기 때문에 로그레벌을 바꿔줘야 메세지가 보안다.
입력해둔 문자열이 잘 출력되는 것을 확인할 수 있다.

### 10-1. Create another system call that will add two numbers given by the user.

`arch/x86/kernel/syscall_table_32.S`:

```s
...
    .long sys_utime         /* 30 */
    .long my_sys_call_sum   /* old stty syscall holder */
    .long sys_ni_syscall    /* old gtty syscall holder */
    .long sys_access
    .long sys_nice
    .long sys_ni_syscall    /* 35 - old ftime syscall holder */
...
```

31번 `sys_ni_syscall`을 `my_sys_call_sum`로 바꿔준다.

`fs/read_write.c`:

```c
asmlinkage int my_sys_call_sum(int a, int b)
{
    return a + b;
}

asmlinkage ssize_t sys_write(unsigned int fd, const char __user * buf, size_t count)
{
...
```

`my_sys_call_sum`에 대응하는 함수를 만들어 준다.
2개의 입력을 가지고 입력과 출력이 모두 정수이므로 `int`로 선언한다.

```c
#include <stdio.h>
#include <unistd.h>

int main(void) {
    int sum = syscall(31, 4, 9);
    printf("sum is %d\n", sum);
    return 0;
}
```

`syscall`로 31번 `my_sys_call_sum`를 호출한다.
`syscall`은 가변인자 함수로, 두 번째 인자부터는 `my_sys_call_sum` 함수에 넘어가는 인자를 나타낸다.

```bash
$ gcc -o hw101 ./hw101.c
$ ./hw101
sum is 13
```

4+9인 13이 츨력되었다.

### 11. Modify the kernel such that it displays the system call number for all system calls. Run a simple program that displays "hello" in the screen and find out what system calls have been called.

`arch/x86/kernel/syscall_table_32.S`:

```s
...
    .long sys_utime          /* 30 */
    .long my_sys_call_number /* old stty syscall holder */
    .long sys_ni_syscall     /* old gtty syscall holder */
    .long sys_access
    .long sys_nice
    .long sys_ni_syscall     /* 35 - old ftime syscall holder */
...
```

31번 `sys_ni_syscall`을 `my_sys_call_number`로 바꿔준다.

`fs/read_write.c`:

```c
asmlinkage void my_sys_call_number(long number)
{
    printk("system call number: %ld\n", number);
}

asmlinkage ssize_t sys_write(unsigned int fd, const char __user * buf, size_t count)
{
...
```

`my_sys_call_number`에 대응하는 함수를 만들어 준다.
`my_sys_call_number`는 시스템 콜 번호를 입력으로 받는데 이는 `long` 타입이다.
`printk`로 받은 입력 값 번호를 출력한다.

`arch/x86/kernel/entry_32.S`:

```s
ENTRY(system_call)
    RING0_INT_FRAME             # can't unwind into user space anyway
    pushl %eax                  # save orig_eax
    CFI_ADJUST_CFA_OFFSET 4
    SAVE_ALL
    GET_THREAD_INFO(%ebp)
                                # system call tracing in operation / emulation
    /* Note, _TIF_SECCOMP is bit number 8, and so it needs testw and not testb */
    testw $(_TIF_SYSCALL_EMU|_TIF_SYSCALL_TRACE|_TIF_SECCOMP|_TIF_SYSCALL_AUDIT),TI_flags(%ebp)
    jnz syscall_trace_entry
    cmpl $(nr_syscalls), %eax
    jae syscall_badsys
syscall_call:
    pushl %eax
    call my_sys_call_number
    popl %eax
    call *sys_call_table(,%eax,4)
    movl %eax,PT_EAX(%esp)      # store the return value
...
```

`syscall_call` 아래에 `my_sys_call_number`를 호출하는 어셈블리 코드르 삽입한다.
함수에 첫 번째 인자로 시스템 콜 번호를 전달하기 위해 호출 전 `pushl %eax`를 한다.
함수 호출이 끝나면 `popl %eax`으로 레지스터 상태를 되돌려 놓는다.

```c
#include <stdio.h>

int main(void) {
    printf("hello\n");
    return 0;
}
```

"hello"를 출력하는 코드를 작성한다.

```bash
$ gcc -o hw11 ./hw11.c
$ echo 8 > /proc/sys/kernel/printk
$ ./hw11
system call number: 45
system call number: 33
system call number: 5
system call number: 197
system call number: 192
system call number: 6
system call number: 5
system call number: 3
system call number: 197
system call number: 192
system call number: 192
system call number: 192
system call number: 192
system call number: 6
system call number: 192
system call number: 243
system call number: 125
system call number: 125
system call number: 125
system call number: 91
hello
system call number: 119
$ echo 1 > /proc/sys/kernel/printk
```

위와 같이 시스템 함수들이 호출되는 것을 볼 수 있다.

### 12. What system calls are being called when you remove a file? Use `system()` function to run a Linux command as below. Explain what each system call is doing. You need to make `f1` file before you run it.

```c
#include <stdlib.h>

int main(void) {
    system("rm f1");
    return 0;
}
```

`system`은 리눅스 명령어를 직접 실행하는 함수이다.
`rm f1`는 `f1`이란 이름을 가진 파일을 삭제하는 명령이다.

```bash
$ touch f1
$ gcc -o hw12 ./hw12.c
$ echo 8 > /proc/sys/kernel/printk
$ ./hw12
```

`touch`는 빈 파일을 만들기 위해 사용했다.
사용된 시스템 콜에 번호와 대응되는 이름을 붙여 나열하면 아래와 같다.

| number | name                |
| ------ | ------------------- |
| 45     | sys_brk             |
| 33     | sys_access          |
| 5      | sys_open            |
| 197    | sys_fstat64         |
| 192    | sys_mmap2           |
| 6      | sys_close           |
| 5      | sys_open            |
| 197    | sys_fstat64         |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 6      | sys_close           |
| 192    | sys_mmap2           |
| 243    | sys_set_thread_area |
| 125    | sys_mprotect        |
| 125    | sys_mprotect        |
| 125    | sys_mprotect        |
| 91     | sys_munmap          |
| 45     | sys_brk             |
| 33     | sys_access          |
| 5      | sys_open            |
| 197    | sys_fstat64         |
| 192    | sys_mmap2           |
| 6      | sys_close           |
| 5      | sys_open            |
| 4      | sys_write           |
| 197    | sys_fstat64         |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 6      | sys_close           |
| 5      | sys_open            |
| 3      | sys_read            |
| 197    | sys_fstat64         |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 6      | sys_close           |
| 5      | sys_open            |
| 3      | sys_read            |
| 197    | sys_fstat64         |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 6      | sys_close           |
| 192    | sys_mmap2           |
| 243    | sys_set_thread_area |
| 125    | sys_mprotect        |
| 125    | sys_mprotect        |
| 125    | sys_mprotect        |
| 125    | sys_mprotect        |
| 125    | sys_mprotect        |
| 91     | sys_munmap          |
| 45     | sys_brk             |
| 33     | sys_access          |
| 5      | sys_open            |
| 197    | sys_fstat64         |
| 192    | sys_mmap2           |
| 6      | sys_close           |
| 5      | sys_open            |
| 3      | sys_read            |
| 197    | sys_fstat64         |
| 192    | sys_mmap2           |
| 6      | sys_close           |
| 5      | sys_open            |
| 3      | sys_read            |
| 197    | sys_fstat64         |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 192    | sys_mmap2           |
| 6      | sys_close           |
| 192    | sys_mmap2           |
| 243    | sys_set_thread_area |
| 125    | sys_mprotect        |
| 125    | sys_mprotect        |
| 125    | sys_mprotect        |
| 91     | sys_munmap          |
| 119    | sys_sigreturn       |

- `sys_brk`는 힙(데이터) 영역을 확장하거나 축소하는 함수이다.
- `sys_fstat64`는 파일 정보를 읽는 함수이다.
- `sys_mmap2`는 파일이나 장치를 메모리에 대응시키는 함수이다.
- `sys_set_thread_area`는 thread-local storage를 조작하는 함수이다.
- `sys_mprotect`는 메모리 영역에 대한 접근을 제어하는 함수이다.
- `sys_munmap`은 mmap으로 만들어진 매핑을 제거하는 함수이다.
- `sys_sigreturn`는 시그널 핸들러에서 값을 반환하고 스택 프레임을 정리하는 함수이다.

System Call 흐름을 보면 `rm`파일이 있는지, 파일에 대한 정보를 확인하고 파일을 실행한다.
`rm`에서는 `f1`에 대한 정보를 확인하고 파일을 삭제한다.

`sys_mmap2`과 `sys_munmap` 기억장치에 저장되어 있는 파일에 접근하기 위해 사용된 것으로 보이며,
여러 프로그램이 동시에 한 파일에 접근해서 발생하는 문제를 막기 위해 `sys_set_thread_area`와 `sys_mprotect`으로 락을 걸어 하나의 쓰레드만 파일에 접근할 수 있게한 것으로 보인다.

### 13. Find `rm.c` in `busybox-1.31.1` and show the code that actually removes `f1`. Note all linux commands are actually a program, and running `rm` command means running `rm.c` program. `rm` needs a system call defined in `uClibc-0.9.33.2` to remove a file. You may want to continue the code tracing all the way up to `INT 0x80` in uClibc for this system call.

`busybox/coreutils/rm.c`:

```c
int rm_main(int argc UNUSED_PARAM, char **argv)
{
    int status = 0;
    int flags = 0;
    unsigned opt;

    opt = getopt32(argv, "^" "fiRrv" "\0" "f-i:i-f");
    argv += optind;
    if (opt & 1)
        flags |= FILEUTILS_FORCE;
    if (opt & 2)
        flags |= FILEUTILS_INTERACTIVE;
    if (opt & (8|4))
        flags |= FILEUTILS_RECUR;
    if ((opt & 16) && FILEUTILS_VERBOSE)
        flags |= FILEUTILS_VERBOSE;

    if (*argv != NULL) {
        do {
            const char *base = bb_get_last_path_component_strip(*argv);

            if (DOT_OR_DOTDOT(base)) {
                bb_error_msg("can't remove '.' or '..'");
            } else if (remove_file(*argv, flags) >= 0) {
                continue;
            }
            status = 1;
        } while (*++argv);
    } else if (!(flags & FILEUTILS_FORCE)) {
        bb_show_usage();
    }

    return status;
}
```

`rm` 프로그램 코드는 위와 같다.
각종 플래그를 확인하고, `remove_file` 함수를 호출해 파일을 삭제한다.

`busybox/libbb/remove_file.c`:

```c
int FAST_FUNC remove_file(const char *path, int flags)
{
    ...
    if (S_ISDIR(path_stat.st_mode)) {
        ...

        while ((d = readdir(dp)) != NULL) {
            char *new_path;

            new_path = concat_subpath_file(path, d->d_name);
            if (new_path == NULL)
                continue;
            if (remove_file(new_path, flags) < 0)
                status = -1;
            free(new_path);
        }

        ...
        if (status == 0 && rmdir(path) < 0) {
            bb_perror_msg("can't remove '%s'", path);
            return -1;
        }
        ...
        return status;
    }
    ...
    return 0;
}
```

파일 삭제와 직접적으로 관련 없는 플래그 확인 등의 코드는 생략했다.
함수가 실행되면 입력한 경로가 폴더인지 확인하고 폴더라면 재귀적으로 함수를 실행한다.
경로가 파일이라면 `rmdir`을 호출해 파일을 삭제한다.

`uClibc/libc/sysdeps/linux/common/rmdir.c`:

```c
_syscall1(int, rmdir, const char *, pathname)
libc_hidden_def(rmdir)
```

rmdir의 실제 코드는 uClibc에 있다.
`_syscall1`를 따라가보자.

`uClibc/libc/sysdeps/linux/common/bits/syscalls-common.h`:

```c
#define _syscall1(args...)  SYSCALL_FUNC(1, args)

#define SYSCALL_FUNC(nargs, type, name, args...)                    \
type name(C_DECL_ARGS_##nargs(args)) {                              \
    return (type)INLINE_SYSCALL(name, nargs, C_ARGS_##nargs(args)); \
}

#define INLINE_SYSCALL(name, nr, args...) INLINE_SYSCALL_NCS(__NR_##name, nr, args)

#define INLINE_SYSCALL_NCS(name, nr, args...)                       \
(__extension__                                                      \
 ({                                                                 \
    INTERNAL_SYSCALL_DECL(__err);                                   \
    (__extension__                                                  \
     ({                                                             \
       long __res = INTERNAL_SYSCALL_NCS(name, __err, nr, args);    \
       if (unlikely(INTERNAL_SYSCALL_ERROR_P(__res, __err))) {      \
        __set_errno(INTERNAL_SYSCALL_ERRNO(__res, __err));          \
        __res = -1L;                                                \
       }                                                            \
       __res;                                                       \
      })                                                            \
    );                                                              \
  })                                                                \
)
```

위 함수는 C 매크로 함수이기 때문에 실제 함수가 아니지만 호출한다고 설명하겠다.
`_syscall1`는 `SYSCALL_FUNC`를 호출한다.
`SYSCALL_FUNC`는 `INLINE_SYSCALL`를 호출하고, `INLINE_SYSCALL`는 `INLINE_SYSCALL_NCS`를 호출한다.
마지막으로 `INLINE_SYSCALL_NCS`에서는 `INTERNAL_SYSCALL_NCS`를 호출한다.

`uClibc/libc/sysdeps/linux/i386/bits/syscalls.h`:

```c
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)    \
(__extension__                                          \
 ({                                                     \
    register unsigned int resultvar;                    \
    __asm__ __volatile__ (                              \
        LOADARGS_##nr                                   \
        "movl   %1, %%eax\n\t"                          \
        "int    $0x80\n\t"                              \
        RESTOREARGS_##nr                                \
        : "=a" (resultvar)                              \
        : "g" (name) ASMFMT_##nr(args) : "memory", "cc" \
    );                                                  \
    (int) resultvar;                                    \
  })                                                    \
)
```

코드를 보면 알 수 있듯, `INTERNAL_SYSCALL_NCS`는 `int $0x80`으로 rmdir의 인터럽트를 생성하는 어셈블리 코드를 생성한다.
이때, x80은 10진수 128로 System Call의 인터럽트 번호이다.
