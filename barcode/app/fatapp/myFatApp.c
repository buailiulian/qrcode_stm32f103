#include"myfatapp.h"

/****************************************************************************
* Function Name  : FATFS_GetFree
* Description    : 读取总磁盘大小和空闲磁盘大小
* Input          : drv：磁盘编号
*                * total：保存读取到的总磁盘大小
*                * free：保存读取到的空闲磁盘大小
* Output         : None
* Return         : res：读取状态。0：成功；其他则是其他相应的错误信息。
****************************************************************************/

uint8_t FATFS_GetFree(uint8_t *drv, uint32_t *total, uint32_t *free)
{
	FATFS *fs1;
	u8 res;
    DWORD fre_clust=0, fre_sect=0, tot_sect=0;

    /* 得到磁盘信息及空闲簇数量 */
    res = f_getfree((const TCHAR*)drv, &fre_clust, &fs1);

    if(res == 0)  //表示读取成功
	{											   
	    tot_sect = (fs1->n_fatent-2) * fs1->csize;	//得到总扇区数
	    fre_sect = fre_clust * fs1->csize;			//得到空闲扇区数
			   
#if _MAX_SS!=512		        //扇区大小不是512字节,则转换为512字节
		tot_sect *= fs1->ssize/512;
		fre_sect *= fs1->ssize/512;
#endif	  

		*total = tot_sect >> 1;	//单位为KB
		*free = fre_sect >> 1;	//单位为KB 
	}

	return res;	
}

/****************************************************************************
* Function Name  : FATFS_ScanFiles
* Description    : 读取指定文件下所有文件的名字和后缀
* Input          : dir：扫描的文件路径
*                * file：用来存放所有文件的名字和后缀的地址
* Output         : None
* Return         : res：返回读取的状态
****************************************************************************/

uint8_t FATFS_ScanFiles(uint8_t *dir, FileNameTypeDef *file)
{
	FRESULT res;
	FILINFO  fileInfo;
	DIR dirs;
	char *fn, i = 0; 

#if _USE_LFN
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fileInfo.lfname = lfn;
    fileInfo.lfsize = sizeof(lfn);
#endif
	/* 打开文件夹 */
	res = f_opendir(&dirs, (const TCHAR*)dir);
	
	if (res == FR_OK)    //成功打开文件夹 
	{    
		while (f_readdir(&dirs, &fileInfo) == FR_OK)   //按照顺序读文件夹 
		{ 	 
			if(!fileInfo.fname[0]) 	  //如果文件名为0,结束 
			{
				break;     
			} 	
			
			/* 判断文件属性 */								
			if((fileInfo.fattrib == AM_ARC)|(fileInfo.fattrib == AM_DIR)) 
			{

#if _USE_LFN	//为长字符串名准备
            	fn = *fileInfo.lfname ? fileInfo.lfname : fileInfo.fname;
#else
           	 	fn = fileInfo.fname;
#endif

				/* 读取名字 */
				i = 0;
	       		while((*fn != '.') && (*fn != '\0')) //有一个成立就跳出循环
				{
					file->name[i] = *fn;
					i++;
					fn++;	
				}

				/* 读取后缀 */
				i = 0;
				while(*fn != '\0')
				{
					file->type[i] = *fn;
					i++;
					fn++;
				} 
				file++;
			}//end of 文件属性判断呢
		} //end of while (f_readdir(&dirs, &fileInfo) == FR_OK)//按照顺序读文件夹
	}
	return res;
}


