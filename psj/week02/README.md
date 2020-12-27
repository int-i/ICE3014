# Compiling Linux

## Homework

### 0. Boot sequence.

#### 1) When you boot the Linux system, the first program that runs is BIOS. Where is this program (the memory location)?

전원이 켜지면 cs=0xF000, ip=0xFFF0으로 초기화가 되므로 첫번째 명령어의 물리적 주소는 0xFFFFFFF0이다.
이 위치에 존재하는 프로그램을 BIOS라 한다.

#### 2) BIOS loads and runs the boot loader program (GRUB in Linux). Where is this GRUB program?

부트 디스크의 첫번째 섹터를 메모리에 로드하고 실행시킨다.
부트로더가 이 동작을 수행하며 리눅스의 경우 주로 GRUB 프로그램이 부트로더로 사용된다.

#### 3) GRUB loads and runs the Linux executable file. Where is Linux executable file? How GRUB knows the location of Linux executable file?

`/boot/grub/grub.conf`의 title 아래의 kernel 명령어가 해당 운영체제의 파일 경로를 나타낸다.
title은 운영체제 이름으로 여러 개가 있다면 사용자가 운영체제를 선택할 수 있는 화면을 보여준다.

### 1. Simple modification of the kernel.

Add `printk("hello from me\n");` after `printk(linux_banner);` in `start_kernel()`.
Go to the Linux top directory and compile the kernel and replace the boot image.
Reboot with this new kernel, and run `dmesg` to see if the kernel is printing "hello from me".

`init/main.c`에 `printk("hello from me\n");`를 추가해줬다.
  
Make는 여러 개의 C파일을 한 번에 컴파일해 바이너리로 만드는 명령어이다.
이에 대한 설정은 Makefile에 저장되어 있다.

`make bzImage`로 바이너리를 만들면 `arch/x86/boot/bzImage`에 결과물이 나온다.
bzImage는 Big Zip Image의 약자로 리눅스 실행파일을 이미지라 부르는데, 리눅스 사이즈가 크기 때문에 이미지 파일에 기본적으로 압축을 수행한다.
이것을 grub의 My Linux 운영체제의 kernel 경로인 `/boot/bzImage`에 복사해준다.

그후 운영체제를 재부팅하고 `dmesg`로 부팅 로그를 확인한다.

### 2. `start_kernel()` calls `trap_init()`, and there are many `trap_init()` functions defined in the kernel code. Make an intelligent guess about which `trap_init()` would be called and insert some `printk()` in the beginning of this `trap_init()` to see if it is really called by the kernel. Use grep in the top directory of the linux source tree to find out the locations of `trap_init()`:

현재 리눅스의 아키텍처가 x86 32비트이기 때문에 `arch/x86/kernel/` 디렉토리의 `traps_32.c` 파일을 수정해야 한다.

Make -> Reboot -> `dmesg`를 하면 부팅 로그에 "trap_init is started"가 잘 출력되는 것을 확인할 수 있다.

### 3. Find also the exact locations of `init_IRQ()` and insert some `printk()` in the beginning of `init_IRQ()` to confirm (actually you insert it in `native_init_IRQ`). Do the same thing for `init_timers()` `and time_init()`.

마찬가지로 `arch/x86/kernel/` 디렉토리에서 파일들을 찾아준다. `init_IRQ`는 `i8259_32.c`에 있다. 함수의 실제 구현은 `native_init_IRQ()`에 있으므로 이 함수를 수정해야 한다.

`init_timers()`는 `kernel/timer.c`에 있다.

`time_init`은 `arch/x86/kernel/time_32.c`에 있다.

Make -> Reboot -> `dmesg`으로 부팅 로그를 확인한다.

native_init_IRQ, init_timers, time_init 모두 잘 나오는 것을 확인할 수 있다.

### 4. Modify `/boot/grub/grub.conf` so that GRUB displays another Linux selection, My Linux2. Set the location of the kernel for this linux as `/boot/bzImage2`. Prepare two versions of My Linux such that when you select "My Linux" the kernel will display "hello from My Linux", and when you select "My Linux2", it displays "hello from My Linux2".

`/boot/grub/grub.conf`로 가서 `/boot/bzImage2`를 실행파일로 가지는 운영체제를 만들어준다.

`init/main.c`의 `start_kernel()` 함수가 "hello from My Linux"를 출력하게 수정한다.

Make 명령어를 이미지 파일을 만들고 `/boot/bzImage`에 복사해준다.
다시 `init/main.c`의 `start_kernel()` 함수로 돌아와 이번에는 "hello from My Linux2"를 출력하게 수정하고 이미지 파일을 만들어준다.
이 이미지 파일은 grub의 My Linux2의 경로인 `/boot/bzImage2`에 복사한다.

재부팅하면 부팅 운영체제 선택화면에 옵션이 3개로 늘어난 것을 확인할 수 있다.

### 5. Where is CPU at the end of the boot sequence when it prints "login" and waits for the user login? Explain your reasoning.

`init/main.c`:

```c
asmlinkage void __init start_kernel(void)
{
...
    cgroup_init();
    cpuset_init();
    taskstats_init_early();
    delayacct_init();

    check_bugs();

    acpi_early_init(); /* before LAPIC and SMP init */

    /* Do the rest non-__init'ed, we're now alive */
    rest_init();
}
```

`start_kernel()` 함수는 가장 마지막으로 `rest_init()`을 호출한다.

`init/main.c`:

```c
static void noinline __init_refok rest_init(void)
    __releases(kernel_lock)
{
...
    /*
     * The boot idle thread must execute schedule()
     * at least once to get things moving:
     */
    init_idle_bootup_task(current);
    preempt_enable_no_resched();
    schedule();
    preempt_disable();

    /* Call into cpu_idle with preempt disabled */
    cpu_idle();
}
```

`rest_init()` 함수는 init 커널 쓰레드를 생성하고 스케줄링한다.
다시 `rest_init()`으로 스케줄링 되었을 때, 마지막으로 `cpu_idle()`을 호출한다.

`arch/x86/kernel/process_32.c`:

```c
void cpu_idle(void)
{
    int cpu = smp_processor_id();

    current_thread_info()->status |= TS_POLLING;

    /* endless idle loop with no priority at all */
    while (1) {
        tick_nohz_stop_sched_tick();
        while (!need_resched()) {
            void (*idle)(void);

            check_pgt_cache();
            rmb();
            idle = pm_idle;

            if (rcu_pending(cpu))
                rcu_check_callbacks(cpu, 0);

            if (!idle)
                idle = default_idle;

            if (cpu_is_offline(cpu))
                play_dead();

            __get_cpu_var(irq_stat).idle_timestamp = jiffies;
            idle();
        }
        tick_nohz_restart_sched_tick();
        preempt_enable_no_resched();
        schedule();
        preempt_disable();
    }
}
```

`cpu_idle()` 함수는 가장 마지막에 실행되는 우선순위가 낮은 프로세스이다.
`while(1)`로 묶여 있어서, 무한루프를 돌면서 idle 프로세스를 실행한다.
CPU는 쉬지 않고 프로세스를 돌려가며 실행시키며, 로그인 프로세스가 실행되고 일정시간이 지나면 다른 프로세스에서 CPU 점유가 넘어가게 된다.

로그인은 부팅 과정 중 가장 마지막에 실행되는 프로세스 중 하나로, 만약 사용자가 로그인 화면이 입력을 하게 되면 keyboard interrupt가 발생하고 입력 값을 처리하기 위해 로그인 프로세스에 CPU가 다시 할당되게 된다.
