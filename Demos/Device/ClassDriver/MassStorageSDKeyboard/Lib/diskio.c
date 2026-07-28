/*-----------------------------------------------------------------------*/
/* Low level disk I/O module glue functions         (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#ifdef DRV_CFC
#include "cfc_avr.h"	/* Header file of existing CF control module */
#endif
#ifdef DRV_MMC
#include "mmc_avr.h"	/* Header file of existing SD control module */
#endif


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
#ifdef DRV_CFC
	case DRV_CFC :
		return cf_disk_status();
#endif
#ifdef DRV_MMC
	case DRV_MMC :
		return mmc_disk_status();
#endif
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
#ifdef DRV_CFC
	case DRV_CFC :
		return cf_disk_initialize();
#endif
#ifdef DRV_MMC
	case DRV_MMC :
		return mmc_disk_initialize();
#endif
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	switch (pdrv) {
#ifdef DRV_CFC
	case DRV_CFC :
		return cf_disk_read(buff, sector, count);
#endif
#ifdef DRV_MMC
	case DRV_MMC :
		return mmc_disk_read(buff, sector, count);
#endif
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	switch (pdrv) {
#ifdef DRV_CFC
	case DRV_CFC :
		return cf_disk_write(buff, sector, count);
#endif
#ifdef DRV_MMC
	case DRV_MMC :
		return mmc_disk_write(buff, sector, count);
#endif
	}
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (pdrv) {
#ifdef DRV_CFC
	case DRV_CFC :
		return cf_disk_ioctl(cmd, buff);
#endif
#ifdef DRV_MMC
	case DRV_MMC :
		return mmc_disk_ioctl(cmd, buff);
#endif
	}
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Timer driven procedure                                                */
/*-----------------------------------------------------------------------*/


void disk_timerproc (void)
{
#ifdef DRV_CFC
	cf_disk_timerproc();
#endif
#ifdef DRV_MMC
	mmc_disk_timerproc();
#endif
}



