// An amalgam of error.h and common.h from Q2Pro

#include <errno.h>

typedef int                     qhandle_t;
typedef long                    int64_t;

static qhandle_t                com_statlogFile;
static int      	            com_printEntered;
int     			            FS_FCloseFile(qhandle_t f);
const char                      *Q_ErrorString(int error);

#define MAX_FILE_HANDLES        32
#define ERRNO_MAX               0x5000

#if EINVAL > 0
#define Q_ERR(e)        (e < 1 || e > ERRNO_MAX ? -ERRNO_MAX : -e)
#else
#define Q_ERR(e)        (e > -1 || e < -ERRNO_MAX ? -ERRNO_MAX : e)
#endif

// bits 0 - 1, enum
#define FS_MODE_APPEND          0x00000000
#define FS_MODE_READ            0x00000001
#define FS_MODE_WRITE           0x00000002
#define FS_MODE_RDWR            0x00000003
#define FS_MODE_MASK            0x00000003

// bits 2 - 3, enum
#define FS_BUF_DEFAULT          0x00000000
#define FS_BUF_FULL             0x00000004
#define FS_BUF_LINE             0x00000008
#define FS_BUF_NONE             0x0000000c
#define FS_BUF_MASK             0x0000000c

// bits 4 - 5, enum
#define FS_TYPE_ANY             0x00000000
#define FS_TYPE_REAL            0x00000010
#define FS_TYPE_PAK             0x00000020
#define FS_TYPE_RESERVED        0x00000030
#define FS_TYPE_MASK            0x00000030

// bits 6 - 7, flag
#define FS_PATH_ANY             0x00000000
#define FS_PATH_BASE            0x00000040
#define FS_PATH_GAME            0x00000080
#define FS_PATH_MASK            0x000000c0

// bits 8 - 13, flag
#define FS_SEARCH_BYFILTER      0x00000100
#define FS_SEARCH_SAVEPATH      0x00000200
#define FS_SEARCH_EXTRAINFO     0x00000400
#define FS_SEARCH_STRIPEXT      0x00000800
#define FS_SEARCH_DIRSONLY      0x00001000
#define FS_SEARCH_RECURSIVE     0x00002000
#define FS_SEARCH_MASK          0x00003f00

// bits 8 - 11, flag
#define FS_FLAG_GZIP            0x00000100
#define FS_FLAG_EXCL            0x00000200
#define FS_FLAG_TEXT            0x00000400
#define FS_FLAG_DEFLATE         0x00000800

#define MAX_LOADFILE            0x4001000   // 64 MiB + some slop

#define FS_Malloc(size)         Z_TagMalloc(size, TAG_FILESYSTEM)
#define FS_Mallocz(size)        Z_TagMallocz(size, TAG_FILESYSTEM)
#define FS_CopyString(string)   Z_TagCopyString(string, TAG_FILESYSTEM)
#define FS_LoadFile(path, buf)  FS_LoadFileEx(path, buf, 0, TAG_FILESYSTEM)
#define FS_FreeFile(buf)        Z_Free(buf)

#define MAXPRINTMSG     		4096
// These values directly map to system errno.
#define Q_ERR_NOENT             Q_ERR(ENOENT)
#define Q_ERR_NAMETOOLONG       Q_ERR(ENAMETOOLONG)
#define Q_ERR_INVAL             Q_ERR(EINVAL)
#define Q_ERR_NOSYS             Q_ERR(ENOSYS)
#define Q_ERR_SPIPE             Q_ERR(ESPIPE)
#define Q_ERR_FBIG              Q_ERR(EFBIG)
#define Q_ERR_ISDIR             Q_ERR(EISDIR)
#define Q_ERR_AGAIN             Q_ERR(EAGAIN)
#define Q_ERR_MFILE             Q_ERR(EMFILE)
#define Q_ERR_EXIST             Q_ERR(EEXIST)
#define Q_ERR_BADF              Q_ERR(EBADF)
#define Q_ERR_PERM              Q_ERR(EPERM)
#define Q_ERR_NOMEM             Q_ERR(ENOMEM)
#define Q_ERR_FAILURE           _Q_ERR(0)   // Unspecified error
#define Q_ERRNO                 Q_ErrorNumber()
static inline int Q_ErrorNumber(void)
{
    int e = errno;
    return Q_ERR(e);
}


void Com_StatPrintf(const char *fmt, ...);
