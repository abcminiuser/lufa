FatFs Module Source Files R0.08                        (C)ChaN, 2010


FILES

  ffconf.h   Configuration file for FatFs module.
  ff.h       Common include file for FatFs and application module.
  ff.c       FatFs module.
  diskio.h   Common include file for FatFs and disk I/O module.
  diskio.c   Skeleton of low level disk I/O module.
  integer.h  Alternative type definitions for integer variables.
  option     Optional external functions.

  Low level disk I/O module is not included in this archive because the FatFs
  module is only a generic file system layer and not depend on any specific
  storage device. You have to provide a low level disk I/O module that written
  to control your storage device.



AGREEMENTS

 FatFs module is an open source software to implement FAT file system to
 small embedded systems. This is a free software and is opened for education,
 research and commercial developments under license policy of following trems.

  Copyright (C) 2010, ChaN, all right reserved.

 * The FatFs module is a free software and there is NO WARRANTY.
 * No restriction on use. You can use, modify and redistribute it for
   personal, non-profit or commercial product UNDER YOUR RESPONSIBILITY.
 * Redistributions of source code must retain the above copyright notice.



REVISION HISTORY

  Feb 26, 2006  R0.00  Prototype

  Apr 29, 2006  R0.01  First release.

  Jun 01, 2006  R0.02  Added FAT12.
                       Removed unbuffered mode.
                       Fixed a problem on small (<32M) partition.

  Jun 10, 2006  R0.02a Added a configuration option _FS_MINIMUM.

  Sep 22, 2006  R0.03  Added f_rename.
                       Changed option _FS_MINIMUM to _FS_MINIMIZE.

  Dec 11, 2006  R0.03a Improved cluster scan algorithm to write files fast.
                       Fixed f_mkdir creates incorrect directory on FAT32.

  Feb 04, 2007  R0.04  Supported multiple drive system. (FatFs)
                       Changed some APIs for multiple drive system.
                       Added f_mkfs. (FatFs)
                       Added _USE_FAT32 option. (Tiny-FatFs)

  Apr 01, 2007  R0.04a Supported multiple partitions on a physical drive. (FatFs)
                       Fixed an endian sensitive code in f_mkfs. (FatFs)
                       Added a capability of extending the file size to f_lseek.
                       Added minimization level 3.
                       Fixed a problem that can collapse a sector when recreate an
                       existing file in any sub-directory at non FAT32 cfg. (Tiny-FatFs)

  May 05, 2007  R0.04b Added _USE_NTFLAG option.
                       Added FSInfo support.
                       Fixed some problems corresponds to FAT32. (Tiny-FatFs)
                       Fixed DBCS name can result FR_INVALID_NAME.
                       Fixed short seek (0 < ofs <= csize) collapses the file object.

  Aug 25, 2007  R0.05  Changed arguments of f_read, f_write.
                       Changed arguments of f_mkfs. (FatFs)
                       Fixed f_mkfs on FAT32 creates incorrect FSInfo. (FatFs)
                       Fixed f_mkdir on FAT32 creates incorrect directory. (FatFs)

  Feb 03, 2008  R0.05a Added f_truncate().
                       Added f_utime().
                       Fixed off by one error at FAT sub-type determination.
                       Fixed btr in f_read() can be mistruncated.
                       Fixed cached sector is not flushed when create and close without write.

  Apr 01, 2008  R0.06  Added f_forward(). (Tiny-FatFs)
                       Added string functions: fputc(), fputs(), fprintf() and fgets().
                       Improved performance of f_lseek() on move to the same or following cluster.

  Apr 01, 2009, R0.07  Merged Tiny-FatFs as a buffer configuration option.
                       Added long file name support.
                       Added multiple code page support.
                       Added re-entrancy for multitask operation.
                       Added auto cluster size selection to f_mkfs().
                       Added rewind option to f_readdir().
                       Changed result code of critical errors.
                       Renamed string functions to avoid name collision.

  Apr 14, 2009, R0.07a Separated out OS dependent code on reentrant cfg.
                       Added multiple sector size support.

  Jun 21, 2009, R0.07c Fixed f_unlink() may return FR_OK on error.
                       Fixed wrong cache control in f_lseek().
                       Added relative path feature.
                       Added f_chdir().
                       Added f_chdrive().
                       Added proper case conversion for extended characters.

  Nov 03, 2009 R0.07e  Separated out configuration options from ff.h to ffconf.h.
                       Added a configuration option, _LFN_UNICODE.
                       Fixed f_unlink() fails to remove a sub-dir on _FS_RPATH.
                       Fixed name matching error on the 13 char boundary.
                       Changed f_readdir() to return the SFN with always upper case on non-LFN cfg.

  May 15, 2010, R0.08  Added a memory configuration option. (_USE_LFN)
                       Added file lock feature. (_FS_SHARE)
                       Added fast seek feature. (_USE_FASTSEEK)
                       Changed some types on the API, XCHAR->TCHAR.
                       Changed fname member in the FILINFO structure on Unicode cfg.
                       String functions support UTF-8 encoding files on Unicode cfg.

