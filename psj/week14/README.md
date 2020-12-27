# Memory 2

## Homework

### 7. Make a system call, `sys_get_phyloc()`, which will display the physical address of `main()`.

#### 1) Write a simple program that prints the address of `main()`.

```c
#include <stdio.h>

int main(void) {
    printf("&main: %p\n", main);
    return 0;
}
```

#### 2) Call `sys_get_phyloc(main)` in this program which passes the address of main.

```c
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("&main: %p\n", main);
    syscall(32);
    return 0;
}
```

#### 3) `sys_get_phyloc(addr)` is a system call that performs following steps in order:

`arch/x86/kernel/syscall_table_32.S`:

```s
...
    .long sys_utime          /* 30 */
    .long sys_ni_syscall     /* old gtty syscall holder */
    .long sys_get_phyloc
    .long sys_access
    .long sys_nice
    .long sys_ni_syscall     /* 35 - old ftime syscall holder */
...
```

`mm/mmap.c`:

```c
asmlinkage void sys_get_phyloc(unsigned int addr) {
    printk("PGDIR_SHIFT: %d, PTRS_PER_PGD: %d, PAGE_SHIFT: %d, PTRS_PER_PTE: %d\n", PGDIR_SHIFT, PTRS_PER_PGD, PAGE_SHIFT, PTRS_PER_PTE);

    unsigned int dir = addr >> PGDIR_SHIFT;
    unsigned int pg = (addr >> PAGE_SHIFT) & 0x3ff;
    unsigned int off = addr & 0xfff;
    printk("directory number: %d, page number: %d, offset from address: %d\n", dir, pg, off);

    unsigned int *x = current->mm->pgd;
    printk("location of directory table of the current process: %p\n", x);

    unsigned int *y = &x[dir];
    printk("location of directory table entry: %p\n", y);

    unsigned int pdir = *y & 0xfffff000;
    printk("physical location of the directory: %x\n", pdir);

    unsigned int *vdir = pdir + 0xc0000000;
    printk("virtual location of the directory: %p\n", vdir);

    unsigned int *k = &vdir[pg];
    printk("location of the frame entry: %p\n", k);

    unsigned int pfr = *k & 0xfffff000;
    printk("physical location of frame: %x\n", pfr);

    unsigned int pmain = pfr + off;
    printk("physical address of main: %x\n", pmain);

    unsigned int *vmain = pmain + 0xc0000000;
    printk("virtual address of main: %x\n", vmain);

    printk("first 4 bytes: %x\n", vmain[0]);
}
```

```bash
$ echo 8 > /proc/sys/kernel/printk
$ ./hw07
&main: 0x80483f4
PGDIR_SHIFT: 22, PTRS_PER_PGD: 1024, PAGE_SHIFT: 12, PTRS_PER_PTE: 1024
directory number: 32, page number: 72, offset from address: 1012
location of directory table of the current process: c07e40000
location of directory table entry: c07e40080
physical location of the directory: d8a9000
virtual location of the directory: cd8a9000
location of the frame entry: cd8a9120
physical location of frame: 7e5000
physical address of main: 7e53f4
virtual address of main: c07e53f4
first 4 bytes: 4244c8d
```

##### step0. `printk`를 이용해 `PGDIR_SHIFT`, `PTRS_PER_PGD`, `PAGE_SHIFT`, `PTRS_PER_PTE` 값을 확인했다.

`include/asm-x86/pgtable-2level-defs.h`:

```c
#define PGDIR_SHIFT     22
#define PTRS_PER_PGD    1024
#define PTRS_PER_PTE    1024
```

`include/asm-x86/page.h`:

```c
#define PAGE_SHIFT    12
```

##### step1. extract directory number (dir), page number(pg), and offset(off) from addr, and display them.

가상 주소에서 맨 앞 10자리는 디렉토리 번호, 그 다음 10자리는 페이지 번호, 마지막 12자리는 오프셋이다.
`main`의 가상 주소 0x080483f4(=0000 1000 0000 0100 1000 0011 1111 0100)의 디렉토리 번호는 32(=0000 1000 00), 페이지 번호는 72=(00 0100 1000), 오프셋은 1012(=0011 1111 0100)이다.

##### step2. print the location of directory table of the current process: `x`

`current->mm->pgd`에 현재 프로세스의 디렉토리 테이블이 있다.

##### step3. print the location of directory table entry for `main()`: `y`

`&x[dir]`으로 접근하면 테이블에서의 값을 가져올 수 있다.

##### step4. print the physical location of the directory (partial page table) for `main()`: `pdir`

`*y & 0xfffff000` pgd의 맨 뒤 2자리는 사용하지 않으므로 AND 연산을 이용해 지워준다.

##### step5. print the virtual address of this directory: `vdir`

프로세스는 `pdir`의 맨 앞에 c를 붙여서 가상 메모리 주소에 접근할 수 있다.
출력 결과를 보면 `pdir`과 `vdir`은 0x80만큼 차이가 나는 것을 확인할 수 있다.

##### step6. print the location of the frame entry for `main()`: `k`

물리 메모리 주소에 직접 접근할 수 없으므로 가상 주소를 이용해 접근한다.
`pdir`에 대응하는 가상주소인 `vdir`을 계산하고, 이것을 이용해 `k`를 찾는다.

##### step7. print the physical location of frame for `main()`: `pfr`

`*k & 0xfffff000` pgd의 맨 뒤 2자리는 사용하지 않으므로 AND 연산을 이용해 지워준다.

##### step8. print the physical address of `main()`: `pmain`

`pfr`에서 `off`만큼 더하면 `main` 함수의 물리 주소를 알아낼 수 있다.

##### step9. print the virtual address for the physical address of `main()`: `vmain`

마찬가지로 C에서 주소에 접근해 값을 가져오기 위해서는 가상 주소가 필요하므로 주소 맨 앞에 c를 붙인다.

##### step10. display the first 4 bytes in it and compare them with the first 4 bytes of main in the original executable file(use "objdump -d program-name" to see the first 4 bytes of main in the original program). If they are same, you have the correct physical address of main.

```bash
$ objdump -d ./hw07
...
080483f4 <main>:
 80483f4:       8d 4c 24 04        lea    0x4(%esp),%ecx
 80483f8:       83 e4 f0           and    $0xfffffff0,%esp
...
```

`main`의 첫 주소와 objdump로 출력한 주소 값이 같은 것을 확인할 수 있다.
