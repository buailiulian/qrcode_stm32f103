#include "mp3player.h"
#include "ff.h"
#include "vs10xx.h"
#include "hc4052.h"
#include "spi.h"

/****************************************************************************
* Function Name  : MP3_Init
* Description    : 初始化MP3
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

int8_t MP3_Init(void)
{
    uint16_t id;
	VS10XX_Config();
    id = VS10XX_RAM_Test();	
	if(id != 0x83FF)
	{	
		return 0xFF;
	}
	
	HC4052_Init();
	VS10XX_SineTest();
	HC4052_OutputSelect(AUDIO_MP3_CHANNEL);
	MP3_BaseSetting(0,0,0,0);
	MP3_EffectSetting(0);

	return 0;
}

/****************************************************************************
* Function Name  : MP3_AudioSetting
* Description    : 设置声音的大小，设置SCI_VOL寄存器
* Input          : vol：声音的大小（0~0xFF）
* Output         : None
* Return         : None
****************************************************************************/

void MP3_AudioSetting(uint8_t vol)
{
	uint16_t volValue = 0;
	
	/* 0是最大音量，0xFE是无声，低8字节控制右通道，高8字节控制左通道 */
	vol = 254 - vol;
	volValue = vol | (vol << 8);

	VS10XX_WriteCmd(SCI_VOL, volValue);
}

/****************************************************************************
* Function Name  : MP3_BaseSetting
* Description    : 基本设置：设置SCI_BASS寄存器
* Input          : amplitudeH：高频增益 0~15(单位:1.5dB,小于9的时候为负数)
*                * freqLimitH：高频上限 2~15(单位:10Hz)
*                * amplitudeL：低频增益 0~15(单位:1dB)
*                * freqLimitL：低频下限 1~15(单位:1Khz)
* Output         : None
* Return         : None
****************************************************************************/

void MP3_BaseSetting(
	uint8_t amplitudeH, uint8_t freqLimitH,
	uint8_t amplitudeL, uint8_t freqLimitL
)
{
	uint16_t bassValue = 0;
	
	/* 高频增益是12 ：15位 */
	bassValue = amplitudeH & 0x0F;
	bassValue <<= 4;
	
	/* 频率下限是 11 ：8位 */
	bassValue |= freqLimitL & 0x0F;
	bassValue <<= 4;
	
	/* 低频增益是 7 ：4 位 */
	bassValue |= amplitudeL & 0x0F;
	bassValue <<= 4;
	
	/* 频率上限是 3 ： 0位 */
	bassValue |= freqLimitH & 0x0F;
	
	VS10XX_WriteCmd(SCI_BASS, bassValue); 
		
}

/****************************************************************************
* Function Name  : MP3_EffectSetting
* Description    : 音效设置。设置SCI_MODE寄存器
* Input          : effect：0,关闭;1,最小;2,中等;3,最大。
* Output         : None
* Return         : None
****************************************************************************/

void MP3_EffectSetting(uint8_t effect)
{
	uint16_t effectValue;

	effectValue = VS10XX_ReadData(SCI_MODE);
	if(effect & 0x01)
	{
		effectValue |= 1 << 4;	
	}
	else
	{
		effectValue &= ~(1 << 5);
	}
	if(effect &0x02)
	{
		effectValue |= 1 << 7;
	}
	else
	{
		effectValue &= ~(1 << 7);
	}

	VS10XX_WriteCmd(SCI_MODE, effectValue);
}

/****************************************************************************
* Function Name  : MP3_PlaySong
* Description    : 播放一首歌曲
* Input          : addr：播放地址和歌名（歌曲名记得加.mp3后缀）
* Output         : None
* Return         : None
****************************************************************************/

void MP3_PlaySong(uint8_t *addr)
{
	FIL file;
	UINT br;
	FRESULT res;
	uint8_t i, musicBuff[512];
	uint16_t k;

    /*open file*/
	res = f_open(&file, (const TCHAR*)addr, FA_READ);
 	
	VS10XX_SoftReset();
	if(res == FR_OK)
	{
		SPI1_SetSpeed(SPI_BaudRatePrescaler_8);
		while(1)
		{
			res = f_read(&file, musicBuff, sizeof(musicBuff), &br);
			k = 0;

			do
			{	
				if(VS_DREQ != 0) //VS空闲
				{
					VS_XDCS_CLR;
					for(i=0; i<32; i++)
					{
						SPI1_WriteReadData(musicBuff[k++]);
					}
					VS_XDCS_SET;
				}
			}
			while(k < br);
			
			if (res || (br == 0))
			{
				break;    // error or eof 
			} 
		}
		f_close(&file);  //不论是打开，还是新建文件，一定记得关闭
	}		
}

