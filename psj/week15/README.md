# Final Exam

## Practice

### 1. Create an empty disk, myfd2, as follows. Note "count" value is "2880", not "1440". Create file f1 in myfd2 and write "hi f1" in it. Compute the block number of f1 and show its block content. You have to explain in detail how you have computed f1's block number. Just searching "hi" in Vi as a string will not be accepted; you have to start from superblock to find the location of the group descriptor, and from group descriptor to find the location of inode table, etc.

```bash
$ dd bs=1024 count=2880 if=/dev/zero of=myfd2
$ mkfs -t ext2 myfd2
```

myfd2를 생성했다.

```bash
$ mount -o loop myfd2 temp
$ cd temp
$ echo hi f1 > f1
$ cd
$ umount temp
$ xxd -g1 myfd2 > x
$ vi x
```

myfd2를 temp 디렉토리에 mount하고 f1을 만들어 넣은 다음 myfd2의 구조를 x로 저장했다.

#### Super Block

`m_first_data_block`=0이므로, 한 블록의 크기는 1024*2^0 =1024byte이다.

#### Group Descriptor

Super Block의 다음 블록이 Group Descriptor이다.
따라서 400h+400h=800h로 가서 블록을 확인하면 된다.

DBM, IBM, inode Table의 위치는 각각 14, 15 16번째 블록이다.

#### DBM

DBM은 비트 단위로 블록의 사용 여부를 저장한다.

#### IBM

IBM은 비트 단위로 inode 의 사용 여부를 저장한다.

#### inode Table

inode Table은 16*1024=4000h 위치부터 시작인데, Root Directory는 2번째 inode이므로 80h를 더해야 한다.
따라서 4080h에서 정보를 확인할 수 있다.

Root Directory의 i_block=00 00 00 3d이다.

#### Root Directory

3dh*1024=f400h가 블록 정보이다.

f1의 inode는 00 00 00 0c(=12)이다. 이곳에 f1에 대한 정보가 저장되어 있다.

#### f1

inode Table은 4000h 부터 시작하므로4000h+(12-1)*80h=4580h에 블록이 있다.

i_block=00 4b(=75)이고 이곳에서 파일 안에 기록된 내용을 볼 수 있다.

#### f1 content

f1의 블록 번호는 75번으로, 4bh*1024=12c00h에서 파일 내용을 볼 수 있다.

12c00h으로 이동하면 f1에 저장했던 hi f1를 확인할 수 있다.

[10주차 16번 과제 소스코드](../week10/hw16/main.c)로 myfd2 정보를 열어보면 예상했던 내용들과 같은 결과가 나온다.

### 2. Predict the order of page numbers that will cause page fault for following program. You have to explain in detail why you have made such prediction. Check your prediction by displaying page fault addresses. Indicate which page fault address corresponds to which page number you have predicted. Again, you have to explain in detail why you are making such matching.

`final.c`:

```c
int y[1024];

int main(void) {
    int x;
    for (x = 0; x < 1024; x += 1) {
        y[x] = 0;
    }
    for (;;) {}
    return 0;
}
```

프로그램이 실행되면 최초로 main 함수를 초기화하기 위한 Code와 C Library, Library Loader 영역에서 Page Fault가 수행될 것이다.
그리고 전역변수 y에 접근하기 위해 Data와 Heap영역(y의 크기가 매우 크기 때문에 Heap 영역까지 침범해서 할당됨)에서 Page Fault가 발생할 것이다.
지역변수 x는 Stack에 저장되기 때문에 별도의 Page Fault를 발생시키지 않을 것이다.

`arch/x86/mm/fault.c:do_page_fault()`:

```c
/* get the address */
address = read_cr2();
if (strcmp(tsk->comm, "final") == 0) {
    printk("page fault for final at: %lx\n", address);
}
```

실행 프로그램 이름이 final이면 Page Fault가 일어나는 메모리 주소를 출력하도록 커널을 수정했다.

printk 출력을 보면 가장 804a014가 출력되었고 이것은 main 함수가 호출됨에 따라 Code Segment가 Page Fault 된 것이다.
이어 b7fxxxxx가 연이어 호출된 것은 main 함수의 초기화에서 C Library Code/C Library Data/Library Loader가 호출되었기 때문이다.
중간 중간에도 8048xxx, 8049xxx가 호출되는데 아직 main 함수를 초기화하는 과정이기 때문에 추가적인 main 함수의 코드와 Data Segment를 불러오는 과정에서 Page Fault가 필요하다.

main 함수 초기화가 완료되면 함수를 실행해 b7def1b4부터 반복문을 돌며 Heap 메모리에 접근해(y가 전역 변수이기 때문에) Page Fault를 발생시킨다.

따라서 Page Fault가 발생하는 순서는 Code -> C Library Code/Data/Loader -> Data -> Heap 이다.

| Segment        | Address                              |
| -------------- | ------------------------------------ |
| Code           | 08048000-08049000                    |
| Read-Only Data | 08049000-0804a000                    |
| Data           | 0804a000-0804b000                    |
| Heap           | 0804b000-0804c000, b7e4a000-b7e4b000 |
| C Library Code | b7e4b000-b7f75000                    |
| C Library Data | b7f75000-b7f78000                    |
| Library Loader | b7f80000-b7f9c000                    |
| Stack          | bfc87000-bfc9c000                    |
