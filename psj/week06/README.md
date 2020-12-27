# Process 2

## Homework 1

### 1. Run the program below. What happens? Explain the result.

See [HW 1](./hw01/main.c)

`fork`는 자신의 body와 process descriptor를 복사해 child process를 만들어낸다.
`fork`가 성공하면 자식 프로세스에서는 0을 반환하고(실패시 -1), 부모 프로세스에서는 자식의 pid를 반환한다.

```bash
$ gcc -o hw01 hw01.c
$ ./hw01
x: 0
x: 4548
```

0은 자식 프로세스의 `printf`로부터 출력된 값이고, 4548은 부모 프로세스의 `printf`로부터 출력된 것이다.

### 2. Try below and explain the result.

See [HW 2](./hw02/main.c)

`fork`를 2번하고 무한루프를 돌고 있다.

```bash
$ gcc -o hw02 hw02.c
$ ./hw02 &
$ ps -ef
```

총 4개의 프로세스가 생성되는데, `fork` 함수가 2개이기 때문에 2^2개의 프로세스가 생성되었다.

### 3. Run following code. What happens? Explain the result.

See [HW 3](./hw03/main.c)

```bash
$ gcc -o hw03 hw03.c
$ ./hw03
```

2번과 동일하게 총 4개의 프로세스가 생성된다.
4개의 프로세스에서 `y`에 대한 연산을 수행하고 자신의 PID를 출력한다.
무한루프이기 때문에 프로세스가 종료될 때까지 계속 연산을 수행하고 PID를 출력할 것이다.

### 4. Try below and explain the result.

See [HW 4](./hw04/)

`execve`는 현재 프로세스를 입력 받은 프로그램으로 프로세스를 교체해 새로 시작하는 함수이다.
첫 번째 인자로 프로그램 경로를 받고 두 번째 인자로 프로그램의 `argv`에 넘어갈 값을 받는다.

```bash
$ gcc -o ex01 ex01.c
$ gcc -o ex02 ex02.c
$ ./ex01
korea
```

`ex01`에서 `execve`를 호출했기 때문에 현재 프로세스 body가 `ex02`의 body로 교체되고 새로 실행되기 때문에 `ex02`의 출력인 "korea"가 출력되었다.

### 5. Run following code and explain the result.

See [HW 5](./hw05/main.c)

실행되는 파일 이름이 `/bin/ls`로 바뀌었다.

```bash
$ gcc -o hw05 hw05.c
$ ./hw05
<result of `ls`>
```

`ls` 명령어은 사실 `/bin/ls`이라는 프로그램을 실행하는 명령어이다.
`execve`로 `/bin/ls`을 실행했기 때문에, `ls`를 실행한 결과와 같은 내용이 출력되었다.

### 6. Run following code and explain the result.

See [HW 6](./hw06/main.c)

`argv`는 문자열 배열로 C언어에서 문자열과 같이 마지막 요소를 `NULL` 표시함으로써 배열의 끝을 나타낸다.
`argv`의 두 번째 요소로 `-l`이 들어왔다.

```bash
$ gcc -o hw06 hw06.c
$ ./hw06
<result of `ls -l`>
```

`-l`은 파일 내용을 자세히 목록으로 보여주는 옵션이다.
5번과 같은 원리로, `ls -l`를 실행한 결과와 같은 내용이 출력되었다.

### 7. Run following code and explain the result.

See [HW 7](./hw07/main.c)

`pthread_create`는 스레드를 생성하는 함수이다.
첫 번째 인자 `thread`는 스레드가 생성되었을 때, 이를 식별하기 위한 값이다.
세 번째 인자는 스레드가 실행될 때, 사용될 함수를 넣어준다.

```bash
$ gcc -o hw07 hw07.c -lpthread
$ ./hw07
hello from child
hello from parent
```

`pthread.h` 헤더의 함수를 사용하려면 PThread 라이브러리를 링크해야 한다.
GCC에서 `-l`은 라이브러리를 링크하는 옵션으로 `-lpthread`는 `/usr/lib/libpthread.so`를 링크한다.

### 8. Run following code and explain the difference.

See [HW 8](./hw08/)

```bash
$ gcc -o ex01 ex01.c -lpthread
$ ./ex01
process child: 1
process parent: 2
$ gcc -o ex02 ex02.c -lpthread
$ ./ex02
thread child: 1
thread parent: 3
```

프로세스는 컴퓨터에서 연속적으로 실행되고 있는 컴퓨터 프로그램으로, 메모리에 올라와 실행되고 있는 프로그램의 인스턴스(독립적인 개체)를 말한다.
각 프로세스는 개별적인 주소 공간에서 실행되며, 프로세스 간 서로의 메모리에 접근할 수 없다.

스레드는 프로세스 내에서 실행되는 흐름의 단위로, 같은 프로세스내의 스레드는 메모리 공간을 공유할 수 있다.

`ex01`은 프로세스 예제로 각 프로세스 마다 개별적인 `y`를 가지고 있기 때문에 각각 1, 2가 출력되었다.
`ex02`는 스레드 예제로 한 프로세스 내의 전역변수 `y`를 공유하기 때문에 자식 프로세스에서 1을 출력하고 부모 프로세스에서 이미 1인 `y`에 2를 더한 3을 출력했다.

## Homework 2

### 1. Try the shell code in section 7. Try Linux command such as `/bin/ls`, `/bin/date`, etc.

See [HW 1](./hw09/main.c)

`errno`보다 `perror`가 코드가 깔끔한 것 같아 임의로 교체했다.
`perror`는 ERRNO 숫자를 해당하는 문자열로 보여주는 함수이다.
오류 메세지가 출력되기 때문에 오류 코드보다 직관적이다.

```bash
$ gcc -o hw09 hw09.c
$ ./hw09
```

`fork`로 프로그램을 실행할 프로세스를 만든다.
`execve`로 입력 받은 프로그램을 실행시킨다.

`execve`는 실패하면 -1을 반환하는데, 프로그램 실행에 실패하면 `exit` 함수로 현재 프로세스를 종료시킨다.

### 2. Print the pid of the current process(`current->pid`) inside `rest_init()` and `kernel_init()`. The pid printed inside `rest_init()` will be 0, but the pid inside `kernel_init()` is 1. 0 is the pid of the kernel itself. Why do we have pid=1 inside `kernel_init()`?

`init/main.c`:

```c
static void noinline __init_refok rest_init(void)
    __releases(kernel_lock)
{
    int pid;

    printk("Current PID: %d\n", current->pid);
...

static int __init kernel_init(void * unused)
{
    printk("Current PID: %d\n", current->pid);
...
```

위와 같이 `rest_init`과 `kernel_init` 함수 시작 부분에 현재 PID를 출력하는 코드를 삽입했다.

코드를 적용하고 재부팅을 하면, PID가 0에서 1로 바뀐다. `rest_init`에서는 `kernel_thread` 함수로 `kernel_init`이라는 task를 만들며 PID를 1로 지정한다. 이때, `kernel_init`은 프로세스이기 때문에 `init_task`와 process body를 공유한다.

### 3. The last function call in `start_kernel()` is `rest_init()`. If you `insert printk()` after `rest_init()`, it is not displayed during the system booting. Explain the reason.

`init/main.c`:

```c
asmlinkage void __init start_kernel(void)
{
    printk("before rest_init\n");
    rest_init();
    printk("after rest_init\n");
}
```

`rest_init`의 `cpu_idle` 함수는 실행이 끝나면 무한루프를 돌며 스케쥴링을 기다린다.
계속 무한 루프를 돌고 있기 때문에, 같은 프로세스의 `rest_init` 이후의 코드는 실행되지 않는다.

### 4. The CPU is either in some application program or in Linux kernel. You always should be able to say where is the CPU currently. Suppose we have a following program(`ex01.c`). When the shell runs this, CPU could be in shell program or in `ex01` or in kernel. Explain where is CPU for each major step of this program. You should indicate the CPU location whenever the cpu changes its location among these three programs. Start the tracing from the moment when the shell prints a prompt until it prints next prompt.

```c
#include <stdio.h>

int main(void) {
    printf("korea\n");
    return 0;
}
```

```txt
shell:  printf("$");                    // shell에서 입력 가능을 나타내는 문자를 출력
        write(STDOUT_FILENO, "$", 1);   // STDOUT_FILENO(=1)에 "$"을 1글자 출력
        INT 128                         // 시스템 콜 인터럽트인 128번을 호출
        mov eax 4                       // write의 시스템 콜 번호는 4
kernel: sys_write                       // "$" 문자열을 출력하기 위한 시스템 콜 호출

키보드 입력 발생
shell:  INT 33                          // 키보드 인터럽트인 33번 호출
kernel: kbd_interrupt                   // 키보드 버퍼에 입력한 문자 저장
Enter가 입력될 때까지 위 과정 반복

shell:  scanf("%s", buf);               // 사용자가 입력한 문자열을 읽음
        read(STDIN_FILENO, buf, len)    // STDIN_FILENO(=1)에서 len 만큼 읽어 buf에 저장
        INT 128                         // 시스템 콜 인터럽트인 128번을 호출
        mov eax 3                       // read의 시스템 콜 번호는 3
kernel: sys_read                        // 입력한 문자열을 읽어오는 시스템 콜 호출

shell:  fork                            // 프로그램을 실행하기 위한 자식 프로세스 생성
        INT 128                         // 시스템 콜 인터럽트인 128번을 호출
kernel: sys_fork                        // fork의 시스템 콜 호출

shell:  execve                          // 입력받은 프로그램을 실행
        INT 128                         // 시스템 콜 인터럽트인 128번을 호출
kernel: sys_execve                      // execve의 시스템 콜 호출

ex01:   printf("korea\n");              // 프로그램이 종료되면 다시 shell에서 입력 가능을 나타내는 문자를 출력
        write(STDOUT_FILENO, "$", 1);   // STDOUT_FILENO(=1)에 "$"을 1글자 출력
        INT 128                         // 시스템 콜 인터럽트인 128번을 호출
        mov eax 4                       // write의 시스템 콜 번호는 4
kernel: sys_write                       // "$" 문자열을 출력하기 위한 시스템 콜 호출

shell:  printf("$");                    // 프로그램이 종료되면 다시 shell에서 입력 가능을 나타내는 문자를 출력
        write(STDOUT_FILENO, "$", 1);   // STDOUT_FILENO(=1)에 "$"을 1글자 출력
        INT 128                         // 시스템 콜 인터럽트인 128번을 호출
        mov eax 4                       // write의 시스템 콜 번호는 4
kernel: sys_write                       // "$" 문자열을 출력하기 위한 시스템 콜 호출
```

### 5. What happens if the kernel calls `kernel_init` directly instead of calling `kernel_thread(kernel_init, ...)` in `rest_init()`? Call `kernel_init` with `NULL` argument and explain why the kenel falls into panic.

부팅 중 Kernel panic이 발생했다.
`kernel_thread`는 프로세스 디스크립터를 복사하는데, `kernel_thread` 없이 `kernel_init`을 실행하게 되면 `kernel_init` 내부의 무한루프로 인해 이후 프로세스가 실행되지 않는다.
또한, `kernel_execve`으로 다른 프로세스를 실행하면 현재 body를 제거했기 때문에 오류가 발생했다.

### 6. Trace fork, exec, exit, wait system call to find the corresponding code for the major steps of each system call.

#### `fork`

`fork`는 `sys_fork` 함수를 호출한다.

`arch/x86/kernel/process_32.c`:

```c
asmlinkage int sys_fork(struct pt_regs regs)
{
    return do_fork(SIGCHLD, regs.sp, &regs, 0, NULL, NULL);
}
```

`kernel/fork.c`:

```c
long do_fork(unsigned long clone_flags,
          unsigned long stack_start,
          struct pt_regs *regs,
          unsigned long stack_size,
          int __user *parent_tidptr,
          int __user *child_tidptr)
{
    struct task_struct *p;
    int trace = 0;
    long nr;
    ...
    p = copy_process(clone_flags, stack_start, regs, stack_size,
            child_tidptr, NULL);
    ...
    return nr;
}

static struct task_struct *copy_process(unsigned long clone_flags,
                    unsigned long stack_start,
                    struct pt_regs *regs,
                    unsigned long stack_size,
                    int __user *child_tidptr,
                    struct pid *pid)
{
    ...
    p = dup_task_struct(current);
    ...
}

static struct task_struct *dup_task_struct(struct task_struct *orig)
{
    struct task_struct *tsk;
    struct thread_info *ti;
    int err;

    prepare_to_copy(orig);

    tsk = alloc_task_struct();
    ...
    ti = alloc_thread_info(tsk);
    ...
    setup_thread_stack(tsk, orig);
    ...
    return tsk;
}
```

`sys_fork` -> `do_fork` -> `copy_process` -> `dup_task_struct`

`do_fork`에서 반환하는 `nr`은 복사된 프로세스의 PID이다.

#### `exec`

`exec`는 `sys_exec` 함수를 호출한다.

`arch/x86/kernel/process_32.c`:

```c
asmlinkage int sys_execve(struct pt_regs regs)
{
    int error;
    char * filename;
    ...
    error = do_execve(filename,
            (char __user * __user *) regs.cx,
            (char __user * __user *) regs.dx,
            &regs);
    ...
    return error;
}
```

`fs/exec.c`:

```c
int do_execve(char * filename,
    char __user *__user *argv,
    char __user *__user *envp,
    struct pt_regs * regs)
{
    struct linux_binprm *bprm;
    struct file *file;
    unsigned long env_p;
    int retval;
    ...
    file = open_exec(filename);
    retval = PTR_ERR(file);
    if (IS_ERR(file))
        goto out_kfree;

    sched_exec();
    ...
    retval = search_binary_handler(bprm,regs);
    if (retval >= 0) {
        /* execve success */
        free_arg_pages(bprm);
        security_bprm_free(bprm);
        acct_update_integrals(current);
        kfree(bprm);
        return retval;
    }
    ...
    return retval;
}
```

`sys_execve` -> `do_execve` -> `open_exec` -> `sched_exec`

`do_execve`에서 파일을 열고, 스케줄에 등록하고, `argv`등의 값을 넘겨준다.

#### `exit`

`exit`는 `sys_exit` 함수를 호출한다.

`kernel/exit.c`:

```c
asmlinkage long sys_exit(int error_code)
{
    do_exit((error_code&0xff)<<8);
}


NORET_TYPE void do_exit(long code)
{
    struct task_struct *tsk = current;
    int group_dead;

    profile_task_exit(tsk);
    ...
    exit_signals(tsk);  /* sets PF_EXITING */
    ...
    exit_mm(tsk);
    ...
    exit_thread();
    ...
    exit_notify(tsk, group_dead);
    ...
    if (tsk->io_context)
        exit_io_context();
    ...
    schedule();
    ...
}
```

`sys_exit` -> `do_exit` -> `exit_signals` -> `exit_mm` -> `exit_thread` -> `exit_notify` -> `schedule`

시그널 보내고(등록된 함수 호출), 메모리 회수하고, 스레드 종료하고, 부모 프로세스에 알리고(시그널 전송), 스케줄링으로 완전히 제거한다.

#### `wait`

`wait(&wstatus)`는 `waitpid(-1, &wstatus, 0)`이다.
따라서 `waitpid`를 찾아야 한다.

`waitpid`는 `sys_waitpid` 함수를 호출한다.

`kernel/exit.c`:

```c
/*
 * sys_waitpid() remains for compatibility. waitpid() should be
 * implemented by calling sys_wait4() from libc.a.
 */
asmlinkage long sys_waitpid(pid_t pid, int __user *stat_addr, int options)
{
    return sys_wait4(pid, stat_addr, options, NULL);
}
```

`sys_waitpid`을 찾아가면 `sys_waitpid`는 호환성을 위해 남겨두었을뿐, `sys_wait4`으로 구현된다고 주석이 남겨있다.
`sys_wait4`를 찾아보자.

`kernel/exit.c`:

```c
asmlinkage long sys_wait4(pid_t upid, int __user *stat_addr,
              int options, struct rusage __user *ru)
{
    struct pid *pid = NULL;
    enum pid_type type;
    long ret;
    ...
    ret = do_wait(type, pid, options | WEXITED, NULL, stat_addr, ru);
    put_pid(pid);
    ...
    return ret;
}

static long do_wait(enum pid_type type, struct pid *pid, int options,
            struct siginfo __user *infop, int __user *stat_addr,
            struct rusage __user *ru)
{
    DECLARE_WAITQUEUE(wait, current);
    struct task_struct *tsk;
    int flag, retval;

    add_wait_queue(&current->signal->wait_chldexit,&wait);
repeat:
    ...

    flag = retval = 0;
    current->state = TASK_INTERRUPTIBLE;
    read_lock(&tasklist_lock);
    tsk = current;
    do {
        struct task_struct *p;

        list_for_each_entry(p, &tsk->children, sibling) {
            int ret = eligible_child(type, pid, options, p);
            if (!ret)
                continue;

            if (unlikely(ret < 0)) {
                retval = ret;
            } else if (task_is_stopped_or_traced(p)) {
                ...
                retval = wait_task_stopped(p,
                        (options & WNOWAIT), infop,
                        stat_addr, ru);
            } else if (p->exit_state == EXIT_ZOMBIE &&
                    !delay_group_leader(p)) {
                ...
                retval = wait_task_zombie(p,
                        (options & WNOWAIT), infop,
                        stat_addr, ru);
            } else if (p->exit_state != EXIT_DEAD) {
                ...
                retval = wait_task_continued(p,
                        (options & WNOWAIT), infop,
                        stat_addr, ru);
            }
            if (retval != 0) /* tasklist_lock released */
                goto end;
        }
        ...
        tsk = next_thread(tsk);
        ...
    } while (tsk != current);
    ...
    return retval;
}
```

`sys_waitpid` -> `sys_wait4` -> `do_wait` -> `wait_task_stopped` / `wait_task_zombie` / `wait_task_continued`
