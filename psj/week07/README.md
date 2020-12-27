# IPC

## Homework

### 1. Try below(`hw01.c1`) and explain the result.

See [HW 1](./hw01/main.c)

```bash
$ gcc -o hw01 hw01.c
$ ./hw01
child sum: 40000000
parent sum: 40000000
```

`fork`로 새로운 프로세스를 만들면 프로세스 간의 변수를 공유하지 않고 새로운 메모리를 할당받는다.
그 결과, Race Condition이 일어나지 않고 두 프로세스 모두 4000만 번의 연산을 수행하고 결과를 출력한다.

### 2. Try below(`hw02.c`) and explain the result.

See [HW 2](./hw02/main.c)

```bash
$ gcc -lpthread -o hw02 hw02.c
$ ./hw02
thread1 sum: 40000000
thread2 sum: 53363030
$ ./hw02
thread1 sum: 40000000
thread2 sum: 52458400
$ ./hw02
thread1 sum: 40000000
thread2 sum: 57090768
```

스레드를 만들면 프로세스 간 일부 변수를 공유하는데, 전역변수인 `sum`이 공유되는 변수 중 하나이다.
그 결과, 그 스레드에서 동시에 한 변수를 읽고 수정하면 Race Condition가 된다.

후위 증감 연산자(`++`)는 변수를 읽고 그 변수에 값을 더해 할당하기 때문에 Atomic한 연산이 아니다.
따라서 스레드 1에서 `sum`을 읽고 값을 더하기 전에 스레드 2에서 증감 연산을 끝마치면 스레드 1에서 값이 더해질 때, 스레드 2 연산 결과가 반영되기 전 상태에서 값을 더하기 때문에 일부 연산 결과가 소실된다.

### 3. Try below(`hw03.c`) and explain the result.

See [HW 3](./hw03/main.c)

```bash
$ gcc -lpthread -o hw03 hw03.c
$ ./hw03
thread1 sum: 63428000
thread2 sum: 80000000
$ ./hw03
thread1 sum: 74916000
thread2 sum: 80000000
$ ./hw03
thread1 sum: 73623000
thread2 sum: 80000000
```

Mutex를 이용해 여러 스레드가 동시에 한 변수에 접근할 수 없게 했다.
그 결과, 최종 계산 값이 유실되지 않고 8000만이 항상 나온다.
thread1의 sum이 8000만이 아닌 이유는 thread2가 끝나기 전에 먼저 종료되고 thread2에서 8000만이 계산되기 때문이다.

### 4. (Deadlock) Try below(`hw04.c`) and explain the result. Modify the code so that it won't have a deadlock.

See [HW 4](./hw04/main.c)

```bash
$ gcc -lpthread -o hw04 hw04.c
$ ./hw04
<deadlock>
$ gcc -lpthread -o hw04_fix hw04_fix.c
$ ./hw04_fix
thread1 sum1: 76982000
thread1 sum2: 80000000
thread2 sum1: 80000000
thread2 sum2: 80000000
$ ./hw04_fix
thread1 sum1: 75048000
thread1 sum2: 77170000
thread2 sum1: 80000000
thread2 sum2: 80000000
$ ./hw04_fix
thread1 sum1: 79360000
thread1 sum2: 80000000
thread2 sum1: 80000000
thread2 sum2: 80000000
```

코드를 그대로 입력하면 Deadlock이 발생해 프로그램이 멈추게 된다.
thread1이 `sum1`을 가지고 thread2가 `sum2`를 가지게 되어 어떤 스레드도 두 변수 모두를 가질 수 없어 영원히 대기하게 된다.

스레드에서 사용할 변수만 Mutex로 가지고 사용이 끝나면 바로 락을 풀어 다른 스레드가 사용할 수 있게 코드를 고치면 Deadlock이 사라진다.

### 5. Find out the ISR2 function for `pthread_mutex_lock()` and trace the code. You can do kernel tracing also in the following site: https://elixir.bootlin.com/linux/latest/ident/. Select the right version(v2.6.25.10) and type the ISR2 name in the search box.

`arch/x86/kernel/syscall_table_32.S`:

```s
...
    .long sys_utime          /* 30 */
    .long my_syscall_number
    .long my_syscall_number_toggle
    .long sys_access
    .long sys_nice
    .long sys_ni_syscall     /* 35 - old ftime syscall holder */
...
```

`fs/read_write.c`:

```c
int enable = 0;

asmlinkage void my_syscall_number(long number)
{
    if (enable == 1) {
        printk("system call number: %ld\n", number);
    }
}

asmlinkage void my_syscall_number_toggle()
{
    if (enable == 1) {
        enable = 0;
    } else {
        enable = 1;
    }
}
```

`arch/x86/kernel/entry_32.S`:

```s
ENTRY(ia32_sysenter_target)
    ...
.previous
    ...
    testw $(_TIF_SYSCALL_EMU|_TIF_SYSCALL_TRACE|_TIF_SECCOMP|_TIF_SYSCALL_AUDIT),TI_flags(%ebp)
    jnz syscall_trace_entry
    cmpl $(nr_syscalls), %eax
    jae syscall_badsys

    pushl %eax
    call my_syscall_number
    popl %eax

    call *sys_call_table(,%eax,4)
    movl %eax,PT_EAX(%esp)
...

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
    call my_syscall_number
    popl %eax

    call *sys_call_table(,%eax,4)
    movl %eax,PT_EAX(%esp)      # store the return value
...
```

이렇게 했지만 `pthread_mutex_lock`의 시스템 콜 호출이 보이지 않았다.
그래서 직접 코드를 찾아보기로 했다.

PThread는 라이브러리 함수이기 때문에 커널 코드에 없다.
PThread 코드를 보기 위해 glibc 코드를 찾아 `pthread_mutex_lock`을 찾아보았다.

`glibc/nptl/pthread_mutex_lock.c`:

```c
# define LLL_MUTEX_LOCK(mutex) \
  lll_lock ((mutex)->__data.__lock, PTHREAD_MUTEX_PSHARED (mutex))

...

int
__pthread_mutex_lock (pthread_mutex_t *mutex)
{
  ...
}
```

파일에는 `__pthread_mutex_lock`가 있는데 이 함수는 `LLL_MUTEX_LOCK`를 호출한다.
`LLL_MUTEX_LOCK`에서는 `lll_lock` 함수를 호출해 실행하므로 이 함수를 따라가보았다.

`glibc/sysdeps/unix/sysv/linux/i386/lowlevellock.h`:

```c
#define lll_lock(futex, private)                                            \
  (void)                                                                    \
    ({ int ignore1, ignore2;                                                \
       if (__builtin_constant_p (private) && (private) == LLL_PRIVATE)      \
     __asm __volatile (__lll_lock_asm_start                                 \
               "jz 18f\n\t"                                                 \
               "1:\tleal %2, %%ecx\n"                                       \
               "2:\tcall __lll_lock_wait_private\n"                         \
               "18:"                                                        \
               : "=a" (ignore1), "=c" (ignore2), "=m" (futex)               \
               : "0" (0), "1" (1), "m" (futex),                             \
                 "i" (MULTIPLE_THREADS_OFFSET)                              \
               : "memory");                                                 \
       else                                                                 \
     {                                                                      \
       int ignore3;                                                         \
       __asm __volatile (__lll_lock_asm_start                               \
                 "jz 18f\n\t"                                               \
                 "1:\tleal %2, %%edx\n"                                     \
                 "0:\tmovl %8, %%ecx\n"                                     \
                 "2:\tcall __lll_lock_wait\n"                               \
                 "18:"                                                      \
                 : "=a" (ignore1), "=c" (ignore2),                          \
                   "=m" (futex), "=&d" (ignore3)                            \
                 : "1" (1), "m" (futex),                                    \
                   "i" (MULTIPLE_THREADS_OFFSET), "0" (0),                  \
                   "g" ((int) (private))                                    \
                 : "memory");                                               \
     }                                                                      \
    })
```

`lll_lock`의 `"2:\tcall __lll_lock_wait\n"`를 보면 `__lll_lock_wait`를 호출하는 매크로 함수인 것을 알 수 있다.

`glibc/sysdeps/unix/sysv/linux/i386/lowlevellock.S`:

```s
__lll_lock_wait:
    cfi_startproc
    pushl    %edx
    cfi_adjust_cfa_offset(4)
    pushl    %ebx
    cfi_adjust_cfa_offset(4)
    pushl    %esi
    cfi_adjust_cfa_offset(4)
    cfi_offset(%edx, -8)
    cfi_offset(%ebx, -12)
    cfi_offset(%esi, -16)

    movl    %edx, %ebx
    movl    $2, %edx
    xorl    %esi, %esi    /* No timeout.  */
    LOAD_FUTEX_WAIT (%ecx)

    cmpl    %edx, %eax    /* NB:     %edx == 2 */
    jne 2f

1:  movl    $SYS_futex, %eax
    ENTER_KERNEL

2:  movl    %edx, %eax
    xchgl    %eax, (%ebx)    /* NB:     lock is implied */

    testl    %eax, %eax
    jnz    1b

    popl    %esi
    cfi_adjust_cfa_offset(-4)
    cfi_restore(%esi)
    popl    %ebx
    cfi_adjust_cfa_offset(-4)
    cfi_restore(%ebx)
    popl    %edx
    cfi_adjust_cfa_offset(-4)
    cfi_restore(%edx)
    ret
    cfi_endproc
    .size    __lll_lock_wait,.-__lll_lock_wait

    /*  %ecx: futex
        %esi: flags
        %edx: timeout
        %eax: futex value
    */
    .globl    __lll_timedlock_wait
    .type    __lll_timedlock_wait,@function
    .hidden    __lll_timedlock_wait
    .align    16
```

`1: movl    $SYS_futex, %eax`를 보면 `SYS_futex`를 호출한다.
커널 코드로 가서 `SYS_futex`에 대해 찾아보자.

`arch/x86/kernel/syscall_table_32.S`:

```s
.long sys_lremovexattr
.long sys_fremovexattr
.long sys_tkill
.long sys_sendfile64
.long sys_futex     /* 240 */
.long sys_sched_setaffinity
.long sys_sched_getaffinity
.long sys_set_thread_area
.long sys_get_thread_area
```

`sys_futex`은 시스템 콜 240번이다.

`kernel/futex.c`:

```c
asmlinkage long sys_futex(u32 __user *uaddr, int op, u32 val,
              struct timespec __user *utime, u32 __user *uaddr2,
              u32 val3)
{
    struct timespec ts;
    ktime_t t, *tp = NULL;
    u32 val2 = 0;
    int cmd = op & FUTEX_CMD_MASK;

    if (utime && (cmd == FUTEX_WAIT || cmd == FUTEX_LOCK_PI ||
              cmd == FUTEX_WAIT_BITSET)) {
        if (copy_from_user(&ts, utime, sizeof(ts)) != 0)
            return -EFAULT;
        if (!timespec_valid(&ts))
            return -EINVAL;

        t = timespec_to_ktime(ts);
        if (cmd == FUTEX_WAIT)
            t = ktime_add_safe(ktime_get(), t);
        tp = &t;
    }
    /*
     * requeue parameter in 'utime' if cmd == FUTEX_REQUEUE.
     * number of waiters to wake in 'utime' if cmd == FUTEX_WAKE_OP.
     */
    if (cmd == FUTEX_REQUEUE || cmd == FUTEX_CMP_REQUEUE ||
        cmd == FUTEX_WAKE_OP)
        val2 = (u32) (unsigned long) utime;

    return do_futex(uaddr, op, val, tp, uaddr2, val2, val3);
}
```

커널의 `sys_futex` 함수는 `do_futex`를 호출한다.
요약하자면, `pthread_mutex_lock` -> `__pthread_mutex_lock` -> `LLL_MUTEX_LOCK` -> `lll_lock` -> `__lll_lock_wait` -> System Call `sys_futex` -> `do_futex`이다.
