# File System 2

## Homework

### 9. Read the inode table and find the block location of the root directory file. What is the byte size and block size of this file? Who is the owner of this file?

`include/linux/ext2_fs.h`:

```c
struct ext2_inode {
    __le16 i_mode;        /* File mode */
    __le16 i_uid;         /* Low 16 bits of Owner Uid */
    __le32 i_size;        /* Size in bytes */
    __le32 i_atime;       /* Access time */
    __le32 i_ctime;       /* Creation time */
    __le32 i_mtime;       /* Modification time */
    __le32 i_dtime;       /* Deletion Time */
    __le16 i_gid;         /* Low 16 bits of Group Id */
    __le16 i_links_count; /* Links count */
    __le32 i_blocks;      /* Blocks count */
    __le32 i_flags;       /* File flags */
    union {
        struct {
            __le32 l_i_reserved1;
        } linux1;
        struct {
            __le32 h_i_translator;
        } hurd1;
        struct {
            __le32 m_i_reserved1;
        } masix1;
    } osd1;                        /* OS dependent 1 */
    __le32 i_block[EXT2_N_BLOCKS]; /* Pointers to blocks */
    ...
};
```

inode의 크기는 80이며 0x2800에 첫 번째 inode가 위치하기 때문에 root directory file은 0x2880에 있는 두 번째 inode를 확인하면 된다.
00 00 00 21은 10진수 33으로 root directory file의 block location은 33이다.

### 10. Read the root directory file. How many member files it has? What are the inode numbers and file names of them? Go to the inode table and find the block location of each member file.

`include/linux/ext2_fs.h`:

```c
struct ext2_dir_entry_2 {
    __le32 inode;   /* Inode number */
    __le16 rec_len; /* Directory entry length */
    __u8 name_len;  /* Name length */
    __u8 file_type;
    char name[EXT2_NAME_LEN]; /* File name */
};
```

`33 * 1024 = 33792 = 0x8400`이다.

`inode`는 00 00 00 02(=2)로 root directory file은 2번 inode에 속해있다.
`rec_len`은 00 0c(=12), `name_len`은 01(=1), `file_type`은 02(=2: directory file), 그리고 `name`은 00 00 00 2e(=".")이다.
`name_len`이 1이지만 00으로 채워진 이유는 구조체의 정렬에 의한 패딩으로 생각된다.

그 다음 `inode`는 00 00 00 02(=2)로 2번 inode를 의미하고.
`rec_len`은 00 0c(=12), `name_len`은 02(=2), `file_type`은 02(=2:), 그리고 `name`은 00 00 2e 2e(="..")이다.

그 다음 `inode`는 00 00 00 0b(=11)로 11번 inode를 의미하고.
`rec_len`은 00 14(=20), `name_len`은 0a(=10), `file_type`은 02(=2), 그리고 `name`은 00 00 6c 6f 73 74 2b 66 6f 75 6e 64(="lost+found")이다.

그 다음 `inode`는 00 00 00 0c(=12)로 12번 inode를 의미하고.
`rec_len`은 03 d4(=980), `name_len`은 02(=2), `file_type`은 01(=1: regular file), 그리고 `name`은 00 00 66 31(="f1")이다.

총 4개의 파일이 존재한다.

### 11. Read the member file and confirm the contents.

`inode`가 12일 때 블록은 `0x2800 + 0x80 * (inode - 1) = 0x2d80`에 있다.

`i_block`은 00 2f(=47)로 실질적인 내용을 담고 있는 블록은 `1024 * 0x002f = 0xbc00`에 있다.

0xbc00로 가서 블록을 보면 "f1"에 저장해둔 "korea"가 보인다.

### 12. You can see all files including hidden ones with `ls -a`. Confirm you can see all files you found in the file system with this command.

```bash
$ ls -a
```

### 13. You can see the inode number of a file with `ls -i`. Confirm the inode numbers of all files.

```bash
$ ls -ai
```

### 14. Make another file in your virtual disk. Confirm the changes in the file system: IBM, DBM, Inode table, and root directory file. Now delete this file (with `rm` command). What happens to the file system? How can you recover this file?

#### 1) Make a new directory("d7") in the root directory with `mkdir` command. Show the disk block content of the root directory file and find out the inode number of "d7".

```bash
$ mount -o loop myfd temp
$ cd temp
$ mkdir d7
$ cd ..
$ umount temp
$ xxd -g1 myfd > y
$ vi y
```

Root directory file의 block이 있는 0x8400으로 가서 "d7"에 대한 정보를 볼 수 있다.

"d7"의 `inode`는 00 00 00 0d(=13)로 13번 inode를 의미하고. `rec_len`은 03 c8(=968), `name_len`은 02(=2), `file_type`은 02(=2), 그리고 `name`은 00 00 64 37(="d7")이다.

총 5개의 파일이 존재한다.

#### 2) Show the inode content of "d7". What is the block location of "d7"? Show the block content of "d7". What files do you have in "d7"?

`inode`가 13일 때 블록은 `0x2800 + 0x80 * (inode - 1) = 0x2e00`에 있다.

`i_block`은 00 30(=48)로 실질적인 내용을 담고 있는 블록은 `1024 * 0x0030 = 0xc000`에 있다.

#### 3) Run `mv f1 d7/f2` and show the changes in the root directory file, "d7" file, and inode table.

```bash
$ mount -o loop myfd temp
$ cd temp
$ mv f1 d7/f2
$ cd ..
$ umount temp
$ xxd -g1 myfd > z
$ vi z
```

Root directory file block 에서 f1에 대한 내용이 수정되었다.
`inode`가 00 00 00 00(=0)으로 수정되어 있고 이것은 파일이 삭제되었음을 의미한다.

기존의 0x0002d80으로 가서 파일을 확인하면 블록 자체는 그대로 남아있다.

"d7"의 file block 으로 가면 "f2"를 확인할 수 있다.
`inode`를 확인해보면 00 00 00 0f(=15)로 0x0002e80에 블록이 있다

`i_block`은 04 19(=1049)로 파일의 내용은 `1024 + 0x0419 = 0x10640`에 있다

### 15. Examine the file system in the hard disk("/dev/sda3") and find file names in the root directory.

```bash
$ dd bs=1024 count=8000 if=/dev/sda3 of=myhd
$ xxd –g1 myhd > x
$ vi x
```

Superblock 이다.
`m_nodes_count`는 00 0e da 80(=973440)이고 `m_blocks_count`는 00 1d a9 39(=1943865)이다.
`m_first_data_block`은 00 00 00 00(=0)이고 `m_log_block_size`은 00 00 00 02(=2)로 블록 사이즈는 `1024 * 2^2 = 4096B = 4KB`이다.

한 블록당 0x1000 만큼 차지하기 때문에 Group descriptor 는 0x0001000에 위치한다. DBM 은 00 00 01 dc(=476)에서 시작하고 IBM 은 00 00 01 dd(=477)에서 시작한다.
inode table은 00 00 01 de(=478)에서 시작한다.

Root file directory 의 inode table은 `4096 * 0x01de = 0x01de000`에 있다.

### 16. Write a program that opens a disk device formatted with ext2 and reads and displays the super block, group descriptor, ibm, dbm, and inode table. Also display the file names in the root directory file, their inode numbers, and their block locations. Use `open()`, `lseek()`, `read()`, etc.

See [HW 16](./hw16/main.c)
