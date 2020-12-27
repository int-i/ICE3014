# Interrupt 1

## Homework

### 1. Following events will cause interrupts in the system. What interrupt number will be assigned to each event? For system call interrupt, also give the system call number.

- A packet has arrived: 42(Network Interface)
- An application program calls `scanf`: 128(System Call 3)
- A key is pressed: 33(Keyboard)
- An application causes a divide-by-zero error: 0(Exception divide-by-zero error)
- An application program calls printf(): 128(System Call 4)
- An application causes a page-fault error: 14(Exception page fault)

### 2. Change `drivers/input/keyboard/atkbd.c` as follows.

`drivers/input/keyboard/atkbd.c`:

```c
static irqreturn_t atkbd_interrupt(...)
{
   return IRQ_HANDLED;  // Add this at the first line
...
```

Recompile the kernel and reboot with it.
What happens and why does this happen? Show the sequence of events that happen when you hit a key in a normal Linux kernel (as detail as possible): hit a key => keyboard controller sends a signal through IRQ line 1 => ......etc.
Now with the changed Linux kernel show which step in this sequence has been modified and prevents the kernel to display the pressed key in the monitor.

```bash
$ make bzImage
$ cp arch/x86/boot/bzImage /boot/bzImage2
$ reboot
```

지난 주에 만든 My Linux2로 접속해보았다.
부팅이 끝나면 로그인 입력이 출력되지만, 키보드를 사용해도 아무런 입력이 되지 않는다.

원래 코드는 인터럽트 발생 후 키보드 입력을 처리하고 `IRQ_HANDLED`를 반환한다.
하지만, 어떤 처리도 없이 `IRQ_HANDLED`를 반환하게 코드를 수정했기 때문에 키보드로 타이핑해도 문자 입력이 처리되지 않는다.
따라서 위와 같이 입력한 어떤 문자도 화면에 보이지 않는다.

### 3. Change the kernel such that it prints "x pressed" for each key pressing, where x is the scan code of the key. After you change the kernel and reboot it, do followings to see the effect of your changing.

`drivers/input/keyboard/atkbd.c`:

```c
static irqreturn_t atkbd_interrupt(struct serio *serio, unsigned char data,
            unsigned int flags)
{
    struct atkbd *atkbd = serio_get_drvdata(serio);
    struct input_dev *dev = atkbd->dev;
    unsigned int code = data;
    printk("%x pressed\n", code);
    int scroll = 0, hscroll = 0, click = -1;
    int value;
    unsigned char keycode;
...
```

`code`를 `printk`로 출력한다.
컴파일하고 재부팅해 새로운 리눅스 커널을 적용한다.

```bash
$ cat /proc/sys/kernel/printk
1   4   1   7
```

위는 현재의 콘솔 로그 레벨, 기본 로그 레벨, 최소 로그 레렐, 최대 로그 레벨을 나타낸다.
현재는 1로 기본 레벨보다 낮기 때문에 `printk`로 출력되는 문자들이 화면에 나타나지 않는다.

```bash
$ echo 8 > /proc/sys/kernel/printk
```

위 명령으로 현재 콘솔 로그 레벨을 8로 바꾸면 아래와 같이 레벨이 바뀐다.

```bash
$ cat /proc/sys/kernel/printk
8   4   1   7
```

현재는 레벨이 8이기 때문에 `printk`로 출력되는 문자들이 화면에 보인다.
위와 같이 입력되는 키 코드가 화면에 보인다.

```bash
$ echo 1 > /proc/sys/kernel/printk
```

`printk` 출력을 다시 안보이게 하기 위해서는 현재 로그 레벨을 1로 되돌리면 된다.

### 4. Change the kernel such that it displays the next character in the keyboard scancode table. For example, when you type "root", the monitor would display "tppy". How can you log in as root with this kernel?

`drivers/input/keyboard/atkbd.c`:

```c
static irqreturn_t atkbd_interrupt(struct serio *serio, unsigned char data,
            unsigned int flags)
{
    struct atkbd *atkbd = serio_get_drvdata(serio);
    struct input_dev *dev = atkbd->dev;
    unsigned int code = data + 1;
    int scroll = 0, hscroll = 0, click = -1;
    int value;
    unsigned char keycode;
...
```

스캔코드 테이블에서 r의 다음 글자는 t이고, o의 다음 글자는 p이다.
즉, 키보드 입력이 들어오면 실제 입력한 글자의 다음 글자를 입력한 것으로 처리하면 된다.

재부팅하고 로그인을 하기 위해 "root"를 입력하면 한 글자씩 밀린 "tppy"가 출력된다.

### 5. Define a function `mydelay` in `init/main.c` which whenever called will stop the booting process until you hit 's'. Call this function after `do_basic_setup` function call in `kernel_init` in order to make the kernel stop and wait for 's' during the booting process. You need to modify `atkbd.c` such that it changes `exit_mydelay` to 1 when the user presses 's'.

`init/main.c`:

```c
int exit_mydelay; // define a global variable

void mydelay(char *str)
{
    printk(str);
    printk("enter s to continue\n");
    exit_mydelay = 0; // init to zero
    for (;;) {        // and wait here until the user press 's'
        msleep(1); // sleep 1 micro-second so that keyboard interrupt ISR can do its job
        if (exit_mydelay == 1)
            break; // if the user press 's', break
    }
}

static int __init kernel_init(void * unused)
{
...
    do_basic_setup();
    mydelay("after do_basic_setup in kernel_init\n"); // wait here
...
}
```

`mydelay`는 `exit_mydelay`가 1이 될 때까지 무한루프를 돌며 함수의 실행을 블록하는 함수이다.
`do_basic_setup` 함수 실행이 끝나면 `mydelay`를 호출해 부팅과정을 일시정지한다.

`drivers/input/keyboard/atkbd.c`:

```c
extern int exit_mydelay; // declare as extern since it is defined in main.c

static irqreturn_t atkbd_interrupt(...)
{
...
    // detect 's' key pressed and change exit_mydelay
    if (code == 31) {
        printk("s pressed\n");
        exit_mydelay = 1;
    }
...
}
```

's'가 입력될 때 `code` 값은 31이다.
만약 s가 입력되면 `extern`으로 가져온 `exit_mydelay` 변수를 1로 바꿔 `mydelay`의 무한루프를 종료한다.

커널을 적용하게 컴퓨터를 다시 부팅하니 s가 입력될 때까지 잠시 부팅과정을 멈췄다가, s가 입력되면 이어서 실행한다.

### 6. Which function call in `atkbd_interrupt` actually displays the pressed key in the monitor?

`atkbd_interrupt`에서 새로운 입력 이벤트를 보고하는 `input_event`를 호출한다.
이 함수에 디바이스, 이벤트 종류, 코드, 값을 넘긴다.  

`drivers/input/input.c`:

```c
void input_event(struct input_dev *dev,
         unsigned int type, unsigned int code, int value)
{
    unsigned long flags;

    if (is_event_supported(type, dev->evbit, EV_MAX)) {

        spin_lock_irqsave(&dev->event_lock, flags);
        add_input_randomness(type, code, value);
        input_handle_event(dev, type, code, value);
        spin_unlock_irqrestore(&dev->event_lock, flags);
    }
}
```

`input_event`는 이벤트를 지원하는지 확인하고 `input_handle_event`를 호출한다.

`drivers/input/input.c`:

```c
static void input_handle_event(struct input_dev *dev,
                   unsigned int type, unsigned int code, int value)
{
    int disposition = INPUT_IGNORE_EVENT;

...

    if (disposition & INPUT_PASS_TO_HANDLERS)
        input_pass_event(dev, type, code, value);
}
```

`input_handle_event`는 이벤트가 `INPUT_PASS_TO_HANDLERS`하다면 핸들러로 이벤트를 전달하는 `input_pass_event`를 호출한다.

`drivers/input/input.c`:

```c
static void input_pass_event(struct input_dev *dev,
                 unsigned int type, unsigned int code, int value)
{
    struct input_handle *handle;

    rcu_read_lock();

    handle = rcu_dereference(dev->grab);
    if (handle)
        handle->handler->event(handle, type, code, value);
    else
        list_for_each_entry_rcu(handle, &dev->h_list, d_node)
            if (handle->open)
                handle->handler->event(handle,
                            type, code, value);
    rcu_read_unlock();
}
```

마지막으로 `input_pass_event`에서 `handle->handler->event`를 호출한다.

`drivers/input/input.c`:

```c
struct input_handle {

    void *private;

    int open;
    const char *name;

    struct input_dev *dev;
    struct input_handler *handler;

    struct list_head    d_node;
    struct list_head    h_node;
};

struct input_handler {
    void *private;

    void (*event)(struct input_handle *handle, unsigned int type, unsigned int code, int value);
    int (*connect)(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id);
    void (*disconnect)(struct input_handle *handle);
    void (*start)(struct input_handle *handle);

    const struct file_operations *fops;
    int minor;
    const char *name;

    const struct input_device_id *id_table;
    const struct input_device_id *blacklist;

    struct list_head    h_list;
    struct list_head    node;
};
```

이때, `input_pass_event`에 정의된 `struct input_handle *handle`는 입력 장치를 입력 처리 핸들러에 연결시켜주는 구조체이며 `struct input_handler *handler`를 속성으로 가지고 있다.
최종적으로 호출되는 `struct input_handler`의 `event`는 실질적인 이벤트 핸들 함수이며 인터럽트를 비활성화 시키고 락을 풀어준다.

### 6-1. What are the interrupt numbers for divide-by-zero exception, keyboard interrupt, and "read" system call? Where is ISR1 and ISR2 for each of them(write the exact code location)? Show their code, too.

| -                        | Interrupt Number | ISR1         | ISR2            |
| ------------------------ | ---------------- | ------------ | --------------- |
| divide-by-zero exception | 0                | divide_error | do_divide_error |
| keyboard interrupt       | 33               | interrupt[1] | atkbd_interrupt |
| read system call         | 128              | system_call  | sys_read        |

divide-by-zero exception은 인터럽트로 0번을 가지며 ISR1은 `divide_error`, ISR2는 `do_divide_error`이다.

`arch/x86/kernel/traps_32.c`:

```c
void __init trap_init(void)
{
    int i;

...

    set_trap_gate(0,&divide_error);
...
```

`arch/x86/kernel/traps_32.c`의 `trap_init`은 `divide_error`를 0으로 등록한다.

`arch/x86/kernel/entry_32.S`:

```s
ENTRY(divide_error)
    RING0_INT_FRAME
    pushl $0            # no error code
    CFI_ADJUST_CFA_OFFSET 4
    pushl $do_divide_error
    CFI_ADJUST_CFA_OFFSET 4
    jmp error_code
    CFI_ENDPROC
END(divide_error)
```

`arch/x86/kernel/entry_32.S`의 `ENTRY(divide_error)`에서 `do_divide_error`를 호출한다.

`arch/x86/kernel/traps_32.c`:

```c
#define DO_VM86_ERROR_INFO(trapnr, signr, str, name, sicode, siaddr) \
void do_##name(struct pt_regs * regs, long error_code) \
{ \
    siginfo_t info; \
    info.si_signo = signr; \
    info.si_errno = 0; \
    info.si_code = sicode; \
    info.si_addr = (void __user *)siaddr; \
    trace_hardirqs_fixup(); \
    if (notify_die(DIE_TRAP, str, regs, error_code, trapnr, signr) \
                        == NOTIFY_STOP) \
        return; \
    do_trap(trapnr, signr, str, 1, regs, error_code, &info); \
}

DO_VM86_ERROR_INFO( 0, SIGFPE,  "divide error", divide_error, FPE_INTDIV, regs->ip)
```

원래는 `arch/x86/kernel/traps_32.c`에서 `do_divide_error`가 컴파일 타임에 생성되지만, 주어진 소스코드에는 나타나지 않으므로 함수의 선언 매크로를 가져왔다.

키보드 인터럽트의 경우, 키를 입력하면 33번 인터럽트가 발생하며 이는 `arch/x86/kernel/entry_32.S`의 `interrupt[1]`으로 전달된다.

`arch/x86/kernel/entry_32.S`:

```s
.section .rodata,"a"
ENTRY(interrupt)
.text

ENTRY(irq_entries_start)
    RING0_INT_FRAME
vector=0
.rept NR_IRQS
    ALIGN
 .if vector
    CFI_ADJUST_CFA_OFFSET -4
 .endif
1:  pushl $~(vector)
    CFI_ADJUST_CFA_OFFSET 4
    jmp common_interrupt
 .previous
    .long 1b
 .text
vector=vector+1
.endr
END(irq_entries_start)

.previous
END(interrupt)
.previous
```

생성된 인터럽트들은 `arch/x86/kernel/i8259_32.c`의 `native_init_IRQ`와 `arch/x86/kernel/traps_32.c`의 `trap_init`에서 할당된다.

`native_init_IRQ` in `arch/x86/kernel/i8259_32.c`:

```c
for (i = 0; i < (NR_VECTORS - FIRST_EXTERNAL_VECTOR); i++) {
    int vector = FIRST_EXTERNAL_VECTOR + i;
    if (i >= NR_IRQS)
        break;
    /* SYSCALL_VECTOR was reserved in trap_init. */
    if (!test_bit(vector, used_vectors))
        set_intr_gate(vector, interrupt[i]);
}
```

하드웨어 드라이버들은 `request_irq`를 호출해 ISR2는 `irq_desc` 테이블에 저장한다.
ISR1은 직접 ISR2를 호출하며 키보드 인터럽트의 ISR2는 `atkbd_interrupt`이다.

`drivers/input/keyboard/atkbd.c`:

```c
static irqreturn_t atkbd_interrupt(struct serio *serio, unsigned char data,
            unsigned int flags)
{
    struct atkbd *atkbd = serio_get_drvdata(serio);
    struct input_dev *dev = atkbd->dev;
    unsigned int code = data;
    int scroll = 0, hscroll = 0, click = -1;
    int value;
    unsigned char keycode;
```

시스템 콜의 인터럽트 번호는 128이다.
모든 시스템 콜의 ISR2는 `arch/x86/kernel/syscall_table_32.S` 파일에 작성되어 있다.

`arch/x86/kernel/syscall_table_32.S`:

```s
ENTRY(sys_call_table)
    .long sys_restart_syscall    /* 0 - old "setup()" system call, used for restarting */
    .long sys_exit
    .long sys_fork
    .long sys_read
    .long sys_write
    .long sys_open        /* 5 */
    .long sys_close
    .long sys_waitpid
    .long sys_creat
    .long sys_link
    .long sys_unlink    /* 10 */
...
```

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

`sys_read`의 시스템 콜 번호는 3번이며 `sys_call_table`을 거쳐 최종적으로 호출된다.
