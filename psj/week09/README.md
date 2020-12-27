# File System 1

## Homework

### 1. Read the disk and analyze the contents of the meta blocks.

Make a virtual floppy disk of size 1.44MB with name "myfd":

```bash
$ dd bs=1024 count=1440 if=/dev/zero of=myfd
```

`dd` 블록 단위로 파일을 복사하거나 변환하는 명령이다.
`bs`는 한 번에 처리할 데이터의 크기를 의미한다.
`count`는 블록의 갯수를 의미한다.
1440 * 1024byte = 1440KB = 1.44MB이다.

`if`는 읽을 파일을 의미하며 `of`는 쓸 파일을 의미한다.
`/dev/zero`는 무수히 많은 0으로 채워진 파일을 의미한다.

### 2. Format

```bash
$ mkfs -t ext2 myfd
```

`mkfs`는 디스크를 포맷할 때 사용하는 명령이다.
`t`는 파일 시스템의 형식을 지정하는 옵션으로 `-t ext2`는 파일 시스템을 ext2로 포맷한다.

### 3. Mount

```bash
$ mkdir temp
```

"temp"라는 이름으로 빈 디렉토리를 생성한다.

```bash
$ mount -o loop myfd temp
```

"myfd"를 "temp"에 연결한다.
Loop device는 파일을 블럭 디바이스처럼 엑세스 할 수 있게 해주는 가상의 장치로, "myfd"는 가상 메모리이기 때문에 `–o loop` 옵션을 줘야 한다.

### 4. Make some files

```bash
$ cd temp
$ echo korea > f1
```

"temp"가서 "korea"란 내용의 "f1" 파일을 생성한다.

### 5. Read

```bash
$ cd ..
$ umount temp
```

"temp"와 "myfd"의 연결을 끊는다.

```bash
$ xxd -g1 myfd > x
$ vi x
```

"myfd"를 바이트 단위로 읽어서 "x"에 저장한다.
`-g1`은 출력에서 group으로 묶이는 바이트의 개수를 1개로 설정하는 옵션이다.

### 6. Read superblock. Superblock starts at offset 1024(400h). Find the superblock and read the total number of inodes in this disk(`m_inodes_count`), total number of blocks in this disk(`m_blocks_count`), the magic number(`m_magic`), block size(1024 * 2^`m_log_block_size`), and the first data block number(`m_first_data_block`). Every multi-byte number is stored in little endian, and you have to reverse the byte order to compute the actual value.

`include/linux/ext2_fs.h`:

```c
struct ext2_super_block {
    __le32 s_inodes_count;      /* Inodes count */
    __le32 s_blocks_count;      /* Blocks count */
    __le32 s_r_blocks_count;    /* Reserved blocks count */
    __le32 s_free_blocks_count; /* Free blocks count */
    __le32 s_free_inodes_count; /* Free inodes count */
    __le32 s_first_data_block;  /* First Data Block */
    __le32 s_log_block_size;    /* Block size */
    __le32 s_log_frag_size;     /* Fragment size */
    __le32 s_blocks_per_group;  /* # Blocks per group */
    __le32 s_frags_per_group;   /* # Fragments per group */
    __le32 s_inodes_per_group;  /* # Inodes per group */
    __le32 s_mtime;             /* Mount time */
    __le32 s_wtime;             /* Write time */
    __le16 s_mnt_count;         /* Mount count */
    __le16 s_max_mnt_count;     /* Maximal mount count */
    __le16 s_magic;             /* Magic signature */
    ...
```

ext2의 Superblock 리눅스 커널 코드를 가져온 것이다.

Superblock은 첫 번째 8바이트 00 00 00 b8(=184)으로 `m_inodes_count`을 의미한다.
`m_blocks_count`은 두 번째 8바이트 00 00 05 a0(=1440)으로, 1번에서 `count=1440` 옵션으로 설정했던 1440개의 블록을 의미한다.
`m_magic`은 매직넘버로 리눅스의 파일 시스템임을 나타낸다.
블록 사이즈는 1024 * 2^`m_log_block_size`으로 구할 수 있는데, `m_log_block_size`는 00 00 00 00(=0)으로 한 블록 당 1KB임을 의미한다.
`m_first_data_block`은 00 00 00 01(=1)으로 Superblock인 첫 번째 블록을 의미한다.

### 7. Read the group descriptor and find the block number of DBM, IBM, and inode table. The group descriptor is located in the next block after the superblock. If the superblock is at block 0, the group descriptor will start at block 1, if the superblock is at block 1, the group descriptor start at block 2, etc. The address of block x will be `x * block_size`.

`include/linux/ext2_fs.h`:

```c
struct ext2_group_desc {
    __le32 bg_block_bitmap;      /* Blocks bitmap block */
    __le32 bg_inode_bitmap;      /* Inodes bitmap block */
    __le32 bg_inode_table;       /* Inodes table block */
    __le16 bg_free_blocks_count; /* Free blocks count */
    __le16 bg_free_inodes_count; /* Free inodes count */
    __le16 bg_used_dirs_count;   /* Directories count */
    __le16 bg_pad;
    __le32 bg_reserved[3];
};
```

ext2의 Group Descriptor 리눅스 커널 코드를 가져온 것이다.

`block_size`는 1024 바이트인데 이것을 16진수로 바꾸면 400이다.
Superblock 바로 다음 블록이 Group Descriptor이므로 800으로 가서 확인하면 된다.

DBM은 첫 번째 8바이트로 00 00 00 08(=8)에 위치해 있다.
즉, 8192부터 DBM이 시작한다.

IBM은 두 번째 8바이트로 00 00 00 09(=9)에 위치해 있다.
즉, 9216부터 IBM이 시작한다.

inode table은 세 번째 8바이트로 00 00 00 0a(=10)에 위치해 있다.
즉, 10240부터 inode table이 시작한다.

### 8. Read the DBM, IBM, and find the inode numbers and block numbers in use. Draw the layout of "myfd" disk that shows the block location of all meta blocks: super block, group descriptor, IBM, DBM, and inode table.

DBM은 블록의 비트 단위로 사용 여부를 저장하며 ff ff ff ff ff 7f를 이진수(리틀 엔디언)으로 바꾸면 01111111 11111111 11111111 11111111 11111111이 되므로 0번부터 46번 블록까지 사용하고 47번 블록은 비어있다는 것을 의미한다.

IBM은 inode의 사용 여부를 저장하며 ff 0f를 이진수(리틀 엔디언)으로 바꾸면 00001111 11111111이 되므로 총 12개의 inode가 사용되었다는 것을 의미한다.
`m_inodes_count`는 184이므로 184개의 inode 중 12개를 사용하면 172개가 남는데, 6번에서 `m_free_inodes_count`에 저장되는 값을 확인해보면 ac 00 00 00으로 10진수(리틀 엔디언)로 변환하면 172가 나온다.

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
    __le32 i_generation;           /* File version (for NFS) */
    __le32 i_file_acl;             /* File ACL */
    __le32 i_dir_acl;              /* Directory ACL */
    __le32 i_faddr;                /* Fragment address */
    union {
        struct {
            __u8 l_i_frag;  /* Fragment number */
            __u8 l_i_fsize; /* Fragment size */
            __u16 i_pad1;
            __le16 l_i_uid_high; /* these 2 fields    */
            __le16 l_i_gid_high; /* were reserved2[0] */
            __u32 l_i_reserved2;
        } linux2;
        struct {
            __u8 h_i_frag;  /* Fragment number */
            __u8 h_i_fsize; /* Fragment size */
            __le16 h_i_mode_high;
            __le16 h_i_uid_high;
            __le16 h_i_gid_high;
            __le32 h_i_author;
        } hurd2;
        struct {
            __u8 m_i_frag;  /* Fragment number */
            __u8 m_i_fsize; /* Fragment size */
            __u16 m_pad1;
            __u32 m_i_reserved2[2];
        } masix2;
    } osd2; /* OS dependent 2 */
};
```

inode table이다.
하나의 크기는 1024바이트로 file mode, uid, size, access time, creation time 등이 저장된다.
예로 첫 번째 블록의 access time은 d2 41 8d 5f로 이진수(리틀 엔디안)으로 변환하면 1603092946인데, 이것을 날짜로 변환하면 Mon Oct 26 2020 11:32:46 GMT+0900 (대한민국 표준시)가 된다.
즉, inode를 생성했던 시각이 나온다.
