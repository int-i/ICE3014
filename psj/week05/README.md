# Process 1

## Homework

### 1. `task_struct` is defined in `include/linux/sched.h` (search for "task_struct {"). Which fields of the `task_struct` contain information for process id, parent process id, user id, process status, the memory location of the process, the files opened, the priority of the process, program name?

- Process ID: `pid_t pid`
- Parent process ID: `pid_t parent->pid`
- User ID: `uid_t uid`
- Process status: `volatile long state`; -1 unrunnable, 0 runnable, >0 stopped
- Memory location of the process: `struct mm_struct *mm`
- Files opened: `struct files_struct *files`
- Priority of the process: `int prio`
- Program name: `char comm[TASK_COMM_LEN]`

### 2. Display all processes with `ps –ef`. Find the pid of `ps -ef`, the process you have just executed. Find the pid and program name of the parent process of it, then the parent of this parent, and so on, until you see the `init_task` whose process ID is 0.

`ps -ef`의 PID는 4568이다.
`ps -ef`의 PPID가 4447이므로 부모 프로세스는 `-bash`이다.

`-bash`의 부모 프로세스는 PID가 4424인 `/bin/login --`이고 이 프로세스의 부모 프로세는 PID가 1인 `init`이다.
`init`의 PPID는 0으로 부모 프로세스는 `init_task`이다.

### 3. Define `display_processes()` in `init/main.c`(right before the first function definition). Call this function in the beginning of `start_kernel()`. Confirm that there is only one process in the beginning. Find the location where the number of processes becomes 2. Find the location where the number of processes is the greatest. Use `dmesg` to see the result of `display_processes()`.

`init/main.c`:

```c
...
static char *ramdisk_execute_command;

void display_processes(void)
{
    struct task_struct *task;
    task = &init_task;
    for (;;) {
        printk("ID: %d, Name: %s, State: %ld\n", task->pid, task->comm, task->state);
        task = next_task(task);
        if (task == &init_task) {
            break;
        }
    }
    printk("\n");
}

#ifdef CONFIG_SMP

...


asmlinkage void __init start_kernel(void)
{
    char * command_line;
    extern struct kernel_param __start___param[], __stop___param[];

    display_processes();

    smp_setup_processor_id();
...
```

`start_kernel` 함수가 호출되면 `display_processes`을 함수를 가장 먼저 호출할 수 있게 했다.

```text
ID: 0, Name: swapper, State: 0
```

실행 결과 하나의 프로세스만 있었고, swapper 프로세스가 ID=0으로 생성된 것을 볼 수 있었다.

실행 중인 프로세스가 2개가 된 첫 번째 시점은 `kernel_init`의 최상단에서 `display_processes`를 호출했을 때이다.

가장 많은 프로세스가 보여지는 곳은 `kernel_init`에서 `init_post`가 호출된 이후이다.

### 4. Make a system call that, when called, displays all processes in the system. Run an application program that calls this system call and see if this program displays all processes in the system.

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
    .long my_sys_display_process
    .long sys_stat
    .long sys_lseek
    .long sys_getpid     /* 20 */
...
```

17번의 `sys_ni_syscall`을 `my_sys_display_process`로 바꿔준다.

`init/main.c`:

```c
asmlinkage void my_sys_display_process(void)
{
    struct task_struct *task;
    task = &init_task;
    for (;;) {
        printk("ID: %d, Name: %s, State: %ld\n", task->pid, task->comm, task->state);
        task = next_task(task);
        if (task == &init_task) {
            break;
        }
    }
    printk("\n");
}
```

`my_sys_call_sum`에 대응하는 함수를 만들어 준다.

```c
#include <unistd.h>

int main(void) {
    syscall(17);
    return 0;
}
```

위에서 만든 시스템 콜 함수를 호출하는 프로그램을 만든다.

로그 레벨을 바꾸고 프로그램을 실행해 시스템 콜 함수를 호출하면 프로세스 목록이 보여진다.

### 5. Run three user programs, `f1`, `f2`, and `f3`, and run another program that calls the above system call as follows. State 0 means runnable and 1 means blocked. Observe the state changes in `f1`, `f2`, `f3` and explain what these changes mean.

`f1.c`:

```c
#include <unistd.h>

int main(void) {
    int i, j;
    double x = 1.2;
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 1000000; j++) {
            x = x * x;
        }
        usleep(1000000);
    }
    return 0;
}
```

`f2.c`:

```c
#include <unistd.h>

int main(void) {
    int i, j;
    double x = 1.2;
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 1000000; j++) {
            x = x * x;
        }
        usleep(2000000);
    }
    return 0;
}
```

`f3.c`:

```c
#include <unistd.h>

int main(void) {
    int i, j;
    double x = 1.2;
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 1000000; j++) {
            x = x * x;
        }
        usleep(3000000);
    }
    return 0;
}
```

`ex01.c`:

```c
#include <unistd.h>

int main(void) {
    int i;
    for (i = 0; i < 100; i++) {
        sleep(5);
        syscall(17);
    }
    return 0;
}
```

```bash
$ ./f1 &
$ ./f2 &
$ ./f3 &
$ ./ex01
```

작성한 프로그램을 모두 실행한다.

출력 로그의 대부분은 `f1`만 0(Runnable)이고 `f2`와 `f3`은 1(Block)인 상태이다.

그리고 가끔씩 `f1`과 `f2` 모두 1인 상황이 발생한다.
이것은 `sleep`이 정확하지 않아 발생하는 오차로 보여진다.

런 큐에는 `f1`, `f2`, `f3`, `ex01` 프로세스가 순서대로 들어가는데 `ex01`이 실행이 된 이후에는 `f1`이 실행되므로 항상 `f1`만 Runnable 상태로 나온다.

### 6. Modify your `my_sys_display_process()` so that it can also display the remaining time slice of each process (current->`rt.time_slice`) and repeat 5) as below to see the effect. `chrt -rr 30 ./f1` will run `f1` with priority `value=max_priority-30` (lower priority means higher priority). `-rr` is to set scheduling policy to `SCHED_RR` (whose `max_priority` is 99).

`init/main.c`:

```c
asmlinkage void my_sys_display_process(void)
{
    struct task_struct *task;
    task = &init_task;
    for (;;) {
        printk("ID: %d, Name: %s, State: %ld, Time slice: %u\n", task->pid, task->comm, task->state, task->rt.time_slice);
        task = next_task(task);
        if (task == &init_task) {
            break;
        }
    }
    printk("\n");
}
```

```bash
$ chrt -rr 30 ./f1 &
$ chrt -rr 30 ./f2 &
$ chrt -rr 30 ./f3 &
$ chrt -rr 30 ./ex01
```

`chrt`는 프로세스의 real-time 속성을 수정하는 명령어로, `-rr`은 스케줄링 정책을 `SCHED_RR`으로 설정하는 것을 의미한다.
이때, 프로세스는 낮은 숫자를 가질수록 높은 우선순위를 가진다.

`SCHED_RR`는 `SCHED_FIFO`와 비슷하지만 time quantum(time slice)를 다 쓰면 대기리스트의 뒤로 가서 기다린다.
RR은 Round Robin을 말하며 동일한 우선순위를 가진 쓰레드에게 동일한 시간을 사용 시간을 주는 것을 의미한다.

각 프로세스의 Time slice가 계속 줄어드는 것을 볼 수 있다.

CPU가 어떤 프로세스에 할당될 때는 일종의 시간 제한이 있는데 인터럽트 등의 이유로 이를 다 못 쓰고 CPU를 뺏기는 경우가 있다.
이때 시간 제한 중 다 못 쓰고 남은 시간을 remaining time slice라 한다.
