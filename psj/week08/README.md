# Midterm Exam

## Practice

For each problem, you should provide the source code capture, output result, and brief explanation about your answer.

### 1. Write a system call that displays all processes whose parent has `pid=1`.

`init/main.c`:

```c
...
static char *ramdisk_execute_command;

void display_processes(void)
{
    struct task_struct *task;
    task = &init_task;
    for (;;) {
        if (task->parent->pid == 1) {
            printk("ID: %d, Name: %s, State: %ld\n", task->pid, task->comm, task->state);
        }
        task = next_task(task);
        if (task == &init_task) {
            break;
        }
    }
    printk("\n");
}

#ifdef CONFIG_SMP
...
```

위와 같이 `init/main.c`에 코드를 추가했다.
`task->parent`의 `pid`가 1인지 확인하는 조건문을 추가해 PPID가 1인 프로세스만 출력할 수 있게 했다.

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
    .long display_processes
    .long sys_stat
    .long sys_lseek
    .long sys_getpid     /* 20 */
...
```

17번으로 `display_processes` 함수를 시스템콜로 등록해주었다.

`sol01.c`:

```c
#include <unistd.h>

int main(void) {
    syscall(17);
    return 0;
}
```

`syscall` 함수를 이용해 17번 시스템 콜을 호출한다.

### 2. Modify the kernel so that it displays all processes whose parent has `pid=1` whenever you type 'r'.

`drivers/input/keyboard/atkbd.c`:

```c
extern void display_processes(void);

static irqreturn_t atkbd_interrupt(struct serio *serio, unsigned char data,
            unsigned int flags)
{
    struct atkbd *atkbd = serio_get_drvdata(serio);
    struct input_dev *dev = atkbd->dev;
    unsigned int code = data;

    if (code == 19) {
        display_processes();
    }

    int scroll = 0, hscroll = 0, click = -1;
    int value;
    unsigned char keycode;
...
```

`atkbd_interrupt`의 `code` 변수가 19일 때는 r 키가 눌린 시점이다.
이때 1번에서 만들어둔 `display_processes` 함수를 호출한다.

로그 레벨을 수정하고 r키를 누르면 PPID가 1인 프로세스 목록이 보인다.

### 3. `sleep(1)` will make the program blocked for 1 seconds.

#### 1) Find out the ISR2 for `sleep()` by surrounding `sleep(1)` with system calls that start and stop displaying system call numbers.

`arch/x86/kernel/syscall_table_32.S`:

```s
...
    .long sys_utime         /* 30 */
    .long my_syscall_number
    .long my_syscall_number_toggle
    .long sys_access
    .long sys_nice
    .long sys_ni_syscall    /* 35 - old ftime syscall holder */
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

현재 사용하는 시스템 콜을 보여주는 함수와 위 함수 동작의 활성화를 제어하는 함수 2개를 추가했다.

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

`ia32_sysenter_target`와 `system_call`에 각각 `my_syscall_number`를 호출하는 코드를 추가한다.

`sol03_01.c`:

```c
#include <unistd.h>

int main(void) {
    syscall(32);
    sleep(1);
    syscall(32);
    return 0;
}
```

`sleep` 전후로 32번 시스템 콜인 `my_syscall_number_toggle`을 호출해서 사용되는 시스템 콜 번호를 출력한다.
로그레벨을 바꾸면 출력 결과를 볼 수 있다.

출력 결과가 보이고 그중 시스템콜 162 `sys_nanosleep`이 사용되는 것을 알 수 있다.

#### 2) Trace this ISR2 to find the exact code line that blocks the process that called `sleep()`. You should show all function calls that lead to this exact code line starting from the ISR2 function.

리눅스에서 `sleep` 함수는 `nanosleep` 함수를 통해 구현된다.

`kernel/hrtimer.c`:

```c
asmlinkage long
sys_nanosleep(struct timespec __user *rqtp, struct timespec __user *rmtp)
{
    struct timespec tu;

    if (copy_from_user(&tu, rqtp, sizeof(tu)))
        return -EFAULT;

    if (!timespec_valid(&tu))
        return -EINVAL;

    return hrtimer_nanosleep(&tu, rmtp, HRTIMER_MODE_REL, CLOCK_MONOTONIC);
}
```

`sleep`의 ISR2는 `nanosleep`으로 `kernel/hrtimer.c` 파일의 `sys_nanosleep`을 호출한다.
이 함수는 내부에서 `hrtimer_nanosleep` 함수를 호출한다.

`kernel/hrtimer.c`:

```c
long hrtimer_nanosleep(struct timespec *rqtp, struct timespec __user *rmtp,
               const enum hrtimer_mode mode, const clockid_t clockid)
{
    struct restart_block *restart;
    struct hrtimer_sleeper t;

    hrtimer_init(&t.timer, clockid, mode);
    t.timer.expires = timespec_to_ktime(*rqtp);
    if (do_nanosleep(&t, mode))
        return 0;

    /* Absolute timers do not update the rmtp value and restart: */
    if (mode == HRTIMER_MODE_ABS)
        return -ERESTARTNOHAND;

    if (rmtp) {
        int ret = update_rmtp(&t.timer, rmtp);
        if (ret <= 0)
            return ret;
    }

    restart = &current_thread_info()->restart_block;
    restart->fn = hrtimer_nanosleep_restart;
    restart->arg0 = (unsigned long) t.timer.base->index;
    restart->arg1 = (unsigned long) rmtp;
    restart->arg2 = t.timer.expires.tv64 & 0xFFFFFFFF;
    restart->arg3 = t.timer.expires.tv64 >> 32;

    return -ERESTART_RESTARTBLOCK;
}
```

이 함수에서는 `hrtimer_init`로 타이머를 초기화하고 `do_nanosleep`을 호출한다.
이 함수가 중간에 멈추지 않고 끝까지 실행되면 `-ERESTART_RESTARTBLOCK`를 반환한다.
오류이기 때문에 음수로 값이 나오고 `sys_restart_syscall`를 호출해서 재시작을 명령하는 오류이다.
`include/linux/errno.h`에 정의되어 있다.

`kernel/hrtimer.c`:

```c
static int __sched do_nanosleep(struct hrtimer_sleeper *t, enum hrtimer_mode mode)
{
    hrtimer_init_sleeper(t, current);

    do {
        set_current_state(TASK_INTERRUPTIBLE);
        hrtimer_start(&t->timer, t->timer.expires, mode);
        if (!hrtimer_active(&t->timer))
            t->task = NULL;

        if (likely(t->task))
            schedule();

        hrtimer_cancel(&t->timer);
        mode = HRTIMER_MODE_ABS;

    } while (t->task && !signal_pending(current));

    __set_current_state(TASK_RUNNING);

    return t->task == NULL;
}
```

`do_nanosleep`에서는 `hrtimer_init_sleeper`로 sleeper를 초기화하고 do-while 반복문에 진입한다.
`hrtimer_start`는 현재 CPU의 타이머를 재시작하는 함수이다.
반복문을 돌면서 현재 프로세스의 상태를 Interruptible로 바꾸고 태스크를 스케줄링한다.
반복문이 끝나면 현재 프로세스의 상태를 Running으로 바꾼다.

`include/linux/sched.h`:

```c
#define __set_current_state(state_value)            \
    do { current->state = (state_value); } while (0)
```

`__set_current_state`는 이렇게 생긴 매크로 함수이다.
