#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
// #define PROT_READ 0x000
// #define PROT_WRITE 0x001

// #define MAP_PRIVATE 0x000
// #define MAP_SHARED 0x001

#define MAP_SHARED  1
#define MAP_PRIVATE 2

//rewrote like previous lab format

#define PROT_READ  (1L << 1)
#define PROT_WRITE (1L << 2)