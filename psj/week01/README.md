# Linux Kernel Overview

## Homework

### 1. Install Gentoo Linux on virtual machine.

1. Download Virtualbox from Internet and install.
2. Download Gentoo virtualbox files(`gentoo.zip`) from the I-class and un-compress it.
3. Run VirtualBox, and click File>Import and go to the gentoo directory. Select `Gentoo2.ovf`. Uncheck USB controller. Select Import. This will install Gentoo Linux on your virtual box.
4. Run Gentoo. If you have an error, run VirutalBox as administrator and try again. For USB error, simply disable usb controller in "Setting" tab. For Hyper-V error (Raw-mode is unavailable), turn off Hyper-V feature in control panel>program and feature>window feature. Select My Linux. Login as root and hit Enter for the password prompt. If VirtualBox still cannot open the session, you need to look at the log file (right click on Gentoo VM and select "log file") and see what is the error and fix it. (In some cases, you may need to download and install virtualbox extension package.)
5. Make a simple program, `hw01.c`, with `vi` which displays "hello world". Compile and run it.

See [HW 1](./hw01/main.c)

```bash
$ gcc -o hw01 hw01.c
$ ./hw01
hello world
```

### 2. Go to `linux-2.6.25.10` directory and find all the files referred in Section 1 such as `main.c`, `fork.c`, `entry_32.S`, etc.

`main.c`는 `init` 폴더에 있으며 리눅스의 시작점이다. `start_kernel()` 함수도 main.c에 위치한다.

`fork.c`는 `kernel` 폴더에 있으며 이 폴더에는 프로세스와 관련된 파일이 있다.

`entry_32.S`는 `arch/x86/kernel`에 있다. 이 폴더에는 아키텍처마다 다르게 동작하는 함수들이 담겨있으며 x86은 인텔의 32비트 아키텍처를 의미한다.

### 3. Find the location of `start_kernel()`.

$ grep -nr "start_kernel" * | more 명령어를 입력하니 위와 같은 목록이 나왔다.
`start_kernel()` 함수는 `init/main.c`에 위치한다.

Vi를 이용해 `start_kernel()` 함수를 찾아보았다.
Vi로 파일을 열어 `/start_kernel`를 입력하면 검색이 된다. `/`을 입력하면 다음 검색 결과를 보여준다.

### 4. `start_kernel()` is the first C function run by Linux. Predict what will be the first message appearing on the screen by analyzing `start_kernel()`. Note that `printk()` (not `printf`) is the function to print something in the kernel. Confirm your prediction with `dmesg > x` and `vi x`. The kernel remembers the booting message in the system buffer and dmesg displays the content of this buffer to the screen. `dmesg > x" will send the booting message to file `x`. With `vi x` you can look at the file `x`.

`printk()`는 리눅스 커널에서 글자를 출력하기 위한 함수로, 첫번째로 화면에 출력되는 글자는 첫번째로 호출되는 `printk()`의 값일 것이다.

`init/main.c`:

```c
printk(KERN_NOTICE);
printk(linux_banner);
```

첫 번째 출력은 위 두 함수 호출로 KERN_NOTICE는 출력의 로그 레벨을 설정하는 기호로 다음 출력이 정상적인 정보에 해당하는 로그임을 나타낸다.
그렇다면 linux_banner가 진짜 출력인데 해당 값이 정의된 `init/version.c`으로 가면 아래와 같이 리눅스의 버전과 컴파일 환경을 출력하는 문자열이 구성되어 있다.

`init/version.c`:

```c
/* FIXED STRINGS! Don't touch! */
const char linux_banner[] =
    "Linux version " UTS_RELEASE " (" LINUX_COMPILE_BY "@"
    LINUX_COMPILE_HOST ") (" LINUX_COMPILER ") " UTS_VERSION "\n";
```

따라서 리눅스가 부팅되면 리눅스의 버전과 컴파일 환경이 가장 먼저 출력될 것이다.

실제로 `dmesg`를 입력해 시스템 부팅 메시지를 확인하면 리눅스 버전과 GCC 정보가 가장 먼저 출력된다.

### 5. Find the location of the following functions called in `start_kernel()` and briefly explain your guessing about what each function is doing (but do not copy the whole code).

arch 디렉토리는 cpu 마다 다른 함수들을 모아놓은 디렉토리이므로 trap_init, init_IRQ 등은 cpudependent 한 함수이다.
`trap_init()` 함수는 아키텍처마다 다르게 동작하기 때문에 x86 32비트를 기준으로 작성했다.

`arch/x86/kernel/traps_32.c`:

```c
set_trap_gate(0,&divide_error);
set_intr_gate(1,&debug);
set_intr_gate(2,&nmi);
set_system_intr_gate(3, &int3); /* int3/4 can be called from all */
set_system_gate(4,&overflow);
set_trap_gate(5,&bounds);
set_trap_gate(6,&invalid_op);
set_trap_gate(7,&device_not_available);
set_task_gate(8,GDT_ENTRY_DOUBLEFAULT_TSS);
set_trap_gate(9,&coprocessor_segment_overrun);
set_trap_gate(10,&invalid_TSS);
set_trap_gate(11,&segment_not_present);
set_trap_gate(12,&stack_segment);
set_trap_gate(13,&general_protection);
set_intr_gate(14,&page_fault);
set_trap_gate(15,&spurious_interrupt_bug);
set_trap_gate(16,&coprocessor_error);
set_trap_gate(17,&alignment_check);
```

`trap_init()` 함수에는 위와 같이 set 함수가 나열되어 있다.
트랩은 인터럽트와는 달리 정해진 곳으로 분기하고 번호로 정해져 있다.
위는 트랩 게이트를 설정하는 코드이다.

`init_IRQ()` 함수 역시 아키텍처마다 다르게 작동한다.
이 역시 x86 32비트를 기준으로 했다.

`arch/x86/kernel/paravirt.c`:

```c
struct pv_irq_ops pv_irq_ops = {
    .init_IRQ = native_init_IRQ,
    .save_fl = native_save_fl,
    .restore_fl = native_restore_fl,
    .irq_disable = native_irq_disable,
    .irq_enable = native_irq_enable,
    .safe_halt = native_safe_halt,
    .halt = native_halt,
};

...

void init_IRQ(void)
{
    pv_irq_ops.init_IRQ();
}
```

`arch/x86/kernel/i8259_32.c`:

```c
void __init native_init_IRQ(void)
{
    int i;

    /* all the set up before the call gates are initialised */
    pre_intr_init_hook();

    /*
     * Cover the whole vector space, no vector can escape
     * us. (some of these will be overridden and become
     * 'special' SMP interrupts)
     */
    for (i = 0; i < (NR_VECTORS - FIRST_EXTERNAL_VECTOR); i++) {
        int vector = FIRST_EXTERNAL_VECTOR + i;
        if (i >= NR_IRQS)
            break;
        /* SYSCALL_VECTOR was reserved in trap_init. */
        if (!test_bit(vector, used_vectors))
            set_intr_gate(vector, interrupt[i]);
    }

    /* setup after call gates are initialised (usually add in
     * the architecture specific gates)
     */
    intr_init_hook();

    /*
     * External FPU? Set up irq13 if so, for
     * original braindamaged IBM FERR coupling.
     */
    if (boot_cpu_data.hard_math && !cpu_has_fpu)
        setup_irq(FPU_IRQ, &fpu_irq);

    irq_ctx_init(smp_processor_id());
}
```

IRQ는 Interrupt ReQuest의 약자로, 인터럽트 신호를 처리하는 데에 쓰이는 컴퓨터 버스 라인의 인터럽트 동작을 말한다.

`kernel/sched.c`:

```c
void __init sched_init(void)
{
    int highest_cpu = 0;
    int i, j;

#ifdef CONFIG_SMP
    init_defrootdomain();
#endif

#ifdef CONFIG_GROUP_SCHED
    list_add(&init_task_group.list, &task_groups);
#endif

    for_each_possible_cpu(i) {
        struct rq *rq;

        rq = cpu_rq(i);
        spin_lock_init(&rq->lock);
        lockdep_set_class(&rq->lock, &rq->rq_lock_key);
        rq->nr_running = 0;
        rq->clock = 1;
        init_cfs_rq(&rq->cfs, rq);
        init_rt_rq(&rq->rt, rq);
#ifdef CONFIG_FAIR_GROUP_SCHED
        init_task_group.shares = init_task_group_load;
        INIT_LIST_HEAD(&rq->leaf_cfs_rq_list);
        init_tg_cfs_entry(rq, &init_task_group,
                &per_cpu(init_cfs_rq, i),
                &per_cpu(init_sched_entity, i), i, 1);

#endif
#ifdef CONFIG_RT_GROUP_SCHED
        init_task_group.rt_runtime =
            sysctl_sched_rt_runtime * NSEC_PER_USEC;
        INIT_LIST_HEAD(&rq->leaf_rt_rq_list);
        init_tg_rt_entry(rq, &init_task_group,
                &per_cpu(init_rt_rq, i),
                &per_cpu(init_sched_rt_entity, i), i, 1);
#endif
        rq->rt_period_expire = 0;
        rq->rt_throttled = 0;

        for (j = 0; j < CPU_LOAD_IDX_MAX; j++)
            rq->cpu_load[j] = 0;
#ifdef CONFIG_SMP
        rq->sd = NULL;
        rq->rd = NULL;
        rq->active_balance = 0;
        rq->next_balance = jiffies;
        rq->push_cpu = 0;
        rq->cpu = i;
        rq->migration_thread = NULL;
        INIT_LIST_HEAD(&rq->migration_queue);
        rq_attach_root(rq, &def_root_domain);
#endif
        init_rq_hrtick(rq);
        atomic_set(&rq->nr_iowait, 0);
        highest_cpu = i;
    }

    set_load_weight(&init_task);

#ifdef CONFIG_PREEMPT_NOTIFIERS
    INIT_HLIST_HEAD(&init_task.preempt_notifiers);
#endif

#ifdef CONFIG_SMP
    nr_cpu_ids = highest_cpu + 1;
    open_softirq(SCHED_SOFTIRQ, run_rebalance_domains, NULL);
#endif

#ifdef CONFIG_RT_MUTEXES
    plist_head_init(&init_task.pi_waiters, &init_task.pi_lock);
#endif

    /*
     * The boot idle thread does lazy MMU switching as well:
     */
    atomic_inc(&init_mm.mm_count);
    enter_lazy_tlb(&init_mm, current);

    /*
     * Make us the idle thread. Technically, schedule() should not be
     * called from this thread, however somewhere below it might be,
     * but because we are the idle thread, we just pick up running again
     * when this runqueue becomes "idle".
     */
    init_idle(current, smp_processor_id());
    /*
     * During early bootup we pretend to be a normal task:
     */
    current->sched_class = &fair_sched_class;

    scheduler_running = 1;
}
```

`sched_init`은 init 태스크가 사용하는 CPU 번호를 할당해 주고 pid hash table을 초기화 한다.
이어 타이머 인터럽트 벡터를 초기화 한다.

`arch/x86/kernel/time_32.c`:

```c
void __init time_init(void)
{
    tsc_init();
    late_time_init = choose_time_init();
}
```

`arch/x86/kernel/tsc_32.c`:

```c
void __init tsc_init(void)
{
    int cpu;

    if (!cpu_has_tsc)
        goto out_no_tsc;

    cpu_khz = calculate_cpu_khz();
    tsc_khz = cpu_khz;

    if (!cpu_khz)
        goto out_no_tsc;

    printk("Detected %lu.%03lu MHz processor.\n",
                (unsigned long)cpu_khz / 1000,
                (unsigned long)cpu_khz % 1000);

    /*
     * Secondary CPUs do not run through tsc_init(), so set up
     * all the scale factors for all CPUs, assuming the same
     * speed as the bootup CPU. (cpufreq notifiers will fix this
     * up if their speed diverges)
     */
    for_each_possible_cpu(cpu)
        set_cyc2ns_scale(cpu_khz, cpu);

    use_tsc_delay();

    /* Check and install the TSC clocksource */
    dmi_check_system(bad_tsc_dmi_table);

    unsynchronized_tsc();
    check_geode_tsc_reliable();
    current_tsc_khz = tsc_khz;
    clocksource_tsc.mult = clocksource_khz2mult(current_tsc_khz,
                            clocksource_tsc.shift);
    /* lower the rating if we already know its unstable: */
    if (check_tsc_unstable()) {
        clocksource_tsc.rating = 0;
        clocksource_tsc.flags &= ~CLOCK_SOURCE_IS_CONTINUOUS;
    } else
        tsc_enabled = 1;

    clocksource_register(&clocksource_tsc);

    return;

out_no_tsc:
    setup_clear_cpu_cap(X86_FEATURE_TSC);
}
```

CMOS에서 시간을 읽고 CPU의 속도를 얻어낸다.
또한 부팅 메세지의 "Detected 2894.506 MHz processor." 부분의 출력을 담당하는 함수이기도 하다.

`drivers/char/tty_io.c`:

```c
void __init console_init(void)
{
    initcall_t *call;

    /* Setup the default TTY line discipline. */
    (void) tty_register_ldisc(N_TTY, &tty_ldisc_N_TTY);

    /*
     * set up the console device so that later boot sequences can
     * inform about problems etc..
     */
    call = __con_initcall_start;
    while (call < __con_initcall_end) {
        (*call)();
        call++;
    }
}
```

콘솔 디바이스를 초기화 한다.
모든 초기화를 수행하는 것은 아니고 초기에 필요한 정도만 하고 나머지는 나중에 한다.

`arch/x86/mm/init_32.c`:

```c
void __init mem_init(void)
{
    int codesize, reservedpages, datasize, initsize;
    int tmp, bad_ppro;

#ifdef CONFIG_FLATMEM
    BUG_ON(!mem_map);
#endif
    bad_ppro = ppro_with_ram_bug();

#ifdef CONFIG_HIGHMEM
    /* check that fixmap and pkmap do not overlap */
    if (PKMAP_BASE + LAST_PKMAP*PAGE_SIZE >= FIXADDR_START) {
        printk(KERN_ERR
            "fixmap and kmap areas overlap - this will crash\n");
        printk(KERN_ERR "pkstart: %lxh pkend: %lxh fixstart %lxh\n",
                PKMAP_BASE, PKMAP_BASE + LAST_PKMAP*PAGE_SIZE,
                FIXADDR_START);
        BUG();
    }
#endif
    /* this will put all low memory onto the freelists */
    totalram_pages += free_all_bootmem();

    reservedpages = 0;
    for (tmp = 0; tmp < max_low_pfn; tmp++)
        /*
         * Only count reserved RAM pages:
         */
        if (page_is_ram(tmp) && PageReserved(pfn_to_page(tmp)))
            reservedpages++;

    set_highmem_pages_init(bad_ppro);

    codesize =  (unsigned long) &_etext - (unsigned long) &_text;
    datasize =  (unsigned long) &_edata - (unsigned long) &_etext;
    initsize =  (unsigned long) &__init_end - (unsigned long) &__init_begin;

    kclist_add(&kcore_mem, __va(0), max_low_pfn << PAGE_SHIFT);
    kclist_add(&kcore_vmalloc, (void *)VMALLOC_START,
           VMALLOC_END-VMALLOC_START);

    printk(KERN_INFO "Memory: %luk/%luk available (%dk kernel code, "
            "%dk reserved, %dk data, %dk init, %ldk highmem)\n",
        (unsigned long) nr_free_pages() << (PAGE_SHIFT-10),
        num_physpages << (PAGE_SHIFT-10),
        codesize >> 10,
        reservedpages << (PAGE_SHIFT-10),
        datasize >> 10,
        initsize >> 10,
        (unsigned long) (totalhigh_pages << (PAGE_SHIFT-10))
           );

#if 1 /* double-sanity-check paranoia */
    printk(KERN_INFO "virtual kernel memory layout:\n"
        "    fixmap  : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#ifdef CONFIG_HIGHMEM
        "    pkmap   : 0x%08lx - 0x%08lx   (%4ld kB)\n"
#endif
        "    vmalloc : 0x%08lx - 0x%08lx   (%4ld MB)\n"
        "    lowmem  : 0x%08lx - 0x%08lx   (%4ld MB)\n"
        "      .init : 0x%08lx - 0x%08lx   (%4ld kB)\n"
        "      .data : 0x%08lx - 0x%08lx   (%4ld kB)\n"
        "      .text : 0x%08lx - 0x%08lx   (%4ld kB)\n",
        FIXADDR_START, FIXADDR_TOP,
        (FIXADDR_TOP - FIXADDR_START) >> 10,

#ifdef CONFIG_HIGHMEM
        PKMAP_BASE, PKMAP_BASE+LAST_PKMAP*PAGE_SIZE,
        (LAST_PKMAP*PAGE_SIZE) >> 10,
#endif

        VMALLOC_START, VMALLOC_END,
        (VMALLOC_END - VMALLOC_START) >> 20,

        (unsigned long)__va(0), (unsigned long)high_memory,
        ((unsigned long)high_memory - (unsigned long)__va(0)) >> 20,

        (unsigned long)&__init_begin, (unsigned long)&__init_end,
        ((unsigned long)&__init_end -
         (unsigned long)&__init_begin) >> 10,

        (unsigned long)&_etext, (unsigned long)&_edata,
        ((unsigned long)&_edata - (unsigned long)&_etext) >> 10,

        (unsigned long)&_text, (unsigned long)&_etext,
        ((unsigned long)&_etext - (unsigned long)&_text) >> 10);

#ifdef CONFIG_HIGHMEM
    BUG_ON(PKMAP_BASE + LAST_PKMAP*PAGE_SIZE    > FIXADDR_START);
    BUG_ON(VMALLOC_END                > PKMAP_BASE);
#endif
    BUG_ON(VMALLOC_START                > VMALLOC_END);
    BUG_ON((unsigned long)high_memory        > VMALLOC_START);
#endif /* double-sanity-check paranoia */

    if (boot_cpu_data.wp_works_ok < 0)
        test_wp_bit();

    cpa_init();

    /*
     * Subtle. SMP is doing it's boot stuff late (because it has to
     * fork idle threads) - but it also needs low mappings for the
     * protected-mode entry to work. We zap these entries only after
     * the WP-bit has been tested.
     */
#ifndef CONFIG_SMP
    zap_low_mappings();
#endif
}
```

`mem_init()` 함수는 메모리 시스템에 대한 초기화를 담당하는 함수이다.
부팅 메시지의 메모리 관련 정보 출력 역시 이 함수에서 출력한다.

`init/main.c`:

```c
static void noinline __init_refok rest_init(void)
    __releases(kernel_lock)
{
    int pid;

    kernel_thread(kernel_init, NULL, CLONE_FS | CLONE_SIGHAND);
    numa_default_policy();
    pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
    kthreadd_task = find_task_by_pid(pid);
    unlock_kernel();

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

init 프로세스를 실행하고 `start_kernel()`의 커널을 unlock 한 후 idle 상태로 들어간다.
idle 상태로 들어가도 이미 init 프로세스가 생성된 후기 때문에 상관 없이 커널의 부팅은 진행된다.
idle 프로세스는 0번의 프로세스 번호를 갖는다.

### 6. Why can't we use `printf` instead of `printk` in Linux kernel?

`kernel/printk.c`:

```c
asmlinkage int printk(const char *fmt, ...)
{
    va_list args;
    int r;

    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);

    return r;
}

...

asmlinkage int vprintk(const char *fmt, va_list args)
{
    static int log_level_unknown = 1;
    static char printk_buf[1024];

...

    /*
     * Copy the output into log_buf.  If the caller didn't provide
     * appropriate log level tags, we insert them here
     */
    for (p = printk_buf; *p; p++) {
        if (log_level_unknown) {
                        /* log_level_unknown signals the start of a new line */
            if (printk_time) {
                int loglev_char;
                char tbuf[50], *tp;
                unsigned tlen;
                unsigned long long t;
                unsigned long nanosec_rem;

                /*
                 * force the log level token to be
                 * before the time output.
                 */
                if (p[0] == '<' && p[1] >='0' &&
                   p[1] <= '7' && p[2] == '>') {
                    loglev_char = p[1];
                    p += 3;
                    printed_len -= 3;
                } else {
                    loglev_char = default_message_loglevel
                        + '0';
                }
                t = cpu_clock(printk_cpu);
                nanosec_rem = do_div(t, 1000000000);
                tlen = sprintf(tbuf,
                        "<%c>[%5lu.%06lu] ",
                        loglev_char,
                        (unsigned long)t,
                        nanosec_rem/1000);

                for (tp = tbuf; tp < tbuf + tlen; tp++)
                    emit_log_char(*tp);
                printed_len += tlen;
            } else {
                if (p[0] != '<' || p[1] < '0' ||
                   p[1] > '7' || p[2] != '>') {
                    emit_log_char('<');
                    emit_log_char(default_message_loglevel
                        + '0');
                    emit_log_char('>');
                    printed_len += 3;
                }
            }
            log_level_unknown = 0;
            if (!*p)
                break;
        }
        emit_log_char(*p);
        if (*p == '\n')
            log_level_unknown = 1;
    }

...
}
```

`printk`는 `printf`와 다르게 로그 레벨을 지정할 수 있다.
로그 레벨을 지정하는 이유는 부팅 메세지에서 원하는 정보만 빠르게 확인하기 위해서이다.

또한, 리눅스 커널은 glibc와 같은 Standard C 라이브러리를 사용하지 않기 때문에 `printf`, `open`등의 함수를 사용할 수가 없다.
