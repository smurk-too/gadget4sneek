#ifndef MAIN_H
#define MAIN_H

#ifndef NULL
# define NULL            ((void *)0)
#endif

/* Data types */
typedef char            s8;
typedef short            s16;
typedef long            s32;
typedef unsigned char        u8;
typedef unsigned short        u16;
typedef unsigned int        u32;
typedef unsigned __int64     u64;

typedef int            bool;

/* Boolean values */
#define true            1
#define false            0

bool IsDirectory(char path[]);
char *path_cat (const char str1[], const char str2[]);
int AddPath(const char dir_path[], int thisItem);
u64 getFileSize(const char filepath[]);
u32 ReadFileToBuffer(const char in[], u8 ** outbuf);
void SetEntry(int num, int type, int nameoffset, int offset, int size);
int pathWalk(const char dir_path[], int tabs);
int PathCount(const char dir_path[]);
void hexdump(void *d, int len);
u32 be32( unsigned int i );
u64 be64(const u8 *p);
u16 be16(const u8 *p);

#endif // MAIN_H