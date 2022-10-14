#include <errno.h>

typedef int                     qhandle_t;
typedef long long               int64_t;

static qhandle_t                com_statlogFile;
static int      	            com_printEntered;
int     			            FS_FCloseFile(qhandle_t f);

#define ERRNO_MAX               0x5000

#if EINVAL > 0
#define Q_ERR(e)        (e < 1 || e > ERRNO_MAX ? -ERRNO_MAX : -e)
#else
#define Q_ERR(e)        (e > -1 || e < -ERRNO_MAX ? -ERRNO_MAX : e)
#endif

#define EBADF                   9
#define ENOSYS                  40
#define FS_MODE_APPEND          0x00000000
#define FS_MODE_READ            0x00000001
#define FS_MODE_WRITE           0x00000002
#define FS_BUF_NONE             0x0000000c
#define FS_FLAG_TEXT            0x00000400
#define MAXPRINTMSG     		4096
#define Q_ERR_NOSYS             Q_ERR(ENOSYS)
#define Q_ERR_BADF              Q_ERR(EBADF)
#define Q_ERRNO                 Q_ErrorNumber()

void Com_StatPrintf(const char *fmt, ...);
