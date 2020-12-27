# Memory 1

## Homework

### 1

#### 1) Draw the memory map (process image) of the following program (hw01_01.c). What are the starting addresses of the code, data, heap, stack segment of this program and how many pages each segment occupies? What is the address of main function, the addresses of the global variables and local variables?

See [HW 1-1](./hw01_01/main.c)

`printf`의 출력 결과는 Logical 주소를 의미하는 것이기 때문에 실제 메모리에 저장되는 위치를 알기 위해서는 Process Table을 열어 Physical 주소를 확인해야 한다.

| Segment        | Address                              | Identifier       |
| -------------- | ------------------------------------ | ---------------- |
| Code           | 08048000-08049000                    | `&main`          |
| Read-Only Data | 08049000-0804a000                    |                  |
| Data           | 0804a000-0804b000                    | `&x`, `&y`       |
| Heap           | 0804b000-08075000, b7e22000-b7e23000 | `&y[9999]`, `pk` |
| C Library Code | b7e23000-b7f4d000                    |                  |
| C Library Data | b7f4d000-b7f50000                    |                  |
| Library Loader | b7f58000-b7f74000                    |                  |
| Stack          | bfc5f000-bfc74000                    | `&k`, `&pk`      |

main 함수는 Code 영역, 전역변수는 Data 영역, 매우 큰 일부 전역변수와 동적할당(`malloc`)된 메모리는 Heap 영역, 지역변수는 Stack영역에 할당되는 것을 확인했다.
`&y[9999]`는 전역변수로써 Data에 할당되는 것이 맞지만 배열 `y`의 크기가 크기 때문에 Heap 영역까지 침범해 할당되었다.

#### 2) Write another simple program, hw01_02.c(see below), and run hw01_01, hw01_02 at the same time. Confirm they have the same address for main function. How can they run at the same location at the same time?

See [HW 1-2](./hw01_02/main.c)

두 프로그램을 동시에 실행하면 같은 메모리 주소를 출력한다.
이는 프로세스 상의 주소는 Logical 메모리의 주소이기 때문이며 각 프로세스 구조의 같은 부분에 메모리를 할당했기 때문에 발생한다.
프로그램이 실행되면 Logical 메모리 주소는 물리적 메모리의 빈 부분에 할당되며 Physical 메모리 주소로 변환된다.
이때 두 프로그램의 Physical 메모리 주소는 서로 다를 것이다.

### 2. Show the memory map of the following program. Which pages does the program access during the run time? Show the page numbers that the program accesses in the order they are accessed. Indicate which pages are for code and which are for global data and which are for local data.

See [HW 2](./hw02/main.c)

| Segment | Page        | Identifier    |
| ------- | ----------- | ------------- |
| Code    | 8048-8049   | `&main`       |
| Data    | 804a-804b   | `&A`          |
| Heap    | 804b-8050   | `&A[4][1024]` |
| Stack   | bfc5d-bfc72 | `&i`, `&j`    |

기본적으로 Page의 크기는 4KB이기 때문에 "/proc/pid/maps"에서 0x1000당 한 Page라고 볼 수 있다.
그렇기 때문에 시작과 끝 메모리 주소에서 뒤 3자리를 제하면 Page 번호를 구할 수 있다.
프로그램은 Code(`main`) -> Data(전역변수) -> Heap(전역변수) -> Stack(지역변수) 순으로 접근한다.

### 3. How many page faults will the program in hw 2) generate? Explain your reasoning. Remember in the beginning the system has no page of the current process in the memory.

최초로 main 함수를 실행하기 위해 Code 영역의 Page를 불러올 것이다.
그 다음 전역변수가 위치한 Data와 Heap의 804a부터 804f까지의 6 Page를 불러올 것이다
프로그램이 Stack에 접근하면서는 bfc6f Page를 읽어온다.
따라서 hw02 프로그램에 의한 Page Fault는 8번이며, `printf` 등의 C 라이브러리도 고려한다면 더 많은 Page Fault가 발생할 것이다.

### 4. Confirm your answer in hw 3) by defining a new system call, `sys_show_pfcnt()`, in "mm/mmap.c", which displays the number of page faults generated so far.

The `pfcnt` should be increased by one whenever there is a page fault.
Remember a page fault will raise INT 14 which will be processed by `page_fault()` in "arch/x86/kernel/entry_32.S", which in turn calls `do_page_fault()` in "arch/x86/mm/fault.c".
Define `int pfcnt=0` in this file and increase it inside `do_page_fault()`.
Now you call this system call before and after the double loop in hw 2) and see the difference.

`arch/x86/kernel/syscall_table_32.S`:

```s
...
    .long sys_utime          /* 30 */
    .long sys_show_pfcnt
    .long sys_ni_syscall     /* old gtty syscall holder */
    .long sys_access
    .long sys_nice
    .long sys_ni_syscall     /* 35 - old ftime syscall holder */
...
```

`mm/mmap.c`:

```c
...
extern int pfcnt;

asmlinkage void sys_show_pfcnt(void) {
    printk("page fault count so far: %d\n", pfcnt);
}
...
```

`arch/x86/mm/fault.c`:

```c
...
int pfcnt = 0;

/*
 * This routine handles page faults.  It determines the address,
 * and the problem, and then passes it off to one of the appropriate
 * routines.
 */
#ifdef CONFIG_X86_64
asmlinkage
#endif
void __kprobes do_page_fault(struct pt_regs *regs, unsigned long error_code)
{
    pfcnt += 1;
...
```

`do_page_fault` 함수가 호출되는 회수를 기록하는 변수를 선언하고, 변수 값을 출력하는 31번 시스템 콜을 생성했다.

See [HW 4](./hw04/main.c)

이렇게 코드를 작성하면 두 시스템 콜 출력의 차이를 이용해 `syscall` 함수 사이에서 발생한 Page Fault 회수를 알아낼 수 있다.
위는 반복문에서 발생한 Page Fault를 알아내기 위한 코드이다.

```bash
$ echo 8 > /proc/sys/kernel/printk
$ ./hw04 &
&main: 0x80483f4, &A: 0x804a040, &A[4][1024]: 0x804f040, &i: 0xbfc95b90, &j: 0xbfc95b8c
page fault count so far: 632889
page fault count so far: 632894
```

632894-632889=5

`syscall` 사이의 반복문에서 총 5번의 Page Fault가 추가적으로 발생했다.
전역변수를 불러오기 위해 `A[0]`가 위치한 804a Page를 제외하면 `A[4][1024]`까지 데이터를 불러오기 위해서는 804f까지의 5번의 Page Fault가 필요하다.
따라서 프로그램의 출력 결과가 잘 나왔다.

#### 1) You can display the exact address where page fault has happend. Make hw04_01.c and insert following code in `arch/x86/mm/fault.c:do_page_fault()`. When you run hw04_01, the kernel will display the page fault addresses generated by hw04_01. Explain the result.

`arch/x86/mm/fault.c:do_page_fault()`:

```c
/* get the address */
address = read_cr2();
if (strcmp(tsk->comm, "hw04_01") == 0) {
    printk("page fault for hw04_01 at: %lx\n", address);
}
```

See [HW 4-1](./hw04_01/main.c)

`do_page_fault` 함수에 현재 실행하는 프로그램 이름이 "hw04_01"이면 Page Fault 주소를 출력하도록 커널 코드를 수정했다.
프로그램이 시작되면 Code(804a) Page를 먼저 읽고, libc 등의 라이브러리가 저장된 페이지를 읽는다.
그리고 Heap 영역을 메모리에 올린다.
프로그램의 코드에 비해 많은 Page Fault가 생성된 이유는 main함수가 초기화되어 호출되기까지의 메모리 접근이 포함되었기 때문이다.

#### 2) Repeat hw4) with modified hw2) code as below. Why is `pfcnt` increased?

See [HW 4-2](./hw04_02/main.c)

5에서 620108-620083=25로 값이 증가한 이유는 `printf` 호출에서 추가적인 Page Fault가 일어났기 때문이다.

### 5. Make a system call that prints vma information of the current process, and write a user program that displays the VMA list with it. Confirm that this result matches to those in "/proc/xxxx/maps".

`arch/x86/kernel/syscall_table_32.S`:

```s
...
    .long sys_utime          /* 30 */
    .long sys_get_vma_list
    .long sys_ni_syscall     /* old gtty syscall holder */
    .long sys_access
    .long sys_nice
    .long sys_ni_syscall     /* 35 - old ftime syscall holder */
...
```

`mm/mmap.c`:

```c
asmlinkage void sys_get_vma_list(void) {
    struct vm_area_struct *temp = current->mm->mmap;
    for (;;) {
        if (temp == NULL) {
            break;
        }
        if (temp->vm_file == NULL) {
            printk("vm_start: %lx, vm_end: %lx\n", temp->vm_start, temp->vm_end);
        } else {
            printk("vm_start: %lx, vm_end: %lx, name: %s\n", temp->vm_start, temp->vm_end, temp->vm_file->f_dentry->dname.name);
        }
        temp = temp->vm_next;
    }
}
```

See [HW 5](./hw05/main.c)

VMA List는 Linked List 형태로 구현되었기 때문에 `temp = temp->vm_next`로 다음 값을 지정하며 `NULL`이 될 때까지 이를 반복한다.
`vm_file` 값이 `NULL`일 경우에는 name을 출력하지 않는다.

출력 결과를 보면 vDSO부분을 제외하고 "/proc/pid/maps"와 같은 결과를 보여주는 것을 확인할 수 있었다.

### 6. Count the number of page faults when you run following ex01 and ex02 by using `sys_show_pfcnt()`. Explain the results. Also compare the running time of each code (use `gettimeofday()` function) and explain why they differ. Run several times and compute the average.

See [HW 6](./hw06/)

| Program | Page Fault | Elapsed Time |
| ------- | ---------- | ------------ |
| ex01    | 65,539     | 3.01         |
| ex02    | 74,113     | 4.08         |

ex01과 ex02의 차이점은 `A[i][j]`와 `A[j][i]`이다.
ex02에서 더 많은 Page Fault가 발생했는데, ex01은 `A`에 메모리 적재된 순서대로 접근한 반면 ex02는 순서대로 접근하지 않았기(참조의 지역성을 만족하지 않았기) 때문에 LRU 알고리즘에 의해 사용하지 않은 Page를 삭제하고 메모리에서 내려간 Page를 새로 읽어오는 작업이 반복되어 더 많은 Page Fault가 일어났다.

Page Fault 과정은 발생한 회수만큼 프로그램의 성능을 저하시키므로 ex02이 ex01보다 느리게 실행을 끝마쳤다.
