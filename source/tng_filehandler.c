//-----------------------------------------------------------------------------
// Statistics Related Code
//
// $Id: tng_stats.c,v 1.33 2004/05/18 20:35:45 slicerdw Exp $
//
//-----------------------------------------------------------------------------
// $Log: tng_stats.c,v $
// Revision 1.33  2004/05/18 20:35:45  slicerdw
// Fixed a bug on stats command
//
// Revision 1.32  2002/04/03 15:05:03  freud
// My indenting broke something, rolled the source back.
//
// Revision 1.30  2002/04/01 16:08:59  freud
// Fix in hits/shots counter for each weapon
//
// Revision 1.29  2002/04/01 15:30:38  freud
// Small stat fix
//
// Revision 1.28  2002/04/01 15:16:06  freud
// Stats code redone, tng_stats now much more smarter. Removed a few global
// variables regarding stats code and added kevlar hits to stats.
//
// Revision 1.27  2002/03/28 12:10:12  freud
// Removed unused variables (compiler warnings).
// Added cvar mm_allowlock.
//
// Revision 1.26  2002/03/15 19:28:36  deathwatch
// Updated with stats rifle name fix
//
// Revision 1.25  2002/02/26 23:09:20  freud
// Stats <playerid> not working, fixed.
//
// Revision 1.24  2002/02/21 23:38:39  freud
// Fix to a BAD stats bug. CRASH
//
// Revision 1.23  2002/02/18 23:47:33  freud
// Fixed FPM if time was 0
//
// Revision 1.22  2002/02/18 19:31:40  freud
// FPM fix.
//
// Revision 1.21  2002/02/18 17:21:14  freud
// Changed Knife in stats to Slashing Knife
//
// Revision 1.20  2002/02/17 21:48:56  freud
// Changed/Fixed allignment of Scoreboard
//
// Revision 1.19  2002/02/05 09:27:17  freud
// Weapon name changes and better alignment in "stats list"
//
// Revision 1.18  2002/02/03 01:07:28  freud
// more fixes with stats
//
// Revision 1.14  2002/01/24 11:29:34  ra
// Cleanup's in stats code
//
// Revision 1.13  2002/01/24 02:24:56  deathwatch
// Major update to Stats code (thanks to Freud)
// new cvars:
// stats_afterround - will display the stats after a round ends or map ends
// stats_endmap - if on (1) will display the stats scoreboard when the map ends
//
// Revision 1.12  2001/12/31 13:29:06  deathwatch
// Added revision header
//
//
//-----------------------------------------------------------------------------

#include "g_local.h"
#include "tng_filehandler.h"
#include <time.h>

typedef enum {
    FS_FREE,
    FS_REAL,
    FS_PAK,
#if USE_ZLIB
    FS_ZIP,
    FS_GZ,
#endif
    FS_BAD
} filetype_t;

typedef struct packfile_s {
    char        *name;
    unsigned    namelen;
    unsigned    filepos;
    unsigned    filelen;
#if USE_ZLIB
    unsigned    complen;
    byte        compmtd;    // compression method, 0 (stored) or Z_DEFLATED
    bool        coherent;   // true if local file header has been checked
#endif
    struct packfile_s *hash_next;
} packfile_t;

typedef struct {
    filetype_t  type;       // FS_PAK or FS_ZIP
    unsigned    refcount;   // for tracking pack users
    FILE        *fp;
    unsigned    num_files;
    unsigned    hash_size;
    packfile_t  *files;
    packfile_t  **file_hash;
    char        *names;
    char        *filename;
} pack_t;

typedef struct {
    filetype_t  type;
    unsigned    mode;
    FILE        *fp;
#if USE_ZLIB
    void        *zfp;       // gzFile for FS_GZ or zipstream_t for FS_ZIP
#endif
    packfile_t  *entry;     // pack entry this handle is tied to
    pack_t      *pack;      // points to the pack entry is from
    qboolean        unique;     // if true, then pack must be freed on close
    int         error;      // stream error indicator from read/write operation
    unsigned    rest_out;   // remaining unread length for FS_PAK/FS_ZIP
    int64_t     length;     // total cached file length
} file_t;

// Writing to independent log file

/// Stole all of these from Q2Pro

static qboolean     com_logNewline;

int FS_FCloseFile(qhandle_t f)
{
    file_t *file = file_for_handle(f);
    int ret;

    if (!file)
        return Q_ERR_BADF;

    ret = file->error;
    switch (file->type) {
    case FS_REAL:
        if (fclose(file->fp))
            ret = Q_ERRNO;
        break;
    case FS_PAK:
        if (file->unique) {
            fclose(file->fp);
            pack_put(file->pack);
        }
        break;
#if USE_ZLIB
    case FS_GZ:
        if (gzclose(file->zfp))
            ret = Q_ERR_LIBRARY_ERROR;
        break;
    case FS_ZIP:
        if (file->unique) {
            close_zip_file(file);
            pack_put(file->pack);
        }
        break;
#endif
    default:
        ret = Q_ERR_NOSYS;
        break;
    }

    memset(file, 0, sizeof(*file));
    return ret;
}

static void statlogfile_close(void)
{
    if (!com_statlogFile) {
        return;
    }

    Com_Printf("Closing stat log.\n");

    FS_FCloseFile(com_statlogFile);
    com_statlogFile = 0;
}

static void statlogfile_open(void)
{
    char buffer[MAX_OSPATH];
    unsigned mode;
    qhandle_t f;

    mode = statlogfile->value > 1 ? FS_MODE_APPEND : FS_MODE_WRITE;
    // Always write the full stat line, don't buffer
    mode |= FS_BUF_NONE;

    f = FS_EasyOpenFile(buffer, sizeof(buffer), mode | FS_FLAG_TEXT,
                        "stats/", logfile_name->string, ".stats");
    if (!f) {
        Cvar_Set("statlogfile", "0");
        return;
    }

    com_statlogFile = f;
    com_logNewline = true;
    Com_Printf("Logging stats to %s\n", buffer);
}

static void statlogfile_changed(cvar_t *self)
{
    statlogfile_close();
    if (self->value) {
        statlogfile_open();
    }
}

static void statlogfile_write(const char *s)
{
    char text[MAXPRINTMSG];
    char buf[MAX_QPATH];
    char *p, *maxp;
    size_t len;
    int ret;
    int c;

    len = 0;
    p = text;
    maxp = text + sizeof(text) - 1;
    while (*s) {
        if (com_logNewline) {
            if (len > 0 && len < maxp - p) {
                memcpy(p, buf, len);
                p += len;
            }
            com_logNewline = false;
        }

        if (p == maxp) {
            break;
        }

        c = *s++;
        if (c == '\n') {
            com_logNewline = true;
        } else {
            c = Q_charascii(c);
        }

        *p++ = c;
    }
    *p = 0;

    len = p - text;
    ret = FS_Write(text, len, com_statlogFile);
    if (ret != len) {
        // zero handle BEFORE doing anything else to avoid recursion
        qhandle_t tmp = com_statlogFile;
        com_statlogFile = 0;
        FS_FCloseFile(tmp);
        Com_EPrintf("Couldn't write stat log: %s\n", Q_ErrorString(ret));
        Cvar_Set("statlogfile", "0");
    }
}

#ifndef _WIN32
/*
=============
Com_FlushLogs

When called from SIGHUP handler on UNIX-like systems,
will close and reopen logfile handle for rotation.
=============
*/

void Com_StatFlushLogs(void)
{
    if (statlogfile) {
        statlogfile_changed(statlogfile_enable);
    }
}

#endif

void Com_StatPrintf(const char *fmt, ...)
{
    va_list     argptr;
    char        msg[MAXPRINTMSG];
    size_t      len;

    // may be entered recursively only once
    if (com_printEntered >= 2) {
        return;
    }

    com_printEntered++;

    va_start(argptr, fmt);
    len = Q_vscnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

        // logfile
        if (com_statlogFile) {
            statlogfile_write(msg);
		}
    com_printEntered--;
}

// End log file handling
