/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "sd.h"
#include "flash.h"

/* Definitions of physical drive number for each media */
#define SD_CARD		0
#define EX_FLASH	1
#define USB		    2

#define  FLASH_SECTOR_SIZE 	512			 //定义FLASH一个扇区长度 
uint16_t FLASH_SECTOR_COUNT = 2048*5;     //5M字节(2048 * 5 * 512)
#define  FLASH_BLOCK_SIZE  	8            //每个BLOCK有8个扇区

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive  初始化磁盘                                                  */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
    DRESULT res = RES_ERROR;
	int result;

	switch (pdrv) {
	case SD_CARD :
		result = SD_Init();
		if(result == 0)
        {
            res = RES_OK; 
        }
		break;

	case EX_FLASH :
        if(FLASH_Init() == EN25Q64)
        {
            FLASH_SECTOR_COUNT = 2048 * 5;
            res = RES_OK;    
        }
		break;

	case USB :
		break;
	default:
		break;

	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status 获取磁盘信息                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
    pdrv = pdrv;
    return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)    读取扇区                                                    */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{
    DRESULT res = RES_ERROR;
	int result;

	switch (pdrv) {
	case SD_CARD :
		result = SD_ReadDisk(buff, sector, count);
        if(result)
        {
            res = RES_ERROR;
        }
        else
        {
            res = RES_OK;
        }
		break;

	case EX_FLASH :
        for(; count>0; count--)
		{
			FLASH_ReadData(buff, sector*FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
			sector++;
			buff += FLASH_SECTOR_SIZE;
		}
        res = RES_OK;

	case USB :
        break;

	default:
		break;
	}
   
	return res; 
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{
	DRESULT res = RES_ERROR;
	int result;

	switch (pdrv) {
	case SD_CARD :
		result = SD_WriteDisk((uint8_t*)buff, sector, count);
		if(result == 0)	  
		{
			res = RES_OK;	
		}
		break;

	case EX_FLASH :
        for(;count>0;count--)
		{										    
			FLASH_WriteData((u8*)buff, sector*FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
			sector++;
			buff += FLASH_SECTOR_SIZE;
		}
        res = RES_OK;
		break;

	case USB :
		break;

	default:
		break;
	}
	return res;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions  其他表参数的获取                                             */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;

	switch (pdrv) {
	case SD_CARD :
	    switch(cmd)
	    {
		    case CTRL_SYNC:       //刷新磁盘
                res = RES_OK;
		        break;	 
		    case GET_SECTOR_SIZE: //得到媒体大小
		        *(WORD*)buff = 512;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:  //块大小
		        *(WORD*)buff = 8;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        if(SD_ReadCapacity((uint32_t *)buff) == 0)
                {
                    res = RES_OK;    
                }	        
		        break;
		    default:
		        break;
	    }
		// post-process here
		break;

	case EX_FLASH :
	    switch(cmd)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
		        *(WORD*)buff = FLASH_SECTOR_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
		        *(WORD*)buff = FLASH_BLOCK_SIZE;
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = FLASH_SECTOR_COUNT;
		        res = RES_OK;
		        break;
		    default:
		        break;
        }
		break;

	case USB :
        res = RES_PARERR;
		break;
	}

	return res;
}
#endif

DWORD get_fattime(void)
{
	return 0;
}
